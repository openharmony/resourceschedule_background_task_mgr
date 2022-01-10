/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

void BackgroundTaskSubscriberProxy::OnTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("BackgroundTaskSubscriberProxy::%{public}s remote is dead.", __func__);
        return;
    }

    MessageParcel data;
    bool res = data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor());
    if (!res) {
        BGTASK_LOGE("BackgroundTaskSubscriberProxy::%{public}s, write descriptor failed.", __func__);
        return;
    }
    if (!info->Marshalling(data)) {
        BGTASK_LOGE("BackgroundTaskSubscriberProxy::%{public}s, write parcel failed.", __func__);
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int ret = remote->SendRequest(ON_TRANSIENT_TASK_START, data, reply, option);
    if (ret != ERR_NONE) {
        BGTASK_LOGE("BackgroundTaskSubscriberProxy::%{public}s, SendRequest failed, error code: %d", __func__, ret);
        return;
    }
}

void BackgroundTaskSubscriberProxy::OnTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) 
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("BackgroundTaskSubscriberProxy::%{public}s remote is dead.", __func__);
        return;
    }

    MessageParcel data;
    bool res = data.WriteInterfaceToken(BackgroundTaskSubscriberProxy::GetDescriptor());
    if (!res) {
        BGTASK_LOGE("BackgroundTaskSubscriberProxy::%{public}s, write descriptor failed.", __func__);
        return;
    }
    if (!info->Marshalling(data)) {
        BGTASK_LOGE("BackgroundTaskSubscriberProxy::%{public}s, write parcel failed.", __func__);
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int ret = remote->SendRequest(ON_TRANSIENT_TASK_END, data, reply, option);
    if (ret != ERR_NONE) {
        BGTASK_LOGE("BackgroundTaskSubscriberProxy::%{public}s, SendRequest failed, error code: %d", __func__, ret);
        return;
    }
}
} // namespace BackgroundTaskMgr
} // namespace OHOS