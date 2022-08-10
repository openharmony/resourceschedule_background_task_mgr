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

#include "resources_application_record.h"

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

<<<<<<< Updated upstream
=======
PersistTime::PersistTime(int32_t resourceIndex, bool isPersist, int64_t endTime) :
    resourceIndex_(resourceIndex), isPersist_(isPersist), endTime_(endTime) {}

>>>>>>> Stashed changes
std::string ResourceApplicationRecord::GetBundleName() const
{
    return bundleName_;
}

int32_t ResourceApplicationRecord::GetUid() const
{
    return uid_;
}

<<<<<<< Updated upstream
pid_t ResourceApplicationRecord::GetPid() const
=======
int32_t ResourceApplicationRecord::GetPid() const
>>>>>>> Stashed changes
{
    return pid_;
}

<<<<<<< Updated upstream
bool ResourceApplicationRecord::IsProcess() const
{
    return isProcess_;
}

=======
>>>>>>> Stashed changes
uint32_t ResourceApplicationRecord::GetResourceNumber() const
{
    return resourceNumber_;
}

std::string ResourceApplicationRecord::GetReason() const
{
    return reason_;
}

<<<<<<< Updated upstream
std::map<uint32_t, PersistTime> ResourceApplicationRecord::GetResourceUnitMap() const
{
    return resourceUnitMap_;
=======
std::list<PersistTime>& ResourceApplicationRecord::GetResourceUnitList()
{
    return resourceUnitList_;
>>>>>>> Stashed changes
}

std::string ResourceApplicationRecord::ParseToJsonStr()
{
    Json::Value root;
<<<<<<< Updated upstream
    root["bundleName"] = bundleName_;
    root["uid"] = uid_;
    root["pid"] = pid_;
    root["isProcess"] = isProcess_;
    root["resourceNumber"] = resourceNumber_;
    root["reason"] = reason_;

    if (!resourceUnitMap_.empty()) {
        Json::Value resource;
        for (auto &iter : resourceUnitMap_) {
            Json::Value info;
            info["isPersist"] = iter.second.isPersist_;
            info["endTime"] = iter.second.endTime_;
            resource[std::to_string(iter.first)] = info;
        }
        root["resourceUnitMap"] = resource;
    }
    return WriteString(root);
=======
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
>>>>>>> Stashed changes
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

<<<<<<< Updated upstream
bool ResourceApplicationRecord::ParseFromJson(const Json::Value value)
=======
bool ResourceApplicationRecord::ParseFromJson(const Json::Value& value)
>>>>>>> Stashed changes
{
    if (value.empty()) {
        return false;
    }
<<<<<<< Updated upstream
    this->bundleName_ = value["bundleName"].asString();
    this->uid_ = value["uid"].asInt();
    this->pid_ = value["pid"].asInt();
    this->isProcess_ = value["isProcess"].asBool();
    this->resourceNumber_ = value["resourceNumber"].asUInt();
    if (value.isMember("resourceUnitMap")) {
        Json::Value resourceVal = value["resourceUnitMap"];
        std::map<uint32_t, PersistTime> resourceUnitMap;
        for (auto it : resourceVal.getMemberNames()) {
            Json::Value persistTime = resourceVal[it];
            resourceUnitMap.emplace(static_cast<uint32_t>(std::atoi(it.c_str())), 
                PersistTime{persistTime["isPersist"].asBool(), persistTime["endTime"].asInt64()}); 
        }
        this->resourceUnitMap_ = resourceUnitMap;
=======
    this->uid_ = value["uid"].asInt();
    this->pid_ = value["pid"].asInt();
    this->bundleName_ = value["bundleName"].asString();
    this->resourceNumber_ = value["resourceNumber"].asUInt();
    this->reason_ = value["reason"].asString();
    if (value.isMember("resourceUnitList")) {
        Json::Value resourceVal = value["resourceUnitList"];
        for (int i=0; i<resourceVal.size(); ++i) {
            Json::Value persistTime = resourceVal[i];
            this->resourceUnitList_.emplace_back(PersistTime{persistTime["resourceIndex"].asInt(),
                persistTime["isPersist"].asBool(), persistTime["endTime"].asInt64()}); 
        }
>>>>>>> Stashed changes
    }
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS