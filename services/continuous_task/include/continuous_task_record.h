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
#include "nlohmann/json.hpp"
#include "parcel.h"
#include "want_agent.h"

namespace OHOS {
namespace BackgroundTaskMgr {
extern const char *g_continuousTaskModeName[11];

// stop task reason
enum : int32_t {
    USER_CANCEL,
    REMOVE_NOTIFICATION_CANCEL,
    FREEZE_CANCEL,
    SYSTEM_CANCEL,
};

struct WantAgentInfo {
    std::string bundleName_ {""};
    std::string abilityName_ {""};
};

class ContinuousTaskRecord {
public:
    ContinuousTaskRecord() = default;
    ContinuousTaskRecord(const std::string &bundleName, const std::string &abilityName,
         int32_t uid, int32_t pid, uint32_t bgModeId, bool isBatchApi = false,
         const std::vector<uint32_t> &bgModeIds = {}, int32_t abilityId = -1);
    std::string GetBundleName() const;
    std::string GetAbilityName() const;
    bool IsNewApi() const;
    bool IsFromWebview() const;
    uint32_t GetBgModeId() const;
    int32_t GetUserId() const;
    int32_t GetUid() const;
    pid_t GetPid() const;
    int32_t GetAbilityId() const;
    std::string GetNotificationLabel() const;
    int32_t GetNotificationId() const;
    int32_t GetContinuousTaskId() const;
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> GetWantAgent() const;
    std::string ParseToJsonStr();
    bool ParseFromJson(const nlohmann::json &value);
    std::string ToString(std::vector<uint32_t> &bgmodes);
    bool IsSystem() const;

private:
    std::vector<uint32_t> ToVector(std::string &str);
    void SetRecordValue(const nlohmann::json &value);

    std::string bundleName_ {""};
    std::string abilityName_ {""};
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent_ {nullptr};
    int32_t userId_ {0};
    int32_t uid_ {0};
    pid_t pid_ {0};
    uint32_t bgModeId_ {0};
    bool isNewApi_ {false};
    bool isFromWebview_ {false};
    std::string notificationLabel_ {""};
    int32_t notificationId_ {-1};
    std::shared_ptr<WantAgentInfo> wantAgentInfo_ {nullptr};
    std::string appName_ {""};
    uint64_t fullTokenId_ {0};
    bool isBatchApi_ {false};
    std::vector<uint32_t> bgModeIds_ {};
    std::vector<uint32_t> bgSubModeIds_ {};
    int32_t abilityId_ {-1};
    int32_t reason_ {0};
    bool isSystem_ {false};
    int32_t continuousTaskId_ {-1};
    bool suspendState_ {false};
    int32_t suspendReason_ {0};

    friend class BgContinuousTaskMgr;
    friend class NotificationTools;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_CONTINUOUS_TASK_INFO_H