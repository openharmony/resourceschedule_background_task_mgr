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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_IPC_INCLUDE_IBACKGROUND_TASK_SUBSCRIBER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_IPC_INCLUDE_IBACKGROUND_TASK_SUBSCRIBER_H

#include <ipc_types.h>
#include <iremote_broker.h>
#include <nocopyable.h>

#include "transient_task_app_info.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class IBackgroundTaskSubscriber : public IRemoteBroker {
public:
    IBackgroundTaskSubscriber() = default;
    ~IBackgroundTaskSubscriber() override = default;
    DISALLOW_COPY_AND_MOVE(IBackgroundTaskSubscriber);

    virtual void OnTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info) = 0;
    virtual void OnTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) = 0;
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.resourceschedule.IBackgroundTaskSubscriber");

protected:
    enum InterfaceId : uint32_t {
        ON_TRANSIENT_TASK_START = FIRST_CALL_TRANSACTION,
        ON_TRANSIENT_TASK_END
    };
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_IPC_INCLUDE_IBACKGROUND_TASK_SUBSCRIBER_H