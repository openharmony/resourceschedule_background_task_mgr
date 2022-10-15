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

#include "common.h"

#include "cancel_suspend_delay.h"
#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
const uint32_t STR_MAX_SIZE = 64;
const uint32_t EXPIRE_CALLBACK_PARAM_NUM = 1;
const uint32_t ASYNC_CALLBACK_PARAM_NUM = 2;

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

napi_value Common::NapiGetboolean(const napi_env &env, bool isValue)
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
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String or function expected.");
    if (valuetype == napi_string) {
        char str[STR_MAX_SIZE] = {0};
        size_t strLen = 0;
        NAPI_CALL(env, napi_get_value_string_utf8(env, value, str, STR_MAX_SIZE - 1, &strLen));

        result = Str8ToStr16((std::string)str);
        BGTASK_LOGD("GetU16StringValue result: %{public}s", Str16ToStr8(result).c_str());
    } else {
        return nullptr;
    }

    return Common::NapiGetNull(env);
}

napi_value Common::GetInt32NumberValue(const napi_env &env, const napi_value &value, int32_t &result)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number or function expected.");
    napi_get_value_int32(env, value, &result);
    BGTASK_LOGI("GetInt32NumberValue result: %{public}d", result);
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
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String or function expected.");
    if (valuetype != napi_string) {
        return nullptr;
    }

    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    napi_status status = napi_get_value_string_utf8(env, value, str, STR_MAX_SIZE - 1, &strLen);
    if (status != napi_ok) {
        return nullptr;
    }
    result = std::string(str);
    BGTASK_LOGD("GetStringValue result: %{public}s", result.c_str());
    return Common::NapiGetNull(env);
}

void Common::HandleErrCode(const napi_env &env, int32_t errCode, bool isThrow)
{
    BGTASK_LOGI("HandleErrCode errCode = %{public}d, isThrow = %{public}d", errCode, isThrow);
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
    BGTASK_LOGI("HandleParamErr errCode = %{public}d, isThrow = %{public}d", errCode, isThrow);
    if (!isThrow || errCode == ERR_OK) {
        return false;
    }
    auto iter = paramErrCodeMsgMap.find(errCode);
    if (iter != paramErrCodeMsgMap.end()) {
        std::string errMessage = "BussinessError 401: Parameter error. ";
        errMessage.append(paramErrCodeMsgMap[errCode]);
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
    auto iter = saErrCodeMsgMap.find(errCode);
    if (iter != saErrCodeMsgMap.end()) {
        std::string errMessage = "BussinessError ";
        int32_t errCodeInfo = FindErrCode(env, errCode);
        errMessage.append(std::to_string(errCodeInfo)).append(": ").append(iter->second);
        return errMessage;
    }
    iter = paramErrCodeMsgMap.find(errCode);
    if (iter != paramErrCodeMsgMap.end()) {
        std::string errMessage = "BussinessError 401: Parameter error. ";
        errMessage.append(paramErrCodeMsgMap[errCode]);
        return errMessage;
    }
    return "Inner error.";
}

int32_t Common::FindErrCode(const napi_env &env, const int32_t errCodeIn)
{
    auto iter = paramErrCodeMsgMap.find(errCodeIn);
    if (iter != paramErrCodeMsgMap.end()) {
        return ERR_BGTASK_INVALID_PARAM;
    }
    return errCodeIn > THRESHOLD ? errCodeIn / OFFSET : errCodeIn;
}

napi_value Common::GetBooleanValue(const napi_env &env, const napi_value &value, bool &result)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Boolean or function expected.");
    NAPI_CALL(env, napi_get_value_bool(env, value, &result));
    BGTASK_LOGI("GetBooleanValue result: %{public}d", result);

    return Common::NapiGetNull(env);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS