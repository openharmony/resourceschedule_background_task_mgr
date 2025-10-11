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

#include "banner_notification_record.h"

namespace OHOS {
namespace BackgroundTaskMgr {
BannerNotificationRecord::BannerNotificationRecord(const std::string &bundleName, int32_t uid, int32_t notificationId,
    const std::string &notificationLabel, const std::string &appName)
    : bundleName_(bundleName), uid_(uid), notificationId_(notificationId), notificationLabel_(notificationLabel),
    appName_(appName) {}

std::string BannerNotificationRecord::GetBundleName() const
{
    return bundleName_;
}

int32_t BannerNotificationRecord::GetUid() const
{
    return uid_;
}

int32_t BannerNotificationRecord::GetNotificationId() const
{
    return notificationId_;
}

std::string BannerNotificationRecord::GetNotificationLabel() const
{
    return notificationLabel_;
}
    
std::string BannerNotificationRecord::GetAppName() const
{
    return appName_;
}

int32_t BannerNotificationRecord::GetAuthResult() const
{
    return authResult_;
}

int32_t BannerNotificationRecord::GetUserId() const
{
    return userId_;
}

int32_t BannerNotificationRecord::GetAppIndex() const
{
    return appIndex_;
}

void BannerNotificationRecord::SetBundleName(const std::string &bundleName)
{
    bundleName_ = bundleName;
}

void BannerNotificationRecord::SetNotificationLabel(const std::string &notificationLabel)
{
    notificationLabel_ = notificationLabel;
}

void BannerNotificationRecord::SetAppName(const std::string &appName)
{
    appName_ = appName;
}

void BannerNotificationRecord::SetUid(int32_t uid)
{
    uid_ = uid;
}

void BannerNotificationRecord::SetNotificationId(int32_t notificationId)
{
    notificationId_ = notificationId;
}

void BannerNotificationRecord::SetAuthResult(int32_t authResult)
{
    authResult_ = authResult;
}

void BannerNotificationRecord::SetUserId(int32_t userId)
{
    userId_ = userId;
}

void BannerNotificationRecord::SetAppIndex(int32_t appIndex)
{
    appIndex_ = appIndex;
}

static const char *g_taskPromptResNamesSubMode[] = {
    "ohos_bgsubmode_prompt_car_key",
    "ohos_bgsubmode_prompt_media_process",
    "ohos_bgsubmode_prompt_video_broadcast",
};

static const char *g_btnBannerNotification[] = {
    "btn_allow_time",
    "btn_allow_allowed",
};

static const char *g_textBannerNotification[] = {
    "text_notification_allow_background_activity",
};

if (recordedBgMode == BGMODE_SPECIAL_SCENARIO_PROCESSING) {
            ErrCode ret = AllowUseSpecial(continuousTaskRecord);
            if (ret != ERR_OK) {
                return ret;
            }
        }


ErrCode BgContinuousTaskMgr::AllowUseSpecial(const std::shared_ptr<ContinuousTaskRecord> record)
{
    if (!BundleManagerHelper::GetInstance()->CheckACLPermission(BGMODE_PERMISSION_SYSTEM, record->callingTokenId_)) {
        return ERR_BGTASK_CONTINUOUS_APP_NOT_HAVE_BGMODE_PERMISSION_SYSTEM;
    }
    if (record->isSystem_) {
        return ERR_BGTASK_CONTINUOUS_SYSTEM_APP_NOT_SUPPORT_ACL;
    }
    AppExecFwk::BundleInfo bundleInfo;
    if (BundleManagerHelper::GetInstance()->GetBundleInfo(record->bundleName_,
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, record->userId_)) {
        record->appIndex_ = bundleInfo.appIndex;
    } else {
        return ERR_BGTASK_GET_APP_INDEX_FAIL;
    }
    return ERR_OK;
}

ErrCode BgContinuousTaskMgr::CheckNotificationText(std::string &notificationText,
    const std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord)
{
    auto iter = avSessionNotification_.find(continuousTaskRecord->uid_);
    bool isPublish = (iter != avSessionNotification_.end()) ? iter->second : false;
    BGTASK_LOGD("AVSession Notification isPublish: %{public}d", isPublish);
    // 子类型带有avsession,不发通知
    bool isPublishAvsession = isPublish || (CommonUtils::CheckExistMode(continuousTaskRecord->bgSubModeIds_,
        BackgroundTaskSubmode::SUBMODE_AVSESSION_AUDIO_PLAYBACK) && continuousTaskRecord->isByRequestObject_);
    for (auto mode : continuousTaskRecord->bgModeIds_) {
        if ((mode == BackgroundMode::AUDIO_PLAYBACK && isPublishAvsession) || ((mode == BackgroundMode::VOIP ||
            mode == BackgroundMode::AUDIO_RECORDING) && continuousTaskRecord->IsSystem())) {
            continue;
        }
        BGTASK_LOGD("mode %{public}d", mode);
        if (mode == BackgroundMode::SPECIAL_SCENARIO_PROCESSING || mode == BackgroundMode::BLUETOOTH_INTERACTION) {
            ErrCode ret = CheckSpecialNotificationText(notificationText, continuousTaskRecord, mode);
            if (ret != ERR_OK) {
                BGTASK_LOGE("check special notification fail.");
                return ret;
            }
            continue;
        }
        uint32_t index = GetBgModeNameIndex(mode, continuousTaskRecord->isNewApi_);
        if (index < continuousTaskText_.size()) {
            notificationText += continuousTaskText_.at(index);
            notificationText += "\n";
        } else {
            BGTASK_LOGI("index is invalid");
            return ERR_BGTASK_NOTIFICATION_VERIFY_FAILED;
        }
    }
    return ERR_OK;
}

ErrCode BgContinuousTaskMgr::CheckSpecialNotificationText(std::string &notificationText,
    const std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord, uint32_t mode)
{
    if (continuousTaskSubText_.empty()) {
        BGTASK_LOGE("get subMode notification prompt info failed, continuousTaskSubText_ is empty");
        return ERR_BGTASK_NOTIFICATION_VERIFY_FAILED;
    }
    if (mode == BackgroundMode::BLUETOOTH_INTERACTION &&
        CommonUtils::CheckExistMode(continuousTaskRecord->bgSubModeIds_, BackgroundSubMode::CAR_KEY)) {
        uint32_t index = BackgroundSubMode::CAR_KEY - 1;
        if (index < continuousTaskSubText_.size()) {
            notificationText += continuousTaskSubText_.at(index);
            notificationText += "\n";
            return ERR_OK;
        }
    }
    if (CommonUtils::CheckExistMode(continuousTaskRecord->bgSubModeIds_,
        BackgroundTaskSubmode::SUBMODE_MEDIA_PROCESS_NORMAL_NOTIFICATION)) {
        if (NOTIFICATION_TEXT_MEDIA_PROCESS_INDEX < continuousTaskSubText_.size()) {
            notificationText += continuousTaskSubText_.at(NOTIFICATION_TEXT_MEDIA_PROCESS_INDEX);
            notificationText += "\n";
            return ERR_OK;
        }
    }
    if (CommonUtils::CheckExistMode(continuousTaskRecord->bgSubModeIds_,
        BackgroundTaskSubmode::SUBMODE_VIDEO_BROADCAST_NORMAL_NOTIFICATION)) {
        if (NOTIFICATION_TEXT_VIDEO_BROADCAST_INDEX < continuousTaskSubText_.size()) {
            notificationText += continuousTaskSubText_.at(NOTIFICATION_TEXT_VIDEO_BROADCAST_INDEX);
            notificationText += "\n";
            return ERR_OK;
        }
    }
    BGTASK_LOGE("sub index is invalid");
    return ERR_BGTASK_NOTIFICATION_VERIFY_FAILED;
}

std::string btnText {""};
    for (std::string name : g_btnBannerNotification) {
        resourceManager->GetStringByName(name.c_str(), btnText);
        if (btnText.empty()) {
            BGTASK_LOGE("get banner notification btn text failed!");
            return false;
        }
        BGTASK_LOGI("get btn text: %{public}s", btnText.c_str());
        bannerNotificaitonBtn_.push_back(btnText);
    }

// 语言切换，刷新横幅通知
    std::map<std::string, std::pair<std::string, std::string>> newBannerPromptInfos;
    for (const auto &iter : bannerNotificationRecord_) {
        std::string bannerNotificationText {""};
        std::string appName = iter.second->GetAppName();
        int32_t uid = iter.second->GetUid();
        if (!FormatBannerNotificationContext(appName, bannerNotificationText)) {
            BGTASK_LOGE("get banner notification text fail, uid: %{public}d", uid);
            continue;
        }
        std::string bannerNotificationLabel = iter.second->GetNotificationLabel();
        BGTASK_LOGI("bannerNotificationLabel: %{public}s, mainAbilityLabel: %{public}s, "
            "notificationText: %{public}s,", bannerNotificationLabel.c_str(), appName.c_str(),
            bannerNotificationText.c_str());
        newBannerPromptInfos.emplace(bannerNotificationLabel, std::make_pair(appName, bannerNotificationText));
    }
    NotificationTools::GetInstance()->RefreshBannerNotifications(newBannerPromptInfos, bgTaskUid_);

ErrCode BgContinuousTaskMgr::RequestAuthFromUser(const sptr<ContinuousTaskParam> &taskParam)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    if (!taskParam) {
        BGTASK_LOGE("continuous task param is null!");
        return ERR_BGTASK_CHECK_TASK_PARAM;
    }
    int32_t specialModeSize = std::count(taskParam->bgModeIds_.begin(), taskParam->bgModeIds_.end(),
        BackgroundMode::SPECIAL_SCENARIO_PROCESSING);
    if (specialModeSize > 1) {
        return ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_ONLY_ALLOW_ONE_APPLICATION;
    }
    if (specialModeSize == 1 && taskParam->bgModeIds_.size() > 1) {
        return ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_CONFLICTS_WITH_OTHER_TASK;
    }
    if (!BundleManagerHelper::GetInstance()->CheckPermission(BGMODE_PERMISSION)) {
        BGTASK_LOGE("background mode permission is not passed");
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    int32_t userId = -1;
#ifdef HAS_OS_ACCOUNT_PART
    AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(callingUid, userId);
#else // HAS_OS_ACCOUNT_PART
    GetOsAccountIdFromUid(callingUid, userId);
#endif // HAS_OS_ACCOUNT_PART
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    std::string bundleName = BundleManagerHelper::GetInstance()->GetClientBundleName(callingUid);
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>(bundleName,
        "", callingUid, callingPid, taskParam->bgModeId_, true, taskParam->bgModeIds_);
    InitRecordParam(continuousTaskRecord, taskParam, userId);
    if (!BundleManagerHelper::GetInstance()->CheckACLPermission(BGMODE_PERMISSION_SYSTEM, callingUid)) {
        return ERR_BGTASK_CONTINUOUS_APP_NOT_HAVE_BGMODE_PERMISSION_SYSTEM;
    }
    if (continuousTaskRecord->isSystem_) {
        return ERR_BGTASK_CONTINUOUS_SYSTEM_APP_NOT_SUPPORT_ACL;
    }
    ErrCode ret = ERR_OK;
    handler_->PostSyncTask([this, &continuousTaskRecord, &ret]() {
        ret = this->SendBannerNotification(continuousTaskRecord);
        }, AppExecFwk::EventQueue::Priority::HIGH);
    return ret;
}

ErrCode BgContinuousTaskMgr::SendBannerNotification(std::shared_ptr<ContinuousTaskRecord> record)
{
    std::string appName = GetMainAbilityLabel(record->bundleName_, record->userId_);
    if (appName == "") {
        BGTASK_LOGE("get main ability label fail.");
        return ERR_BGTASK_NOTIFICATION_VERIFY_FAILED;
    }
    std::string bannerContent {""};
    if (!FormatBannerNotificationContext(appName, bannerContent)) {
        BGTASK_LOGE("bannerContent is empty");
        return ERR_BGTASK_NOTIFICATION_VERIFY_FAILED;
    }
    AppExecFwk::BundleInfo bundleInfo;
    if (!BundleManagerHelper::GetInstance()->GetBundleInfo(record->bundleName_,
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, record->userId_)) {
        BGTASK_LOGE("get bundle info: %{public}s failure!", record->bundleName_.c_str());
        return ERR_BGTASK_NOTIFICATION_VERIFY_FAILED;
    }
    std::shared_ptr<BannerNotificationRecord> bannerNotification = std::make_shared<BannerNotificationRecord>();
    bannerNotification->SetAppName(appName);
    bannerNotification->SetBundleName(record->bundleName_);
    bannerNotification->SetUid(record->uid_);
    bannerNotification->SetUserId(record->userId_);
    bannerNotification->SetAppIndex(bundleInfo.appIndex);
    ErrCode ret = NotificationTools::GetInstance()->PublishBannerNotification(bannerNotification, bannerContent,
        bgTaskUid_, bannerNotificaitonBtn_);
    if (ret == ERR_OK) {
        // 横幅通知发送成功，保存
        std::string key = bannerNotification->GetNotificationLabel();
        BGTASK_LOGI("send banner notification, label key: %{public}s", key.c_str());
        bannerNotificationRecord_.erase(key);
        bannerNotificationRecord_.emplace(key, bannerNotification);
    }
    return ret;
}

bool BgContinuousTaskMgr::FormatBannerNotificationContext(const std::string &appName,
    std::string &bannerContent)
{
    AppExecFwk::BundleInfo bundleInfo;
    if (!BundleManagerHelper::GetInstance()->GetBundleInfo(BG_TASK_RES_BUNDLE_NAME,
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo)) {
        BGTASK_LOGE("get background task res: %{public}s bundle info failed", BG_TASK_RES_BUNDLE_NAME);
        return false;
    }
    auto resourceManager = GetBundleResMgr(bundleInfo);
    if (resourceManager == nullptr) {
        BGTASK_LOGE("Get bgtask resource hap manager failed");
        return false;
    }
    for (std::string name : g_textBannerNotification) {
        resourceManager->GetStringFormatByName(bannerContent, name.c_str(), appName.c_str());
        if (bannerContent.empty()) {
            BGTASK_LOGE("get banner notification title text failed!");
            return false;
        }
        BGTASK_LOGI("get banner title text: %{public}s", bannerContent.c_str());
    }
    return true;
}

ErrCode BgContinuousTaskMgr::CheckSpecialScenarioAuth(uint32_t &authResult)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    if (!BundleManagerHelper::GetInstance()->CheckPermission(BGMODE_PERMISSION)) {
        BGTASK_LOGE("background mode permission is not passed");
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    int32_t userId = -1;
#ifdef HAS_OS_ACCOUNT_PART
    AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(callingUid, userId);
#else // HAS_OS_ACCOUNT_PART
    GetOsAccountIdFromUid(callingUid, userId);
#endif // HAS_OS_ACCOUNT_PART
    if (!BundleManagerHelper::GetInstance()->CheckACLPermission(BGMODE_PERMISSION_SYSTEM, callingUid)) {
        return ERR_BGTASK_CONTINUOUS_APP_NOT_HAVE_BGMODE_PERMISSION_SYSTEM;
    }
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (BundleManagerHelper::GetInstance()->IsSystemApp(fullTokenId)) {
        return ERR_BGTASK_CONTINUOUS_SYSTEM_APP_NOT_SUPPORT_ACL;
    }
    std::string bundleName = BundleManagerHelper::GetInstance()->GetClientBundleName(callingUid);
    AppExecFwk::BundleInfo bundleInfo;
    if (!BundleManagerHelper::GetInstance()->GetBundleInfo(bundleName,
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, userId)) {
        BGTASK_LOGE("get bundle info: %{public}s failure!", bundleName.c_str());
        return ERR_BGTASK_GET_APP_INDEX_FAIL;
    }
    int32_t appIndex = bundleInfo.appIndex;
    handler_->PostSyncTask([this, &authResult, bundleName, userId, appIndex]() {
        this->CheckSpecialScenarioAuthInner(authResult, bundleName, userId, appIndex);
        }, AppExecFwk::EventQueue::Priority::HIGH);
    return ERR_OK;
}

void BgContinuousTaskMgr::CheckSpecialScenarioAuthInner(uint32_t &authResult, const std::string &bundleName,
    int32_t userId, int32_t appIndex)
{
    std::string key = NotificationTools::GetInstance()->CreateBannerNotificationLabel(bundleName, userId, appIndex);
    BGTASK_LOGI("check auth result, label key: %{public}s", key.c_str());
    authResult = UserAuthResult::NOT_SUPPORTED;
    if (bannerNotificationRecord_.find(key) == bannerNotificationRecord_.end()) {
        return;
    }
    auto iter = bannerNotificationRecord_.at(key);
    int32_t auth = iter->GetAuthResult();
    if (auth != UserAuthResult::NOT_SUPPORTED) {
        authResult = static_cast<uint32_t>(auth);
    }
}

void BgContinuousTaskMgr::OnBannerNotificationActionButtonClick(const int32_t buttonType,
    const int32_t uid, const std::string &label)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }
    handler_->PostSyncTask([this, buttonType, uid, label]() {
        this->OnBannerNotificationActionButtonClickInner(buttonType, uid, label);
        }, AppExecFwk::EventQueue::Priority::HIGH);
}

void BgContinuousTaskMgr::OnBannerNotificationActionButtonClickInner(const int32_t buttonType,
    const int32_t uid, const std::string &label)
{
    BGTASK_LOGI("banner notification click, label key: %{public}s!", label.c_str());
    if (bannerNotificationRecord_.find(label) == bannerNotificationRecord_.end()) {
        return;
    }
    auto iter = bannerNotificationRecord_.at(label);
    if (buttonType == BGTASK_BANNER_NOTIFICATION_BTN_ALLOW_TIME) {
        BGTASK_LOGI("user click allow time, uid: %{public}d", uid);
        iter->SetAuthResult(UserAuthResult::GRANTED_ONCE);
    } else if (buttonType == BGTASK_BANNER_NOTIFICATION_BTN_ALLOW_ALLOWED) {
        BGTASK_LOGI("user click allow allowed, uid: %{public}d", uid);
        iter->SetAuthResult(UserAuthResult::GRANTED_ALWAYS);
    }
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS