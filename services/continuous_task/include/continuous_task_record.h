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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_CONTINUOUS_TASK_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_CONTINUOUS_TASK_INFO_H

#include "iremote_object.h"
#include "parcel.h"
#include "want_agent.h"

#include "task_notification_subscriber.h"

namespace OHOS {
namespace BackgroundTaskMgr {
extern const char *ContinuousTaskModeName[10];

class ContinuousTaskRecord {
public:
    ContinuousTaskRecord() = default;
    ContinuousTaskRecord(const std::string &bundleName, const std::string &abilityName,
        const std::shared_ptr<Notification::WantAgent::WantAgent> &wantAgent, const sptr<IRemoteObject> &abilityToken,
        int32_t userId, uid_t uid, pid_t pid, uint32_t bgModeId, bool isNewApi);
    std::string GetBundleName() const;
    std::string GetAbilityName() const;
    bool IsNewApi() const;
    uint32_t GetBgModeId() const;
    int32_t GetUserId() const;
    uid_t GetUid() const;
    pid_t GetPid() const;
    std::string GetNotificationLabel() const;
    std::shared_ptr<Notification::WantAgent::WantAgent> GetWantAgent() const;
    sptr<IRemoteObject> GetAbilityToken() const;

private:
    std::string bundleName_ {""};
    std::string abilityName_ {""};
    std::shared_ptr<Notification::WantAgent::WantAgent> wantAgent_ {nullptr};
    sptr<IRemoteObject> abilityToken_ {nullptr};
    int32_t userId_ {0};
    uid_t uid_ {0};
    pid_t pid_ {0};
    uint32_t bgModeId_ {0};
    bool isNewApi_ {false};
    std::string notificationLabel_ {""};

    friend class BgContinuousTaskMgr;
};
}
}
#endif