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

#include "expired_callback_stub.h"

#include <errors.h>

#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
ExpiredCallbackStub::ExpiredCallbackStub() {}
ExpiredCallbackStub::~ExpiredCallbackStub() {}

int ExpiredCallbackStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    std::u16string descriptor = ExpiredCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        BGTASK_LOGE("local descriptor not match remote.");
        return ERR_TRANSACTION_FAILED;
    }
    switch (code) {
        case ON_EXPIRED:
            return HandleOnExpired(data);
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t ExpiredCallbackStub::HandleOnExpired(MessageParcel& data)
{
    OnExpired();
    return ERR_NONE;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS