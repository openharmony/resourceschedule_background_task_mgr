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

#include "resource_application_record.h"

#include "common_utils.h"
#include "iremote_object.h"

#include "efficiency_resource_log.h"
#include "bg_efficiency_resources_mgr.h"
namespace OHOS {
namespace BackgroundTaskMgr {
PersistTime::PersistTime(const uint32_t resourceIndex, const bool isPersist, const int64_t endTime,
    const std::string &reason) : resourceIndex_(resourceIndex), isPersist_(isPersist), endTime_(endTime),
    reason_(reason) {}

bool PersistTime::operator < (const PersistTime& rhs) const
{
    return resourceIndex_ < rhs.resourceIndex_;
}

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

std::list<PersistTime>& ResourceApplicationRecord::GetResourceUnitList()
{
    return resourceUnitList_;
}

std::string ResourceApplicationRecord::ParseToJsonStr()
{
    nlohmann::json root;
    ParseToJson(root);
    return root.dump(CommonUtils::jsonFormat_);
}

void ResourceApplicationRecord::ParseToJson(nlohmann::json &root)
{
    root["bundleName"] = bundleName_;
    root["uid"] = uid_;
    root["pid"] = pid_;
    root["resourceNumber"] = resourceNumber_;

    if (!resourceUnitList_.empty()) {
        nlohmann::json resource;
        for (const auto &iter : resourceUnitList_) {
            nlohmann::json info;
            info["resourceIndex"] = iter.resourceIndex_;
            info["isPersist"] = iter.isPersist_;
            info["endTime"] = iter.endTime_;
            info["reason"] = iter.reason_;
            resource.push_back(info);
        }
        root["resourceUnitList"] = resource;
    }
}

bool ResourceApplicationRecord::ParseFromJson(const nlohmann::json& value)
{
    if (value.empty()) {
        BGTASK_LOGE("value is empty");
        return false;
    }
    if (!CommonUtils::CheckJsonValue(value, {"uid", "pid", "bundleName", "resourceNumber"}) || 
        !value.at("uid").is_number_integer() || !value.at("pid").is_number_integer() || 
        !value.at("bundleName").is_string() || !value.at("resourceNumber").is_number_integer()) {
        BGTASK_LOGE("checkJsonValue of value is failed");
        return false;
    }
    this->uid_ = value.at("uid").get<int32_t>();
    this->pid_ = value.at("pid").get<int32_t>();
    this->bundleName_ = value.at("bundleName").get<std::string>();
    this->resourceNumber_ = value.at("resourceNumber").get<uint32_t>();
    if (value.count("resourceUnitList") > 0 && value.at("resourceUnitList").is_object()) {
        const nlohmann::json &resourceVal = value.at("resourceUnitList");
        auto nums = static_cast<int32_t>(resourceVal.size());
        for (int i = 0; i < nums; ++i) {
            if (resourceVal.at(i).is_null() || !resourceVal.at(i).is_object()) {
                BGTASK_LOGE("resourceVal.at(%{public}d) is null or is not object", i);
                continue;
            }
            const nlohmann::json &persistTime = resourceVal.at(i);
            if (!CommonUtils::CheckJsonValue(persistTime, {"resourceIndex", "isPersist", "endTime", "reason"}) || 
                !persistTime.at("resourceIndex").is_number_integer() || !persistTime.at("isPersist").is_boolean() || 
                !persistTime.at("endTime").is_number_integer() || !persistTime.at("reason").is_string()) {
                BGTASK_LOGE("checkJsonValue of persistTime is failed");
                continue;
            }
            uint32_t resourceIndex = persistTime.at("resourceIndex").get<uint32_t>();
            bool isPersist_ = persistTime.at("isPersist").get<bool>();
            int64_t endTime_ = persistTime.at("endTime").get<int64_t>();
            std::string reason_ = persistTime.at("reason").get<std::string>();
            this->resourceUnitList_.emplace_back(PersistTime {resourceIndex, isPersist_, endTime_,
                reason_});
        }
    }
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS