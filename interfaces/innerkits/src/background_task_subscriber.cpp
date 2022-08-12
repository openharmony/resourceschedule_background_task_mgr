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

#include "background_task_subscriber.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace BackgroundTaskMgr {
BackgroundTaskSubscriber::BackgroundTaskSubscriber()
{
    impl_ = new (std::nothrow) BackgroundTaskSubscriberImpl(*this);
};

BackgroundTaskSubscriber::~BackgroundTaskSubscriber() {}

void BackgroundTaskSubscriber::OnConnected() {}

void BackgroundTaskSubscriber::OnDisconnected() {}

void BackgroundTaskSubscriber::OnTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info) {}

void BackgroundTaskSubscriber::OnTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) {}

void BackgroundTaskSubscriber::OnAppTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info) {}

void BackgroundTaskSubscriber::OnAppTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) {}

void BackgroundTaskSubscriber::OnContinuousTaskStart(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) {}

void BackgroundTaskSubscriber::OnContinuousTaskStop(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) {}

void BackgroundTaskSubscriber::OnAppContinuousTaskStop(int32_t uid) {}

void BackgroundTaskSubscriber::OnRemoteDied(const wptr<IRemoteObject> &object) {}

void BackgroundTaskSubscriber::OnEfficiencyResourcesApply(
    const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) {}

void BackgroundTaskSubscriber::OnEfficiencyResourcesReset(
    const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) {}

void BackgroundTaskSubscriber::OnAppEfficiencyResourcesApply(
    const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) {}

void BackgroundTaskSubscriber::OnAppEfficiencyResourcesReset(
    const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) {}

const sptr<BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl> BackgroundTaskSubscriber::GetImpl() const
{
    return impl_;
}

BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::BackgroundTaskSubscriberImpl(
    BackgroundTaskSubscriber &subscriber) : subscriber_(subscriber)
{
    recipient_ = new (std::nothrow) DeathRecipient(*this);
}

void BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnConnected()
{
    if (GetBackgroundTaskMgrProxy() && recipient_ != nullptr) {
        proxy_->AsObject()->AddDeathRecipient(recipient_);
    }
    subscriber_.OnConnected();
}

void BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnDisconnected()
{
    if (GetBackgroundTaskMgrProxy() && recipient_ != nullptr) {
        proxy_->AsObject()->RemoveDeathRecipient(recipient_);
    }
    subscriber_.OnDisconnected();
}

void BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnAppEfficiencyResourcesApply(
    const std::shared_ptr<ResourceCallbackInfo> &resourceInfo)
{
    subscriber_.OnAppEfficiencyResourcesApply(resourceInfo);
}

void BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnAppEfficiencyResourcesReset(
    const std::shared_ptr<ResourceCallbackInfo> &resourceInfo)
{
    subscriber_.OnAppEfficiencyResourcesReset(resourceInfo);
}

void BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnEfficiencyResourcesApply(
    const std::shared_ptr<ResourceCallbackInfo> &resourceInfo)
{
    subscriber_.OnEfficiencyResourcesApply(resourceInfo);
}

void BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnEfficiencyResourcesReset(
    const std::shared_ptr<ResourceCallbackInfo> &resourceInfo)
{
    subscriber_.OnEfficiencyResourcesReset(resourceInfo);
}

void BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnTransientTaskStart(
    const std::shared_ptr<TransientTaskAppInfo>& info)
{
    subscriber_.OnTransientTaskStart(info);
}

void BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnTransientTaskEnd(
    const std::shared_ptr<TransientTaskAppInfo>& info)
{
    subscriber_.OnTransientTaskEnd(info);
}

void BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnAppTransientTaskStart(
    const std::shared_ptr<TransientTaskAppInfo>& info)
{
    subscriber_.OnAppTransientTaskStart(info);
}

void BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnAppTransientTaskEnd(
    const std::shared_ptr<TransientTaskAppInfo>& info)
{
    subscriber_.OnAppTransientTaskEnd(info);
}

void BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnContinuousTaskStart(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    subscriber_.OnContinuousTaskStart(continuousTaskCallbackInfo);
}

void BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnContinuousTaskStop(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    subscriber_.OnContinuousTaskStop(continuousTaskCallbackInfo);
}

void BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnAppContinuousTaskStop(int32_t uid)
{
    subscriber_.OnAppContinuousTaskStop(uid);
}

bool BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::GetBackgroundTaskMgrProxy()
{
    if (proxy_) {
        return true;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        return false;
    }

    sptr<IRemoteObject> remoteObject =
        systemAbilityManager->GetSystemAbility(BACKGROUND_TASK_MANAGER_SERVICE_ID);
    if (!remoteObject) {
        return false;
    }

    proxy_ = iface_cast<IBackgroundTaskMgr>(remoteObject);
    if ((!proxy_) || (proxy_->AsObject() == nullptr)) {
        return false;
    }
    return true;
}

BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::DeathRecipient::DeathRecipient(
    BackgroundTaskSubscriberImpl &subscriberImpl) : subscriberImpl_(subscriberImpl) {}

BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::DeathRecipient::~DeathRecipient() {}

void BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::DeathRecipient::OnRemoteDied(
    const wptr<IRemoteObject> &object)
{
    subscriberImpl_.proxy_ = nullptr;
    subscriberImpl_.subscriber_.OnRemoteDied(object);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS