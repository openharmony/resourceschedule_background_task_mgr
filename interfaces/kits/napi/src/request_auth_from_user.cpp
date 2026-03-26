/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "request_auth_from_user.h"

#include <uv.h>

#include "singleton.h"
#ifdef SUPPORT_JSSTACK
#include "xpower_event_js.h"
#endif
#include "ability.h"
#include "napi_base_context.h"
#include "background_task_manager.h"
#include "hitrace_meter.h"
#include "continuous_task_log.h"
#include "background_task_mode.h"
#include "continuous_task_param.h"
#include "continuous_task_request.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace BackgroundTaskMgr {
std::map<int32_t, std::shared_ptr<ExpiredCallback>> authCallbackInstances_;
std::mutex authCallbackLock_;
static const uint32_t REQUEST_AUTH_FORM_USER_PARAMS = 2;

struct CallbackReceiveDataWorker {
    napi_env env = nullptr;
    napi_ref ref = nullptr;
    std::shared_ptr<ExpiredCallback> callback = nullptr;
};

AuthCallbackInstance::AuthCallbackInstance() {}

AuthCallbackInstance::~AuthCallbackInstance()
{
    if (expiredCallbackInfo_.ref == nullptr) {
        return;
    }
    DeleteNapiRef();
}

void AuthCallbackInstance::DeleteNapiRef()
{
    std::shared_ptr<CallbackReceiveDataWorker> dataWorker = std::make_shared<CallbackReceiveDataWorker>();
    if (dataWorker == nullptr) {
        BGTASK_LOGE("DeleteNapiRef new dataWorker failed");
        return;
    }
    dataWorker->env = expiredCallbackInfo_.env;
    dataWorker->ref = expiredCallbackInfo_.ref;
    auto task = [dataWorker]() {
        napi_delete_reference(dataWorker->env, dataWorker->ref);
    };
    if (napi_status::napi_ok != napi_send_event(expiredCallbackInfo_.env, task, napi_eprio_high,
        "AuthCallbackInstance::DeleteNapiRef")) {
        BGTASK_LOGE("DeleteNapiRef: Failed to SendEvent");
        dataWorker = nullptr;
    }
}

__attribute__((no_sanitize("cfi"))) void AuthCallbackInstance::OnExpired() {}

__attribute__((no_sanitize("cfi"))) void AuthCallbackInstance::OnExpiredAuth(int32_t authResult)
{
    BGTASK_LOGI("OnExpiredAuth authresult: %{public}d", authResult);
    std::lock_guard<std::mutex> lock(authCallbackLock_);
    auto findCallback = std::find_if(authCallbackInstances_.begin(), authCallbackInstances_.end(),
        [&](const auto& callbackInstance) { return callbackInstance.second.get() == this; }
    );
    if (findCallback == authCallbackInstances_.end()) {
        BGTASK_LOGI("expired callback is not found");
        return;
    }
    if (expiredCallbackInfo_.ref == nullptr) {
        BGTASK_LOGE("expired callback unset");
        authCallbackInstances_.erase(findCallback);
        return;
    }
    std::shared_ptr<CallbackReceiveDataWorker> dataWorker = std::make_shared<CallbackReceiveDataWorker>();
    if (dataWorker == nullptr) {
        BGTASK_LOGE("OnExpired new dataWorker failed");
        authCallbackInstances_.erase(findCallback);
        return;
    }
    dataWorker->env = expiredCallbackInfo_.env;
    dataWorker->ref = expiredCallbackInfo_.ref;
    dataWorker->callback = shared_from_this();
    auto task = [dataWorker, authResult]() {
        Common::SetAuthCallback(dataWorker->env, dataWorker->ref, authResult);
        std::lock_guard<std::mutex> lock(authCallbackLock_);
        auto findCallback = std::find_if(authCallbackInstances_.begin(), authCallbackInstances_.end(),
            [&](const auto& callbackInstance) { return callbackInstance.second == dataWorker->callback; }
        );
        if (findCallback != authCallbackInstances_.end()) {
            authCallbackInstances_.erase(findCallback);
        }
    };
    if (napi_status::napi_ok != napi_send_event(expiredCallbackInfo_.env, task, napi_eprio_high,
        "AuthCallbackInstance::OnExpiredAuth")) {
        BGTASK_LOGE("OnExpired: Failed to SendEvent");
        dataWorker = nullptr;
        authCallbackInstances_.erase(findCallback);
    }
}

void AuthCallbackInstance::SetCallbackInfo(const napi_env &env, const napi_ref &ref)
{
    expiredCallbackInfo_.env = env;
    expiredCallbackInfo_.ref = ref;
}

napi_value CheckAbilityContext(const napi_env &env, const napi_value &value,
    std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext)
{
    bool stageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, value, stageMode);
    BGTASK_LOGD("is stage mode: %{public}s", stageMode ? "true" : "false");

    if (status == napi_ok && stageMode) {
        auto context = AbilityRuntime::GetStageModeContext(env, value);
        if (!context) {
            BGTASK_LOGE("get context failed");
            return nullptr;
        }
        abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
        if (!abilityContext) {
            BGTASK_LOGE("get Stage model ability context failed");
            return nullptr;
        }
        const std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = abilityContext->GetAbilityInfo();
        if (abilityInfo == nullptr) {
            BGTASK_LOGE("ability info is null");
            return nullptr;
        }
        return Common::NapiGetNull(env);
    }
    return nullptr;
}

napi_value GetExpiredCallback(
    const napi_env &env, const napi_value &value, std::shared_ptr<AuthCallbackInstance> &callback)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    if (valuetype != napi_function) {
        return Common::NapiGetNull(env);
    }
    napi_ref result = nullptr;
    callback = std::make_shared<AuthCallbackInstance>();
    callback->Init();
    napi_create_reference(env, value, 1, &result);
    callback->SetCallbackInfo(env, result);
    return Common::NapiGetNull(env);
}

bool CheckReqestParam(napi_env env, const std::shared_ptr<ContinuousTaskRequest> request)
{
    std::vector<uint32_t> backgroundTaskModesValue = request->GetBackgroundTaskModes();
    std::vector<uint32_t> backgroundTaskSubModesValue = request->GetBackgroundTaskSubmodes();
    if (backgroundTaskModesValue.size() == 0 || backgroundTaskSubModesValue.size() == 0) {
        BGTASK_LOGE("backgroundTaskModes or backgroundTaskSubModes is empty.");
        Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_IS_EMPTY, true);
        return false;
    }
    int32_t specialModeSize = std::count(backgroundTaskModesValue.begin(), backgroundTaskModesValue.end(),
        BackgroundTaskMode::MODE_SPECIAL_SCENARIO_PROCESSING);
    if (specialModeSize == 0) {
        Common::HandleErrCode(env, ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_EMPTY, true);
        BGTASK_LOGE("mode: SPECIAL_SCENARIO_PROCESSING is empty.");
        return false;
    }
    if (specialModeSize > 1) {
        Common::HandleErrCode(env, ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_ONLY_ALLOW_ONE_APPLICATION, true);
        BGTASK_LOGE("mode: SPECIAL_SCENARIO_PROCESSING quantity exceeds one.");
        return false;
    }
    if (specialModeSize == 1 && backgroundTaskModesValue.size() > 1) {
        Common::HandleErrCode(env, ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_CONFLICTS_WITH_OTHER_TASK, true);
        BGTASK_LOGE("request auth cannot have other background mode. except SPECIAL_SCENARIO_PROCESSING.");
        return false;
    }
    // 主类型与子类型是否匹配
    for (const auto &subMode : backgroundTaskSubModesValue) {
        if (BackgroundTaskMode::GetSubModeTypeMatching(subMode)
            != BackgroundTaskMode::MODE_SPECIAL_SCENARIO_PROCESSING) {
            Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_TYPE_MISMATCH, true);
            BGTASK_LOGE("background task submodes mismatch. mode: SPECIAL_SCENARIO_PROCESSING.");
            return false;
        }
    }
    return true;
}

bool CheckRequestAuthFromUserParam(napi_env env, napi_callback_info info,
    std::shared_ptr<ContinuousTaskRequest> request)
{
    napi_value jsThis;
    napi_get_cb_info(env, info, nullptr, nullptr, &jsThis, nullptr);
    bool hasProperty = false;
    napi_status status = napi_has_named_property(env, jsThis, "backgroundTaskModes", &hasProperty);
    if (!hasProperty || status != napi_ok) {
        Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_REQUEST_NULL_OR_TYPE, true);
        BGTASK_LOGE("do not have property backgroundTaskModes");
        return false;
    }
    status = napi_has_named_property(env, jsThis, "backgroundTaskSubmodes", &hasProperty);
    if (!hasProperty || status != napi_ok) {
        Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_REQUEST_NULL_OR_TYPE, true);
        BGTASK_LOGE("do not have property backgroundTaskSubmodes");
        return false;
    }
    napi_value backgroundTaskModes;
    status = napi_get_named_property(env, jsThis, "backgroundTaskModes", &backgroundTaskModes);
    if (status != napi_ok ||
        !Common::GetBackgroundTaskModesFromArray(env, backgroundTaskModes, request)) {
        Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_REQUEST_NULL_OR_TYPE, true);
        BGTASK_LOGE("failed to format backgroundTaskModes params");
        return false;
    }
    napi_value backgroundTaskSubModes;
    status = napi_get_named_property(env, jsThis, "backgroundTaskSubmodes", &backgroundTaskSubModes);
    if (status != napi_ok ||
        !Common::GetBackgroundTaskSubmodesFromArray(env, backgroundTaskSubModes, request)) {
        Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_REQUEST_NULL_OR_TYPE, true);
        BGTASK_LOGE("failed to format backgroundTaskSubmodes params");
        return false;
    }
    if (!CheckReqestParam(env, request)) {
        return false;
    }
    return true;
}

bool SendRequest(napi_env env, const ContinuousTaskParam &taskParam, const ExpiredCallback &callback,
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext, int32_t &callbackUid)
{
    // 进行权限请求弹窗处理
    // 1、先查询是否已经有授权，有的话，则返回错误码，不再弹窗
    ErrCode errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        RequestAuthFromUser(taskParam, callback, callbackUid);
    if (errCode != ERR_OK) {
        Common::HandleErrCode(env, errCode, true);
        return false;
    }
    // 2、没有授权，则弹窗授权
    if (!CreateUIExtension(abilityContext, taskParam)) {
        BGTASK_LOGE("CreateUIExtension failed");
        // 拉起授权弹窗失败，取消已经申请的授权记录
        DelayedSingleton<BackgroundTaskManager>::GetInstance()->RemoveAuthRecord(taskParam);
        Common::HandleErrCode(env, ERR_BGTASK_SYS_NOT_READY, true);
        return false;
    }
    return true;
}

napi_value RequestAuthFromUser(napi_env env, napi_callback_info info)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Napi::RequestAuthFromUser");
#ifdef SUPPORT_JSSTACK
    HiviewDFX::ReportXPowerJsStackSysEvent(env, "REQUEST_AUTH_FROM_USER");
#endif
    std::shared_ptr<ContinuousTaskRequest> request = std::make_shared<ContinuousTaskRequest>();
    if (!CheckRequestAuthFromUserParam(env, info, request)) {
        return Common::NapiGetNull(env);
    }
    size_t argc = REQUEST_AUTH_FORM_USER_PARAMS;
    napi_value argv[REQUEST_AUTH_FORM_USER_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != REQUEST_AUTH_FORM_USER_PARAMS) {
        Common::HandleParamErr(env, ERR_PARAM_NUMBER_ERR, true);
        return Common::NapiGetNull(env);
    }
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext {nullptr};
    if (CheckAbilityContext(env, argv[0], abilityContext) == nullptr) {
        Common::HandleParamErr(env, ERR_CONTEXT_NULL_OR_TYPE_ERR, true);
        return Common::NapiGetNull(env);
    }
    std::shared_ptr<AuthCallbackInstance> callback = nullptr;
    if (GetExpiredCallback(env, argv[1], callback) == nullptr) {
        BGTASK_LOGE("ExpiredCallback parse failed");
        Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_CALLBACK_NULL_OR_TYPE_ERR, true);
        return Common::NapiGetNull(env);
    }
    std::vector<uint32_t> backgroundTaskModesValue = request->GetBackgroundTaskModes();
    ContinuousTaskParam taskParam = ContinuousTaskParam(true, backgroundTaskModesValue[0],
        nullptr, "", nullptr, "", true, backgroundTaskModesValue, -1);
    taskParam.isByRequestObject_ = true;
    taskParam.bgSubModeIds_ = request->GetBackgroundTaskSubmodes();
    taskParam.appIndex_ = abilityContext->GetAbilityInfo()->appIndex;
    int32_t callbackUid = 0;
    if (!SendRequest(env, taskParam, *callback, abilityContext, callbackUid)) {
        return Common::NapiGetNull(env);
    }
    {
        std::lock_guard<std::mutex> lock(authCallbackLock_);
        authCallbackInstances_[callbackUid] = callback;
    }
    napi_value result = nullptr;
    napi_create_int32(env, 0, &result);
    return result;
}

bool CreateUIExtension(std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> abilityContext,
    const ContinuousTaskParam &taskParam)
{
    if (abilityContext == nullptr) {
        BGTASK_LOGE("null context");
        return false;
    }
    auto uiContent = abilityContext->GetUIContent();
    if (uiContent == nullptr) {
        BGTASK_LOGE("null uiContent");
        return false;
    }
    if (taskParam.bgSubModeIds_.size() == 0) {
        return false;
    }
    AAFwk::Want want;
    std::string targetBundleName = "com.ohos.backgroundtaskmgr.resources";
    std::string targetAbilityName = "EnableBgTaskAuthDialog";
    want.SetElementName(targetBundleName, targetAbilityName);
    std::string typeKey = "ability.want.params.uiExtensionType";
    std::string typeValue = "sysDialog/common";
    want.SetParam(typeKey, typeValue);
    int32_t appIndex = abilityContext->GetAbilityInfo()->appIndex;
    std::string bundleName = abilityContext->GetBundleName();
    want.SetParam("appIndex", appIndex);
    int32_t taskMode = static_cast<int32_t>(taskParam.bgSubModeIds_[0]);
    want.SetParam("taskMode", taskMode);
    auto uiExtCallback = std::make_shared<ModalExtensionCallback>();
    uiExtCallback->SetAbilityContext(abilityContext);
    uiExtCallback->SetBundleName(bundleName);
    Ace::ModalUIExtensionCallbacks uiExtensionCallbacks = {
        .onRelease = std::bind(&ModalExtensionCallback::OnRelease, uiExtCallback, std::placeholders::_1),
        .onResult = std::bind(&ModalExtensionCallback::OnResult, uiExtCallback,
            std::placeholders::_1, std::placeholders::_2),
        .onReceive = std::bind(&ModalExtensionCallback::OnReceive, uiExtCallback, std::placeholders::_1),
        .onError = std::bind(&ModalExtensionCallback::OnError, uiExtCallback,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        .onRemoteReady = std::bind(&ModalExtensionCallback::OnRemoteReady, uiExtCallback, std::placeholders::_1),
        .onDestroy = std::bind(&ModalExtensionCallback::OnDestroy, uiExtCallback),
    };
    Ace::ModalUIExtensionConfig config;
    config.isProhibitBack = true;
    int32_t sessionId = uiContent->CreateModalUIExtension(want, uiExtensionCallbacks, config);
    if (sessionId == 0) {
        BGTASK_LOGE("Create component failed, sessionId is 0");
        return false;
    }
    uiExtCallback->SetSessionId(sessionId);
    return true;
}
 
ModalExtensionCallback::ModalExtensionCallback()
{}

ModalExtensionCallback::~ModalExtensionCallback()
{}

/*
* when UIExtensionAbility use terminateSelfWithResult
*/
void ModalExtensionCallback::OnResult(int32_t resultCode, const AAFwk::Want& result)
{
    BGTASK_LOGI("called OnResult");
}

/*
* when UIExtensionAbility send message to UIExtensionComponent
*/
void ModalExtensionCallback::OnReceive(const AAFwk::WantParams& receive)
{
    BGTASK_LOGI("called OnReceive");
}

/*
* when UIExtensionAbility disconnect or use terminate or process die
* releaseCode is 0 when process normal exit
*/
void ModalExtensionCallback::OnRelease(int32_t releaseCode)
{
    BGTASK_LOGI("called OnRelease");
    ReleaseOrErrorHandle(releaseCode);
}

/*
* when UIExtensionComponent init or turn to background or destroy UIExtensionAbility occur error
*/
void ModalExtensionCallback::OnError(int32_t code, const std::string& name, const std::string& message)
{
    BGTASK_LOGI("called OnError, name = %{public}s, message = %{public}s", name.c_str(), message.c_str());
    ReleaseOrErrorHandle(code);
}

/*
* when UIExtensionComponent connect to UIExtensionAbility, ModalUIExtensionProxy will init,
* UIExtensionComponent can send message to UIExtensionAbility by ModalUIExtensionProxy
*/
void ModalExtensionCallback::OnRemoteReady(const std::shared_ptr<Ace::ModalUIExtensionProxy>& uiProxy)
{
    BGTASK_LOGI("called OnRemoteReady");
}

/*
* when UIExtensionComponent destructed
*/
void ModalExtensionCallback::OnDestroy()
{
    BGTASK_LOGI("called OnDestroy");
}

void ModalExtensionCallback::SetSessionId(int32_t sessionId)
{
    this->sessionId_ = sessionId;
}

void ModalExtensionCallback::SetBundleName(std::string bundleName)
{
    this->bundleName_ = bundleName;
}

void ModalExtensionCallback::SetAbilityContext(std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> abilityContext)
{
    this->abilityContext_ = abilityContext;
}

void ModalExtensionCallback::ReleaseOrErrorHandle(int32_t code)
{
    Ace::UIContent* uiContent = this->abilityContext_->GetUIContent();
    if (uiContent == nullptr) {
        BGTASK_LOGE("null uiContent");
        return;
    }
    uiContent->CloseModalUIExtension(this->sessionId_);
    return;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS