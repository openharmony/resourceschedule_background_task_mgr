/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#include "want_agent_helper.h"
#endif
#include "continuous_task_log.h"
#include "string_wrapper.h"
#include "common_utils.h"
#include "int_wrapper.h"

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
constexpr int BACKGROUND_MODE_VOIP = 8;
constexpr int PUBLISH_DELAY_TIME = 3;
constexpr int TYPE_CODE_VOIP = 0;
constexpr int TYPE_CODE_AUDIO_RECORDING = 7;
constexpr int TYPE_CODE_DATA_TANSFER = 8;
constexpr int BANNER_NOTIFICATION_CONTROL_FLAG = 1 << 9;
#endif
}

int32_t NotificationTools::notificationIdIndex_ = -1;
NotificationTools::NotificationTools() {}

NotificationTools::~NotificationTools() {}

void NotificationTools::SetNotificationIdIndex(const int32_t id)
{
    notificationIdIndex_ = id;
}

std::string CreateNotificationLabel(int32_t uid, const std::string &abilityName, int32_t abilityId,
    bool isByRequestObject, const int32_t continuousTaskId)
{
    std::stringstream stream;
    stream.clear();
    stream.str("");
    stream << NOTIFICATION_PREFIX << SEPARATOR << uid << SEPARATOR << std::hash<std::string>()(abilityName) <<
        SEPARATOR << abilityId;
    if (isByRequestObject) {
        stream << SEPARATOR << continuousTaskId;
    }
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
    } else if (std::find(continuousTaskRecord->bgModeIds_.begin(), continuousTaskRecord->bgModeIds_.end(),
        BACKGROUND_MODE_VOIP) != continuousTaskRecord->bgModeIds_.end()) {
        liveContent->SetType(TYPE_CODE_VOIP);
    }

    std::shared_ptr<AAFwk::WantParams> extraInfo = std::make_shared<AAFwk::WantParams>();
    extraInfo->SetParam("abilityName", AAFwk::String::Box(continuousTaskRecord->abilityName_));

    std::string notificationLabel = CreateNotificationLabel(continuousTaskRecord->uid_,
        continuousTaskRecord->abilityName_, continuousTaskRecord->abilityId_,
        continuousTaskRecord->isByRequestObject_, continuousTaskRecord->continuousTaskId_);

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
    if (newPromptInfos.empty()) {
        BGTASK_LOGD("continuous task is not exist.");
        return;
    }
    std::vector<sptr<Notification::NotificationRequest>> notificationRequests;
    ErrCode ret = Notification::NotificationHelper::GetActiveNotifications(notificationRequests);
    if (ret != ERR_OK) {
        BGTASK_LOGE("get all active notification fail!");
        return;
    }
    for (const Notification::NotificationRequest *var : notificationRequests) {
        std::string label = var->GetLabel();
        if (newPromptInfos.count(label) == 0 || var->GetCreatorUid() != serviceUid) {
            continue;
        }
        auto &content = var->GetContent();
        auto const &normalContent = content->GetNotificationContent();
        normalContent->SetTitle(newPromptInfos.at(label).first);
        normalContent->SetText(newPromptInfos.at(label).second);
        if (Notification::NotificationHelper::PublishContinuousTaskNotification(*var) != ERR_OK) {
            BGTASK_LOGE("refresh notification error");
        }
    }
#endif
}

WEAK_FUNC ErrCode NotificationTools::RefreshContinuousNotificationWantAndContext(int32_t serviceUid,
    const std::map<std::string, std::pair<std::string, std::string>> &newPromptInfos,
    const std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord, bool updateContent)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    std::vector<sptr<Notification::NotificationRequest>> notificationRequests;
    ErrCode ret = Notification::NotificationHelper::GetActiveNotifications(notificationRequests);
    if (ret != ERR_OK) {
        BGTASK_LOGE("refresh task, get all active notification fail!");
        return ERR_BGTASK_CONTINUOUS_UPDATE_NOTIFICATION_FAIL;
    }
    for (Notification::NotificationRequest *var : notificationRequests) {
        if (!var) {
            BGTASK_LOGE("NotificationRequest is null!");
            return ERR_BGTASK_CONTINUOUS_UPDATE_NOTIFICATION_FAIL;
        }
        std::string label = var->GetLabel();
        if (newPromptInfos.count(label) == 0 || var->GetCreatorUid() != serviceUid) {
            continue;
        }
        var->SetWantAgent(continuousTaskRecord->GetWantAgent());
        if (updateContent) {
            auto &content = var->GetContent();
            if (!content) {
                BGTASK_LOGE("content is null!");
                return ERR_BGTASK_CONTINUOUS_UPDATE_NOTIFICATION_FAIL;
            }
            auto const &normalContent = content->GetNotificationContent();
            if (!normalContent) {
                BGTASK_LOGE("normalContent is null!");
                return ERR_BGTASK_CONTINUOUS_UPDATE_NOTIFICATION_FAIL;
            }
            normalContent->SetTitle(newPromptInfos.at(label).first);
            normalContent->SetText(newPromptInfos.at(label).second);
        }
        ret = Notification::NotificationHelper::PublishContinuousTaskNotification(*var);
        if (ret != ERR_OK) {
            BGTASK_LOGE("refresh notification error, label: %{public}s", label.c_str());
            return ERR_BGTASK_CONTINUOUS_UPDATE_NOTIFICATION_FAIL;
        }
    }
#endif
    return ERR_OK;
}

std::string CreateSubNotificationLabel(int32_t uid, const std::string &abilityName, int32_t abilityId,
    const int32_t continuousTaskId, int32_t pid)
{
    std::stringstream stream;
    stream.clear();
    stream.str("");
    stream << NOTIFICATION_PREFIX << SEPARATOR << uid << SEPARATOR << std::hash<std::string>()(abilityName) <<
        SEPARATOR << abilityId << SEPARATOR << continuousTaskId << SEPARATOR << pid;
    std::string label = stream.str();
    BGTASK_LOGD("sub notification label: %{public}s", label.c_str());
    return label;
}

WEAK_FUNC ErrCode NotificationTools::PublishMainNotification(const std::shared_ptr<ContinuousTaskRecord> subRecord,
    const std::string &appName, const std::string &prompt, int32_t serviceUid,
    std::shared_ptr<ContinuousTaskRecord> mainRecord)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    std::shared_ptr<Notification::NotificationLocalLiveViewContent> liveContent =
        std::make_shared<Notification::NotificationLocalLiveViewContent>();
    liveContent->SetTitle(appName);
    liveContent->SetText(prompt);
    liveContent->SetType(TYPE_CODE_DATA_TANSFER);

    std::shared_ptr<AAFwk::WantParams> extraInfo = std::make_shared<AAFwk::WantParams>();
    extraInfo->SetParam("abilityName", AAFwk::String::Box(subRecord->abilityName_));

    std::string notificationLabel = CreateNotificationLabel(subRecord->uid_,
        subRecord->abilityName_, subRecord->abilityId_,
        subRecord->isByRequestObject_, subRecord->continuousTaskId_);

    Notification::NotificationRequest notificationRequest = Notification::NotificationRequest();
    notificationRequest.SetContent(std::make_shared<Notification::NotificationContent>(liveContent));
    notificationRequest.SetAdditionalData(extraInfo);
    notificationRequest.SetWantAgent(subRecord->wantAgent_);
    notificationRequest.SetCreatorUid(serviceUid);
    notificationRequest.SetOwnerUid(subRecord->GetUid());
    notificationRequest.SetInProgress(true);
    notificationRequest.SetIsAgentNotification(true);
    notificationRequest.SetPublishDelayTime(PUBLISH_DELAY_TIME);
    notificationRequest.SetUpdateByOwnerAllowed(true);
    notificationRequest.SetSlotType(Notification::NotificationConstant::SlotType::LIVE_VIEW);
    notificationRequest.SetLabel(notificationLabel);
    if (mainRecord->GetNotificationId() == -1) {
        notificationRequest.SetNotificationId(++notificationIdIndex_);
    } else {
        notificationRequest.SetNotificationId(mainRecord->GetNotificationId());
    }
    if (Notification::NotificationHelper::PublishNotification(notificationRequest) != ERR_OK) {
        BGTASK_LOGE("publish notification error, %{public}s, %{public}d", notificationLabel.c_str(),
            mainRecord->notificationId_);
        return ERR_BGTASK_NOTIFICATION_ERR;
    }
    mainRecord->notificationLabel_ = notificationLabel;
    if (mainRecord->GetNotificationId() == -1) {
        mainRecord->notificationId_ = notificationIdIndex_;
    }
#endif
    return ERR_OK;
}

WEAK_FUNC ErrCode NotificationTools::PublishSubNotification(const std::shared_ptr<ContinuousTaskRecord> subRecord,
    const std::string &appName, const std::string &prompt, int32_t serviceUid,
    std::shared_ptr<ContinuousTaskRecord> mainRecord)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    std::shared_ptr<Notification::NotificationLocalLiveViewContent> liveContent
        = std::make_shared<Notification::NotificationLocalLiveViewContent>();
    liveContent->SetTitle(appName);
    liveContent->SetText(prompt);
    if (std::find(subRecord->bgModeIds_.begin(), subRecord->bgModeIds_.end(),
        BACKGROUND_MODE_AUDIO_RECORDING) != subRecord->bgModeIds_.end()) {
        liveContent->SetType(TYPE_CODE_AUDIO_RECORDING);
    } else if (std::find(subRecord->bgModeIds_.begin(), subRecord->bgModeIds_.end(),
        BACKGROUND_MODE_VOIP) != subRecord->bgModeIds_.end()) {
        liveContent->SetType(TYPE_CODE_VOIP);
    }

    std::shared_ptr<AAFwk::WantParams> extraInfo = std::make_shared<AAFwk::WantParams>();
    extraInfo->SetParam("abilityName", AAFwk::String::Box(subRecord->abilityName_));

    std::string notificationLabel = CreateSubNotificationLabel(subRecord->uid_,
        subRecord->abilityName_, subRecord->abilityId_,
        subRecord->continuousTaskId_, subRecord->pid_);

    Notification::NotificationRequest notificationRequest = Notification::NotificationRequest();
    notificationRequest.SetContent(std::make_shared<Notification::NotificationContent>(liveContent));
    notificationRequest.SetAdditionalData(extraInfo);
    notificationRequest.SetWantAgent(subRecord->wantAgent_);
    notificationRequest.SetCreatorUid(serviceUid);
    notificationRequest.SetOwnerUid(subRecord->GetUid());
    notificationRequest.SetInProgress(true);
    notificationRequest.SetIsAgentNotification(true);
    notificationRequest.SetPublishDelayTime(PUBLISH_DELAY_TIME);
    notificationRequest.SetUpdateByOwnerAllowed(true);
    notificationRequest.SetSlotType(Notification::NotificationConstant::SlotType::LIVE_VIEW);
    notificationRequest.SetLabel(notificationLabel);
    if (mainRecord->GetSubNotificationId() == -1) {
        notificationRequest.SetNotificationId(++notificationIdIndex_);
    } else {
        notificationRequest.SetNotificationId(mainRecord->GetSubNotificationId());
    }
    if (Notification::NotificationHelper::PublishNotification(notificationRequest) != ERR_OK) {
        BGTASK_LOGE("publish notification error, %{public}s, %{public}d", notificationLabel.c_str(),
            notificationIdIndex_);
        return ERR_BGTASK_NOTIFICATION_ERR;
    }
    mainRecord->subNotificationLabel_ = notificationLabel;
    if (mainRecord->GetSubNotificationId() == -1) {
        mainRecord->subNotificationId_ = notificationIdIndex_;
    }
#endif
    return ERR_OK;
}

std::shared_ptr<Notification::NotificationNormalContent> CreateNotificationNormalContent(
    const std::string &appName, const std::string &prompt)
{
    std::shared_ptr<Notification::NotificationNormalContent> normalContent =
        std::make_shared<Notification::NotificationNormalContent>();
    normalContent->SetTitle(appName);
    normalContent->SetText(prompt);
    return normalContent;
}

bool SetActionButton(const std::shared_ptr<BannerNotificationRecord> &bannerNotification,
    const std::string& buttonName, Notification::NotificationRequest& notificationRequest,
    const int32_t btnTypeValue, const std::string &label)
{
    auto want = std::make_shared<AAFwk::Want>();
    want->SetAction(BGTASK_BANNER_NOTIFICATION_ACTION_NAME);

    std::shared_ptr<OHOS::AAFwk::WantParams> wantParams = std::make_shared<OHOS::AAFwk::WantParams>();
    wantParams->SetParam(BGTASK_BANNER_NOTIFICATION_ACTION_PARAM_BTN, AAFwk::Integer::Box(btnTypeValue));
    int32_t uid = bannerNotification->GetUid();
    wantParams->SetParam(BGTASK_BANNER_NOTIFICATION_ACTION_PARAM_UID, AAFwk::Integer::Box(uid));
    wantParams->SetParam(BGTASK_BANNER_NOTIFICATION_ACTION_LABEL, AAFwk::String::Box(label));

    want->SetParams(*wantParams);
    std::vector<std::shared_ptr<AAFwk::Want>> wants;
    wants.push_back(want);
 
    std::vector<AbilityRuntime::WantAgent::WantAgentConstant::Flags> flags;
    flags.push_back(AbilityRuntime::WantAgent::WantAgentConstant::Flags::CONSTANT_FLAG);
 
    AbilityRuntime::WantAgent::WantAgentInfo wantAgentInfo(
        0, AbilityRuntime::WantAgent::WantAgentConstant::OperationType::SEND_COMMON_EVENT,
        flags, wants, wantParams
    );
    auto wantAgentDeal = AbilityRuntime::WantAgent::WantAgentHelper::GetWantAgent(wantAgentInfo);
    std::shared_ptr<Notification::NotificationActionButton> actionButtonDeal =
        Notification::NotificationActionButton::Create(nullptr, buttonName, wantAgentDeal);
    if (actionButtonDeal == nullptr) {
        BGTASK_LOGE("get notification actionButton nullptr");
        return false;
    }
    notificationRequest.AddActionButton(actionButtonDeal);
    return true;
}

std::string NotificationTools::CreateBannerNotificationLabel(const std::string &bundleName, int32_t userId,
    int32_t appIndex)
{
    std::stringstream stream;
    stream.clear();
    stream.str("");
    stream << BANNER_NOTIFICATION_PREFIX << SEPARATOR << userId << SEPARATOR << appIndex << SEPARATOR << bundleName;
    std::string label = stream.str();
    BGTASK_LOGD("banner notification label: %{public}s", label.c_str());
    return label;
}

WEAK_FUNC ErrCode NotificationTools::PublishBannerNotification(
    std::shared_ptr<BannerNotificationRecord> bannerNotification, const std::string &prompt, int32_t serviceUid,
    const std::vector<std::string> &bannerNotificaitonBtn)
{
    if (BGTASK_BANNER_NOTIFICATION_BTN_ALLOW_TIME >= bannerNotificaitonBtn.size() ||
        BGTASK_BANNER_NOTIFICATION_BTN_ALLOW_ALLOWED >= bannerNotificaitonBtn.size()) {
        return ERR_BGTASK_NOTIFICATION_ERR;
    }
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    Notification::NotificationRequest notificationRequest;
    notificationRequest.SetCreatorUid(serviceUid);
    int32_t pid = getpid();
    notificationRequest.SetCreatorPid(pid);
    int32_t userId = bannerNotification->GetUserId();
    notificationRequest.SetCreatorUserId(userId);
    notificationRequest.SetTapDismissed(false);
    notificationRequest.SetSlotType(OHOS::Notification::NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    notificationRequest.SetOwnerUid(bannerNotification->GetUid());
    notificationRequest.SetNotificationId(++notificationIdIndex_);
    notificationRequest.SetNotificationControlFlags(BANNER_NOTIFICATION_CONTROL_FLAG);
    notificationRequest.SetBadgeIconStyle(Notification::NotificationRequest::BadgeStyle::LITTLE);
    notificationRequest.SetInProgress(true);
    std::string bannerNotificationLabel = CreateBannerNotificationLabel(bannerNotification->GetBundleName(),
        userId, bannerNotification->GetAppIndex());
    notificationRequest.SetLabel(bannerNotificationLabel);
    std::string appName = bannerNotification->GetAppName();
    std::shared_ptr<Notification::NotificationContent> content =
        std::make_shared<Notification::NotificationContent>(CreateNotificationNormalContent(appName, prompt));
    notificationRequest.SetContent(content);

    std::string allowTimeBtnName = bannerNotificaitonBtn.at(BGTASK_BANNER_NOTIFICATION_BTN_ALLOW_TIME);
    if (!SetActionButton(bannerNotification, allowTimeBtnName, notificationRequest,
        BGTASK_BANNER_NOTIFICATION_BTN_ALLOW_TIME, bannerNotificationLabel)) {
        BGTASK_LOGE("create banner notification action button fail");
        return ERR_BGTASK_NOTIFICATION_ERR;
    }
    std::string allowAllowed = bannerNotificaitonBtn.at(BGTASK_BANNER_NOTIFICATION_BTN_ALLOW_ALLOWED);
    if (!SetActionButton(bannerNotification, allowAllowed, notificationRequest,
        BGTASK_BANNER_NOTIFICATION_BTN_ALLOW_ALLOWED, bannerNotificationLabel)) {
        BGTASK_LOGE("create banner notification action button fail");
        return ERR_BGTASK_NOTIFICATION_ERR;
    }
    ErrCode ret = Notification::NotificationHelper::PublishNotification(notificationRequest);
    if (ret != ERR_OK) {
        BGTASK_LOGE("publish banner notificaiton, errcode: %{public}d", ret);
        return ERR_BGTASK_NOTIFICATION_ERR;
    }
    bannerNotification->SetNotificationId(notificationIdIndex_);
    bannerNotification->SetNotificationLabel(bannerNotificationLabel);
#endif
    return ERR_OK;
}

WEAK_FUNC void NotificationTools::RefreshBannerNotifications(const std::vector<std::string> &bannerNotificaitonBtn,
    const std::map<std::string, std::pair<std::string, std::string>> &newPromptInfos,
    const std::shared_ptr<BannerNotificationRecord> bannerNotification, int32_t serviceUid)
{
    if (BGTASK_BANNER_NOTIFICATION_BTN_ALLOW_TIME >= bannerNotificaitonBtn.size() ||
        BGTASK_BANNER_NOTIFICATION_BTN_ALLOW_ALLOWED >= bannerNotificaitonBtn.size()) {
        BGTASK_LOGE("bannerNotificaitonBtn index fail.");
        return;
    }
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    std::vector<sptr<Notification::NotificationRequest>> notificationRequests;
    ErrCode ret = Notification::NotificationHelper::GetActiveNotifications(notificationRequests);
    if (ret != ERR_OK) {
        BGTASK_LOGE("get all active notification fail!");
        return;
    }
    for (Notification::NotificationRequest *var : notificationRequests) {
        std::string label = var->GetLabel();
        if (newPromptInfos.count(label) == 0 || var->GetCreatorUid() != serviceUid) {
            continue;
        }
        auto &content = var->GetContent();
        auto const &normalContent = content->GetNotificationContent();
        normalContent->SetTitle(newPromptInfos.at(label).first);
        normalContent->SetText(newPromptInfos.at(label).second);
        var->ClearActionButtons();
        std::string allowTimeBtnName = bannerNotificaitonBtn.at(BGTASK_BANNER_NOTIFICATION_BTN_ALLOW_TIME);
        if (!SetActionButton(bannerNotification, allowTimeBtnName, *var,
            BGTASK_BANNER_NOTIFICATION_BTN_ALLOW_TIME, label)) {
            BGTASK_LOGE("create banner notification action button fail");
            return;
        }
        std::string allowAllowed = bannerNotificaitonBtn.at(BGTASK_BANNER_NOTIFICATION_BTN_ALLOW_ALLOWED);
        if (!SetActionButton(bannerNotification, allowAllowed, *var,
            BGTASK_BANNER_NOTIFICATION_BTN_ALLOW_ALLOWED, label)) {
            BGTASK_LOGE("create banner notification action button fail");
            return;
        }
        if (Notification::NotificationHelper::PublishNotification(*var) != ERR_OK) {
            BGTASK_LOGE("refresh notification error");
        }
    }
#endif
}
}
}
