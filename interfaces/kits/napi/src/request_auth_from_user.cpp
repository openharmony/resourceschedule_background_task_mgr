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

#include "background_task_manager.h"
#include "hitrace_meter.h"
#include "continuous_task_log.h"
#include "background_task_mode.h"
#include "continuous_task_param.h"
#include "continuous_task_request.h"

namespace OHOS {
namespace BackgroundTaskMgr {
std::map<int32_t, std::shared_ptr<ExpiredCallback>> authCallbackInstances_;
std::mutex authCallbackLock_;
static const uint32_t REQUEST_AUTH_FORM_USER_PARAMS = 1;

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
    if (napi_status::napi_ok != napi_send_event(expiredCallbackInfo_.env, task, napi_eprio_high)) {
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
    if (napi_status::napi_ok != napi_send_event(expiredCallbackInfo_.env, task, napi_eprio_high)) {
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

napi_value GetExpiredCallback(
    const napi_env &env, const napi_value &value, std::shared_ptr<AuthCallbackInstance> &callback)
{
    napi_ref result = nullptr;
    callback = std::make_shared<AuthCallbackInstance>();
    callback->Init();
    napi_create_reference(env, value, 1, &result);
    callback->SetCallbackInfo(env, result);
    return Common::NapiGetNull(env);
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
    napi_value backgroundTaskModes;
    status = napi_get_named_property(env, jsThis, "backgroundTaskModes", &backgroundTaskModes);
    if (status != napi_ok ||
        !Common::GetBackgroundTaskModesFromArray(env, backgroundTaskModes, request)) {
        Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_REQUEST_NULL_OR_TYPE, true);
        BGTASK_LOGE("failed to check params");
        return false;
    }
    std::vector<uint32_t> backgroundTaskModesValue = request->GetBackgroundTaskModes();
    if (backgroundTaskModesValue.size() == 0) {
        Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_IS_EMPTY, true);
        return false;
    }
    uint32_t specialModeSize = std::count(backgroundTaskModesValue.begin(), backgroundTaskModesValue.end(),
        BackgroundTaskMode::MODE_SPECIAL_SCENARIO_PROCESSING);
    if (specialModeSize == 0) {
        Common::HandleErrCode(env, ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_EMPTY, true);
        return false;
    }
    if (specialModeSize > 1) {
        Common::HandleErrCode(env, ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_ONLY_ALLOW_ONE_APPLICATION, true);
        return false;
    }
    if (specialModeSize == 1 && backgroundTaskModesValue.size() > 1) {
        Common::HandleErrCode(env, ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_CONFLICTS_WITH_OTHER_TASK, true);
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
        BGTASK_LOGE("check request auth params fail.");
        return Common::NapiGetNull(env);
    }
    size_t argc = REQUEST_AUTH_FORM_USER_PARAMS;
    napi_value argv[REQUEST_AUTH_FORM_USER_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != REQUEST_AUTH_FORM_USER_PARAMS) {
        Common::HandleParamErr(env, ERR_PARAM_NUMBER_ERR, true);
        return Common::NapiGetNull(env);
    }
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    if (valuetype != napi_function) {
        Common::HandleParamErr(env, ERR_BGTASK_CONTINUOUS_CALLBACK_NULL_OR_TYPE_ERR, true);
        return Common::NapiGetNull(env);
    }
    std::shared_ptr<AuthCallbackInstance> callback = nullptr;
    if (GetExpiredCallback(env, argv[0], callback) == nullptr) {
        BGTASK_LOGE("ExpiredCallback parse failed");
        Common::HandleParamErr(env, ERR_BGTASK_CONTINUOUS_CALLBACK_NULL_OR_TYPE_ERR, true);
        return Common::NapiGetNull(env);
    }
    int32_t notificationId = -1;
    std::vector<uint32_t> backgroundTaskModesValue = request->GetBackgroundTaskModes();
    ContinuousTaskParam taskParam = ContinuousTaskParam(true, backgroundTaskModesValue[0],
        nullptr, "", nullptr, "", true, backgroundTaskModesValue, -1);
    taskParam.isByRequestObject_ = true;
    ErrCode errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        RequestAuthFromUser(taskParam, *callback, notificationId);
    if (notificationId == -1) {
        Common::HandleParamErr(env, ERR_BGTASK_NOTIFICATION_ERR, true);
        return Common::NapiGetNull(env);
    }
    Common::HandleErrCode(env, errCode, true);
    {
        std::lock_guard<std::mutex> lock(authCallbackLock_);
        authCallbackInstances_[notificationId] = callback;
    }
    napi_value result = nullptr;
    napi_create_int32(env, 0, &result);
    return result;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS