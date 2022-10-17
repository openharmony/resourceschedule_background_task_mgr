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

#include "get_remaining_delay_time.h"

#include "singleton.h"

#include "background_task_manager.h"
#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
static const uint32_t GET_REMAINING_DELAY_TIME_MIN_PARAMS = 1;
static const uint32_t GET_REMAINING_DELAY_TIME_PARAMS = 2;

struct AsyncCallbackInfoGetRemainingDelayTime : public AsyncWorkData {
    explicit AsyncCallbackInfoGetRemainingDelayTime(napi_env env) : AsyncWorkData(env) {}
    int32_t requestId = 0;
    int32_t delayTime = 0;
};

struct GetRemainingDelayTimeParamsInfo {
    int32_t requestId = 0;
    napi_ref callback = nullptr;
};

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info,
    GetRemainingDelayTimeParamsInfo &params, bool isThrow)
{
    size_t argc = GET_REMAINING_DELAY_TIME_PARAMS;
    napi_value argv[GET_REMAINING_DELAY_TIME_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != GET_REMAINING_DELAY_TIME_MIN_PARAMS && argc != GET_REMAINING_DELAY_TIME_PARAMS) {
        Common::HandleParamErr(env, ERR_PARAM_NUMBER_ERR, isThrow);
        return nullptr;
    }

    // argv[0] : requestId
    if (Common::GetInt32NumberValue(env, argv[0], params.requestId) == nullptr) {
        BGTASK_LOGE("ParseParameters failed, requestId is nullptr.");
        Common::HandleParamErr(env, ERR_REQUESTID_NULL_OR_ID_TYPE_ERR, isThrow);
        return nullptr;
    }

    // argv[1]: callback
    if (argc == GET_REMAINING_DELAY_TIME_PARAMS) {
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        if (valuetype != napi_function) {
            Common::HandleParamErr(env, ERR_CALLBACK_NULL_OR_TYPE_ERR, isThrow);
            return nullptr;
        }
        NAPI_CALL(env, napi_create_reference(env, argv[1], 1, &params.callback));
    }

    if (params.requestId <= 0) {
        BGTASK_LOGI("ParseParameters failed, requestId is illegal.");
        Common::HandleParamErr(env, ERR_REQUESTID_ILLEGAL, isThrow);
        return nullptr;
    }
    return Common::NapiGetNull(env);
}

napi_value GetRemainingDelayTime(napi_env env, napi_callback_info info, bool isThrow)
{
    GetRemainingDelayTimeParamsInfo params;
    if (ParseParameters(env, info, params, isThrow) == nullptr) {
        return nullptr;
    }

    napi_value promise = nullptr;
    AsyncCallbackInfoGetRemainingDelayTime *asyncCallbackInfo =
        new (std::nothrow) AsyncCallbackInfoGetRemainingDelayTime(env);
    if (!asyncCallbackInfo) {
        if (isThrow) {
            return nullptr;
        }
        return Common::JSParaError(env, params.callback);
    }
    std::unique_ptr<AsyncCallbackInfoGetRemainingDelayTime> callbackPtr {asyncCallbackInfo};
    asyncCallbackInfo->requestId = params.requestId;
    Common::PaddingAsyncWorkData(env, params.callback, *asyncCallbackInfo, promise);

    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, "GetRemainingDelayTime", NAPI_AUTO_LENGTH, &resourceName));

    NAPI_CALL(env, napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncCallbackInfoGetRemainingDelayTime *asyncCallbackInfo =
                static_cast<AsyncCallbackInfoGetRemainingDelayTime *>(data);
            if (asyncCallbackInfo != nullptr) {
                asyncCallbackInfo->errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->
                    GetRemainingDelayTime(asyncCallbackInfo->requestId, asyncCallbackInfo->delayTime);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfoGetRemainingDelayTime *asyncCallbackInfo =
                static_cast<AsyncCallbackInfoGetRemainingDelayTime *>(data);
            std::unique_ptr<AsyncCallbackInfoGetRemainingDelayTime> callbackPtr {asyncCallbackInfo};
            if (asyncCallbackInfo != nullptr) {
                napi_value result = nullptr;
                napi_create_int32(env, asyncCallbackInfo->delayTime, &result);
                Common::ReturnCallbackPromise(env, *asyncCallbackInfo, result);
            }
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork));

    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    callbackPtr.release();
    if (asyncCallbackInfo->isCallback) {
        return Common::NapiGetNull(env);
    }
    return promise;
}

napi_value GetRemainingDelayTime(napi_env env, napi_callback_info info)
{
    return GetRemainingDelayTime(env, info, false);
}

napi_value GetRemainingDelayTimeThrow(napi_env env, napi_callback_info info)
{
    return GetRemainingDelayTime(env, info, true);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS