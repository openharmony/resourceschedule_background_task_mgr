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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_RESOURCE_APPLICATION_RECORD_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_RESOURCE_APPLICATION_RECORD_H

#include "iremote_object.h"
#include "parcel.h"
#include "nlohmann/json.hpp"
#include <list>
#include <sstream>

namespace OHOS {
namespace BackgroundTaskMgr {

extern const char *ResourceTypeName[7];
class BgEfficiencyResourcesMgr;

struct PersistTime{
    uint32_t resourceIndex_ {0};
    bool isPersist_ {false};
    int64_t endTime_ {0};
    PersistTime() = default;
    PersistTime(uint32_t resourceIndex, bool isPersist, int64_t endTime);
};

class ResourceApplicationRecord {
public:
    ResourceApplicationRecord() = default;
    ResourceApplicationRecord(int32_t uid, int32_t pid, uint32_t resourceNumber, std::string bundleName) :
        uid_(uid), pid_(pid), resourceNumber_(resourceNumber), bundleName_(bundleName) {}
    ~ResourceApplicationRecord() = default;
    int32_t GetUid() const;
    int32_t GetPid() const;
    std::string GetBundleName() const;
    uint32_t GetResourceNumber() const;
    std::string GetReason() const;
    std::list<PersistTime>& GetResourceUnitList();
    void ParseToJson(nlohmann::json &root);
    std::string ParseToJsonStr();
    bool ParseFromJson(const nlohmann::json& value);

private:
    int32_t uid_ {0};
    int32_t pid_ {0};
    uint32_t resourceNumber_ {0};
    std::string bundleName_ {""};
    std::string reason_ {""};
    std::list<PersistTime> resourceUnitList_ {};

    friend class BgEfficiencyResourcesMgr;  
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_RESOURCE_APPLICATION_RECORD_H