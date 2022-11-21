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

#include "cancel_suspend_delay.h"

#include "singleton.h"

#include "background_task_manager.h"
#include "request_suspend_delay.h"
#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
static const uint32_t CANCEL_SUSPEND_DELAY_PARAMS = 1;

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, int32_t &requestId)
{
    size_t argc = CANCEL_SUSPEND_DELAY_PARAMS;
    napi_value argv[CANCEL_SUSPEND_DELAY_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc == 1, "Wrong number of arguments");

    // argv[0] :requestId
    if (Common::GetInt32NumberValue(env, argv[0], requestId) == nullptr) {
        BGTASK_LOGE("ParseParameters failed, requestId is nullptr.");
        return nullptr;
    }
    if (requestId <= 0) {
        BGTASK_LOGI("ParseParameters failed, requestId is illegal.");
        return nullptr;
    }
    return Common::NapiGetNull(env);
}

napi_value CancelSuspendDelay(napi_env env, napi_callback_info info)
{
    int32_t requestId;
    if (ParseParameters(env, info, requestId) == nullptr) {
        return Common::NapiGetNull(env);
    }

    DelayedSingleton<BackgroundTaskManager>::GetInstance()->CancelSuspendDelay(requestId);
    std::lock_guard<std::mutex> lock(callbackLock_);
    auto findCallback = callbackInstances_.find(requestId);
    if (findCallback != callbackInstances_.end()) {
        callbackInstances_.erase(findCallback);
    }
    return Common::NapiGetNull(env);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS