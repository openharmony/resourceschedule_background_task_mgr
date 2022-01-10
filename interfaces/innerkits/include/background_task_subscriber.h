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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_INTERFACES_INNERKITS_TRANSIENT_TASK_INCLUDE_TRANSIENT_TASK_SUBSCRIBER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_INTERFACES_INNERKITS_TRANSIENT_TASK_INCLUDE_TRANSIENT_TASK_SUBSCRIBER_H

#include <iremote_broker.h>

#include "ibackground_task_mgr.h"
#include "background_task_subscriber_stub.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundTaskSubscriber {
public:
    /**
     * Default constructor used to create a instance.
     */
    BackgroundTaskSubscriber();

    /**
     * Default destructor.
     */
    virtual ~BackgroundTaskSubscriber();

    /**
     * Called back when a transient task start.
     *
     * @param info Transient task app info.
     **/
    virtual void OnTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info) = 0;

    /**
     * Called back when a transient task end.
     *
     * @param info Transient task app info.
     **/
    virtual void OnTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) = 0;

private:
    class BackgroundTaskSubscriberImpl final : public BackgroundTaskSubscriberStub {
    public:
        BackgroundTaskSubscriberImpl(BackgroundTaskSubscriber &subscriber);
        ~BackgroundTaskSubscriberImpl(){};
        void OnTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info) override;
        void OnTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) override;

    public:
        BackgroundTaskSubscriber &subscriber_;
    };

private:
    const sptr<BackgroundTaskSubscriberImpl> GetImpl() const;

private:
    sptr<BackgroundTaskSubscriberImpl> impl_ = nullptr;

    friend class BackgroundTaskManager;
};

} // namespace BackgroundTaskMgr
} // namespace OHOS
#endif