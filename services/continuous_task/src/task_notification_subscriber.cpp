/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "task_notification_subscriber.h"

#include <sstream>

#include "bg_continuous_task_mgr.h"
#include "bgtask_hitrace_chain.h"
#include "continuous_task_log.h"
#include "string_wrapper.h"


namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr char LABEL_SPLITER = '_';
static constexpr char NOTIFICATION_PREFIX[] = "bgmode";
static constexpr uint32_t LABEL_BGMODE_PREFIX_POS = 0;
static constexpr uint32_t LABEL_APP_UID_POS = 1;
static constexpr uint32_t LABEL_SIZE = 5;
static constexpr uint32_t LABEL_ABILITYID_INDEX = 3;
static constexpr uint32_t LABEL_TASKID_INDEX = 4;
}

std::shared_ptr<BgContinuousTaskMgr> TaskNotificationSubscriber::continuousTaskMgr_
    = BgContinuousTaskMgr::GetInstance();

TaskNotificationSubscriber::TaskNotificationSubscriber() {}

TaskNotificationSubscriber::~TaskNotificationSubscriber() {}

void TaskNotificationSubscriber::OnConnected() {}

void TaskNotificationSubscriber::OnDisconnected() {}

void TaskNotificationSubscriber::OnCanceled(const std::shared_ptr<Notification::Notification> &notification,
    const std::shared_ptr<Notification::NotificationSortingMap> &sortingMap, int deleteReason)
{
    BgTaskHiTraceChain traceChain(__func__);
    if (notification == nullptr) {
        BGTASK_LOGW("notification param is null");
        return;
    }
    Notification::NotificationRequest request = notification->GetNotificationRequest();
    if (request.GetCreatorUid() != continuousTaskMgr_->GetBgTaskUid()) {
        return;
    }

    // continuous task notification label is consisted of bgmode prefix, app uid, abilityName hash code.
    std::string notificationLabel = request.GetLabel();
    std::vector<std::string> labelSplits = StringSplit(notificationLabel, LABEL_SPLITER);

    if (labelSplits.empty() || labelSplits[LABEL_BGMODE_PREFIX_POS] != NOTIFICATION_PREFIX
        || labelSplits.size() > LABEL_SIZE + 1 || labelSplits.size() < LABEL_SIZE - 1) {
        BGTASK_LOGW("callback notification label is invalid");
        return;
    }

    if (deleteReason == Notification::NotificationConstant::APP_CANCEL_REASON_DELETE) {
        BGTASK_LOGD("notification remove action is already triggered by cancel method.");
        return;
    }

    std::shared_ptr<AAFwk::WantParams> extraInfo = request.GetAdditionalData();
    if (extraInfo == nullptr) {
        BGTASK_LOGE("notification extraInfo is null");
        return;
    }
    BGTASK_LOGI("stop continuous task by user, the label is : %{public}s, the reason is %{public}d",
        notificationLabel.c_str(), deleteReason);

    std::string abilityName = AAFwk::String::Unbox(AAFwk::IString::Query(extraInfo->GetParam("abilityName")));
    std::string taskInfoMapKey = labelSplits[LABEL_APP_UID_POS] + LABEL_SPLITER + abilityName +
        LABEL_SPLITER + labelSplits[LABEL_ABILITYID_INDEX];
    if (labelSplits.size() == LABEL_SIZE || labelSplits.size() == LABEL_SIZE + 1) {
        taskInfoMapKey = taskInfoMapKey + LABEL_SPLITER + labelSplits[LABEL_TASKID_INDEX];
    }
    bool isSubNotification = false;
    if (labelSplits.size() == LABEL_SIZE + 1) {
        isSubNotification = true;
    }
    if (continuousTaskMgr_->StopContinuousTaskByUser(taskInfoMapKey, isSubNotification)) {
        BGTASK_LOGI("remove continuous task record Key: %{public}s", taskInfoMapKey.c_str());
    }
}

void TaskNotificationSubscriber::OnConsumed(const std::shared_ptr<Notification::Notification> &notification,
    const std::shared_ptr<Notification::NotificationSortingMap> &sortingMap) {}

void TaskNotificationSubscriber::OnUpdate(
    const std::shared_ptr<Notification::NotificationSortingMap> &sortingMap) {}

void TaskNotificationSubscriber::OnDied() {}

void TaskNotificationSubscriber::OnDoNotDisturbDateChange(
    const std::shared_ptr<Notification::NotificationDoNotDisturbDate> &date) {}

void TaskNotificationSubscriber::OnEnabledNotificationChanged(
    const std::shared_ptr<Notification::EnabledNotificationCallbackData> &callbackData) {}

void TaskNotificationSubscriber::OnBadgeChanged(
    const std::shared_ptr<Notification::BadgeNumberCallbackData> &badgeData) {}

void TaskNotificationSubscriber::OnBadgeEnabledChanged(
    const sptr<Notification::EnabledNotificationCallbackData> &callbackData) {}

void TaskNotificationSubscriber::OnBatchCanceled(const std::vector<std::shared_ptr<Notification::Notification>>
    &requestList, const std::shared_ptr<Notification::NotificationSortingMap> &sortingMap, int32_t deleteReason) {}

std::vector<std::string> TaskNotificationSubscriber::StringSplit(const std::string &str, const char &delim)
{
    std::vector<std::string> ret;
    std::stringstream ss(str);
    std::string tmp;
    while (getline(ss, tmp, delim)) {
        ret.push_back(tmp);
    }
    return ret;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
