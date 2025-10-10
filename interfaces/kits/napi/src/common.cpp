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

#include "common.h"

#include "background_mode.h"
#include "background_sub_mode.h"
#include "cancel_suspend_delay.h"
#include "background_task_mode.h"
#include "background_task_submode.h"
#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
const uint32_t STR_MAX_SIZE = 64;
const uint32_t EXPIRE_CALLBACK_PARAM_NUM = 1;
const uint32_t ASYNC_CALLBACK_PARAM_NUM = 2;
const int32_t INVALID_CONTINUOUSTASK_ID = -2;
const int32_t INVALID_MODE_ID = -1;

AsyncWorkData::AsyncWorkData(napi_env napiEnv)
{
    env = napiEnv;
}

AsyncWorkData::~AsyncWorkData()
{
    if (callback) {
        BGTASK_LOGD("AsyncWorkData::~AsyncWorkData delete callback");
        napi_delete_reference(env, callback);
        callback = nullptr;
    }
    if (asyncWork) {
        BGTASK_LOGD("AsyncWorkData::~AsyncWorkData delete asyncWork");
        napi_delete_async_work(env, asyncWork);
        asyncWork = nullptr;
    }
}

napi_value Common::NapiGetboolean(const napi_env &env, const bool isValue)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, isValue, &result));
    return result;
}

napi_value Common::NapiGetNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

napi_value Common::GetCallbackErrorValue(napi_env env, const int32_t errCode, const std::string errMsg)
{
    if (errCode == ERR_OK) {
        return NapiGetNull(env);
    }
    napi_value error = nullptr;
    napi_value eCode = nullptr;
    napi_value eMsg = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_string_utf8(env, errMsg.c_str(),
        errMsg.length(), &eMsg));
    NAPI_CALL(env, napi_create_object(env, &error));
    NAPI_CALL(env, napi_set_named_property(env, error, "code", eCode));
    NAPI_CALL(env, napi_set_named_property(env, error, "message", eMsg));
    return error;
}

napi_value Common::GetExpireCallbackValue(napi_env env, int32_t errCode, const napi_value &value)
{
    napi_value result = nullptr;
    napi_value eCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_set_named_property(env, result, "code", eCode));
    NAPI_CALL(env, napi_set_named_property(env, result, "data", value));
    return result;
}

void Common::SetCallback(const napi_env &env, const napi_ref &callbackIn, const napi_value &result)
{
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_value res = nullptr;
    res = GetExpireCallbackValue(env, 0, result);
    napi_get_reference_value(env, callbackIn, &callback);
    NAPI_CALL_RETURN_VOID(env,
        napi_call_function(env, undefined, callback, EXPIRE_CALLBACK_PARAM_NUM, &res, &resultout));
}

void Common::SetCallback(
    const napi_env &env, const napi_ref &callbackIn, const int32_t &errCode, const napi_value &result)
{
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_get_reference_value(env, callbackIn, &callback);
    napi_value results[ASYNC_CALLBACK_PARAM_NUM] = {nullptr};
    if (errCode == ERR_OK) {
        results[0] = NapiGetNull(env);
    } else {
        int32_t errCodeInfo = FindErrCode(env, errCode);
        std::string errMsg = FindErrMsg(env, errCode);
        results[0] = GetCallbackErrorValue(env, errCodeInfo, errMsg);
    }
    results[1] = result;
    NAPI_CALL_RETURN_VOID(env,
        napi_call_function(env, undefined, callback, ASYNC_CALLBACK_PARAM_NUM, &results[0], &resultout));
}

napi_value Common::SetPromise(
    const napi_env &env, const AsyncWorkData &info, const napi_value &result)
{
    if (info.errCode == ERR_OK) {
        napi_resolve_deferred(env, info.deferred, result);
    } else {
        int32_t errCodeInfo = FindErrCode(env, info.errCode);
        std::string errMsg = FindErrMsg(env, info.errCode);
        napi_value error = nullptr;
        napi_value eCode = nullptr;
        napi_value eMsg = nullptr;
        NAPI_CALL(env, napi_create_int32(env, errCodeInfo, &eCode));
        NAPI_CALL(env, napi_create_string_utf8(env, errMsg.c_str(),
            errMsg.length(), &eMsg));
        NAPI_CALL(env, napi_create_object(env, &error));
        NAPI_CALL(env, napi_set_named_property(env, error, "data", eCode));
        NAPI_CALL(env, napi_set_named_property(env, error, "code", eCode));
        NAPI_CALL(env, napi_set_named_property(env, error, "message", eMsg));
        napi_reject_deferred(env, info.deferred, error);
    }
    return result;
}

void Common::ReturnCallbackPromise(const napi_env &env, const AsyncWorkData &info, const napi_value &result)
{
    if (info.isCallback) {
        SetCallback(env, info.callback, info.errCode, result);
    } else {
        SetPromise(env, info, result);
    }
}

napi_value Common::JSParaError(const napi_env &env, const napi_ref &callback)
{
    if (callback) {
        SetCallback(env, callback, ERR_BGTASK_INVALID_PARAM, nullptr);
        return Common::NapiGetNull(env);
    } else {
        napi_value promise = nullptr;
        napi_deferred deferred = nullptr;
        napi_create_promise(env, &deferred, &promise);

        napi_value res = nullptr;
        napi_value eCode = nullptr;
        napi_value eMsg = nullptr;
        std::string errMsg = FindErrMsg(env, ERR_BGTASK_INVALID_PARAM);
        NAPI_CALL(env, napi_create_int32(env, ERR_BGTASK_INVALID_PARAM, &eCode));
        NAPI_CALL(env, napi_create_string_utf8(env, errMsg.c_str(),
            errMsg.length(), &eMsg));
        NAPI_CALL(env, napi_create_object(env, &res));
        NAPI_CALL(env, napi_set_named_property(env, res, "data", eCode));
        NAPI_CALL(env, napi_set_named_property(env, res, "code", eCode));
        NAPI_CALL(env, napi_set_named_property(env, res, "message", eMsg));
        napi_reject_deferred(env, deferred, res);
        return promise;
    }
}

napi_value Common::GetU16StringValue(const napi_env &env, const napi_value &value, std::u16string &result)
{
    napi_valuetype valuetype = napi_undefined;

    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    if (valuetype == napi_string) {
        char str[STR_MAX_SIZE] = {0};
        size_t strLen = 0;
        NAPI_CALL(env, napi_get_value_string_utf8(env, value, str, STR_MAX_SIZE - 1, &strLen));

        result = Str8ToStr16((std::string)str);
        BGTASK_LOGD("GetU16StringValue result: %{public}s", Str16ToStr8(result).c_str());
    } else {
        BGTASK_LOGE("valuetype is not stringU16");
        return nullptr;
    }

    return Common::NapiGetNull(env);
}

napi_value Common::GetInt32NumberValue(const napi_env &env, const napi_value &value, int32_t &result)
{
    napi_valuetype valuetype = napi_undefined;
    BGTASK_NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    if (valuetype != napi_number) {
        BGTASK_LOGE("valuetype is not number");
        return nullptr;
    }
    BGTASK_NAPI_CALL(env, napi_get_value_int32(env, value, &result));
    BGTASK_LOGD("GetInt32NumberValue result: %{public}d", result);
    return Common::NapiGetNull(env);
}

void Common::PaddingAsyncWorkData(
    const napi_env &env, const napi_ref &callback, AsyncWorkData &info, napi_value &promise)
{
    if (callback) {
        info.callback = callback;
        info.isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        info.deferred = deferred;
        info.isCallback = false;
    }
}

napi_value Common::SetDelaySuspendInfo(
    const napi_env &env, std::shared_ptr<DelaySuspendInfo>& delaySuspendInfo, napi_value &result)
{
    if (delaySuspendInfo == nullptr) {
        BGTASK_LOGI("delaySuspendInfo is nullptr");
        return NapiGetboolean(env, false);
    }
    napi_value value = nullptr;

    // readonly requestId?: number
    napi_create_int32(env, delaySuspendInfo->GetRequestId(), &value);
    napi_set_named_property(env, result, "requestId", value);

    // readonly actualDelayTime?: number
    napi_create_int32(env, delaySuspendInfo->GetActualDelayTime(), &value);
    napi_set_named_property(env, result, "actualDelayTime", value);

    return NapiGetboolean(env, true);
}

napi_value Common::GetStringValue(const napi_env &env, const napi_value &value, std::string &result)
{
    napi_valuetype valuetype = napi_undefined;
    BGTASK_NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    if (valuetype != napi_string) {
        BGTASK_LOGE("valuetype is not string");
        return nullptr;
    }

    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    napi_status status = napi_get_value_string_utf8(env, value, str, STR_MAX_SIZE - 1, &strLen);
    if (status != napi_ok) {
        BGTASK_LOGE("get value string utf8 failed");
        return nullptr;
    }
    result = std::string(str);
    BGTASK_LOGD("GetStringValue result: %{public}s", result.c_str());
    return Common::NapiGetNull(env);
}

void Common::HandleErrCode(const napi_env &env, int32_t errCode, bool isThrow)
{
    BGTASK_LOGD("HandleErrCode errCode = %{public}d, isThrow = %{public}d", errCode, isThrow);
    if (!isThrow || errCode == ERR_OK) {
        return;
    }
    std::string errMsg = FindErrMsg(env, errCode);
    int32_t errCodeInfo = FindErrCode(env, errCode);
    if (errMsg != "") {
        napi_throw_error(env, std::to_string(errCodeInfo).c_str(), errMsg.c_str());
    }
}

bool Common::HandleParamErr(const napi_env &env, int32_t errCode, bool isThrow)
{
    BGTASK_LOGD("HandleParamErr errCode = %{public}d, isThrow = %{public}d", errCode, isThrow);
    if (!isThrow || errCode == ERR_OK) {
        return false;
    }
    auto iter = PARAM_ERRCODE_MSG_MAP.find(errCode);
    if (iter != PARAM_ERRCODE_MSG_MAP.end()) {
        std::string errMessage = "BussinessError 401: Parameter error. ";
        errMessage.append(iter->second);
        napi_throw_error(env, std::to_string(ERR_BGTASK_INVALID_PARAM).c_str(), errMessage.c_str());
        return true;
    }
    return false;
}

std::string Common::FindErrMsg(const napi_env &env, const int32_t errCode)
{
    if (errCode == ERR_OK) {
        return "";
    }
    auto iter = SA_ERRCODE_MSG_MAP.find(errCode);
    if (iter != SA_ERRCODE_MSG_MAP.end()) {
        std::string errMessage = "BussinessError ";
        int32_t errCodeInfo = FindErrCode(env, errCode);
        errMessage.append(std::to_string(errCodeInfo)).append(": ").append(iter->second);
        return errMessage;
    }
    iter = PARAM_ERRCODE_MSG_MAP.find(errCode);
    if (iter != PARAM_ERRCODE_MSG_MAP.end()) {
        std::string errMessage = "BussinessError 401: Parameter error. ";
        errMessage.append(iter->second);
        return errMessage;
    }
    return "Inner error.";
}

int32_t Common::FindErrCode(const napi_env &env, const int32_t errCodeIn)
{
    auto iter = PARAM_ERRCODE_MSG_MAP.find(errCodeIn);
    if (iter != PARAM_ERRCODE_MSG_MAP.end()) {
        return ERR_BGTASK_INVALID_PARAM;
    }
    return errCodeIn > THRESHOLD ? errCodeIn / OFFSET : errCodeIn;
}

napi_value Common::GetBooleanValue(const napi_env &env, const napi_value &value, bool &result)
{
    napi_valuetype valuetype = napi_undefined;
    BGTASK_NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    if (valuetype != napi_boolean) {
        BGTASK_LOGE("valuetype is not boolean");
        return nullptr;
    }
    BGTASK_NAPI_CALL(env, napi_get_value_bool(env, value, &result));
    BGTASK_LOGD("GetBooleanValue result: %{public}d", result);

    return Common::NapiGetNull(env);
}

napi_value Common::NapiSetBgTaskMode(napi_env env, napi_value napiInfo,
    const std::shared_ptr<ContinuousTaskInfo> &continuousTaskInfo)
{
    // backgroundmode
    napi_value napiBackgroundModes = nullptr;
    NAPI_CALL(env, napi_create_array(env, &napiBackgroundModes));
    uint32_t count = 0;
    for (auto mode : continuousTaskInfo->GetBackgroundModes()) {
        if (mode < BackgroundMode::END) {
            napi_value napiModeText = nullptr;
            std::string modeStr = BackgroundMode::GetBackgroundModeStr(mode);
            NAPI_CALL(env, napi_create_string_utf8(env, modeStr.c_str(), modeStr.length(), &napiModeText));
            NAPI_CALL(env, napi_set_element(env, napiBackgroundModes, count, napiModeText));
            count++;
        }
    }
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "backgroundModes", napiBackgroundModes));

    // backgroundsubmode
    napi_value napiBackgroundSubModes = nullptr;
    NAPI_CALL(env, napi_create_array(env, &napiBackgroundSubModes));
    count = 0;
    for (auto subMode : continuousTaskInfo->GetBackgroundSubModes()) {
        if (subMode < BackgroundSubMode::END) {
            napi_value napiSubModeText = nullptr;
            std::string subModeStr = BackgroundSubMode::GetBackgroundSubModeStr(subMode);
            NAPI_CALL(env, napi_create_string_utf8(env, subModeStr.c_str(), subModeStr.length(),
                &napiSubModeText));
                NAPI_CALL(env, napi_set_element(env, napiBackgroundSubModes, count, napiSubModeText));
            count++;
        }
    }
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "backgroundSubModes", napiBackgroundSubModes));
    return napiInfo;
}

napi_value Common::GetNapiContinuousTaskInfo(napi_env env,
    const std::shared_ptr<ContinuousTaskInfo> &continuousTaskInfo)
{
    if (continuousTaskInfo == nullptr) {
        BGTASK_LOGE("continuousTaskInfo is null");
        return NapiGetNull(env);
    }
    napi_value napiInfo = nullptr;
    NAPI_CALL(env, napi_create_object(env, &napiInfo));

    // ability name
    napi_value napiAbilityName = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, continuousTaskInfo->GetAbilityName().c_str(),
        continuousTaskInfo->GetAbilityName().length(), &napiAbilityName));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "abilityName", napiAbilityName));

    // uid
    napi_value napiUid = nullptr;
    NAPI_CALL(env, napi_create_int32(env, continuousTaskInfo->GetUid(), &napiUid));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "uid", napiUid));

    // pid
    napi_value napiPid = nullptr;
    NAPI_CALL(env, napi_create_int32(env, continuousTaskInfo->GetPid(), &napiPid));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "pid", napiPid));

    // Set isFromWebView.
    napi_value napiIsFromWebView = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, continuousTaskInfo->IsFromWebView(), &napiIsFromWebView));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "isFromWebView", napiIsFromWebView));

    if (NapiSetBgTaskMode(env, napiInfo, continuousTaskInfo) == nullptr) {
        BGTASK_LOGE("set bavktask mode fail.");
        return NapiGetNull(env);
    }

    // notificationId
    napi_value napiNotificationId = nullptr;
    NAPI_CALL(env, napi_create_int32(env, continuousTaskInfo->GetNotificationId(), &napiNotificationId));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "notificationId", napiNotificationId));

    // continuousTaskId
    napi_value napiContinuousTaskId = nullptr;
    NAPI_CALL(env, napi_create_int32(env, continuousTaskInfo->GetContinuousTaskId(), &napiContinuousTaskId));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "continuousTaskId", napiContinuousTaskId));

    // abilityId
    napi_value napiAbilityId = nullptr;
    NAPI_CALL(env, napi_create_int32(env, continuousTaskInfo->GetAbilityId(), &napiAbilityId));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "abilityId", napiAbilityId));

    // want agent bundle name
    napi_value napiWantAgentBundleName = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, continuousTaskInfo->GetWantAgentBundleName().c_str(),
        continuousTaskInfo->GetWantAgentBundleName().length(), &napiWantAgentBundleName));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "wantAgentBundleName", napiWantAgentBundleName));

    // want agent ability name
    napi_value napiWantAgentAbilityName = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, continuousTaskInfo->GetWantAgentAbilityName().c_str(),
        continuousTaskInfo->GetWantAgentAbilityName().length(), &napiWantAgentAbilityName));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "wantAgentAbilityName", napiWantAgentAbilityName));

    // suspend state.
    napi_value napiSuspendState = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, continuousTaskInfo->GetSuspendState(), &napiSuspendState));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "suspendState", napiSuspendState));
    return napiInfo;
}

napi_value Common::GetNapiDelaySuspendInfo(napi_env env,
    const std::shared_ptr<DelaySuspendInfo> &delaySuspendInfo)
{
    if (delaySuspendInfo == nullptr) {
        BGTASK_LOGE("delaySuspendInfo is null");
        return NapiGetNull(env);
    }
    napi_value napiInfo = nullptr;
    NAPI_CALL(env, napi_create_object(env, &napiInfo));

    // requestId
    napi_value napiRequestId = nullptr;
    NAPI_CALL(env, napi_create_int32(env, delaySuspendInfo->GetRequestId(), &napiRequestId));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "requestId", napiRequestId));

    // actualDelayTime
    napi_value napiActualDelayTime = nullptr;
    NAPI_CALL(env, napi_create_int32(env, delaySuspendInfo->GetActualDelayTime(), &napiActualDelayTime));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "actualDelayTime", napiActualDelayTime));
    return napiInfo;
}

napi_value Common::GetNapiEfficiencyResourcesInfo(const napi_env &env,
    std::shared_ptr<EfficiencyResourceInfo> efficiencyResourceInfo)
{
    if (efficiencyResourceInfo == nullptr) {
        BGTASK_LOGE("efficiencyResourceInfo is null");
        return NapiGetNull(env);
    }
    napi_value napiInfo = nullptr;
    NAPI_CALL(env, napi_create_object(env, &napiInfo));

    napi_value napiResourceType = nullptr;
    napi_value napiTimeOut = nullptr;
    napi_value napiIsPersist = nullptr;
    napi_value napiIsProcess = nullptr;
    napi_value napiReason = nullptr;
    napi_value napiUid = nullptr;
    napi_value napiPid = nullptr;

    NAPI_CALL(env, napi_create_int32(env, efficiencyResourceInfo->GetResourceNumber(), &napiResourceType));
    NAPI_CALL(env, napi_create_int32(env, efficiencyResourceInfo->GetTimeOut(), &napiTimeOut));
    NAPI_CALL(env, napi_get_boolean(env, efficiencyResourceInfo->IsPersist(), &napiIsPersist));
    NAPI_CALL(env, napi_get_boolean(env, efficiencyResourceInfo->IsProcess(), &napiIsProcess));
    NAPI_CALL(env, napi_create_string_utf8(env, efficiencyResourceInfo->GetReason().c_str(),
        efficiencyResourceInfo->GetReason().length(), &napiReason));
    NAPI_CALL(env, napi_create_int32(env, efficiencyResourceInfo->GetUid(), &napiUid));
    NAPI_CALL(env, napi_create_int32(env, efficiencyResourceInfo->GetPid(), &napiPid));

    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "resourceTypes", napiResourceType));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "timeout", napiTimeOut));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "isPersistent", napiIsPersist));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "isForProcess", napiIsProcess));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "reason", napiReason));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "uid", napiUid));
    NAPI_CALL(env, napi_set_named_property(env, napiInfo, "pid", napiPid));

    return napiInfo;
}

bool Common::GetContinuousTaskRequest(napi_env env, napi_value objValue,
    std::shared_ptr<ContinuousTaskRequest> &request)
{
    // Get backgroundTaskModes.
    if (!GetBackgroundTaskModesProperty(env, objValue, "backgroundTaskModes", request)) {
        return false;
    }

    // Get backgroundTaskSubmodes.
    if (!GetBackgroundTaskSubmodesProperty(env, objValue, "backgroundTaskSubmodes", request)) {
        return false;
    }

    // Get wantAgent.
    if (!GetWantAgentProperty(env, objValue, "wantAgent", request)) {
        return false;
    }

    // Get continuousTaskId.
    int32_t continuousTaskId = GetIntProperty(env, objValue, "continuousTaskId");
    if (continuousTaskId != INVALID_CONTINUOUSTASK_ID) {
        request->SetContinuousTaskId(continuousTaskId);
    } else {
        return false;
    }
    
    // Get combinedTaskNotification.
    bool combinedTaskNotification = GetBoolProperty(env, objValue, "combinedTaskNotification");
    if (combinedTaskNotification) {
        request->SetCombinedTaskNotification(combinedTaskNotification);
    }
    return true;
}

bool Common::GetBackgroundTaskModesProperty(napi_env env, napi_value object, const std::string &propertyName,
    std::shared_ptr<ContinuousTaskRequest> &request)
{
    bool boolValue = false;
    napi_value value = nullptr;
    napi_status getNameStatus = napi_get_named_property(env, object, propertyName.c_str(), &value);
    if (getNameStatus != napi_ok) {
        return boolValue;
    }
    boolValue = GetBackgroundTaskModesFromArray(env, value, request);
    return boolValue;
}

bool Common::GetBackgroundTaskModesFromArray(napi_env env, napi_value arrayValue,
    std::shared_ptr<ContinuousTaskRequest> &request)
{
    bool boolValue = false;
    if (napi_is_array(env, arrayValue, &boolValue) != napi_ok || boolValue == false) {
        return boolValue;
    }
    uint32_t length;
    napi_get_array_length(env, arrayValue, &length);
    std::vector<uint32_t> backgroundTaskModes {};
    for (uint32_t i = 0; i < length; i++) {
        napi_value napiMode;
        if (napi_get_element(env, arrayValue, i, &napiMode) != napi_ok) {
            return false;
        } else {
            int32_t intValue = INVALID_MODE_ID;
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, napiMode, &valueType);
            if (valueType == napi_undefined) {
                return false;
            } else if (valueType != napi_number) {
                return false;
            }
            napi_get_value_int32(env, napiMode, &intValue);
            if (intValue < 0) {
                return false;
            }
            uint32_t mode = static_cast<uint32_t>(intValue);
            backgroundTaskModes.push_back(mode);
        }
    }
    request->SetBackgroundTaskMode(backgroundTaskModes);
    return boolValue;
}

bool Common::GetBackgroundTaskSubmodesProperty(napi_env env, napi_value object, const std::string &propertyName,
    std::shared_ptr<ContinuousTaskRequest> &request)
{
    bool boolValue = false;
    napi_value value = nullptr;
    napi_status getNameStatus = napi_get_named_property(env, object, propertyName.c_str(), &value);
    if (getNameStatus != napi_ok) {
        return boolValue;
    }
    boolValue = GetBackgroundTaskSubmodesFromArray(env, value, request);
    return boolValue;
}

bool Common::GetBackgroundTaskSubmodesFromArray(napi_env env, napi_value arrayValue,
    std::shared_ptr<ContinuousTaskRequest> &request)
{
    bool boolValue = false;
    if (napi_is_array(env, arrayValue, &boolValue) != napi_ok || boolValue == false) {
        return boolValue;
    }
    uint32_t length;
    napi_get_array_length(env, arrayValue, &length);
    std::vector<uint32_t> backgroundTaskSubModes {};
    for (uint32_t i = 0; i < length; i++) {
        napi_value napiSubMode;
        if (napi_get_element(env, arrayValue, i, &napiSubMode) != napi_ok) {
            return false;
        } else {
            int32_t intValue = INVALID_MODE_ID;
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, napiSubMode, &valueType);
            if (valueType == napi_undefined) {
                return false;
            } else if (valueType != napi_number) {
                return false;
            }
            napi_get_value_int32(env, napiSubMode, &intValue);
            if (intValue < 0) {
                return false;
            }
            uint32_t subMode = static_cast<uint32_t>(intValue);
            backgroundTaskSubModes.push_back(subMode);
        }
    }
    request->SetBackgroundTaskSubMode(backgroundTaskSubModes);
    return boolValue;
}

bool Common::GetWantAgentProperty(napi_env env, napi_value object, const std::string &propertyName,
    std::shared_ptr<ContinuousTaskRequest> &request)
{
    napi_value value = nullptr;
    napi_status getNameStatus = napi_get_named_property(env, object, propertyName.c_str(), &value);
    if (getNameStatus != napi_ok) {
        return false;
    }
    napi_valuetype valuetype = napi_undefined;
    AbilityRuntime::WantAgent::WantAgent *wantAgentPtr = nullptr;
    napi_typeof(env, value, &valuetype);
    if (valuetype != napi_object) {
        return false;
    }
    napi_unwrap(env, value, (void **)&wantAgentPtr);
    if (wantAgentPtr == nullptr) {
        return false;
    }
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent =
        std::make_shared<AbilityRuntime::WantAgent::WantAgent>(*wantAgentPtr);
    request->SetWantAgent(wantAgent);
    return true;
}

int32_t Common::GetIntProperty(napi_env env, napi_value object, const std::string &propertyName)
{
    int32_t intValue = INVALID_CONTINUOUSTASK_ID;
    napi_value value = nullptr;
    napi_status getNameStatus = napi_get_named_property(env, object, propertyName.c_str(), &value);
    if (getNameStatus != napi_ok) {
        return intValue;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_undefined && valueType == napi_number) {
        napi_get_value_int32(env, value, &intValue);
    } else {
        intValue = -1;
    }
    return intValue;
}

bool Common::GetBoolProperty(napi_env env, napi_value object, const std::string &propertyName)
{
    bool boolValue = false;
    napi_value value = nullptr;
    napi_status getNameStatus = napi_get_named_property(env, object, propertyName.c_str(), &value);
    if (getNameStatus != napi_ok) {
        return boolValue;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_undefined && valueType == napi_boolean) {
        napi_get_value_bool(env, value, &boolValue);
    }
    return boolValue;
}

void Common::TaskModeTypeConversion(std::shared_ptr<ContinuousTaskRequest> &request)
{
    if (request == nullptr) {
        return;
    }
    std::vector<uint32_t> backgroundTaskSubModes = request->GetBackgroundTaskSubmodes();
    std::vector<uint32_t> backgroundTaskModes = request->GetBackgroundTaskModes();
    std::vector<uint32_t> modes {};
    std::vector<uint32_t> subModes {};
    for (uint32_t index = 0; index < backgroundTaskSubModes.size(); index++) {
        uint32_t subMode = backgroundTaskSubModes[index];
        subModes.push_back(subMode);
        uint32_t mode = 0;
        if (subMode == BackgroundTaskSubmode::SUBMODE_NORMAL_NOTIFICATION) {
            mode = BackgroundTaskMode::GetV9BackgroundModeByMode(backgroundTaskModes[index]);
        } else {
            mode = BackgroundTaskMode::GetV9BackgroundModeBySubMode(subMode);
        }
        modes.push_back(mode);
    }
    request->SetBackgroundTaskMode(modes);
    request->SetBackgroundTaskSubMode(subModes);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
