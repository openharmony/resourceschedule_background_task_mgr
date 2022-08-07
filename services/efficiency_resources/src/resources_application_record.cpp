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

std::string ResourceApplicationRecord::GetBundleName() const
{
    return bundleName_;
}

int32_t ResourceApplicationRecord::GetUid() const
{
    return uid_;
}

pid_t ResourceApplicationRecord::GetPid() const
{
    return pid_;
}

bool ResourceApplicationRecord::IsProcess() const
{
    return isProcess_;
}

uint32_t ResourceApplicationRecord::GetResourceNumber() const
{
    return resourceNumber_;
}

std::string ResourceApplicationRecord::GetReason() const
{
    return reason_;
}

std::map<uint32_t, PersistTime> ResourceApplicationRecord::GetResourceUnitMap() const
{
    return resourceUnitMap_;
}

std::string ResourceApplicationRecord::ParseToJsonStr()
{
    Json::Value root;
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

bool ResourceApplicationRecord::ParseFromJson(const Json::Value value)
{
    if (value.empty()) {
        return false;
    }
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
    }
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS