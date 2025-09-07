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

private:
    static int32_t notificationIdIndex_;

    DECLARE_DELAYED_SINGLETON(NotificationTools)
};
} // BackgroundTaskMgr
} // OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_NOTIFICATION_TOOLS_H
