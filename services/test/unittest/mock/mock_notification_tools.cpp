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

#include "notification_tools.h"

namespace OHOS {
namespace BackgroundTaskMgr {
ErrCode NotificationTools::PublishNotification(const std::shared_ptr<ContinuousTaskRecord> &continuousTaskRecord,
    const std::string &appName, const std::string &prompt, int32_t serviceUid)
{
    return ERR_OK;
}

ErrCode NotificationTools::CancelNotification(const std::string &label, int32_t id)
{
    return ERR_OK;
}

void NotificationTools::GetAllActiveNotificationsLabels(std::set<std::string> &notificationLabels) {}

void NotificationTools::RefreshContinuousNotifications(
    const std::map<std::string, std::pair<std::string, std::string>> &newPromptInfos, int32_t serviceUid) {}

ErrCode NotificationTools::RefreshContinuousNotificationWantAndContext(int32_t serviceUid,
    const std::map<std::string, std::pair<std::string, std::string>> &newPromptInfos,
    const std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord, bool updateContent)
{
    return ERR_OK;
}
}
}
