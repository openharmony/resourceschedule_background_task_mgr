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

#include "expired_callback_proxy.h"

#include <message_parcel.h>

#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
ExpiredCallbackProxy::ExpiredCallbackProxy(const sptr<IRemoteObject>& impl)
    : IRemoteProxy<IExpiredCallback>(impl) {}
ExpiredCallbackProxy::~ExpiredCallbackProxy() {}

void ExpiredCallbackProxy::OnExpired()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("remote is dead.");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    bool res = data.WriteInterfaceToken(ExpiredCallbackProxy::GetDescriptor());
    if (!res) {
        BGTASK_LOGE("write descriptor failed.");
        return;
    }
    remote->SendRequest(ON_EXPIRED, data, reply, option);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS