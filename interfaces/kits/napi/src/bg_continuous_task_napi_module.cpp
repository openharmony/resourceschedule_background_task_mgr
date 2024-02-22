/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "bg_continuous_task_napi_module.h"

#include "ability.h"
#include "bundle_mgr_interface.h"
#include "iservice_registry.h"
#include "napi_base_context.h"
#include "system_ability_definition.h"
#include "want_agent.h"
#ifdef SUPPORT_JSSTACK
#include "xpower_event_js.h"
#endif

#include "background_mode.h"
#include "background_task_mgr_helper.h"
#include "bgtaskmgr_inner_errors.h"
#include "common.h"
#include "continuous_task_log.h"
#include "continuous_task_param.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr uint32_t MAX_START_BG_RUNNING_PARAMS = 4;
static constexpr uint32_t MAX_STOP_BG_RUNNING_PARAMS = 2;
static constexpr uint32_t CALLBACK_RESULT_PARAMS_NUM = 2;
static constexpr uint32_t BG_MODE_ID_BEGIN = 1;
static constexpr uint32_t BG_MODE_ID_END = 9;
static std::vector<std::string> g_backgroundModes = {
    "dataTransfer",
    "audioPlayback",
    "audioRecording",
    "location",
    "bluetoothInteraction",
    "multiDeviceConnection",
    "wifiInteraction",
    "voip",
    "taskKeeping"
    };
}

struct AsyncCallbackInfo : public AsyncWorkData {
    explicit AsyncCallbackInfo(napi_env env) : AsyncWorkData(env) {}
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext {nullptr};
    uint32_t bgMode {0};
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent {nullptr};
    std::vector<uint32_t> bgModes;
    bool isBatchApi {false};
};

napi_value WrapVoidToJS(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

napi_value WrapUndefinedToJS(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value GetCallbackErrorValue(napi_env env, int32_t errCode)
{
    napi_value jsObject = nullptr;
    napi_value jsValue = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &jsValue));
    NAPI_CALL(env, napi_create_object(env, &jsObject));
    NAPI_CALL(env, napi_set_named_property(env, jsObject, "code", jsValue));
    return jsObject;
}

napi_value GetAbilityContext(const napi_env &env, const napi_value &value,
    std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext)
{
    bool stageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, value, stageMode);
    BGTASK_LOGI("is stage mode: %{public}s", stageMode ? "true" : "false");

    if (status != napi_ok || !stageMode) {
        BGTASK_LOGI("Getting context with FA model");
        auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
        if (!ability) {
            BGTASK_LOGE("Failed to get native ability instance");
            return nullptr;
        }
        abilityContext = ability->GetAbilityContext();
        if (!abilityContext) {
            BGTASK_LOGE("get FA model ability context failed");
            return nullptr;
        }
        return WrapVoidToJS(env);
    } else {
        BGTASK_LOGI("Getting context with stage model");
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
        return WrapVoidToJS(env);
    }
}

std::string GetMainAbilityLabel(const std::string &bundleName)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        BGTASK_LOGE("get SystemAbilityManager failed");
        return "";
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        BGTASK_LOGE("get Bundle Manager object failed");
        return "";
    }

    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (bundleMgr == nullptr) {
        BGTASK_LOGE("get Bundle Manager Proxy failed");
        return "";
    }

    AAFwk::Want want;
    want.SetAction("action.system.home");
    want.AddEntity("entity.system.home");
    want.SetElementName("", bundleName, "", "");
    AppExecFwk::AbilityInfo abilityInfo;
    bundleMgr->QueryAbilityInfo(want, abilityInfo);
    return bundleMgr->GetAbilityLabel(bundleName, abilityInfo.name);
}

bool CheckBackgroundMode(napi_env env, AsyncCallbackInfo *asyncCallbackInfo, bool isThrow)
{
    if (!asyncCallbackInfo->isBatchApi) {
        if (asyncCallbackInfo->bgMode < BG_MODE_ID_BEGIN || asyncCallbackInfo->bgMode > BG_MODE_ID_END) {
            BGTASK_LOGE("request background mode id: %{public}u out of range", asyncCallbackInfo->bgMode);
            Common::HandleParamErr(env, ERR_BGMODE_RANGE_ERR, isThrow);
            asyncCallbackInfo->errCode = ERR_BGMODE_RANGE_ERR;
            return false;
        }
    } else {
        for (unsigned int  i = 0; i < asyncCallbackInfo->bgModes.size(); i++) {
            if (asyncCallbackInfo->bgModes[i] < BG_MODE_ID_BEGIN || asyncCallbackInfo->bgModes[i] > BG_MODE_ID_END) {
                BGTASK_LOGE("request background mode id: %{public}u out of range", asyncCallbackInfo->bgModes[i]);
                Common::HandleParamErr(env, ERR_BGMODE_RANGE_ERR, isThrow);
                asyncCallbackInfo->errCode = ERR_BGMODE_RANGE_ERR;
                return false;
            }
        }
    }
    BGTASK_LOGE("ok");
    return true;
}

bool StartBackgroundRunningCheckParam(napi_env env, AsyncCallbackInfo *asyncCallbackInfo, bool isThrow)
{
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("asyncCallbackInfo is nullptr");
        return false;
    }
    if (asyncCallbackInfo->errCode != ERR_OK) {
        BGTASK_LOGE("input params parse failed");
        return false;
    }
    if (asyncCallbackInfo->abilityContext == nullptr) {
        asyncCallbackInfo->errCode = ERR_CONTEXT_NULL_OR_TYPE_ERR;
        Common::HandleParamErr(env, ERR_CONTEXT_NULL_OR_TYPE_ERR, isThrow);
        BGTASK_LOGE("abilityContext is null");
        return false;
    }
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    if (info == nullptr) {
        BGTASK_LOGE("ability info is null");
        Common::HandleParamErr(env, ERR_ABILITY_INFO_EMPTY, isThrow);
        asyncCallbackInfo->errCode = ERR_ABILITY_INFO_EMPTY;
        return false;
    }
    if (asyncCallbackInfo->wantAgent == nullptr) {
        BGTASK_LOGE("wantAgent param is nullptr");
        Common::HandleParamErr(env, ERR_WANTAGENT_NULL_OR_TYPE_ERR, isThrow);
        asyncCallbackInfo->errCode = ERR_WANTAGENT_NULL_OR_TYPE_ERR;
        return false;
    }
    sptr<IRemoteObject> token = asyncCallbackInfo->abilityContext->GetToken();
    if (!token) {
        BGTASK_LOGE("get ability token info failed");
        Common::HandleParamErr(env, ERR_GET_TOKEN_ERR, isThrow);
        asyncCallbackInfo->errCode = ERR_GET_TOKEN_ERR;
        return false;
    }
    if (!CheckBackgroundMode(env, asyncCallbackInfo, isThrow)) {
        return false;
    }
    return true;
}

void UpdateBackgroundRunningExecuteCB(napi_env env, void *data)
{
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr || asyncCallbackInfo->errCode != ERR_OK) {
        BGTASK_LOGE("input params error");
        return;
    }
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    ContinuousTaskParam taskParam = ContinuousTaskParam(true, asyncCallbackInfo->bgMode, nullptr, info->name,
        asyncCallbackInfo->abilityContext->GetToken(), GetMainAbilityLabel(info->bundleName), true, asyncCallbackInfo->bgModes);
    BGTASK_LOGD("StartBackgroundRunningExecuteCB %{public}d, %{public}u", taskParam.isBatchApi_, static_cast<uint32_t>(taskParam.bgModeIds_.size()));
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestUpdateBackgroundRunning(taskParam);
}

void StartBackgroundRunningExecuteCB(napi_env env, void *data)
{
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr || asyncCallbackInfo->errCode != ERR_OK) {
        BGTASK_LOGE("input params error");
        return;
    }
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    ContinuousTaskParam taskParam = ContinuousTaskParam(true, asyncCallbackInfo->bgMode, asyncCallbackInfo->wantAgent,
        info->name, asyncCallbackInfo->abilityContext->GetToken(), GetMainAbilityLabel(info->bundleName),
        asyncCallbackInfo->isBatchApi, asyncCallbackInfo->bgModes);
    BGTASK_LOGD("StartBackgroundRunningExecuteCB %{public}d, %{public}u", taskParam.isBatchApi_, static_cast<uint32_t>(taskParam.bgModeIds_.size()));
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestStartBackgroundRunning(taskParam);
}

void CallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    std::unique_ptr<AsyncCallbackInfo> callbackPtr {asyncCallbackInfo};
    napi_value callback {nullptr};
    napi_value undefined {nullptr};
    napi_value result[CALLBACK_RESULT_PARAMS_NUM] = {nullptr};
    napi_value callResult = {nullptr};
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    if (asyncCallbackInfo->errCode == ERR_OK) {
        result[0] = WrapUndefinedToJS(env);
        napi_create_int32(env, 0, &result[1]);
    } else {
        result[1] = WrapUndefinedToJS(env);
        std::string errMsg = Common::FindErrMsg(env, asyncCallbackInfo->errCode);
        int32_t errCodeInfo = Common::FindErrCode(env, asyncCallbackInfo->errCode);
        result[0] = Common::GetCallbackErrorValue(env, errCodeInfo, errMsg);
    }

    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, asyncCallbackInfo->callback, &callback));
    NAPI_CALL_RETURN_VOID(env,
        napi_call_function(env, undefined, callback, CALLBACK_RESULT_PARAMS_NUM, result, &callResult));
}

void PromiseCompletedCB(napi_env env, napi_status status, void *data)
{
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    std::unique_ptr<AsyncCallbackInfo> callbackPtr {asyncCallbackInfo};
    napi_value result {nullptr};
    if (asyncCallbackInfo->errCode == ERR_OK) {
        napi_create_int32(env, 0, &result);
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, asyncCallbackInfo->deferred, result));
    } else {
        std::string errMsg = Common::FindErrMsg(env, asyncCallbackInfo->errCode);
        int32_t errCodeInfo = Common::FindErrCode(env, asyncCallbackInfo->errCode);
        result = Common::GetCallbackErrorValue(env, errCodeInfo, errMsg);
        NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, asyncCallbackInfo->deferred, result));
    }
}

void ReportXPowerJsStackSysEventByType(napi_env env, const std::string &taskType)
{
    #ifdef SUPPORT_JSSTACK
        HiviewDFX::ReportXPowerJsStackSysEvent(env, taskType);
    #endif
}

napi_value StartBackgroundRunningAsync(napi_env env, napi_value *argv,
    const uint32_t argCallback, AsyncCallbackInfo *asyncCallbackInfo, bool isThrow)
{
    if (argv == nullptr || asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("param is nullptr");
        return nullptr;
    }
    if (isThrow && asyncCallbackInfo->errCode != ERR_OK) {
        return nullptr;
    }
    napi_value resourceName {nullptr};
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[argCallback], &valuetype));
    if (valuetype != napi_function) {
        Common::HandleParamErr(env, ERR_CALLBACK_NULL_OR_TYPE_ERR, isThrow);
        return nullptr;
    }
    NAPI_CALL(env, napi_create_reference(env, argv[argCallback], 1, &asyncCallbackInfo->callback));
    if (!StartBackgroundRunningCheckParam(env, asyncCallbackInfo, isThrow) && isThrow) {
        return nullptr;
    }

    NAPI_CALL(env, napi_create_async_work(env,
        nullptr,
        resourceName,
        StartBackgroundRunningExecuteCB,
        CallbackCompletedCB,
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

    return WrapVoidToJS(env);
}

napi_value UpdateBackgroundRunningPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo, bool isThrow)
{
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("param is nullptr");
        return nullptr;
    }
    if (!CheckBackgroundMode(env, asyncCallbackInfo, isThrow)) {
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise {nullptr};
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    asyncCallbackInfo->deferred = deferred;
    BGTASK_LOGE("11");
    NAPI_CALL(env, napi_create_async_work(env,
        nullptr,
        resourceName,
        UpdateBackgroundRunningExecuteCB,
        PromiseCompletedCB,
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    return promise;
}

napi_value StartBackgroundRunningPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo, bool isThrow)
{
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("param is nullptr");
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise {nullptr};
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    asyncCallbackInfo->deferred = deferred;
    if (!StartBackgroundRunningCheckParam(env, asyncCallbackInfo, isThrow) && isThrow) {
        return nullptr;
    }
    BGTASK_LOGE("11");
    NAPI_CALL(env, napi_create_async_work(env,
        nullptr,
        resourceName,
        StartBackgroundRunningExecuteCB,
        PromiseCompletedCB,
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    return promise;
}

bool CheckTypeForNapiValue(napi_env env, napi_value param, napi_valuetype expectType)
{
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, param, &valueType) != napi_ok) {
        return false;
    }
    return valueType == expectType;
}

napi_value GetBackgroundMode(const napi_env &env, const napi_value &value, AsyncCallbackInfo *asyncCallbackInfo)
{
    bool isArray = false;
    if (napi_is_array(env, value, &isArray) != napi_ok || isArray == false) {
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, value, &valuetype));
        if (valuetype != napi_number) {
            Common::HandleParamErr(env, ERR_BGMODE_NULL_OR_TYPE_ERR, true);
            return nullptr;
        }
        napi_get_value_uint32(env, value, &asyncCallbackInfo->bgMode);
        asyncCallbackInfo->isBatchApi = false;
        BGTASK_LOGI("get bgmode info: %{public}u", asyncCallbackInfo->bgMode);
        return WrapVoidToJS(env);
    } else {
        uint32_t arrayLen = 0;
        napi_get_array_length(env, value, &arrayLen);
        BGTASK_LOGI("get bgModes arraylen: %{public}u", arrayLen);
        if (arrayLen == 0 ) {
            Common::HandleParamErr(env, ERR_BGMODE_NULL_OR_TYPE_ERR, true);
            return nullptr;
        }
        asyncCallbackInfo->isBatchApi = true;
        for (uint32_t i = 0; i < arrayLen; i++) {
            napi_value mode = nullptr;
            napi_get_element(env, value, i, &mode);
            std::string result;
            if (Common::GetStringValue(env, mode, result) == nullptr) {
                BGTASK_LOGE("GetBackgroundMode failed.");
                Common::HandleParamErr(env, ERR_BGMODE_NULL_OR_TYPE_ERR, true);
                return nullptr;
            }
            BGTASK_LOGI("GetBackgroundMode %{public}s.", result.c_str());
            auto it = std::find(g_backgroundModes.begin(), g_backgroundModes.end(), result);
            if (it != g_backgroundModes.end()) {
                auto index = std::distance(g_backgroundModes.begin(), it);
                auto modeIter = std::find(asyncCallbackInfo->bgModes.begin(), asyncCallbackInfo->bgModes.end(), index + 1);
                if (modeIter == asyncCallbackInfo->bgModes.end()) {
                    asyncCallbackInfo->bgModes.push_back(index + 1);
                }
            } else {
                BGTASK_LOGE("mode string is invalid");
                Common::HandleParamErr(env, ERR_BGMODE_NULL_OR_TYPE_ERR, true);
                return nullptr;
            }
        }
        return WrapVoidToJS(env);
    }
}

napi_value GetWantAgent(const napi_env &env, const napi_value &value,
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> &wantAgent)
{
    napi_valuetype valuetype = napi_undefined;
    AbilityRuntime::WantAgent::WantAgent *wantAgentPtr = nullptr;
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    if (valuetype != napi_object) {
        Common::HandleParamErr(env, ERR_WANTAGENT_NULL_OR_TYPE_ERR, true);
        return nullptr;
    }
    napi_unwrap(env, value, (void **)&wantAgentPtr);
    wantAgent = std::make_shared<AbilityRuntime::WantAgent::WantAgent>(*wantAgentPtr);

    return WrapVoidToJS(env);
}

bool StartBackgroundRunningCheckParamBeforeSubmit(napi_env env, napi_value *argv, size_t len, bool isThrow,
    AsyncCallbackInfo *asyncCallbackInfo)
{
    // argv[0] : context : AbilityContext
    if (GetAbilityContext(env, argv[0], asyncCallbackInfo->abilityContext) == nullptr) {
        BGTASK_LOGE("Get ability context failed");
        Common::HandleParamErr(env, ERR_CONTEXT_NULL_OR_TYPE_ERR, isThrow);
        asyncCallbackInfo->errCode = ERR_CONTEXT_NULL_OR_TYPE_ERR;
        return false;
    }

    // argv[1] : bgMode : BackgroundMode
    if (GetBackgroundMode(env, argv[1], asyncCallbackInfo) == nullptr) {
        BGTASK_LOGE("input bgmode param not number");
        Common::HandleParamErr(env, ERR_BGMODE_NULL_OR_TYPE_ERR, isThrow);
        asyncCallbackInfo->errCode = ERR_BGMODE_NULL_OR_TYPE_ERR;
        return false;
    }

    // argv[2] : wantAgent: WantAgent
    if (GetWantAgent(env, argv[2], asyncCallbackInfo->wantAgent) == nullptr) {
        BGTASK_LOGE("input wantAgent param is not object");
        Common::HandleParamErr(env, ERR_WANTAGENT_NULL_OR_TYPE_ERR, isThrow);
        asyncCallbackInfo->errCode = ERR_WANTAGENT_NULL_OR_TYPE_ERR;
        return false;
    }
    return true;
}


napi_value UpdateBackgroundRunning(napi_env env, napi_callback_info info, bool isThrow)
{
    ReportXPowerJsStackSysEventByType(env, "CONTINUOUS_TASK_UPDATE");
    AsyncCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("asyncCallbackInfo == nullpter");
        return WrapVoidToJS(env);
    }

    size_t argc = 2;
    napi_value argv[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != 2) {
        BGTASK_LOGE("wrong param nums");
        Common::HandleParamErr(env, ERR_PARAM_NUMBER_ERR, isThrow);
        return WrapVoidToJS(env);
    }

    // argv[0] : context : AbilityContext
    if (GetAbilityContext(env, argv[0], asyncCallbackInfo->abilityContext) == nullptr) {
        BGTASK_LOGE("Get ability context failed");
        Common::HandleParamErr(env, ERR_CONTEXT_NULL_OR_TYPE_ERR, isThrow);
        asyncCallbackInfo->errCode = ERR_CONTEXT_NULL_OR_TYPE_ERR;
        return WrapVoidToJS(env);
    }

    // argv[1] : bgMode : BackgroundMode
    if (GetBackgroundMode(env, argv[1], asyncCallbackInfo) == nullptr) {
        BGTASK_LOGE("input bgmode param not number");
        Common::HandleParamErr(env, ERR_BGMODE_NULL_OR_TYPE_ERR, isThrow);
        asyncCallbackInfo->errCode = ERR_BGMODE_NULL_OR_TYPE_ERR;
        return WrapVoidToJS(env);
    }

    napi_value ret {nullptr};
    ret = UpdateBackgroundRunningPromise(env, asyncCallbackInfo, isThrow);
    if (ret == nullptr) {
        BGTASK_LOGE("ret is nullpter");
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    return ret;
}

napi_value StartBackgroundRunning(napi_env env, napi_callback_info info, bool isThrow)
{
    ReportXPowerJsStackSysEventByType(env, "CONTINUOUS_TASK_APPLY");
    AsyncCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("asyncCallbackInfo == nullpter");
        return WrapVoidToJS(env);
    }
    std::unique_ptr<AsyncCallbackInfo> callbackPtr {asyncCallbackInfo};

    size_t argc = MAX_START_BG_RUNNING_PARAMS;
    napi_value argv[MAX_START_BG_RUNNING_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc > MAX_START_BG_RUNNING_PARAMS) {
        BGTASK_LOGE("wrong param nums");
        Common::HandleParamErr(env, ERR_PARAM_NUMBER_ERR, isThrow);
        return WrapVoidToJS(env);
    }

    if (!StartBackgroundRunningCheckParamBeforeSubmit(env, argv, MAX_START_BG_RUNNING_PARAMS, isThrow,
        asyncCallbackInfo)) {
        return WrapVoidToJS(env);
    }

    napi_value ret {nullptr};

    if (argc == MAX_START_BG_RUNNING_PARAMS) {
        ret = StartBackgroundRunningAsync(env, argv, MAX_START_BG_RUNNING_PARAMS - 1, asyncCallbackInfo, isThrow);
    } else {
        ret = StartBackgroundRunningPromise(env, asyncCallbackInfo, isThrow);
    }

    if (ret == nullptr) {
        BGTASK_LOGE("ret is nullpter");
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    callbackPtr.release();
    return ret;
}

bool StopBackgroundRunningCheckParam(napi_env env, AsyncCallbackInfo *asyncCallbackInfo, bool isThrow)
{
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("asyncCallbackInfo is nullptr");
        return false;
    }
    if (asyncCallbackInfo->abilityContext == nullptr) {
        BGTASK_LOGE("ability context is null");
        Common::HandleParamErr(env, ERR_CONTEXT_NULL_OR_TYPE_ERR, isThrow);
        asyncCallbackInfo->errCode = ERR_CONTEXT_NULL_OR_TYPE_ERR;
        return false;
    }
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    if (info == nullptr) {
        BGTASK_LOGE("abilityInfo is null");
        Common::HandleParamErr(env, ERR_ABILITY_INFO_EMPTY, isThrow);
        asyncCallbackInfo->errCode = ERR_ABILITY_INFO_EMPTY;
        return false;
    }

    sptr<IRemoteObject> token = asyncCallbackInfo->abilityContext->GetToken();
    if (!token) {
        BGTASK_LOGE("get ability token info failed");
        Common::HandleParamErr(env, ERR_GET_TOKEN_ERR, isThrow);
        asyncCallbackInfo->errCode = ERR_GET_TOKEN_ERR;
        return false;
    }
    return true;
}

void StopBackgroundRunningExecuteCB(napi_env env, void *data)
{
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr || asyncCallbackInfo->errCode != ERR_OK) {
        BGTASK_LOGE("input param error");
        return;
    }
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    sptr<IRemoteObject> token = asyncCallbackInfo->abilityContext->GetToken();
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestStopBackgroundRunning(info->name, token);
}

napi_value StopBackgroundRunningAsync(napi_env env, napi_value *argv,
    const uint32_t argCallback, AsyncCallbackInfo *asyncCallbackInfo, bool isThrow)
{
    if (argv == nullptr || asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("param is nullptr");
        return nullptr;
    }
    if (isThrow && asyncCallbackInfo->errCode != ERR_OK) {
        return nullptr;
    }
    napi_value resourceName {nullptr};
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, argv[argCallback], 1, &asyncCallbackInfo->callback));
    }
    if (!StopBackgroundRunningCheckParam(env, asyncCallbackInfo, isThrow) && isThrow) {
        return nullptr;
    }

    NAPI_CALL(env, napi_create_async_work(env,
        nullptr,
        resourceName,
        StopBackgroundRunningExecuteCB,
        CallbackCompletedCB,
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    return WrapVoidToJS(env);
}

napi_value StopBackgroundRunningPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo, bool isThrow)
{
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("param is nullptr");
        return nullptr;
    }
    napi_value resourceName {nullptr};
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise {nullptr};
    napi_create_promise(env, &deferred, &promise);

    asyncCallbackInfo->deferred = deferred;
    if (!StopBackgroundRunningCheckParam(env, asyncCallbackInfo, isThrow) && isThrow) {
        return nullptr;
    }

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        StopBackgroundRunningExecuteCB,
        PromiseCompletedCB,
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return promise;
}

napi_value StopBackgroundRunning(napi_env env, napi_callback_info info, bool isThrow)
{
    ReportXPowerJsStackSysEventByType(env, "CONTINUOUS_TASK_CANCEL");
    AsyncCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("asyncCallbackInfo is nullpter");
        return WrapVoidToJS(env);
    }
    std::unique_ptr<AsyncCallbackInfo> callbackPtr {asyncCallbackInfo};

    size_t argc = MAX_STOP_BG_RUNNING_PARAMS;
    napi_value argv[MAX_STOP_BG_RUNNING_PARAMS] = {nullptr};

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc > MAX_STOP_BG_RUNNING_PARAMS) {
        BGTASK_LOGE("wrong param nums");
        Common::HandleParamErr(env, ERR_PARAM_NUMBER_ERR, isThrow);
        return nullptr;
    }

    // argv[0] : context : AbilityContext
    if (GetAbilityContext(env, argv[0], asyncCallbackInfo->abilityContext) == nullptr) {
        BGTASK_LOGE("Get ability context failed");
        Common::HandleParamErr(env, ERR_CONTEXT_NULL_OR_TYPE_ERR, isThrow);
        asyncCallbackInfo->errCode = ERR_CONTEXT_NULL_OR_TYPE_ERR;
        return nullptr;
    }

    napi_value ret {nullptr};
    if (argc == MAX_STOP_BG_RUNNING_PARAMS) {
        ret = StopBackgroundRunningAsync(env, argv, MAX_STOP_BG_RUNNING_PARAMS - 1, asyncCallbackInfo, isThrow);
    } else {
        ret = StopBackgroundRunningPromise(env, asyncCallbackInfo, isThrow);
    }

    if (ret == nullptr) {
        BGTASK_LOGE("ret is nullpter");
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    callbackPtr.release();
    return ret;
}

napi_value StartBackgroundRunning(napi_env env, napi_callback_info info)
{
    return StartBackgroundRunning(env, info, false);
}

napi_value StopBackgroundRunning(napi_env env, napi_callback_info info)
{
    return StopBackgroundRunning(env, info, false);
}

napi_value StartBackgroundRunningThrow(napi_env env, napi_callback_info info)
{
    return StartBackgroundRunning(env, info, true);
}

napi_value UpdateBackgroundRunningThrow(napi_env env, napi_callback_info info)
{
    return UpdateBackgroundRunning(env, info, true);
}

napi_value StopBackgroundRunningThrow(napi_env env, napi_callback_info info)
{
    return StopBackgroundRunning(env, info, true);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS