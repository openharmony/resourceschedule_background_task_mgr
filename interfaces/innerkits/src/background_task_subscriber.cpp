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

void BackgroundTaskSubscriber::OnTransientTaskErr(const std::shared_ptr<TransientTaskAppInfo>& info) {}

void BackgroundTaskSubscriber::OnAppTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info) {}

void BackgroundTaskSubscriber::OnAppTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) {}

void BackgroundTaskSubscriber::OnContinuousTaskStart(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) {}

void BackgroundTaskSubscriber::OnContinuousTaskUpdate(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) {}

void BackgroundTaskSubscriber::OnContinuousTaskStop(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) {}

void BackgroundTaskSubscriber::OnAppContinuousTaskStop(int32_t uid) {}

void BackgroundTaskSubscriber::OnRemoteDied(const wptr<IRemoteObject> &object) {}

void BackgroundTaskSubscriber::OnProcEfficiencyResourcesApply(
    const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) {}

void BackgroundTaskSubscriber::OnProcEfficiencyResourcesReset(
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
    BackgroundTaskSubscriber &subscriber) : subscriber_(subscriber) {}

ErrCode BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnConnected() {return ERR_OK;}

ErrCode BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnDisconnected() {return ERR_OK;}

ErrCode BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnAppEfficiencyResourcesApply(
    const ResourceCallbackInfo &resourceInfo)
{
    std::shared_ptr<ResourceCallbackInfo> sharedResourceInfo = std::make_shared<ResourceCallbackInfo>(resourceInfo);
    subscriber_.OnAppEfficiencyResourcesApply(sharedResourceInfo);
    return ERR_OK;
}

ErrCode BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnAppEfficiencyResourcesReset(
    const ResourceCallbackInfo &resourceInfo)
{
    std::shared_ptr<ResourceCallbackInfo> sharedResourceInfo = std::make_shared<ResourceCallbackInfo>(resourceInfo);
    subscriber_.OnAppEfficiencyResourcesReset(sharedResourceInfo);
    return ERR_OK;
}

ErrCode BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnProcEfficiencyResourcesApply(
    const ResourceCallbackInfo &resourceInfo)
{
    std::shared_ptr<ResourceCallbackInfo> sharedResourceInfo = std::make_shared<ResourceCallbackInfo>(resourceInfo);
    subscriber_.OnProcEfficiencyResourcesApply(sharedResourceInfo);
    return ERR_OK;
}

ErrCode BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnProcEfficiencyResourcesReset(
    const ResourceCallbackInfo &resourceInfo)
{
    std::shared_ptr<ResourceCallbackInfo> sharedResourceInfo = std::make_shared<ResourceCallbackInfo>(resourceInfo);
    subscriber_.OnProcEfficiencyResourcesReset(sharedResourceInfo);
    return ERR_OK;
}

ErrCode BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnTransientTaskStart(
    const TransientTaskAppInfo& info)
{
    std::shared_ptr<TransientTaskAppInfo> sharedInfo = std::make_shared<TransientTaskAppInfo>(info);
    subscriber_.OnTransientTaskStart(sharedInfo);
    return ERR_OK;
}

ErrCode BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnTransientTaskEnd(
    const TransientTaskAppInfo& info)
{
    std::shared_ptr<TransientTaskAppInfo> sharedInfo = std::make_shared<TransientTaskAppInfo>(info);
    subscriber_.OnTransientTaskEnd(sharedInfo);
    return ERR_OK;
}

ErrCode BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnTransientTaskErr(
    const TransientTaskAppInfo& info)
{
    std::shared_ptr<TransientTaskAppInfo> sharedInfo = std::make_shared<TransientTaskAppInfo>(info);
    subscriber_.OnTransientTaskErr(sharedInfo);
    return ERR_OK;
}

ErrCode BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnAppTransientTaskStart(
    const TransientTaskAppInfo& info)
{
    std::shared_ptr<TransientTaskAppInfo> sharedInfo = std::make_shared<TransientTaskAppInfo>(info);
    subscriber_.OnAppTransientTaskStart(sharedInfo);
    return ERR_OK;
}

ErrCode BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnAppTransientTaskEnd(
    const TransientTaskAppInfo& info)
{
    std::shared_ptr<TransientTaskAppInfo> sharedInfo = std::make_shared<TransientTaskAppInfo>(info);
    subscriber_.OnAppTransientTaskEnd(sharedInfo);
    return ERR_OK;
}

ErrCode BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnContinuousTaskStart(
    const ContinuousTaskCallbackInfo &continuousTaskCallbackInfo)
{
    std::shared_ptr<ContinuousTaskCallbackInfo> sharedInfo = 
        std::make_shared<ContinuousTaskCallbackInfo>(continuousTaskCallbackInfo);
    subscriber_.OnContinuousTaskStart(sharedInfo);
    return ERR_OK;
}

ErrCode BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnContinuousTaskUpdate(
    const ContinuousTaskCallbackInfo &continuousTaskCallbackInfo)
{
    std::shared_ptr<ContinuousTaskCallbackInfo> sharedInfo = 
        std::make_shared<ContinuousTaskCallbackInfo>(continuousTaskCallbackInfo);
    subscriber_.OnContinuousTaskUpdate(sharedInfo);
    return ERR_OK;
}

ErrCode BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnContinuousTaskStop(
    const ContinuousTaskCallbackInfo &continuousTaskCallbackInfo)
{
    std::shared_ptr<ContinuousTaskCallbackInfo> sharedInfo = 
        std::make_shared<ContinuousTaskCallbackInfo>(continuousTaskCallbackInfo);
    subscriber_.OnContinuousTaskStop(sharedInfo);
    return ERR_OK;
}

ErrCode BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl::OnAppContinuousTaskStop(int32_t uid)
    __attribute__((no_sanitize("cfi")))
{
    subscriber_.OnAppContinuousTaskStop(uid);
    return ERR_OK;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS