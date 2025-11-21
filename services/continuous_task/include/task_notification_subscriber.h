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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_NOTIFICATION_SUBSCRIBER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_NOTIFICATION_SUBSCRIBER_H

#include "notification_helper.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class BgContinuousTaskMgr;

class TaskNotificationSubscriber : public Notification::NotificationSubscriber {
public:
    TaskNotificationSubscriber();
    ~TaskNotificationSubscriber() override;
    void OnConnected() override;
    void OnDisconnected() override;
    void OnCanceled(const std::shared_ptr<Notification::Notification> &request,
        const std::shared_ptr<Notification::NotificationSortingMap> &sortingMap, int deleteReason) override;
    void OnConsumed(const std::shared_ptr<Notification::Notification> &request,
        const std::shared_ptr<Notification::NotificationSortingMap> &sortingMap) override;
    void OnUpdate(const std::shared_ptr<Notification::NotificationSortingMap> &sortingMap) override;
    void OnDied() override;
    void OnDoNotDisturbDateChange(
        const std::shared_ptr<Notification::NotificationDoNotDisturbDate> &date) override;
    void OnEnabledNotificationChanged(
        const std::shared_ptr<Notification::EnabledNotificationCallbackData> &callbackData) override;
    void OnBadgeChanged(const std::shared_ptr<Notification::BadgeNumberCallbackData> &badgeData) override;
    void OnBadgeEnabledChanged(const sptr<Notification::EnabledNotificationCallbackData> &callbackData) override;
    void OnBatchCanceled(const std::vector<std::shared_ptr<Notification::Notification>> &requestList,
        const std::shared_ptr<Notification::NotificationSortingMap> &sortingMap, int32_t deleteReason) override;
    void CancelNotificationByBgTask(const Notification::NotificationRequest &request, int32_t deleteReason);
    void GetLiveViewExtraInfo(const Notification::NotificationRequest &request,
        std::string &eventName, int32_t &capsuleStatus);
private:
    std::vector<std::string> StringSplit(const std::string &str, const char &delim);

private:
    static std::shared_ptr<BgContinuousTaskMgr> continuousTaskMgr_;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_NOTIFICATION_SUBSCRIBER_H
