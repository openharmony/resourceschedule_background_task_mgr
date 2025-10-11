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
}  // namespace BackgroundTaskMgr
}  // namespace OHOS