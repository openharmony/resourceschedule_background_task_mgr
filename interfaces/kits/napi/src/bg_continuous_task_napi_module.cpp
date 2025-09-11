/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "hitrace_meter.h"
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
#include "continuous_task_info.h"
#include "continuous_task_log.h"
#include "continuous_task_mode.h"
#include "continuous_task_param.h"
#include "continuous_task_request.h"
#include "continuous_task_submode.h"
#include "js_backgroundtask_subscriber.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr uint32_t MAX_START_BG_RUNNING_PARAMS = 4;
static constexpr uint32_t MAX_START_BG_RUNNING_BY_RQUEST_PARAMS = 2;
static constexpr uint32_t MAX_STOP_BG_RUNNING_PARAMS = 2;
static constexpr uint32_t MAX_UPDATE_BG_RUNNING_PARAMS = 2;
static constexpr uint32_t MAX_GET_ALL_CONTINUOUSTASK_PARAMS = 2;
static constexpr uint32_t CALLBACK_RESULT_PARAMS_NUM = 2;
static constexpr uint32_t BG_MODE_ID_BEGIN = 1;
static constexpr uint32_t BG_MODE_ID_END = 9;
static constexpr int32_t SYSTEM_LIVE_CONTENT_TYPE = 8;
static constexpr int32_t SLOT_TYPE = 4;
static constexpr uint32_t ARGC_ONE = 1;
static constexpr uint32_t ARGC_TWO = 2;
static constexpr uint32_t INDEX_ZERO = 0;
static constexpr uint32_t INDEX_ONE = 1;
static constexpr uint32_t CONTINUOUS_TASK_CANCEL = 1 << 0;
static constexpr uint32_t CONTINUOUS_TASK_SUSPEND = 1 << 1;
static constexpr uint32_t CONTINUOUS_TASK_ACTIVE = 1 << 2;
static std::shared_ptr<JsBackgroundTaskSubscriber> backgroundTaskSubscriber_ = nullptr;
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

static std::vector<std::string> g_callbackTypes = {
    "continuousTaskCancel",
    "continuousTaskSuspend",
    "continuousTaskActive"
    };
}

struct AsyncCallbackInfo : public AsyncWorkData {
    explicit AsyncCallbackInfo(napi_env env) : AsyncWorkData(env) {}
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext {nullptr};
    uint32_t bgMode {0};
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent {nullptr};
    std::vector<uint32_t> bgModes {};
    bool isBatchApi {false};
    bool includeSuspended {false};
    std::shared_ptr<ContinuousTaskRequest> request = std::make_shared<ContinuousTaskRequest>();
    int32_t removeTaskId {-1}; // in
    int32_t notificationId {-1}; // out
    int32_t continuousTaskId {-1}; // out
    std::vector<std::shared_ptr<ContinuousTaskInfo>> list; // out
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
    BGTASK_LOGD("is stage mode: %{public}s", stageMode ? "true" : "false");

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
        BGTASK_LOGD("Getting context with stage model");
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
        BGTASK_LOGE("check background mode failed.");
        return false;
    }
    return true;
}

void UpdateBackgroundRunningExecuteCB(napi_env env, void *data)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Napi::UpdateBackgroundRunningExecuteCB");
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr || asyncCallbackInfo->errCode != ERR_OK) {
        BGTASK_LOGE("input params error");
        return;
    }
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    ContinuousTaskParam taskParam = ContinuousTaskParam(true, asyncCallbackInfo->bgMode, nullptr, info->name,
        asyncCallbackInfo->abilityContext->GetToken(), "", true, asyncCallbackInfo->bgModes,
        asyncCallbackInfo->abilityContext->GetAbilityRecordId());
    BGTASK_LOGI("RequestUpdateBackgroundRunning isBatch: %{public}d, bgModeSize: %{public}u",
        taskParam.isBatchApi_, static_cast<uint32_t>(taskParam.bgModeIds_.size()));
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestUpdateBackgroundRunning(taskParam);
    asyncCallbackInfo->notificationId = taskParam.notificationId_;
    asyncCallbackInfo->continuousTaskId = taskParam.continuousTaskId_;
    BGTASK_LOGI("notification %{public}d, continuousTaskId %{public}d", taskParam.notificationId_,
        taskParam.continuousTaskId_);
}

void UpdateBackgroundRunningByRequestExecuteCB(napi_env env, void *data)
{
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr || asyncCallbackInfo->errCode != ERR_OK ||
        asyncCallbackInfo->request == nullptr) {
        BGTASK_LOGE("input params error");
        return;
    }
    Common::TaskModeTypeConversion(asyncCallbackInfo->request);
    std::vector<uint32_t> continuousTaskModes = asyncCallbackInfo->request->GetContinuousTaskModes();
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    ContinuousTaskParam taskParam = ContinuousTaskParam(true, continuousTaskModes[0],
        asyncCallbackInfo->request->GetWantAgent(), info->name, asyncCallbackInfo->abilityContext->GetToken(),
        "", true, continuousTaskModes, asyncCallbackInfo->abilityContext->GetAbilityRecordId());
    taskParam.isByRequestObject_ = true;
    taskParam.isCombinedTaskNotification_ = asyncCallbackInfo->request->IsCombinedTaskNotification();
    taskParam.updateTaskId_ = asyncCallbackInfo->request->GetContinuousTaskId();
    taskParam.bgSubModeIds_ = asyncCallbackInfo->request->GetContinuousTaskSubmodes();
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestUpdateBackgroundRunning(taskParam);
    asyncCallbackInfo->bgModes = continuousTaskModes;
    asyncCallbackInfo->notificationId = taskParam.notificationId_;
    asyncCallbackInfo->continuousTaskId = taskParam.continuousTaskId_;
}

void StartBackgroundRunningExecuteCB(napi_env env, void *data)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Napi::StartBackgroundRunningExecuteCB");
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr || asyncCallbackInfo->errCode != ERR_OK) {
        BGTASK_LOGE("input params error");
        return;
    }
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    ContinuousTaskParam taskParam = ContinuousTaskParam(true, asyncCallbackInfo->bgMode, asyncCallbackInfo->wantAgent,
        info->name, asyncCallbackInfo->abilityContext->GetToken(), "",
        asyncCallbackInfo->isBatchApi, asyncCallbackInfo->bgModes,
        asyncCallbackInfo->abilityContext->GetAbilityRecordId());
    BGTASK_LOGD("RequestStartBackgroundRunning isBatch: %{public}d, bgModeSize: %{public}u",
        taskParam.isBatchApi_, static_cast<uint32_t>(taskParam.bgModeIds_.size()));
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestStartBackgroundRunning(taskParam);
    asyncCallbackInfo->notificationId = taskParam.notificationId_;
    asyncCallbackInfo->continuousTaskId = taskParam.continuousTaskId_;
    BGTASK_LOGI("notification %{public}d, continuousTaskId %{public}d", taskParam.notificationId_,
        taskParam.continuousTaskId_);
}

void CallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Napi::CallbackCompletedCB");
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
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Napi::PromiseCompletedCB");
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    std::unique_ptr<AsyncCallbackInfo> callbackPtr {asyncCallbackInfo};
    napi_value result {nullptr};
    if (asyncCallbackInfo->errCode == ERR_OK) {
        if (asyncCallbackInfo->bgModes.size() > 0) {
            napi_value slotType = nullptr;
            napi_value contentType = nullptr;
            napi_value notificationId = nullptr;
            napi_value continuousTaskId = nullptr;
            napi_create_object(env, &result);
            napi_create_int32(env, SLOT_TYPE, &slotType);
            napi_create_int32(env, SYSTEM_LIVE_CONTENT_TYPE, &contentType);
            napi_create_int32(env, asyncCallbackInfo->notificationId, &notificationId);
            napi_create_int32(env, asyncCallbackInfo->continuousTaskId, &continuousTaskId);
            napi_set_named_property(env, result, "slotType", slotType);
            napi_set_named_property(env, result, "contentType", contentType);
            napi_set_named_property(env, result, "notificationId", notificationId);
            napi_set_named_property(env, result, "continuousTaskId", continuousTaskId);
        } else {
            napi_create_int32(env, 0, &result);
        }
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
        BGTASK_LOGE("valuetype is no napi_function.");
        return nullptr;
    }
    NAPI_CALL(env, napi_create_reference(env, argv[argCallback], 1, &asyncCallbackInfo->callback));
    if (!StartBackgroundRunningCheckParam(env, asyncCallbackInfo, isThrow) && isThrow) {
        BGTASK_LOGE("start bgytask check param fail.");
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

napi_value UpdateBackgroundRunningByRequestPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo, bool isThrow)
{
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("param is nullptr");
        return nullptr;
    }
    if (asyncCallbackInfo->request->GetContinuousTaskId() < 0) {
        Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_TASKID_INVALID, true);
        asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_TASKID_INVALID;
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise {nullptr};
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    asyncCallbackInfo->deferred = deferred;
    NAPI_CALL(env, napi_create_async_work(env,
        nullptr,
        resourceName,
        UpdateBackgroundRunningByRequestExecuteCB,
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
        BGTASK_LOGE("start bgytask check param fail.");
        return nullptr;
    }
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

napi_value GetMode(const napi_env &env, const napi_value &value, AsyncCallbackInfo *asyncCallbackInfo)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    if (valuetype != napi_number) {
        Common::HandleParamErr(env, ERR_BGMODE_NULL_OR_TYPE_ERR, true);
        return nullptr;
    }
    napi_get_value_uint32(env, value, &asyncCallbackInfo->bgMode);
    asyncCallbackInfo->isBatchApi = false;
    BGTASK_LOGD("get bgmode info: %{public}u", asyncCallbackInfo->bgMode);
    return WrapVoidToJS(env);
}

napi_value GetModes(const napi_env &env, const napi_value &value, AsyncCallbackInfo *asyncCallbackInfo)
{
    uint32_t arrayLen = 0;
    napi_get_array_length(env, value, &arrayLen);
    BGTASK_LOGD("get bgModes arraylen: %{public}u", arrayLen);
    if (arrayLen == 0) {
        BGTASK_LOGE("get bgModes arraylen is 0");
        Common::HandleParamErr(env, ERR_BGMODE_NULL_OR_TYPE_ERR, true);
        return nullptr;
    }
    asyncCallbackInfo->isBatchApi = true;
    for (uint32_t i = 0; i < arrayLen; i++) {
        napi_value mode = nullptr;
        napi_get_element(env, value, i, &mode);
        std::string result;
        if (Common::GetStringValue(env, mode, result) == nullptr) {
            BGTASK_LOGE("GetStringValue failed.");
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

napi_value GetBackgroundMode(const napi_env &env, const napi_value &value, AsyncCallbackInfo *asyncCallbackInfo)
{
    bool isArray = false;
    if (napi_is_array(env, value, &isArray) != napi_ok || isArray == false) {
        return GetMode(env, value, asyncCallbackInfo);
    } else {
        return GetModes(env, value, asyncCallbackInfo);
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
    if (wantAgentPtr == nullptr) {
        return nullptr;
    }
    wantAgent = std::make_shared<AbilityRuntime::WantAgent::WantAgent>(*wantAgentPtr);

    return WrapVoidToJS(env);
}

napi_value GetIncludeSuspended(const napi_env &env, const napi_value &value, bool &includeSuspended)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    if (valuetype != napi_boolean) {
        Common::HandleParamErr(env, ERR_WANTAGENT_NULL_OR_TYPE_ERR, true);
        return nullptr;
    }
    napi_get_value_bool(env, value, &includeSuspended);
    return WrapVoidToJS(env);
}

bool GetAllContinuousTasksCheckParamBeforeSubmit(napi_env env, size_t argc, napi_value *argv, bool isThrow,
    AsyncCallbackInfo *asyncCallbackInfo)
{
    if (argc > MAX_GET_ALL_CONTINUOUSTASK_PARAMS || argc < ARGC_ONE) {
        BGTASK_LOGE("wrong param nums");
        Common::HandleParamErr(env, ERR_PARAM_NUMBER_ERR, isThrow);
        asyncCallbackInfo->errCode = ERR_PARAM_NUMBER_ERR;
        return false;
    }
    // argv[0] : context : AbilityContext
    if (GetAbilityContext(env, argv[0], asyncCallbackInfo->abilityContext) == nullptr) {
        BGTASK_LOGE("GetAllContinuousTasks Get ability context failed");
        Common::HandleParamErr(env, ERR_CONTEXT_NULL_OR_TYPE_ERR, isThrow);
        asyncCallbackInfo->errCode = ERR_CONTEXT_NULL_OR_TYPE_ERR;
        return false;
    }

    // argv[1] : includeSuspended : includeSuspended
    if (argc == ARGC_TWO && GetIncludeSuspended(env, argv[1], asyncCallbackInfo->includeSuspended) == nullptr) {
        BGTASK_LOGE("GetAllContinuousTasks Get includeSuspended failed");
        Common::HandleParamErr(env, ERR_BGMODE_NULL_OR_TYPE_ERR, isThrow);
        asyncCallbackInfo->errCode = ERR_BGMODE_NULL_OR_TYPE_ERR;
        return false;
    }
    return true;
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

bool StartBackgroundRunningCheckModes(napi_env env, bool isThrow, AsyncCallbackInfo *asyncCallbackInfo)
{
    std::vector<uint32_t> continuousTaskModes = asyncCallbackInfo->request->GetContinuousTaskModes();
    std::vector<uint32_t> continuousTaskSubmodes = asyncCallbackInfo->request->GetContinuousTaskSubmodes();
    if (continuousTaskModes.size() == 0 || continuousTaskSubmodes.size() == 0) {
        Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_IS_EMPTY, true);
        asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_IS_EMPTY;
        return false;
    }
    if (continuousTaskModes.size() != continuousTaskSubmodes.size()) {
        Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_LENGTH_MISMATCH, true);
        asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_LENGTH_MISMATCH;
        return false;
    }
    for (uint32_t index = 0; index < continuousTaskSubmodes.size(); index++) {
        uint32_t subMode = continuousTaskSubmodes[index];
        if (subMode >= ContinuousTaskSubmode::END ||
            subMode < ContinuousTaskSubmode::SUBMODE_CAR_KEY_NORMAL_NOTIFICATION) {
            Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_CROSS_BORDER, true);
            asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_CROSS_BORDER;
            return false;
        }
        if (subMode == ContinuousTaskSubmode::SUBMODE_NORMAL_NOTIFICATION) {
            if (!ContinuousTaskMode::IsModeTypeMatching(continuousTaskModes[index])) {
                Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_TYPE_MISMATCH, true);
                asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_TYPE_MISMATCH;
                return false;
            }
        } else {
            if (ContinuousTaskMode::GetSubModeTypeMatching(subMode) != continuousTaskModes[index]) {
                Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_TYPE_MISMATCH, true);
                asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_TYPE_MISMATCH;
                return false;
            }
        }
    }
    return true;
}

bool StartBackgroundRunningCheckRequest(napi_env env, napi_value *argv, uint32_t argc, bool isThrow,
    AsyncCallbackInfo *asyncCallbackInfo)
{
    if (argc != MAX_UPDATE_BG_RUNNING_PARAMS) {
        Common::HandleParamErr(env, ERR_PARAM_NUMBER_ERR, isThrow);
        asyncCallbackInfo->errCode = ERR_PARAM_NUMBER_ERR;
        return false;
    }
    if (GetAbilityContext(env, argv[0], asyncCallbackInfo->abilityContext) == nullptr) {
        BGTASK_LOGE("Get ability context failed");
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
    if (!Common::GetContinuousTaskRequest(env, argv[1], asyncCallbackInfo->request)) {
        Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_REQUEST_NULL_OR_TYPE, true);
        asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_REQUEST_NULL_OR_TYPE;
        return false;
    }
    if (!StartBackgroundRunningCheckModes(env, isThrow, asyncCallbackInfo)) {
        BGTASK_LOGE("check continuous modes fail");
        return false;
    }
    return true;
}

bool UpdateBackgroundRunningCheckParam(napi_env env, napi_value *argv, uint32_t argc,
    AsyncCallbackInfo *asyncCallbackInfo, bool isThrow)
{
    if (argc != MAX_UPDATE_BG_RUNNING_PARAMS) {
        Common::HandleParamErr(env, ERR_PARAM_NUMBER_ERR, isThrow);
        asyncCallbackInfo->errCode = ERR_PARAM_NUMBER_ERR;
        return false;
    }
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
    return true;
}

napi_value UpdateBackgroundRunning(napi_env env, napi_callback_info info, bool isThrow)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Napi::UpdateBackgroundRunning");
    ReportXPowerJsStackSysEventByType(env, "CONTINUOUS_TASK_UPDATE");
    AsyncCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("asyncCallbackInfo == nullpter");
        return WrapVoidToJS(env);
    }
    std::unique_ptr<AsyncCallbackInfo> callbackPtr {asyncCallbackInfo};

    size_t argc = MAX_UPDATE_BG_RUNNING_PARAMS;
    napi_value argv[MAX_UPDATE_BG_RUNNING_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != MAX_UPDATE_BG_RUNNING_PARAMS) {
        BGTASK_LOGE("wrong param nums");
        Common::HandleParamErr(env, ERR_PARAM_NUMBER_ERR, isThrow);
        return WrapVoidToJS(env);
    }
    bool isArray = false;
    napi_value value = argv[1];
    napi_is_array(env, value, &isArray);
    napi_value ret {nullptr};
    if (isArray) {
        if (UpdateBackgroundRunningCheckParam(env, argv, argc, asyncCallbackInfo, isThrow)) {
            ret = UpdateBackgroundRunningPromise(env, asyncCallbackInfo, isThrow);
        }
    } else {
        if (StartBackgroundRunningCheckRequest(env, argv, argc, isThrow, asyncCallbackInfo)) {
            ret = UpdateBackgroundRunningByRequestPromise(env, asyncCallbackInfo, isThrow);
        }
    }
    callbackPtr.release();
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

void StartBackgroundRunningByRequestExecuteCB(napi_env env, void *data)
{
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr || asyncCallbackInfo->errCode != ERR_OK ||
        asyncCallbackInfo->request == nullptr) {
        BGTASK_LOGE("input params error");
        return;
    }
    Common::TaskModeTypeConversion(asyncCallbackInfo->request);
    std::vector<uint32_t> continuousTaskModes = asyncCallbackInfo->request->GetContinuousTaskModes();
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    ContinuousTaskParam taskParam = ContinuousTaskParam(true, continuousTaskModes[0],
        asyncCallbackInfo->request->GetWantAgent(), info->name, asyncCallbackInfo->abilityContext->GetToken(),
        "", true, continuousTaskModes, asyncCallbackInfo->abilityContext->GetAbilityRecordId());
    taskParam.isByRequestObject_ = true;
    taskParam.isCombinedTaskNotification_ = asyncCallbackInfo->request->IsCombinedTaskNotification();
    taskParam.combinedNotificationTaskId_ = asyncCallbackInfo->request->GetContinuousTaskId();
    taskParam.bgSubModeIds_ = asyncCallbackInfo->request->GetContinuousTaskSubmodes();
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestStartBackgroundRunning(taskParam);
    asyncCallbackInfo->bgModes = continuousTaskModes;
    asyncCallbackInfo->notificationId = taskParam.notificationId_;
    asyncCallbackInfo->continuousTaskId = taskParam.continuousTaskId_;
}

napi_value StartBackgroundRunningByRequestPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo, bool isThrow)
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
    NAPI_CALL(env, napi_create_async_work(env,
        nullptr,
        resourceName,
        StartBackgroundRunningByRequestExecuteCB,
        PromiseCompletedCB,
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    return promise;
}

napi_value StartBackgroundRunningSubmit(napi_env env, napi_value *argv, uint32_t argc,
    AsyncCallbackInfo *asyncCallbackInfo, bool isThrow)
{
    if (!StartBackgroundRunningCheckRequest(env, argv, argc, isThrow, asyncCallbackInfo)) {
        return nullptr;
    } else {
        return StartBackgroundRunningByRequestPromise(env, asyncCallbackInfo, isThrow);
    }
}

napi_value StartBackgroundRunning(napi_env env, napi_callback_info info, bool isThrow)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Napi::StartBackgroundRunning");
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
    napi_value ret {nullptr};
    if (MAX_START_BG_RUNNING_BY_RQUEST_PARAMS == argc) {
        ret = StartBackgroundRunningSubmit(env, argv, argc, asyncCallbackInfo, isThrow);
    } else {
        if (!StartBackgroundRunningCheckParamBeforeSubmit(env, argv, MAX_START_BG_RUNNING_PARAMS, isThrow,
            asyncCallbackInfo)) {
            BGTASK_LOGE("failed to check parameters before start bgtask running.");
            return WrapVoidToJS(env);
        }
        if (argc == MAX_START_BG_RUNNING_PARAMS) {
            ret = StartBackgroundRunningAsync(env, argv, MAX_START_BG_RUNNING_PARAMS - 1, asyncCallbackInfo, isThrow);
        } else {
            ret = StartBackgroundRunningPromise(env, asyncCallbackInfo, isThrow);
        }
    }
    callbackPtr.release();
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
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Napi::StopBackgroundRunningExecuteCB");
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr || asyncCallbackInfo->errCode != ERR_OK) {
        BGTASK_LOGE("input param error");
        return;
    }
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    sptr<IRemoteObject> token = asyncCallbackInfo->abilityContext->GetToken();
    int32_t abilityId = asyncCallbackInfo->abilityContext->GetAbilityRecordId();
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestStopBackgroundRunning(info->name, token, abilityId);
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
    if (valuetype != napi_function) {
        Common::HandleParamErr(env, ERR_CALLBACK_NULL_OR_TYPE_ERR, isThrow);
        return nullptr;
    }
    NAPI_CALL(env, napi_create_reference(env, argv[argCallback], 1, &asyncCallbackInfo->callback));
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

void StopBackgroundRunningByTaskIdExecuteCB(napi_env env, void *data)
{
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr || asyncCallbackInfo->errCode != ERR_OK) {
        BGTASK_LOGE("input param error");
        return;
    }
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    sptr<IRemoteObject> token = asyncCallbackInfo->abilityContext->GetToken();
    int32_t abilityId = asyncCallbackInfo->abilityContext->GetAbilityRecordId();
    int32_t continuousTaskId = asyncCallbackInfo->removeTaskId;
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestStopBackgroundRunning(info->name, token, abilityId,
        continuousTaskId);
}

napi_value StopBackgroundRunningByTaskIdPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo, bool isThrow)
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
        StopBackgroundRunningByTaskIdExecuteCB,
        PromiseCompletedCB,
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return promise;
}

napi_value StopBackgroundRunningBySubmit(napi_env env, napi_value *argv, uint32_t argc,
    AsyncCallbackInfo *asyncCallbackInfo, bool isThrow)
{
    if (argc > MAX_STOP_BG_RUNNING_PARAMS) {
        Common::HandleParamErr(env, ERR_PARAM_NUMBER_ERR, isThrow);
        asyncCallbackInfo->errCode = ERR_PARAM_NUMBER_ERR;
        return nullptr;
    }
    napi_valuetype valueType = napi_undefined;
    napi_value value = argv[1];
    napi_typeof(env, value, &valueType);
    if (valueType == napi_number) {
        int32_t taskIdValue = -1;
        napi_get_value_int32(env, value, &taskIdValue);
        if (taskIdValue < 0) {
            Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_TASKID_INVALID, true);
            asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_TASKID_INVALID;
            return nullptr;
        } else {
            asyncCallbackInfo->removeTaskId = taskIdValue;
            return StopBackgroundRunningByTaskIdPromise(env, asyncCallbackInfo, isThrow);
        }
    } else {
        return StopBackgroundRunningAsync(env, argv, MAX_STOP_BG_RUNNING_PARAMS - 1, asyncCallbackInfo, isThrow);
    }
}

napi_value StopBackgroundRunning(napi_env env, napi_callback_info info, bool isThrow)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Napi::StopBackgroundRunning");
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
        Common::HandleParamErr(env, ERR_PARAM_NUMBER_ERR, isThrow);
        callbackPtr.release();
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        return nullptr;
    }

    // argv[0] : context : AbilityContext
    if (GetAbilityContext(env, argv[0], asyncCallbackInfo->abilityContext) == nullptr) {
        BGTASK_LOGE("Get ability context failed");
        Common::HandleParamErr(env, ERR_CONTEXT_NULL_OR_TYPE_ERR, isThrow);
        asyncCallbackInfo->errCode = ERR_CONTEXT_NULL_OR_TYPE_ERR;
        callbackPtr.release();
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        return nullptr;
    }

    napi_value ret {nullptr};
    if (argc == MAX_STOP_BG_RUNNING_PARAMS) {
        ret = StopBackgroundRunningBySubmit(env, argv, argc, asyncCallbackInfo, isThrow);
    } else {
        ret = StopBackgroundRunningPromise(env, asyncCallbackInfo, isThrow);
    }
    callbackPtr.release();
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

bool CheckOnParam(napi_env env, uint32_t argc, napi_value argv[], std::string& typeString)
{
    if (argc < ARGC_TWO) {
        BGTASK_LOGE("wrong param nums");
        return false;
    }
    // argv[0] : type
    if (!AbilityRuntime::ConvertFromJsValue(env, argv[INDEX_ZERO], typeString)) {
        BGTASK_LOGE("type must be string");
        return false;
    }
    auto it = std::find(g_callbackTypes.begin(), g_callbackTypes.end(), typeString);
    if (it == g_callbackTypes.end()) {
        BGTASK_LOGE("continuousTask type error: %{public}s", typeString.c_str());
        return false;
    }
    // arg[1] : callback
    if (!CheckTypeForNapiValue(env, argv[INDEX_ONE], napi_function)) {
        return false;
    }
    return true;
}

bool CheckOffParam(napi_env env, uint32_t argc, napi_value argv[], std::string& typeString)
{
    if (argc < ARGC_ONE) {
        BGTASK_LOGE("wrong param nums < 1");
        return false;
    }
 
    // argv[0] : type
    if (!AbilityRuntime::ConvertFromJsValue(env, argv[INDEX_ZERO], typeString)) {
        BGTASK_LOGE("type must be string");
        return false;
    }
    auto it = std::find(g_callbackTypes.begin(), g_callbackTypes.end(), typeString);
    if (it == g_callbackTypes.end()) {
        BGTASK_LOGE("continuousTask type error: %{public}s", typeString.c_str());
        return false;
    }
    // arg[1] : callback
    if (argc == ARGC_TWO) {
        if (!CheckTypeForNapiValue(env, argv[INDEX_ONE], napi_function)) {
            return false;
        }
    }
    return true;
}

bool SubscribeBackgroundTask(napi_env env, uint32_t flag = 0)
{
    if (backgroundTaskSubscriber_ == nullptr) {
        backgroundTaskSubscriber_ = std::make_shared<JsBackgroundTaskSubscriber>(env);
        if (backgroundTaskSubscriber_ == nullptr) {
            BGTASK_LOGE("ret is nullptr");
            Common::HandleErrCode(env, ERR_BGTASK_SERVICE_INNER_ERROR, true);
            return false;
        }
    }
    backgroundTaskSubscriber_->SetFlag(flag, true);
    ErrCode errCode = BackgroundTaskMgrHelper::SubscribeBackgroundTask(*backgroundTaskSubscriber_);
    if (errCode != ERR_OK) {
        BGTASK_LOGE("SubscribeBackgroundTask failed.");
        Common::HandleErrCode(env, errCode, true);
        return false;
    }
    return true;
}

void UnSubscribeBackgroundTask(napi_env env, uint32_t flag = 0)
{
    if (!backgroundTaskSubscriber_->IsEmpty()) {
        return;
    }
    backgroundTaskSubscriber_->SetFlag(flag, false);
    ErrCode errCode = BackgroundTaskMgrHelper::UnsubscribeBackgroundTask(*backgroundTaskSubscriber_);
    if (errCode != ERR_OK) {
        BGTASK_LOGE("UnsubscribeBackgroundTask failed.");
        Common::HandleErrCode(env, errCode, true);
        return;
    }
    backgroundTaskSubscriber_->UnSubscriberBgtaskSaStatusChange();
    backgroundTaskSubscriber_ = nullptr;
}

