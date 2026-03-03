/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_TAIHE_INCLUDE_CALLBACK_INSTANCE_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_TAIHE_INCLUDE_CALLBACK_INSTANCE_H

#include <mutex>
#include "expired_callback.h"
#include "ohos.resourceschedule.backgroundTaskManager.impl.hpp"

namespace OHOS {
namespace BackgroundTaskMgr {
using namespace taihe;
using AuthCallbackType = ::taihe::callback<
 	void(::ohos::resourceschedule::backgroundTaskManager::UserAuthResult data)>;

class Callback : public ExpiredCallback {
public:
    Callback();
    ~Callback() override;
    void OnExpired() override;
    void OnExpiredAuth(int32_t authResult) override;
    void SetCallbackInfo(
        callback_view<void(::ohos::resourceschedule::backgroundTaskManager::UndefinedType const&)> callback);
    void SetAuthCallbackInfo(std::shared_ptr<AuthCallbackType> authCallback);

private:
    std::function<void(::ohos::resourceschedule::backgroundTaskManager::UndefinedType const&)> callback_;
    std::shared_ptr<AuthCallbackType> authCallback_;
    std::map<int32_t, std::shared_ptr<Callback>> callbackInstances_;
    std::mutex callbackLock_;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_TAIHE_INCLUDE_CALLBACK_INSTANCE_H