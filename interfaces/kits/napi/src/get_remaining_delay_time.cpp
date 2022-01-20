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
static const int32_t GET_REMAINING_DELAY_TIME_MIN_PARAMS = 1;
static const int32_t GET_REMAINING_DELAY_TIME_PARAMS = 2;

struct AsyncCallbackInfoGetRemainingDelayTime {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    int32_t requestId;
    int32_t delayTime;
    CallbackPromiseInfo info;
};

struct GetRemainingDelayTimeParamsInfo {
    int32_t requestId;
    napi_ref callback = nullptr;
};

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, GetRemainingDelayTimeParamsInfo &params)
{
    size_t argc = GET_REMAINING_DELAY_TIME_PARAMS;
    napi_value argv[GET_REMAINING_DELAY_TIME_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc == GET_REMAINING_DELAY_TIME_MIN_PARAMS || argc == GET_REMAINING_DELAY_TIME_PARAMS,
        "Wrong number of arguments");

    // argv[0] : requestId
    if (Common::GetInt32NumberValue(env, argv[0], params.requestId) == nullptr) {
        BGTASK_LOGE("ParseParameters failed, requestId is nullptr.");
        return nullptr;
    }

    // argv[1]: callback
    if (argc == GET_REMAINING_DELAY_TIME_PARAMS) {
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &params.callback);
    }

    if (params.requestId <= 0) {
        BGTASK_LOGE("ParseParameters failed, requestId is illegal.");
        return nullptr;
    }
    return Common::NapiGetNull(env);
}

napi_value GetRemainingDelayTime(napi_env env, napi_callback_info info)
{
    GetRemainingDelayTimeParamsInfo params;
    if (ParseParameters(env, info, params) == nullptr) {
        return Common::JSParaError(env, params.callback);
    }

    napi_value promise = nullptr;
    AsyncCallbackInfoGetRemainingDelayTime *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoGetRemainingDelayTime {.env = env, .asyncWork = nullptr};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, params.callback);
    }
    asynccallbackinfo->requestId = params.requestId;
    BGTASK_LOGI(" asynccallbackinfo->requestId: %{public}d", asynccallbackinfo->requestId);
    Common::PaddingCallbackPromiseInfo(env, params.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "GetRemainingDelayTime", NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncCallbackInfoGetRemainingDelayTime *asynccallbackinfo = (AsyncCallbackInfoGetRemainingDelayTime *)data;
            if (asynccallbackinfo != nullptr) {
                asynccallbackinfo->info.errorCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->
                    GetRemainingDelayTime(asynccallbackinfo->requestId, asynccallbackinfo->delayTime);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfoGetRemainingDelayTime *asynccallbackinfo = (AsyncCallbackInfoGetRemainingDelayTime *)data;
            if (asynccallbackinfo != nullptr) {
                napi_value result = nullptr;
                napi_create_int32(env, asynccallbackinfo->delayTime, &result);
                Common::ReturnCallbackPromise(env, asynccallbackinfo->info, result);

                if (asynccallbackinfo->info.callback != nullptr) {
                    napi_delete_reference(env, asynccallbackinfo->info.callback);
                }

                napi_delete_async_work(env, asynccallbackinfo->asyncWork);
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->info.isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS