/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_BANNER_NOTIFICATION_RECORD_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_BANNER_NOTIFICATION_RECORD_H

#include "parcel.h"
#include "user_auth_result.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class BannerNotificationRecord {
public:
    BannerNotificationRecord() = default;
    BannerNotificationRecord(const std::string &bundleName, int32_t uid, int32_t notificationId,
        const std::string &notificationLabel, const std::string &appName);
    std::string GetBundleName() const;
    int32_t GetUid() const;
    int32_t GetNotificationId() const;
    std::string GetNotificationLabel() const;
    std::string GetAppName() const;
    int32_t GetAuthResult() const;
    int32_t GetUserId() const;
    int32_t GetAppIndex() const;
    void SetBundleName(const std::string &bundleName);
    void SetNotificationLabel(const std::string &notificationLabel);
    void SetAppName(const std::string &appName);
    void SetUid(int32_t uid);
    void SetNotificationId(int32_t notificationId);
    void SetAuthResult(int32_t authResult);
    void SetUserId(int32_t userId);
    void SetAppIndex(int32_t appIndex);

private:
    std::string bundleName_ {""};
    int32_t uid_ {0};
    int32_t notificationId_ {-1};
    std::string notificationLabel_ {""};
    std::string appName_ {""};
    int32_t authResult_ {UserAuthResult::NOT_SUPPORTED};
    int32_t userId_ {0};
    int32_t appIndex_ {-1};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_BANNER_NOTIFICATION_RECORD_H