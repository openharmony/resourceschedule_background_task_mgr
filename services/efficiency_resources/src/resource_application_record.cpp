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
#include "bg_efficiency_resources_mgr.h"

#include "iremote_object.h"
#include "json/value.h"

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
    Json::Value root;
    ParseToJson(root);
    return root.toStyledString();
}

void ResourceApplicationRecord::ParseToJson(Json::Value &root)
{
    root["bundleName"] = bundleName_;
    root["uid"] = uid_;
    root["pid"] = pid_;
    root["resourceNumber"] = resourceNumber_;
    root["reason"] = reason_;

    if (!resourceUnitList_.empty()) {
        Json::Value resource;
        for (const auto &iter : resourceUnitList_) {
            Json::Value info;
            info["resourceIndex"] = iter.resourceIndex_;
            info["isPersist"] = iter.isPersist_;
            info["endTime"] = iter.endTime_;
            resource.append(info);
        }
        root["resourceUnitList"] = resource;
    }
}

std::string WriteString(Json::Value &root)
{
    Json::StreamWriterBuilder writerBuilder;
    std::ostringstream os;
    std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
    jsonWriter->write(root, &os);
    std::string result = os.str();
    return result;
}

bool ResourceApplicationRecord::ParseFromJson(const Json::Value& value)
{
    if (value.empty()) {
        return false;
    }
    this->uid_ = value["uid"].asInt();
    this->pid_ = value["pid"].asInt();
    this->bundleName_ = value["bundleName"].asString();
    this->resourceNumber_ = value["resourceNumber"].asUInt();
    this->reason_ = value["reason"].asString();
    if (value.isMember("resourceUnitList")) {
        Json::Value resourceVal = value["resourceUnitList"];
        auto nums = static_cast<int32_t>(resourceVal.size());
        for (int i=0; i<nums; ++i) {
            Json::Value persistTime = resourceVal[i];
            this->resourceUnitList_.emplace_back(PersistTime{persistTime["resourceIndex"].asInt(),
                persistTime["isPersist"].asBool(), persistTime["endTime"].asInt64()}); 
        }
    }
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS