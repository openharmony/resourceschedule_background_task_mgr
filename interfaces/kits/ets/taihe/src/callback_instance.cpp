/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "callback_instance.h"
#include "bgtaskmgr_log_wrapper.h"

namespace OHOS {
namespace BackgroundTaskMgr {
std::map<int32_t, std::shared_ptr<Callback>> callbackInstances_;
std::mutex callbackLock_;

Callback::Callback() {}

Callback::~Callback() {}

void Callback::OnExpired()
{
    BGTASK_LOGD("OnExpired start");
    std::lock_guard<std::mutex> lock(callbackLock_);
    auto findCallback = std::find_if(callbackInstances_.begin(), callbackInstances_.end(),
        [&](const auto& callbackInstance) { return callbackInstance.second.get() == this; });
    if (findCallback == callbackInstances_.end()) {
        BGTASK_LOGE("expired callback not found");
        return;
    }
    auto param = ::ohos::resourceschedule::backgroundTaskManager::UndefinedType::make_undefined();
    findCallback->second->callback_(param);
    callbackInstances_.erase(findCallback);
}

void Callback::SetCallbackInfo(
    callback_view<void(::ohos::resourceschedule::backgroundTaskManager::UndefinedType const&)> callback)
{
    callback_ = callback;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS