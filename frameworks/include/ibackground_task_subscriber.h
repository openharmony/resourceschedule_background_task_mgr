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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_IBACKGROUND_TASK_SUBSCRIBER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_IBACKGROUND_TASK_SUBSCRIBER_H

#include <ipc_types.h>
#include <iremote_broker.h>
#include <nocopyable.h>

#include "continuous_task_callback_info.h"
#include "transient_task_app_info.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class IBackgroundTaskSubscriber : public IRemoteBroker {
public:
    IBackgroundTaskSubscriber() = default;
    ~IBackgroundTaskSubscriber() override = default;
    DISALLOW_COPY_AND_MOVE(IBackgroundTaskSubscriber);

    /**
     * @brief Called back when the subscriber is connected to Background Task Manager Service.
     */
    virtual void OnConnected() = 0;

    /**
     * @brief Called back when the subscriber is disconnected from Background Task Manager Service.
     */
    virtual void OnDisconnected() = 0;

    /**
     * @brief Called back when a transient task start.
     * 
     * @param info Transient task app info.
     */
    virtual void OnTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info) = 0;

    /**
     * @brief Called back when a transient task end.
     * 
     * @param info Transient task app info.
     */
    virtual void OnTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) = 0;

    /**
     * @brief Called back when a continuous task start.
     * 
     * @param continuousTaskCallbackInfo Continuous task app info.
     */
    virtual void OnContinuousTaskStart(
        const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) = 0;

    /**
     * @brief Called back when a continuous task stop.
     * 
     * @param continuousTaskCallbackInfo Continuous task app info.
     */
    virtual void OnContinuousTaskStop(
        const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) = 0;
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.resourceschedule.IBackgroundTaskSubscriber");

protected:
    enum InterfaceId : uint32_t {
        ON_CONNECTED = FIRST_CALL_TRANSACTION,
        ON_DISCONNECTED,
        ON_TRANSIENT_TASK_START,
        ON_TRANSIENT_TASK_END,
        ON_CONTINUOUS_TASK_START,
        ON_CONTINUOUS_TASK_STOP,
    };
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_IBACKGROUND_TASK_SUBSCRIBER_H