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

// 注册横幅通知的点击事件
    matchingSkills.AddEvent(BGTASK_BANNER_NOTIFICATION_ACTION_NAME);


bool BgContinuousTaskMgr::StopBannerContinuousTaskByUser(const std::string &label)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return false;
    }
    bool result = true;
    handler_->PostSyncTask([this, label, &result]() {
        result = StopBannerContinuousTaskByUserInner(label);
    });
    return result;
}

bool BgContinuousTaskMgr::StopBannerContinuousTaskByUserInner(const std::string &label)
{
    if (bannerNotificationRecord_.find(label) != bannerNotificationRecord_.end()) {
        auto iter = bannerNotificationRecord_.at(label);
        // 已经被授权过了，所以通知删除，不取消授权记录
        if (iter->GetAuthResult() != UserAuthResult::GRANTED_ONCE &&
            iter->GetAuthResult() != UserAuthResult::GRANTED_ALWAYS) {
            bannerNotificationRecord_.erase(label);
        } else {
            BGTASK_LOGI("alread auth not remove record, label key: %{public}s", label.c_str());
        }
    }
    return true;
}

continuousTaskCallbackInfo->SetBundleName(continuousTaskInfo->bundleName_);
    continuousTaskCallbackInfo->SetUserId(continuousTaskInfo->userId_);
    continuousTaskCallbackInfo->SetAppIndex(continuousTaskInfo->appIndex_);
}  // namespace BackgroundTaskMgr
}  // namespace OHOS