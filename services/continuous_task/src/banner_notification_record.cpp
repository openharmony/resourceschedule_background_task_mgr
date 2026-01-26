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
#include "common_utils.h"
#include "continuous_task_log.h"

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

void BannerNotificationRecord::SetRecordValue(const nlohmann::json &value)
{
    this->bundleName_ = value.at("bundleName").get<std::string>();
    this->uid_ = value.at("uid").get<int32_t>();
    this->notificationId_ = value.at("notificationId").get<int32_t>();
    this->notificationLabel_ = value.at("notificationLabel").get<std::string>();
    this->appName_ = value.at("appName").get<std::string>();
    this->authResult_ = value.at("authResult").get<int32_t>();
    this->userId_ = value.at("userId").get<int32_t>();
    this->appIndex_ = value.at("appIndex").get<int32_t>();
}

bool CheckRecordKey(const nlohmann::json &value)
{
    return !value["bundleName"].is_string() || !value["uid"].is_number_integer() ||
        !value["notificationId"].is_number_integer() || !value["notificationLabel"].is_string() ||
        !value["appName"].is_string() || !value["authResult"].is_number_integer() ||
        !value["userId"].is_number_integer() || !value["appIndex"].is_number_integer();
}

bool BannerNotificationRecord::ParseFromJson(const nlohmann::json &value)
{
    if (value.is_null() || !value.is_object() || !CommonUtils::CheckJsonValue(value, { "bundleName",
        "uid", "notificationId", "notificationLabel", "appName", "authResult", "userId", "appIndex"})) {
        BGTASK_LOGE("bannerNotificationRecord no key.");
        return false;
    }
    if (CheckRecordKey(value)) {
        BGTASK_LOGE("bannerNotificationRecord parse from json fail.");
        return false;
    }
    SetRecordValue(value);
    return true;
}

std::string BannerNotificationRecord::ParseToJsonStr()
{
    nlohmann::json root;
    root["bundleName"] = bundleName_;
    root["uid"] = uid_;
    root["notificationId"] = notificationId_;
    root["notificationLabel"] = notificationLabel_;
    root["appName"] = appName_;
    root["authResult"] = authResult_;
    root["userId"] = userId_;
    root["appIndex"] = appIndex_;
    return root.dump(CommonUtils::jsonFormat_);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS