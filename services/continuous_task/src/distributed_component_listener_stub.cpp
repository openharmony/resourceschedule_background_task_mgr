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

#include "distributed_component_listener_stub.h"

#include <errors.h>
#include <ipc_skeleton.h>
#include "transient_task_log.h"
#include "continuous_task_detection.h"

namespace OHOS {
namespace BackgroundTaskMgr {
DistributedComponentListenerStub::DistributedComponentListenerStub() {}
DistributedComponentListenerStub::~DistributedComponentListenerStub() {}

ErrCode DistributedComponentListenerStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    std::u16string descriptor = DistributedComponentListenerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        BGTASK_LOGE("Local descriptor not match remote.");
        return ERR_TRANSACTION_FAILED;
    }

    switch (code) {
        case COMPONENT_CHANGE: {
            return HandleDisComponentChange(data);
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

ErrCode DistributedComponentListenerStub::HandleDisComponentChange(MessageParcel &data)
{
    std::string compInfo;
    if (!data.ReadString(compInfo)) {
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    ContinuousTaskDetection::GetInstance()->HandleDisComponentChange(compInfo);
    return ERR_OK;
}
}
}