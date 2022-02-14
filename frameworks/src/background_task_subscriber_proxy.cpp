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
        BGTASK_LOGE("remote is dead.");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor())) {
        BGTASK_LOGE("write interface token failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    ErrCode ret = remote->SendRequest(ON_CONNECTED, data, reply, option);
    if (ret!= ERR_OK) {
        BGTASK_LOGE("SendRequest failed, error code: %d", ret);
    }
}

void BackgroundTaskSubscriberProxy::OnDisconnected()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("remote is dead.");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor())) {
        BGTASK_LOGE("write interface token failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    ErrCode ret = remote->SendRequest(ON_DISCONNECTED, data, reply, option);
    if (ret != ERR_OK) {
        BGTASK_LOGE("SendRequest failed, error code: %d", ret);
    }
}

void BackgroundTaskSubscriberProxy::OnTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("remote is dead.");
        return;
    }

    MessageParcel data;
    bool res = data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor());
    if (!res) {
        BGTASK_LOGE("write descriptor failed.");
        return;
    }
    if (!info->Marshalling(data)) {
        BGTASK_LOGE("write parcel failed.");
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int ret = remote->SendRequest(ON_TRANSIENT_TASK_START, data, reply, option);
    if (ret != ERR_NONE) {
        BGTASK_LOGE("SendRequest failed, error code: %d", ret);
    }
}

void BackgroundTaskSubscriberProxy::OnTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("remote is dead.");
        return;
    }

    MessageParcel data;
    bool res = data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor());
    if (!res) {
        BGTASK_LOGE("write descriptor failed.");
        return;
    }
    if (!info->Marshalling(data)) {
        BGTASK_LOGE("write parcel failed.");
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int ret = remote->SendRequest(ON_TRANSIENT_TASK_END, data, reply, option);
    if (ret != ERR_NONE) {
        BGTASK_LOGE("SendRequest failed, error code: %d", ret);
    }
}

void BackgroundTaskSubscriberProxy::OnContinuousTaskStart(
    const sptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("begin");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("remote is dead.");
        return;
    }
    if (continuousTaskCallbackInfo == nullptr) {
        BGTASK_LOGE("continuousTaskCallbackInfo is nullptr.");
        return;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor())) {
        BGTASK_LOGE("write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(continuousTaskCallbackInfo)) {
        BGTASK_LOGE("write continuousTaskCallbackInfo failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    ErrCode result = remote->SendRequest(ON_CONTINUOUS_TASK_START, data, reply, option);
    if (result != ERR_OK) {
        BGTASK_LOGE("SendRequest error");
    }
}

void BackgroundTaskSubscriberProxy::OnContinuousTaskStop(
    const sptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("begin");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("remote is dead.");
        return;
    }
    if (continuousTaskCallbackInfo == nullptr) {
        BGTASK_LOGE("continuousTaskCallbackInfo is nullptr.");
        return;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor())) {
        BGTASK_LOGE("write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(continuousTaskCallbackInfo)) {
        BGTASK_LOGE("write notification failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    ErrCode result = remote->SendRequest(ON_CONTINUOUS_TASK_STOP, data, reply, option);
    if (result != ERR_OK) {
        BGTASK_LOGE("SendRequest error");
    }
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS