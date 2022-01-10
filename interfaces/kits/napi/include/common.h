/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <string_ex.h>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "delay_suspend_info.h"

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_INTERFACES_KITS_NAPI_INCLUDE_COMMON_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_INTERFACES_KITS_NAPI_INCLUDE_COMMON_H

namespace OHOS {
namespace BackgroundTaskMgr {
const std::int32_t STR_MAX_SIZE = 64;

struct CallbackPromiseInfo {
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    bool isCallback = false;
    int errorCode = 0;
};

class Common {

public:
    static napi_value NapiGetboolean(napi_env env, const bool &isValue);

    static napi_value NapiGetNull(napi_env env);

    static napi_value NapiGetUndefined(napi_env env);

    static napi_value GetCallbackErrorValue(napi_env env, int errCode);

    static napi_value GetExpireCallbackValue(napi_env env, int errCode, const napi_value &value);

    static void PaddingCallbackPromiseInfo(
        const napi_env &env, const napi_ref &callback, CallbackPromiseInfo &info, napi_value &promise);

    static void ReturnCallbackPromise(const napi_env &env, const CallbackPromiseInfo &info, const napi_value &result);

    static void SetCallback(
        const napi_env &env, const napi_ref &callbackIn, const int &errorCode, const napi_value &result);

    static void SetCallback(
        const napi_env &env, const napi_ref &callbackIn, const napi_value &result);

    static void SetPromise(const napi_env &env, const napi_deferred &deferred, const napi_value &result);

    static napi_value JSParaError(const napi_env &env, const napi_ref &callback);

    static napi_value GetU16StringValue(const napi_env &env, const napi_value &value, std::u16string &result);

    static napi_value GetInt32NumberValue(const napi_env &env, const napi_value &value, int32_t &result);

    static napi_value SetDelaySuspendInfo(
        const napi_env &env, std::shared_ptr<DelaySuspendInfo>& delaySuspendInfo, napi_value &result);
};

} // namespace BackgroundTaskMgr
} // namespace OHOS
#endif