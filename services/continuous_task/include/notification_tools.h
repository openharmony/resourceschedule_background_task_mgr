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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_NOTIFICATION_TOOLS_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_NOTIFICATION_TOOLS_H

#include "singleton.h"
#include "continuous_task_record.h"
#include "bgtaskmgr_inner_errors.h"
#include "banner_notification_record.h"

authCallbackDeathRecipient_ = new (std::nothrow)
        AuthExpiredCallbackDeathRecipient(DelayedSingleton<BackgroundTaskMgrService>::GetInstance().get());

ErrCode BgContinuousTaskMgr::RequestAuthFromUser(const sptr<ContinuousTaskParam> &taskParam,
    const sptr<IExpiredCallback>& callback, int32_t &notificationId)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    if (!taskParam) {
        BGTASK_LOGE("continuous task param is null!");
        return ERR_BGTASK_CHECK_TASK_PARAM;
    }
    if (callback == nullptr) {
        BGTASK_LOGE("callback is nullptr");
        return ERR_BGTASK_CONTINUOUS_CALLBACK_NULL_OR_TYPE_ERR;
    }
    ErrCode ret = CheckSpecialModePermission(taskParam);
    if (ret != ERR_OK) {
        return ret;
    }
    uint32_t specialModeSize = std::count(taskParam->bgModeIds_.begin(), taskParam->bgModeIds_.end(),
        BackgroundMode::SPECIAL_SCENARIO_PROCESSING);
    if (specialModeSize == 0) {
        BGTASK_LOGE("not have bgmode: special scenario process");
        return ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_EMPTY;
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
    uint64_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    if (!BundleManagerHelper::GetInstance()->CheckACLPermission(BGMODE_PERMISSION_SYSTEM, callingTokenId)) {
        return ERR_BGTASK_CONTINUOUS_APP_NOT_HAVE_BGMODE_PERMISSION_SYSTEM;
    }
    if (continuousTaskRecord->isSystem_) {
        return ERR_BGTASK_CONTINUOUS_SYSTEM_APP_NOT_SUPPORT_ACL;
    }
    handler_->PostSyncTask([this, continuousTaskRecord, callback, &notificationId, &ret]() {
        ret = this->SendBannerNotification(continuousTaskRecord, callback, notificationId);
        }, AppExecFwk::EventQueue::Priority::HIGH);
    return ret;
}

ErrCode BgContinuousTaskMgr::SendBannerNotification(std::shared_ptr<ContinuousTaskRecord> record,
    const sptr<IExpiredCallback>& callback, int32_t &notificationId)
{
    auto findCallback = [&callback](const auto& callbackMap) {
        return callback->AsObject() == callbackMap.second->AsObject();
    };
    auto callbackIter = find_if(expiredCallbackMap_.begin(), expiredCallbackMap_.end(), findCallback);
    if (callbackIter != expiredCallbackMap_.end()) {
        BGTASK_LOGI("request auth form user, callback is already exists.");
        return ERR_BGTASK_CONTINUOUS_CALLBACK_EXISTS;
    }
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
        notificationId = bannerNotification->GetNotificationId();
        auto remote = callback->AsObject();
        expiredCallbackMap_[notificationId] = callback;
        if (authCallbackDeathRecipient_ != nullptr) {
            (void)remote->AddDeathRecipient(authCallbackDeathRecipient_);
        }
        std::string key = bannerNotification->GetNotificationLabel();
        BGTASK_LOGI("send banner notification, label key: %{public}s", key.c_str());
        bannerNotificationRecord_.erase(key);
        bannerNotificationRecord_.emplace(key, bannerNotification);
    }
    return ret;
}

// 点击授权按钮后，取消通知
    int32_t notificatinId = iter->GetNotificationId();
    if (notificatinId != -1) {
        NotificationTools::GetInstance()->CancelNotification(iter->GetNotificationLabel(), iter->GetNotificationId());
    }
    // 触发回调
    auto callbackIter = expiredCallbackMap_.find(notificatinId);
    if (callbackIter != expiredCallbackMap_.end()) {
        BGTASK_LOGE("click banner notificationId: %{public}d, trigger callback.", notificatinId);
        uint32_t authResult = iter->GetAuthResult();
        callbackIter->second->OnExpiredAuth(authResult);
        auto remote = callbackIter->second->AsObject();
        if (remote != nullptr) {
            remote->RemoveDeathRecipient(authCallbackDeathRecipient_);
        }
        expiredCallbackMap_.erase(callbackIter);
    } else {
        BGTASK_LOGE("request expired, callback not found.");
    }
}

AuthExpiredCallbackDeathRecipient::AuthExpiredCallbackDeathRecipient(const wptr<BackgroundTaskMgrService>& service)
    : service_(service) {}

AuthExpiredCallbackDeathRecipient::~AuthExpiredCallbackDeathRecipient() {}

void AuthExpiredCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    auto service = service_.promote();
    if (service == nullptr) {
        BGTASK_LOGE("expired callback died, BackgroundTaskMgrService dead.");
        return;
    }
    service->HandleExpiredCallbackDeath(remote);
}

void BgContinuousTaskMgr::HandleAuthExpiredCallbackDeath(const wptr<IRemoteObject> &remote)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }
    if (remote == nullptr) {
        BGTASK_LOGE("expiredCallback death, remote in callback is null.");
        return;
    }
    handler_->PostSyncTask([this, remote]() {
        this->HandleAuthExpiredCallbackDeathInnre(remote);
        }, AppExecFwk::EventQueue::Priority::HIGH);
}

void BgContinuousTaskMgr::HandleAuthExpiredCallbackDeathInnre(const wptr<IRemoteObject> &remote)
{
    auto findCallback = [&remote](const auto& callbackMap) {
        return callbackMap.second->AsObject() == remote;
    };
    auto callbackIter = find_if(expiredCallbackMap_.begin(), expiredCallbackMap_.end(), findCallback);
    if (callbackIter == expiredCallbackMap_.end()) {
        BGTASK_LOGE("expiredCallback death, remote in callback not found.");
        return;
    }
    int32_t notificationId = callbackIter->first;
    BGTASK_LOGI("expiredCallback death, remote callback notificationId: %{public}d.", notificationId);
    expiredCallbackMap_.erase(callbackIter);
    if (notificationId != -1) {
        auto findRecord = [notificationId](const auto &target) {
            return notificationId == target.second->GetNotificationId();
        };
        auto findRecordIter = find_if(bannerNotificationRecord_.begin(), bannerNotificationRecord_.end(), findRecord);
        if (findRecordIter == bannerNotificationRecord_.end()) {
            BGTASK_LOGE("notificationId: %{public}d not have record", notificationId);
            return;
        }
        std::string notificationLabel = findRecordIter->second->GetNotificationLabel();
        NotificationTools::GetInstance()->CancelNotification(notificationLabel, notificationId);
    }
}

namespace OHOS {
namespace BackgroundTaskMgr {
class NotificationTools : public DelayedSingleton<NotificationTools> {
public:
    static void SetNotificationIdIndex(const int32_t id);
    ErrCode PublishNotification(const std::shared_ptr<ContinuousTaskRecord> &continuousTaskRecord,
        const std::string &appName, const std::string &prompt, int32_t serviceUid);
    ErrCode CancelNotification(const std::string &label, int32_t id);
    void GetAllActiveNotificationsLabels(std::set<std::string> &notificationLabels);
    void RefreshContinuousNotifications(
        const std::map<std::string, std::pair<std::string, std::string>> &newPromptInfos, int32_t serviceUid);
    ErrCode RefreshContinuousNotificationWantAndContext(int32_t serviceUid,
        const std::map<std::string, std::pair<std::string, std::string>> &newPromptInfos,
        const std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord, bool updateContent = false);
    ErrCode PublishSubNotification(const std::shared_ptr<ContinuousTaskRecord> subRecord,
        const std::string &appName, const std::string &prompt, int32_t serviceUid,
        std::shared_ptr<ContinuousTaskRecord> mainRecord);
    ErrCode PublishMainNotification(const std::shared_ptr<ContinuousTaskRecord> subRecord,
        const std::string &appName, const std::string &prompt, int32_t serviceUid,
        std::shared_ptr<ContinuousTaskRecord> mainRecord);
    ErrCode PublishBannerNotification(std::shared_ptr<BannerNotificationRecord> bannerNotification,
        const std::string &prompt, int32_t serviceUid,
        const std::vector<std::string> &bannerNotificaitonBtn);
    void RefreshBannerNotifications(const std::vector<std::string> &bannerNotificaitonBtn,
        const std::map<std::string, std::pair<std::string, std::string>> &newPromptInfos,
        const std::shared_ptr<BannerNotificationRecord> bannerNotification,
        int32_t serviceUid);
    std::string CreateBannerNotificationLabel(const std::string &bundleName, int32_t userId,
        int32_t appIndex);

private:
    static int32_t notificationIdIndex_;

    DECLARE_DELAYED_SINGLETON(NotificationTools)
};
} // BackgroundTaskMgr
} // OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_NOTIFICATION_TOOLS_H
