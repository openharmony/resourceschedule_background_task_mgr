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

#include "iremote_object.h"

#include "resource_application_record.h"
#include "bg_efficiency_resources_mgr.h"
#include "efficiency_resource_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
const char *ResourceTypeName[7] = {
   "CPU",
   "COMMON_EVENT",
   "TIMER",
   "WORK_SCHEDULER",
   "BLUETOOTH",
   "GPS",
   "AUDIO",
};

PersistTime::PersistTime(uint32_t resourceIndex, bool isPersist, int64_t endTime) :
    resourceIndex_(resourceIndex), isPersist_(isPersist), endTime_(endTime) {}

std::string ResourceApplicationRecord::GetBundleName() const
{
    return bundleName_;
}

int32_t ResourceApplicationRecord::GetUid() const
{
    return uid_;
}

int32_t ResourceApplicationRecord::GetPid() const
{
    return pid_;
}

uint32_t ResourceApplicationRecord::GetResourceNumber() const
{
    return resourceNumber_;
}

std::string ResourceApplicationRecord::GetReason() const
{
    return reason_;
}

std::list<PersistTime>& ResourceApplicationRecord::GetResourceUnitList()
{
    return resourceUnitList_;
}

std::string ResourceApplicationRecord::ParseToJsonStr()
{
    nlohmann::json root;
    ParseToJson(root);
    return root.dump();
}

void ResourceApplicationRecord::ParseToJson(nlohmann::json &root)
{
    root["bundleName"] = bundleName_;
    root["uid"] = uid_;
    root["pid"] = pid_;
    root["resourceNumber"] = resourceNumber_;
    root["reason"] = reason_;

    if (!resourceUnitList_.empty()) {
        nlohmann::json resource;
        for (const auto &iter : resourceUnitList_) {
            nlohmann::json info;
            info["resourceIndex"] = iter.resourceIndex_;
            info["isPersist"] = iter.isPersist_;
            info["endTime"] = iter.endTime_;
            resource.push_back(info);
        }
        root["resourceUnitList"] = resource;
    }
}

bool ResourceApplicationRecord::ParseFromJson(const nlohmann::json& value)
{
    if (value.empty()) {
        return false;
    }
    this->uid_ = value.at("uid").get<int32_t>();
    this->pid_ = value.at("pid").get<int32_t>();
    this->bundleName_ = value.at("bundleName").get<std::string>();
    this->resourceNumber_ = value.at("resourceNumber").get<uint32_t>();
    this->reason_ = value.at("reason").get<std::string>();
    if (value.count("resourceUnitList") > 0) {
        const nlohmann::json &resourceVal = value.at("resourceUnitList");
        auto nums = static_cast<int32_t>(resourceVal.size());
        for (int i=0; i<nums; ++i) {
            const nlohmann::json &persistTime = resourceVal.at(i);
            this->resourceUnitList_.emplace_back(PersistTime{persistTime.at("resourceIndex").get<uint32_t>(),
                persistTime.at("isPersist").get<bool>(), persistTime.at("endTime").get<int64_t>()}); 
        }
    }
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS