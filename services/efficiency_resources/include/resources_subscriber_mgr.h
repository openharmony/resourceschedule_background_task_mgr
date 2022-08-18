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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_RESOURCES_INCLUDE_RESOURCES_SUBSCRIBER_MGR_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_RESOURCES_INCLUDE_RESOURCES_SUBSCRIBER_MGR_H

#include <list>
#include <map>
#include <memory>
#include <algorithm>

#include "singleton.h"
#include "iremote_object.h"

#include "bgtaskmgr_inner_errors.h"
#include "ibackground_task_subscriber.h"

namespace OHOS {
namespace BackgroundTaskMgr {

class ObserverDeathRecipient;
enum class EfficiencyResourcesEventType: uint32_t {
    APP_RESOURCE_APPLY,
    APP_RESOURCE_RESET,
    RESOURCE_APPLY,
    RESOURCE_RESET,
};

class ResourcesSubscriberMgr : public std::enable_shared_from_this<ResourcesSubscriberMgr>  {
    DECLARE_DELAYED_SINGLETON(ResourcesSubscriberMgr);
public:
    ErrCode AddSubscriber(const sptr<IBackgroundTaskSubscriber> &subscriber);
    ErrCode RemoveSubscriber(const sptr<IBackgroundTaskSubscriber> &subscriber);
    void OnResourceChanged(const std::shared_ptr<ResourceCallbackInfo> &callbackInfo,
        EfficiencyResourcesEventType type);
    void HandleSubscriberDeath(const wptr<IRemoteObject>& remote);

private:
    std::mutex subscriberLock_;
    std::list<sptr<IBackgroundTaskSubscriber>> subscriberList_ {};
    sptr<ObserverDeathRecipient> deathRecipient_ {nullptr};
};

class ObserverDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    DISALLOW_COPY_AND_MOVE(ObserverDeathRecipient);
    explicit ObserverDeathRecipient();
    ~ObserverDeathRecipient() override;
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;

// private:
//     wptr<ResourcesSubscriberMgr> subscriberMgr_;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_RESOURCES_INCLUDE_RESOURCES_SUBSCRIBER_MGR_H