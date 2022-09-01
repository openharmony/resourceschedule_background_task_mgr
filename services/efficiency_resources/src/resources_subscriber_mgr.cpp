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

#include "resources_subscriber_mgr.h"
#include "efficiency_resource_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
ResourcesSubscriberMgr::ResourcesSubscriberMgr()
{
    deathRecipient_ = new (std::nothrow) ObserverDeathRecipient();
}

ResourcesSubscriberMgr::~ResourcesSubscriberMgr() {}

ErrCode ResourcesSubscriberMgr::AddSubscriber(const sptr<IBackgroundTaskSubscriber>& subscriber)
{
    BGTASK_LOGD("ResourcesSubscriberMgr start subscriber");
    if (subscriber == NULL) {
        BGTASK_LOGI("subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }
    auto remote = subscriber->AsObject();
    if (remote == nullptr) {
        BGTASK_LOGE("remote in subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }

    if (deathRecipient_ == nullptr) {
        BGTASK_LOGE("create death recipient failed");
        return ERR_BGTASK_INVALID_PARAM;
    }
    std::lock_guard<std::mutex> subcriberLock(subscriberLock_);
    auto subscriberIter = std::find_if(subscriberList_.begin(), subscriberList_.end(),
        [&remote](const auto &subscriber) { return subscriber->AsObject() == remote; });
    if (subscriberIter != subscriberList_.end()) {
        BGTASK_LOGE("subscriber has already exist");
        return ERR_BGTASK_OBJECT_EXISTS;
    }

    subscriberList_.emplace_back(subscriber);
    remote->AddDeathRecipient(deathRecipient_);
    BGTASK_LOGD("add resources to efficiency resources mgr succeed!");
    BGTASK_LOGD("suscriber efficient resources, list.size() is %{public}d.",
        static_cast<int32_t>(subscriberList_.size()));
    return ERR_OK;
}

ErrCode ResourcesSubscriberMgr::RemoveSubscriber(const sptr<IBackgroundTaskSubscriber>& subscriber)
{
    if (deathRecipient_ == nullptr) {
        BGTASK_LOGE("deathRecipient is null.");
        return ERR_BGTASK_OBJECT_EXISTS;
    }
    if (subscriber == nullptr) {
        BGTASK_LOGE("subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }
    auto remote = subscriber->AsObject();
    if (remote == nullptr) {
        BGTASK_LOGE("apply efficiency resources failed, remote in subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }
    std::lock_guard<std::mutex> subcriberLock(subscriberLock_);
    auto findSuscriber = [&remote](const auto& subscriberList) {
        return subscriberList->AsObject() == remote;
    };
    auto subscriberIter = std::find_if(subscriberList_.begin(), subscriberList_.end(), findSuscriber);
    if (subscriberIter == subscriberList_.end()) {
        BGTASK_LOGE("request subscriber is not exists.");
        return ERR_BGTASK_OBJECT_EXISTS;
    }
    subscriberList_.erase(subscriberIter);
    remote->RemoveDeathRecipient(deathRecipient_);
    BGTASK_LOGD("remove subscriber from efficiency resources succeed.");
    return ERR_OK;
}

void ResourcesSubscriberMgr::OnResourceChanged(const std::shared_ptr<ResourceCallbackInfo> &callbackInfo,
    EfficiencyResourcesEventType type)
{
    BGTASK_LOGI("start OnResourceChanged");
    if (callbackInfo == nullptr) {
        BGTASK_LOGW("ResourceCallbackInfo is null");
        return;
    }
    std::lock_guard<std::mutex> subcriberLock(subscriberLock_);
    if (subscriberList_.empty()) {
        BGTASK_LOGW("Background Task Subscriber List is empty");
        return;
    }
    switch (type) {
        case EfficiencyResourcesEventType::APP_RESOURCE_APPLY:
            for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); ++iter) {
                BGTASK_LOGD("start callback function of app resources application");
                (*iter)->OnAppEfficiencyResourcesApply(callbackInfo);
            }
            break;
        case EfficiencyResourcesEventType::RESOURCE_APPLY:
            for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); ++iter) {
                BGTASK_LOGD("start callback function of proc resources application");
                (*iter)->OnProcEfficiencyResourcesApply(callbackInfo);
            }
            break;
        case EfficiencyResourcesEventType::APP_RESOURCE_RESET:
            for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); ++iter) {
                BGTASK_LOGD("start callback function of app resources reset");
                (*iter)->OnAppEfficiencyResourcesReset(callbackInfo);
            }
            break;
        case EfficiencyResourcesEventType::RESOURCE_RESET:
            for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); ++iter) {
                BGTASK_LOGD("start callback function of proc resources reset");
                (*iter)->OnProcEfficiencyResourcesReset(callbackInfo);
            }
            break;
    }
    BGTASK_LOGD("efficiency resources on resources changed function succeed");
}

void ResourcesSubscriberMgr::HandleSubscriberDeath(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        BGTASK_LOGE("suscriber death, remote in suscriber is null.");
        return;
    }
    sptr<IRemoteObject> proxy = remote.promote();
    if (!proxy) {
        BGTASK_LOGE("get remote proxy failed");
        return;
    }
    std::lock_guard<std::mutex> subcriberLock(subscriberLock_);
    auto findSuscriber = [&proxy](const auto& subscriber) {
        return subscriber->AsObject() == proxy;
    };
    auto subscriberIter = std::find_if(subscriberList_.begin(), subscriberList_.end(), findSuscriber);
    if (subscriberIter == subscriberList_.end()) {
        BGTASK_LOGI("suscriber death, remote in suscriber not found.");
        return;
    }
    subscriberList_.erase(subscriberIter);
    BGTASK_LOGD("suscriber death, remove it from list");
}


ObserverDeathRecipient::ObserverDeathRecipient() {}

ObserverDeathRecipient::~ObserverDeathRecipient() {}

void ObserverDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    DelayedSingleton<ResourcesSubscriberMgr>::GetInstance()->HandleSubscriberDeath(remote);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS