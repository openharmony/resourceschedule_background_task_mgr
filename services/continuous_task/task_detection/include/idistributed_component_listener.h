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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_IDISTRIBUTED_COMPONENT_LISTENER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_IDISTRIBUTED_COMPONENT_LISTENER_H

#include <ipc_types.h>
#include <iremote_broker.h>
#include <nocopyable.h>

namespace OHOS {
namespace BackgroundTaskMgr {
class IDistributedComponentListener : public IRemoteBroker {
public:
    IDistributedComponentListener() = default;
    ~IDistributedComponentListener() override = default;
    DISALLOW_COPY_AND_MOVE(IDistributedComponentListener);

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.rms.DistributedComponent");

protected:
    enum InterfaceId : uint32_t {
        COMPONENT_CHANGE = FIRST_CALL_TRANSACTION,
    };
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_IDISTRIBUTED_COMPONENT_LISTENER_H