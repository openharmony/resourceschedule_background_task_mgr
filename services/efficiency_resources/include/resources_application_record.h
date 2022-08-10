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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_RESOURCES_APPLICATION_RECORD_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_RESOURCES_APPLICATION_RECORD_H

#include "iremote_object.h"
#include "parcel.h"
#include "json/json.h"
#include "bg_efficiency_resources_mgr.h"

namespace OHOS {
namespace BackgroundTaskMgr {

extern const char *ResourceTypeName[7];
class BgEfficiencyResourcesMgr;

struct PersistTime{
<<<<<<< Updated upstream
    bool isPersist_ {false};
    int64_t endTime_ {0};
=======
    int32_t resourceIndex_ {0};
    bool isPersist_ {false};
    int64_t endTime_ {0};
    PersistTime() = default;
    PersistTime(int32_t resourceIndex, bool isPersist, int64_t endTime);
>>>>>>> Stashed changes
};

class ResourceApplicationRecord {
public:
    ResourceApplicationRecord() = default;
<<<<<<< Updated upstream
    ResourceApplicationRecord(int32_t uid, pid_t pid, bool isProcess,uint32_t resourceNumber, std::string bundleName) :
        uid_(uid), pid_(pid), isProcess_(isProcess_), resourceNumber_(resourceNumber), bundleName_(bundleName) {}
    ~ResourceApplicationRecord() = default;
    inline int32_t GetUid() const;
    inline pid_t GetPid() const;
    inline bool IsProcess() const;
    inline std::string GetBundleName() const;
    inline uint32_t GetResourceNumber() const;
    inline std::string GetReason() const;
    inline std::map<uint32_t, PersistTime> GetResourceUnitMap() const;
    std::string ParseToJsonStr();
    bool ParseFromJson(const Json::Value value);

private:
    int32_t uid_ {0};
    pid_t pid_ {0};
    bool isProcess_ {false};
    uint32_t resourceNumber_ {0};
    std::string bundleName_ {""};
    std::string reason_ {""};
    std::map<uint32_t, PersistTime> resourceUnitMap_ {};

    friend class BgEfficiencyResourcesMgr;
=======
    ResourceApplicationRecord(int32_t uid, int32_t pid, uint32_t resourceNumber, std::string bundleName) :
        uid_(uid), pid_(pid), resourceNumber_(resourceNumber), bundleName_(bundleName) {}
    ~ResourceApplicationRecord() = default;
    inline int32_t GetUid() const;
    inline int32_t GetPid() const;
    inline std::string GetBundleName() const;
    inline uint32_t GetResourceNumber() const;
    inline std::string GetReason() const;
    inline std::list<PersistTime>& GetResourceUnitList();
    void ParseToJson(Json::Value &root);
    std::string ParseToJsonStr();
    bool ParseFromJson(const Json::Value& value);

private:
    int32_t uid_ {0};
    int32_t pid_ {0};
    uint32_t resourceNumber_ {0};
    std::string bundleName_ {""};
    std::string reason_ {""};
    std::list<PersistTime> resourceUnitList_ {};

    friend class BgEfficiencyResourcesMgr;  
>>>>>>> Stashed changes
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_RESOURCES_APPLICATION_RECORD_H