napi_value OnOnContinuousTaskCallback(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    std::string typeString;
    if (!CheckOnParam(env, argc, argv, typeString)) {
        Common::HandleParamErr(env, ERR_BGTASK_INVALID_PARAM, true);
        return WrapUndefinedToJS(env);
    }
    uint32_t type = 0;
    if (typeString == "continuousTaskCancel") {
        type = CONTINUOUS_TASK_CANCEL;
    } else if (typeString == "continuousTaskSuspend") {
        type = CONTINUOUS_TASK_SUSPEND;
    } else if (typeString == "continuousTaskActive") {
        type = CONTINUOUS_TASK_ACTIVE;
    }
    BGTASK_LOGD("SubscribeBackgroundTask type: %{public}s, type: %{public}d", typeString.c_str(), type);
    if (!SubscribeBackgroundTask(env, type)) {
        return WrapUndefinedToJS(env);
    }
    backgroundTaskSubscriber_->AddJsObserverObject(typeString, argv[INDEX_ONE]);
    backgroundTaskSubscriber_->SubscriberBgtaskSaStatusChange();
    return WrapUndefinedToJS(env);
}

napi_value OffOnContinuousTaskCallback(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    std::string typeString;
    if (!CheckOffParam(env, argc, argv, typeString)) {
        Common::HandleParamErr(env, ERR_BGTASK_INVALID_PARAM, true);
        return WrapUndefinedToJS(env);
    }
    if (!backgroundTaskSubscriber_) {
        BGTASK_LOGE("backgroundTaskSubscriber_ is null, return");
        return WrapUndefinedToJS(env);
    }
    if (argc == ARGC_ONE) {
        backgroundTaskSubscriber_->RemoveJsObserverObjects(typeString);
    } else if (argc == ARGC_TWO) {
        backgroundTaskSubscriber_->RemoveJsObserverObject(typeString, argv[INDEX_ONE]);
    }
 
    uint32_t type = 0;
    if (backgroundTaskSubscriber_-> IsTypeEmpty(typeString)) {
        if (typeString == "continuousTaskCancel") {
            type = CONTINUOUS_TASK_CANCEL;
        } else if (typeString == "continuousTaskSuspend") {
            type = CONTINUOUS_TASK_SUSPEND;
        } else if (typeString == "continuousTaskActive") {
            type = CONTINUOUS_TASK_ACTIVE;
        }
    }
    UnSubscribeBackgroundTask(env, type);
    return WrapUndefinedToJS(env);
}

void GetAllContinuousTasksExecuteCB(napi_env env, void *data)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Napi::GetAllContinuousTasksExecuteCB");
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr || asyncCallbackInfo->errCode != ERR_OK) {
        BGTASK_LOGE("input params error");
        return;
    }
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestGetAllContinuousTasks(asyncCallbackInfo->list);
}

void GetAllContinuousTasksIncludeSuspendedExecuteCB(napi_env env, void *data)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Napi::GetAllContinuousTasksIncludeSuspendedExecuteCB");
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr || asyncCallbackInfo->errCode != ERR_OK) {
        BGTASK_LOGE("input params error");
        return;
    }
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestGetAllContinuousTasks(
        asyncCallbackInfo->list, asyncCallbackInfo->includeSuspended);
}

void GetAllContinuousTasksPromiseCompletedCB(napi_env env, napi_status status, void *data)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Napi::GetAllContinuousTasksPromiseCompletedCB");
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    std::unique_ptr<AsyncCallbackInfo> callbackPtr {asyncCallbackInfo};
    napi_value result {nullptr};
    if (asyncCallbackInfo->errCode == ERR_OK) {
        if (asyncCallbackInfo->list.size() > 0) {
            NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &result));
            uint32_t count = 0;
            for (const auto &continuoustTaskInfo : asyncCallbackInfo->list) {
                napi_value napiWork = Common::GetNapiContinuousTaskInfo(env, continuoustTaskInfo);
                NAPI_CALL_RETURN_VOID(env, napi_set_element(env, result, count, napiWork));
                count++;
            }
        } else {
            NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &result));
        }
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, asyncCallbackInfo->deferred, result));
    } else {
        std::string errMsg = Common::FindErrMsg(env, asyncCallbackInfo->errCode);
        int32_t errCodeInfo = Common::FindErrCode(env, asyncCallbackInfo->errCode);
        result = Common::GetCallbackErrorValue(env, errCodeInfo, errMsg);
        NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, asyncCallbackInfo->deferred, result));
    }
    callbackPtr.release();
}

napi_value GetAllContinuousTasksPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo, bool isThrow)
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
    NAPI_CALL(env, napi_create_async_work(env,
        nullptr,
        resourceName,
        GetAllContinuousTasksExecuteCB,
        GetAllContinuousTasksPromiseCompletedCB,
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    return promise;
}

napi_value GetAllContinuousTasksIncludeSuspendedPromise(
    napi_env env, AsyncCallbackInfo *asyncCallbackInfo, bool isThrow)
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
    NAPI_CALL(env, napi_create_async_work(env,
        nullptr,
        resourceName,
        GetAllContinuousTasksIncludeSuspendedExecuteCB,
        GetAllContinuousTasksPromiseCompletedCB,
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    return promise;
}

napi_value GetAllContinuousTasks(napi_env env, napi_callback_info info, bool isThrow)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Napi::GetAllContinuousTasks");
    if (env == nullptr) {
        BGTASK_LOGE("env param invaild.");
        return WrapVoidToJS(env);
    }
    ReportXPowerJsStackSysEventByType(env, "GET_ALL_CONTINUOUS_TASK");
    AsyncCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("asyncCallbackInfo == nullpter");
        return WrapVoidToJS(env);
    }
    std::unique_ptr<AsyncCallbackInfo> callbackPtr {asyncCallbackInfo};

    size_t argc = MAX_GET_ALL_CONTINUOUSTASK_PARAMS;
    napi_value argv[MAX_GET_ALL_CONTINUOUSTASK_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));

    if (!GetAllContinuousTasksCheckParamBeforeSubmit(env, argc, argv, isThrow, asyncCallbackInfo)) {
        callbackPtr.release();
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        return WrapVoidToJS(env);
    }

    napi_value ret {nullptr};
    if (argc == ARGC_ONE) {
        ret = GetAllContinuousTasksPromise(env, asyncCallbackInfo, isThrow);
    } else {
        ret = GetAllContinuousTasksIncludeSuspendedPromise(env, asyncCallbackInfo, isThrow);
    }
    callbackPtr.release();
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

bool IsModeSupportedCheckParamBeforeSubmit(napi_env env, size_t argc, napi_value *argv, bool isThrow,
    AsyncCallbackInfo *asyncCallbackInfo)
{
    // Get continuousTaskModes.
    if (!Common::GetContinuousTaskModesFromArray(env, argv[0], asyncCallbackInfo->request)) {
        Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_REQUEST_NULL_OR_TYPE, true);
        asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_REQUEST_NULL_OR_TYPE;
        return false;
    }

    // Get continuousTaskSubmodes.
    if (!Common::GetContinuousTaskSubmodesFromArray(env, argv[1], asyncCallbackInfo->request)) {
        Common::HandleErrCode(env, ERR_BGTASK_CONTINUOUS_REQUEST_NULL_OR_TYPE, true);
        asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_REQUEST_NULL_OR_TYPE;
        return false;
    }
    return true;
}

bool IsModeSupportedExecuteCB(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    Common::TaskModeTypeConversion(asyncCallbackInfo->request);
    std::vector<uint32_t> continuousTaskModes = asyncCallbackInfo->request->GetContinuousTaskModes();

    ContinuousTaskParam taskParam = ContinuousTaskParam(true, continuousTaskModes[0],
        nullptr, "", nullptr, "", true, continuousTaskModes, -1);
    taskParam.bgSubModeIds_ = asyncCallbackInfo->request->GetContinuousTaskSubmodes();
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::IsModeSupported(taskParam);
    if (asyncCallbackInfo->errCode != ERR_OK) {
        BGTASK_LOGE("get isModeSupported failed");
        Common::HandleErrCode(env, asyncCallbackInfo->errCode, true);
        return false;
    }
    return true;
}

napi_value IsModeSupported(napi_env env, napi_callback_info info)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Napi::IsModeSupported");
    if (env == nullptr) {
        BGTASK_LOGE("env param invaild.");
        return WrapVoidToJS(env);
    }
    AsyncCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("input params error");
        return WrapVoidToJS(env);
    }
    std::unique_ptr<AsyncCallbackInfo> callbackPtr {asyncCallbackInfo};

    size_t argc = MAX_START_BG_RUNNING_BY_RQUEST_PARAMS;
    napi_value argv[MAX_START_BG_RUNNING_BY_RQUEST_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != MAX_START_BG_RUNNING_BY_RQUEST_PARAMS) {
        BGTASK_LOGE("wrong params nums");
        return WrapVoidToJS(env);
    }

    if (!IsModeSupportedCheckParamBeforeSubmit(env, argc, argv, true, asyncCallbackInfo)) {
        BGTASK_LOGE("failed to check params");
        return WrapVoidToJS(env);
    }

    napi_value ret {nullptr};
    bool isModeSupported = IsModeSupportedExecuteCB(env, asyncCallbackInfo);
    napi_get_boolean(env, isModeSupported, &ret);
    callbackPtr.release();
    if (asyncCallbackInfo != nullptr) {
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
    }
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

napi_value GetAllContinuousTasksThrow(napi_env env, napi_callback_info info)
{
    return GetAllContinuousTasks(env, info, true);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS