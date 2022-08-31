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

#include "background_task_subscriber_proxy.h"

#include <message_parcel.h>

#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
BackgroundTaskSubscriberProxy::BackgroundTaskSubscriberProxy(const sptr<IRemoteObject>& impl)
    : IRemoteProxy<IBackgroundTaskSubscriber>(impl) {}
BackgroundTaskSubscriberProxy::~BackgroundTaskSubscriberProxy() {}

void BackgroundTaskSubscriberProxy::OnConnected()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("OnConnected remote is dead.");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor())) {
        BGTASK_LOGE("OnConnected write interface token failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int32_t ret = remote->SendRequest(ON_CONNECTED, data, reply, option);
    if (ret!= ERR_OK) {
        BGTASK_LOGE("OnConnected SendRequest failed, error code: %d", ret);
    }
}

void BackgroundTaskSubscriberProxy::OnDisconnected()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("OnDisconnected remote is dead.");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor())) {
        BGTASK_LOGE("OnDisconnected write interface token failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int32_t ret = remote->SendRequest(ON_DISCONNECTED, data, reply, option);
    if (ret != ERR_OK) {
        BGTASK_LOGE("OnDisconnected SendRequest failed, error code: %d", ret);
    }
}

void BackgroundTaskSubscriberProxy::OnTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("OnTransientTaskStart remote is dead.");
        return;
    }
    if (info == nullptr) {
        BGTASK_LOGE("OnTransientTaskStart TransientTaskAppInfo is null.");
        return;
    }

    MessageParcel data;
    bool res = data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor());
    if (!res) {
        BGTASK_LOGE("OnTransientTaskStart write descriptor failed.");
        return;
    }
    if (!info->Marshalling(data)) {
        BGTASK_LOGE("OnTransientTaskStart write parcel failed.");
        return;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int32_t ret = remote->SendRequest(ON_TRANSIENT_TASK_START, data, reply, option);
    if (ret != ERR_NONE) {
        BGTASK_LOGE("OnTransientTaskStart SendRequest failed, error code: %{public}d", ret);
    }
}

void BackgroundTaskSubscriberProxy::OnTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("remote is dead.");
        return;
    }
    if (info == nullptr) {
        BGTASK_LOGE("OnTransientTaskEnd TransientTaskAppInfo is null.");
        return;
    }

    MessageParcel data;
    bool res = data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor());
    if (!res) {
        BGTASK_LOGE("OnTransientTaskEnd write descriptor failed.");
        return;
    }
    if (!info->Marshalling(data)) {
        BGTASK_LOGE("OnTransientTaskEnd write parcel failed.");
        return;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int32_t ret = remote->SendRequest(ON_TRANSIENT_TASK_END, data, reply, option);
    if (ret != ERR_NONE) {
        BGTASK_LOGE("OnTransientTaskEnd SendRequest failed, error code: %{public}d", ret);
    }
}

void BackgroundTaskSubscriberProxy::OnAppTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("OnAppTransientTaskStart remote is dead.");
        return;
    }
    if (info == nullptr) {
        BGTASK_LOGE("OnAppTransientTaskStart TransientTaskAppInfo is null.");
        return;
    }

    MessageParcel data;
    bool res = data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor());
    if (!res) {
        BGTASK_LOGE("OnAppTransientTaskStart write descriptor failed.");
        return;
    }
    if (!info->Marshalling(data)) {
        BGTASK_LOGE("OnAppTransientTaskStart write parcel failed.");
        return;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int32_t ret = remote->SendRequest(ON_APP_TRANSIENT_TASK_START, data, reply, option);
    if (ret != ERR_NONE) {
        BGTASK_LOGE("OnAppTransientTaskStart SendRequest failed, error code: %{public}d", ret);
    }
}

void BackgroundTaskSubscriberProxy::OnAppTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("OnAppTransientTaskEnd remote is dead.");
        return;
    }
    if (info == nullptr) {
        BGTASK_LOGE("OnAppTransientTaskEnd TransientTaskAppInfo is null.");
        return;
    }

    MessageParcel data;
    bool res = data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor());
    if (!res) {
        BGTASK_LOGE("OnAppTransientTaskEnd write descriptor failed.");
        return;
    }
    if (!info->Marshalling(data)) {
        BGTASK_LOGE("OnAppTransientTaskEnd write parcel failed.");
        return;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int32_t ret = remote->SendRequest(ON_APP_TRANSIENT_TASK_END, data, reply, option);
    if (ret != ERR_NONE) {
        BGTASK_LOGE("OnAppTransientTaskEndSendRequest failed, error code: %{public}d", ret);
    }
}

void BackgroundTaskSubscriberProxy::OnContinuousTaskStart(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("OnContinuousTaskStart remote is dead.");
        return;
    }
    if (continuousTaskCallbackInfo == nullptr) {
        BGTASK_LOGE("OnContinuousTaskStart continuousTaskCallbackInfo is nullptr.");
        return;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor())) {
        BGTASK_LOGE("OnContinuousTaskStart write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(continuousTaskCallbackInfo.get())) {
        BGTASK_LOGE("OnContinuousTaskStart write continuousTaskCallbackInfo failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int32_t result = remote->SendRequest(ON_CONTINUOUS_TASK_START, data, reply, option);
    if (result != ERR_OK) {
        BGTASK_LOGE("OnContinuousTaskStart SendRequest error");
    }
}

void BackgroundTaskSubscriberProxy::OnContinuousTaskStop(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("OnContinuousTaskStop remote is dead.");
        return;
    }
    if (continuousTaskCallbackInfo == nullptr) {
        BGTASK_LOGE("OnContinuousTaskStop continuousTaskCallbackInfo is nullptr.");
        return;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor())) {
        BGTASK_LOGE("OnContinuousTaskStop write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(continuousTaskCallbackInfo.get())) {
        BGTASK_LOGE("OnContinuousTaskStop write notification failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int32_t result = remote->SendRequest(ON_CONTINUOUS_TASK_STOP, data, reply, option);
    if (result != ERR_OK) {
        BGTASK_LOGE("OnContinuousTaskStop SendRequest error");
    }
}

void BackgroundTaskSubscriberProxy::OnAppContinuousTaskStop(int32_t uid)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("OnAppContinuousTaskStop remote is dead.");
        return;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor())) {
        BGTASK_LOGE("OnAppContinuousTaskStop write interface token failed.");
        return;
    }

    if (!data.WriteInt32(uid)) {
        BGTASK_LOGE("OnAppContinuousTaskStop write uid failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int32_t result = remote->SendRequest(ON_APP_CONTINUOUS_TASK_STOP, data, reply, option);
    if (result != ERR_OK) {
        BGTASK_LOGE("OnAppContinuousTaskStop SendRequest error");
    }
}


void BackgroundTaskSubscriberProxy::OnAppEfficiencyResourcesApply(
    const std::shared_ptr<ResourceCallbackInfo> &resourceInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("OnAppEfficiencyResourcesApply remote is dead.");
        return;
    }
    if (resourceInfo == nullptr) {
        BGTASK_LOGE("OnAppEfficiencyResourcesApply resourceInfo is nullptr.");
        return;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor())) {
        BGTASK_LOGE("OnAppEfficiencyResourcesApply write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(resourceInfo.get())) {
        BGTASK_LOGE("OnAppEfficiencyResourcesApply write notification failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t result = remote->SendRequest(ON_APP_EFFICIENCY_RESOURCES_APPLY, data, reply, option);
    if (result != ERR_OK) {
        BGTASK_LOGE("OnAppEfficiencyResourcesApply SendRequest error");
    }
}

void BackgroundTaskSubscriberProxy::OnAppEfficiencyResourcesReset(
    const std::shared_ptr<ResourceCallbackInfo> &resourceInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("OnAppEfficiencyResourcesReset remote is dead.");
        return;
    }

    if (resourceInfo == nullptr) {
        BGTASK_LOGE("OnAppEfficiencyResourcesReset resourceInfo is nullptr.");
        return;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor())) {
        BGTASK_LOGE("OnAppEfficiencyResourcesReset write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(resourceInfo.get())) {
        BGTASK_LOGE("OnAppEfficiencyResourcesReset write notification failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t result = remote->SendRequest(ON_APP_EFFICIENCY_RESOURCES_RESET, data, reply, option);
    if (result != ERR_OK) {
        BGTASK_LOGE("OnAppEfficiencyResourcesReset SendRequest error");
    }
}

void BackgroundTaskSubscriberProxy::OnProcEfficiencyResourcesApply(
    const std::shared_ptr<ResourceCallbackInfo> &resourceInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("OnProcEfficiencyResourcesApply remote is dead.");
        return;
    }
    if (resourceInfo == nullptr) {
        BGTASK_LOGE("OnProcEfficiencyResourcesApply resourceInfo is nullptr.");
        return;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor())) {
        BGTASK_LOGE("OnProcEfficiencyResourcesApply write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(resourceInfo.get())) {
        BGTASK_LOGE("OnProcEfficiencyResourcesApply write notification failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t result = remote->SendRequest(ON_PROC_EFFICIENCY_RESOURCES_APPLY, data, reply, option);
    if (result != ERR_OK) {
        BGTASK_LOGE("OnProcEfficiencyResourcesApply SendRequest error");
    }
}

void BackgroundTaskSubscriberProxy::OnProcEfficiencyResourcesReset(
    const std::shared_ptr<ResourceCallbackInfo> &resourceInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("OnProcEfficiencyResourcesReset remote is dead.");
        return;
    }

    if (resourceInfo == nullptr) {
        BGTASK_LOGE("OnProcEfficiencyResourcesReset resourceInfo is nullptr.");
        return;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor())) {
        BGTASK_LOGE("OnProcEfficiencyResourcesReset write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(resourceInfo.get())) {
        BGTASK_LOGE("OnProcEfficiencyResourcesReset write notification failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t result = remote->SendRequest(ON_PROC_EFFICIENCY_RESOURCES_RESET, data, reply, option);
    if (result != ERR_OK) {
        BGTASK_LOGE("OnProcEfficiencyResourcesReset SendRequest error");
    }
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS