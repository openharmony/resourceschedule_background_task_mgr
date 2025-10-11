/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_SYSTEM_EVENT_OBSERVER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_SYSTEM_EVENT_OBSERVER_H

#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_subscribe_info.h"
#include "event_handler.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class BgContinuousTaskMgr;
class SystemEventObserver : public EventFwk::CommonEventSubscriber,
                            public std::enable_shared_from_this<SystemEventObserver> {
public:
    explicit SystemEventObserver(const EventFwk::CommonEventSubscribeInfo &subscribeInfo);
    ~SystemEventObserver() override = default;
    void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override;
    void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler);
    void SetBgContinuousTaskMgr(const std::shared_ptr<BgContinuousTaskMgr> &bgContinuousTaskMgr_);
    bool Subscribe();
    bool Unsubscribe();

private:
    void OnReceiveEventContinuousTask(const EventFwk::CommonEventData &eventData);
    void OnReceiveEventEfficiencyRes(const EventFwk::CommonEventData &eventData);
    void OnBannerNotificationActionButtonClick(const std::shared_ptr<AppExecFwk::EventHandler> &handler,
        const std::shared_ptr<BgContinuousTaskMgr> &bgContinuousTaskMgr,
        const EventFwk::CommonEventData &eventData);

private:
    std::weak_ptr<AppExecFwk::EventHandler> handler_;
    std::weak_ptr<BgContinuousTaskMgr> bgContinuousTaskMgr_;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_SYSTEM_EVENT_OBSERVER_H