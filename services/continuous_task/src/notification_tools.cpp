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

#ifdef BGTASK_MGR_UNIT_TEST
#define WEAK_FUNC __attribute__((weak))
#else
#define WEAK_FUNC
#endif

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
constexpr char NOTIFICATION_PREFIX[] = "bgmode";
constexpr char SEPARATOR[] = "_";
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
constexpr int BACKGROUND_MODE_DATA_TANSFER = 1;
constexpr int BACKGROUND_MODE_AUDIO_RECORDING = 3;
constexpr int PUBLISH_DELAY_TIME = 3;
constexpr int TYPE_CODE_AUDIO_RECORDING = 7;
constexpr int TYPE_CODE_DATA_TANSFER = 8;
#endif
}

int32_t NotificationTools::notificationIdIndex_ = -1;
NotificationTools::NotificationTools() {}

NotificationTools::~NotificationTools() {}

void NotificationTools::SetNotificationIdIndex(const int32_t id)
{
    notificationIdIndex_ = id;
}

std::string CreateNotificationLabel(int32_t uid, const std::string &bundleName,
    const std::string &abilityName, int32_t abilityId)
{
    std::stringstream stream;
    stream.clear();
    stream.str("");
    stream << NOTIFICATION_PREFIX << SEPARATOR << uid << SEPARATOR << std::hash<std::string>()(abilityName)
        << SEPARATOR << abilityId;
    std::string label = stream.str();
    BGTASK_LOGD("notification label: %{public}s", label.c_str());
    return label;
}

WEAK_FUNC ErrCode NotificationTools::PublishNotification(
    const std::shared_ptr<ContinuousTaskRecord> &continuousTaskRecord,
    const std::string &appName, const std::string &prompt, int32_t serviceUid)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    std::shared_ptr<Notification::NotificationLocalLiveViewContent> liveContent
        = std::make_shared<Notification::NotificationLocalLiveViewContent>();
    liveContent->SetTitle(appName);
    liveContent->SetText(prompt);
    if (std::find(continuousTaskRecord->bgModeIds_.begin(), continuousTaskRecord->bgModeIds_.end(),
        BACKGROUND_MODE_DATA_TANSFER) != continuousTaskRecord->bgModeIds_.end()) {
        liveContent->SetType(TYPE_CODE_DATA_TANSFER);
    } else if (std::find(continuousTaskRecord->bgModeIds_.begin(), continuousTaskRecord->bgModeIds_.end(),
        BACKGROUND_MODE_AUDIO_RECORDING) != continuousTaskRecord->bgModeIds_.end()) {
        liveContent->SetType(TYPE_CODE_AUDIO_RECORDING);
    }

    std::shared_ptr<AAFwk::WantParams> extraInfo = std::make_shared<AAFwk::WantParams>();
    extraInfo->SetParam("abilityName", AAFwk::String::Box(continuousTaskRecord->abilityName_));

    std::string notificationLabel = CreateNotificationLabel(continuousTaskRecord->uid_,
        continuousTaskRecord->bundleName_, continuousTaskRecord->abilityName_,
        continuousTaskRecord->abilityId_);

    Notification::NotificationRequest notificationRequest = Notification::NotificationRequest();
    notificationRequest.SetContent(std::make_shared<Notification::NotificationContent>(liveContent));
    notificationRequest.SetAdditionalData(extraInfo);
    notificationRequest.SetWantAgent(continuousTaskRecord->wantAgent_);
    notificationRequest.SetCreatorUid(serviceUid);
    notificationRequest.SetOwnerUid(continuousTaskRecord->GetUid());
    notificationRequest.SetInProgress(true);
    notificationRequest.SetIsAgentNotification(true);
    notificationRequest.SetPublishDelayTime(PUBLISH_DELAY_TIME);
    notificationRequest.SetUpdateByOwnerAllowed(true);
    notificationRequest.SetSlotType(Notification::NotificationConstant::SlotType::LIVE_VIEW);
    notificationRequest.SetLabel(notificationLabel);
    if (continuousTaskRecord->GetNotificationId() == -1) {
        notificationRequest.SetNotificationId(++notificationIdIndex_);
    } else {
        notificationRequest.SetNotificationId(continuousTaskRecord->GetNotificationId());
    }
    if (Notification::NotificationHelper::PublishNotification(notificationRequest) != ERR_OK) {
        BGTASK_LOGE("publish notification error, %{public}s, %{public}d", notificationLabel.c_str(),
            continuousTaskRecord->notificationId_);
        return ERR_BGTASK_NOTIFICATION_ERR;
    }
    continuousTaskRecord->notificationLabel_ = notificationLabel;
    if (continuousTaskRecord->GetNotificationId() == -1) {
        continuousTaskRecord->notificationId_ = notificationIdIndex_;
    }
#endif
    return ERR_OK;
}

WEAK_FUNC ErrCode NotificationTools::CancelNotification(const std::string &label, int32_t id)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    if (Notification::NotificationHelper::CancelNotification(label, id) != ERR_OK) {
        BGTASK_LOGE("CancelNotification error label %{public}s, id %{public}d", label.c_str(), id);
        return ERR_BGTASK_NOTIFICATION_ERR;
    }
#endif
    return ERR_OK;
}

WEAK_FUNC void NotificationTools::GetAllActiveNotificationsLabels(std::set<std::string> &notificationLabels)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    std::vector<sptr<Notification::Notification>> notifications;
    ErrCode ret = Notification::NotificationHelper::GetAllActiveNotifications(notifications);
    if (ret != ERR_OK) {
        BGTASK_LOGE("get all active notification fail!");
        return;
    }
    for (auto &var : notifications) {
        notificationLabels.emplace(var->GetLabel());
    }
#endif
}

WEAK_FUNC void NotificationTools::RefreshContinuousNotifications(
    const std::map<std::string, std::pair<std::string, std::string>> &newPromptInfos, int32_t serviceUid)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    std::vector<sptr<Notification::Notification>> notifications;
    ErrCode ret = Notification::NotificationHelper::GetAllActiveNotifications(notifications);
    if (ret != ERR_OK) {
        BGTASK_LOGE("get all active notification fail!");
        return;
    }
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
