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

#include "notification_tools.h"

#include <sstream>
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
#include "notification_helper.h"
#endif
#include "continuous_task_log.h"
#include "string_wrapper.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
constexpr char NOTIFICATION_PREFIX[] = "bgmode";
constexpr char SEPARATOR[] = "_";
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
constexpr int32_t DEFAULT_USERID = -2;
#endif
}
NotificationTools::NotificationTools() {}

NotificationTools::~NotificationTools() {}

std::string CreateNotificationLabel(int32_t uid, const std::string &bundleName,
    const std::string &abilityName)
{
    std::stringstream stream;
    stream.clear();
    stream.str("");
    stream << NOTIFICATION_PREFIX << SEPARATOR << uid << SEPARATOR << std::hash<std::string>()(abilityName);
    std::string label = stream.str();
    BGTASK_LOGD("notification label: %{public}s", label.c_str());
    return label;
}

ErrCode NotificationTools::PublishNotification(const std::shared_ptr<ContinuousTaskRecord> &continuousTaskRecord,
    const std::string &appName, const std::string &prompt, int32_t serviceUid)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    std::shared_ptr<Notification::NotificationNormalContent> normalContent
        = std::make_shared<Notification::NotificationNormalContent>();
    normalContent->SetTitle(appName);
    normalContent->SetText(prompt);

    Notification::NotificationRequest notificationRequest = Notification::NotificationRequest();

    std::shared_ptr<AAFwk::WantParams> extraInfo = std::make_shared<AAFwk::WantParams>();
    extraInfo->SetParam("abilityName", AAFwk::String::Box(continuousTaskRecord->abilityName_));

    std::string notificationLabel = CreateNotificationLabel(continuousTaskRecord->uid_,
        continuousTaskRecord->bundleName_, continuousTaskRecord->abilityName_);

    // set extraInfo to save abilityname Info.
    notificationRequest.SetAdditionalData(extraInfo);

    // set basic notification content
    notificationRequest.SetContent(std::make_shared<Notification::NotificationContent>(normalContent));

    // set wantagent param for click jump to target ability
    notificationRequest.SetWantAgent(continuousTaskRecord->wantAgent_);

    // set notification label distinguish different notification
    notificationRequest.SetLabel(notificationLabel);

    // set creator uid as background task manager service uid
    notificationRequest.SetCreatorUid(serviceUid);

    // set creator user id to -2 means to get all user's notification event callback.
    notificationRequest.SetCreatorUserId(DEFAULT_USERID);

    // set tapDismissed param to false make notification retained when clicked.
    notificationRequest.SetTapDismissed(false);

    if (Notification::NotificationHelper::PublishContinuousTaskNotification(notificationRequest) != ERR_OK) {
        BGTASK_LOGE("publish notification error");
        return ERR_BGTASK_NOTIFICATION_ERR;
    }
    continuousTaskRecord->notificationLabel_ = notificationLabel;
#endif
    return ERR_OK;
}

ErrCode NotificationTools::CancelNotification(const std::string &label, int32_t id)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    if (Notification::NotificationHelper::CancelContinuousTaskNotification(label, id) != ERR_OK) {
        return ERR_BGTASK_NOTIFICATION_ERR;
    }
#endif
    return ERR_OK;
}
void NotificationTools::GetAllActiveNotificationsLabels(std::set<std::string> &notificationLabels)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    std::vector<sptr<Notification::Notification>> notifications;
    Notification::NotificationHelper::GetAllActiveNotifications(notifications);
    for (auto &var : notifications) {
        notificationLabels.emplace(var->GetLabel());
    }
#endif
}

void NotificationTools::RefreshContinuousNotifications(
    const std::map<std::string, std::pair<std::string, std::string>> &newPromptInfos, int32_t serviceUid)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    std::vector<sptr<Notification::Notification>> notifications;
    Notification::NotificationHelper::GetAllActiveNotifications(notifications);
    for (auto &var : notifications) {
        Notification::NotificationRequest request = var->GetNotificationRequest();
        std::string label = var->GetLabel();
        if (newPromptInfos.count(label) == 0 || request.GetCreatorUid() != serviceUid) {
            continue;
        }
        auto &content = request.GetContent();
        auto const &normalContent = content->GetNotificationContent();
        normalContent->SetTitle(newPromptInfos.at(label).first);
        normalContent->SetText(newPromptInfos.at(label).second);
        if (Notification::NotificationHelper::PublishContinuousTaskNotification(request) != ERR_OK) {
            BGTASK_LOGE("refresh notification error");
        }
    }
#endif
}
}
}
