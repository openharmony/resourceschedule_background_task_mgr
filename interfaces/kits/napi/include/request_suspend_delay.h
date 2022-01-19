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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_NAPI_INCLUDE_REQUEST_SUSPEND_DELAY_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_NAPI_INCLUDE_REQUEST_SUSPEND_DELAY_H

#include <string>

#include "common.h"
#include "delay_suspend_info.h"
#include "expired_callback.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class CallbackInstance : public ExpiredCallback {
public:
    CallbackInstance();

    virtual ~CallbackInstance();

    virtual void OnExpired() override;

    void SetCallbackInfo(const napi_env &env, const napi_ref &ref);

private:
    struct CallbackInfo {
        napi_env env = nullptr;
        napi_ref ref = nullptr;
    };

    CallbackInfo expiredCallbackInfo_;
};

struct CallbackInstancesInfo {
    napi_ref ref = nullptr;
    CallbackInstance *callback = nullptr;
};

napi_value RequestSuspendDelay(napi_env env, napi_callback_info info);
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_NAPI_INCLUDE_REQUEST_SUSPEND_DELAY_H