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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_REC_APPLY_RECORD_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_REC_APPLY_RECORD_H

#include <list>
#include <sstream>

#include "iremote_object.h"
#include "parcel.h"
#include "nlohmann/json.hpp"
#include "efficiency_resources_cpu_level.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class BgEfficiencyResourcesMgr;

struct PersistTime {
    uint32_t resourceIndex_ {0};
    bool isPersist_ {false};
    int64_t endTime_ {0};
    std::string reason_ {""};
    int64_t timeOut_ {0};
    PersistTime() = default;
    PersistTime(const uint32_t resourceIndex, const bool isPersist, const int64_t endTime,
        const std::string &reason, const int64_t timeOut_ = 0);
    bool operator < (const PersistTime& rhs) const;
};

class ResourceApplicationRecord {
public:
    ResourceApplicationRecord() = default;
    ResourceApplicationRecord(int32_t uid, int32_t pid, uint32_t resourceNumber, std::string bundleName)
        : uid_(uid), pid_(pid), resourceNumber_(resourceNumber), bundleName_(bundleName) {}
    ~ResourceApplicationRecord() = default;
    int32_t GetUid() const;
    int32_t GetPid() const;
    std::string GetBundleName() const;
    uint32_t GetResourceNumber() const;
    std::list<PersistTime>& GetResourceUnitList();
    void SetResourceNumber();
    void ParseToJson(nlohmann::json &root);
    std::string ParseToJsonStr();
    bool ParseFromJson(const nlohmann::json& value);
    int32_t GetCpuLevel() const;
    void SetCpuLevel(int32_t cpuLevel);

private:
    int32_t uid_ {0};
    int32_t pid_ {0};
    uint32_t resourceNumber_ {0};
    EfficiencyResourcesCpuLevel::Type cpuLevel_ {EfficiencyResourcesCpuLevel::DEFAULT};
    std::string bundleName_ {""};
    std::list<PersistTime> resourceUnitList_ {};

    friend class BgEfficiencyResourcesMgr;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_REC_APPLY_RECORD_H