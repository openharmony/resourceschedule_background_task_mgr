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

#include "callback_instance.h"
#include "bgtaskmgr_log_wrapper.h"

namespace OHOS {
namespace BackgroundTaskMgr {

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
    auto param = ::ohos::resourceschedule::backgroundTaskManager::UndefinedType::make_uValue();
    if (findCallback->second->callback_) {
        findCallback->second->callback_(param);
    }
    callbackInstances_.erase(findCallback);
}

void Callback::OnExpiredAuth(int32_t authResult)
{
    std::lock_guard<std::mutex> lock(callbackLock_);
    if (authCallback_ == nullptr) {
        BGTASK_LOGE("authCallback_ is null");
        return;
    }
    ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t authResultRet;
    switch (authResult) {
        case OHOS::BackgroundTaskMgr::UserAuthResult::NOT_SUPPORTED:
            authResultRet = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::NOT_SUPPORTED;
            break;
        case OHOS::BackgroundTaskMgr::UserAuthResult::NOT_DETERMINED:
            authResultRet = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::NOT_DETERMINED;
            break;
        case OHOS::BackgroundTaskMgr::UserAuthResult::DENIED:
            authResultRet = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::DENIED;
            break;
        case OHOS::BackgroundTaskMgr::UserAuthResult::GRANTED_ONCE:
            authResultRet = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::GRANTED_ONCE;
            break;
        case OHOS::BackgroundTaskMgr::UserAuthResult::GRANTED_ALWAYS:
            authResultRet = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::GRANTED_ALWAYS;
            break;
        default:
            authResultRet = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::NOT_DETERMINED;
            break;
    }
    (*authCallback_)(authResultRet);
}

void Callback::SetCallbackInfo(
    callback_view<void(::ohos::resourceschedule::backgroundTaskManager::UndefinedType const&)> callback)
{
    std::lock_guard<std::mutex> lock(callbackLock_);
    callback_ = callback;
}

void Callback::SetAuthCallbackInfo(std::shared_ptr<AuthCallbackType> authCallback)
{
    std::lock_guard<std::mutex> lock(callbackLock_);
    authCallback_ = authCallback;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS