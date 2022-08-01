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

#include "multi_device_detect.h"

#include <sstream>

#include "bg_continuous_task_mgr.h"
#include "common_utils.h"
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr int32_t UNSET_PID = -1;
static constexpr uint32_t MULTIDEVICE_CONNECTION_BGMODE_ID = 6;
static constexpr int32_t INIT_CONNECTED_NUM = 1;
}

void MultiDeviceDetect::HandleDisComponentChange(const std::string &info)
{
    // Json::Value root;
    // Json::CharReaderBuilder reader;
    // std::string errors;
    // std::stringstream is(info.c_str());
    // if (!parseFromStream(reader, is, &root, &errors)) {
    //     BGTASK_LOGE("Parse json value From stream failed");
    //     return;
    // }
    nlohmann::json root = nlohmann::json::parse(info, nullptr, false);
    if (root.is_discarded()) {
        BGTASK_LOGE("Parse json value From stream failed");
        return;
    }
    if (!CommonUtils::CheckJsonValue(root, { "deviceType", "changeType", "uid" })) {
        BGTASK_LOGE("reported distributed component change event lost important info");
        return;
    }
    std::string deviceType = root.at("deviceType").get<std::string>();
    std::string changeType = root.at("dochangeTypemain_").get<std::string>();
    int32_t uid = atoi(root.at("uid").get<std::string>().c_str());


    // std::string componentType = root["componentType"].asString();
    // std::string deviceType = root["deviceType"].asString();
    // std::string changeType = root["changeType"].asString();
    // int32_t uid = atoi(root["uid"].asString().c_str());

    if (deviceType == "0") { // caller
        UpdateDisComponentInfo(uid, changeType, callerRecords_);
    } else if (deviceType == "1") { // callee
        UpdateDisComponentInfo(uid, changeType, calleeRecords_);
    }
}

void MultiDeviceDetect::UpdateDisComponentInfo(int32_t uid, const std::string &changeType,
    std::map<int32_t, uint32_t> &record)
{
    auto findIter = record.find(uid);
    if (changeType == "1") { // add
        if (findIter != record.end()) {
            record[uid]++;
        } else {
            record.emplace(uid, INIT_CONNECTED_NUM);
        }
    } else if (changeType == "2") { // remove
        if (findIter == record.end()) {
            return;
        }
        if (record[uid] != INIT_CONNECTED_NUM) {
            record[uid]--;
            return;
        }
        record.erase(findIter);
        if (!CheckIsDisSchedScene(uid)) {
            BgContinuousTaskMgr::GetInstance()->ReportTaskRunningStateUnmet(uid,
                UNSET_PID, MULTIDEVICE_CONNECTION_BGMODE_ID);
        }
    }
}

bool MultiDeviceDetect::CheckIsDisSchedScene(int32_t uid)
{
    if (callerRecords_.find(uid) != callerRecords_.end() || calleeRecords_.find(uid) != calleeRecords_.end()) {
        return true;
    }
    return false;
}

void MultiDeviceDetect::ParseDisSchedRecordToStr(nlohmann::json &value)
{
    nlohmann::json disScheduleInfo;

    ParseRecordToStrByType(disScheduleInfo, "callerRecords", callerRecords_);
    ParseRecordToStrByType(disScheduleInfo, "calleeRecords", calleeRecords_);
    value["disSchedule"] = disScheduleInfo;
}

void MultiDeviceDetect::ParseRecordToStrByType(nlohmann::json &value, const std::string &type,
    const std::map<int32_t, uint32_t> &record)
{
    auto arrayInfo = nlohmann::json::array();
    for (auto var : record) {
        nlohmann::json jsonRecord;
        jsonRecord["uid"] = var.first;
        jsonRecord["connectedNums"] = var.second;
        arrayInfo.push_back(jsonRecord);
    }
    value[type] = arrayInfo;
}

bool MultiDeviceDetect::ParseDisSchedRecordFromJson(const nlohmann::json &value, std::set<int32_t> &uidSet)
{
    // if (!value.isMember("disSchedule")) {
    //     return false;
    // }
    if (!CommonUtils::CheckJsonValue(value, {"disSchedule"})) {
        return;
    }

    nlohmann::json disScheduleInfo = value["disSchedule"];
    ParseRecordFromJsonByType(disScheduleInfo, uidSet, "callerRecords", callerRecords_);
    ParseRecordFromJsonByType(disScheduleInfo, uidSet, "calleeRecords", calleeRecords_);
    return true;
}

bool MultiDeviceDetect::ParseRecordFromJsonByType(const nlohmann::json &value, std::set<int32_t> &uidSet,
    const std::string &type, std::map<int32_t, uint32_t> &record)
{
    // nlohmann::json arrayObj = value[type];
    int32_t uid;
    int32_t nums;
    // for (uint32_t i = 0; i < arrayObj.size(); i++) {
    //     uid = arrayObj[i]["uid"].asInt();
    //     uidSet.emplace(uid);
    //     record[uid] = arrayObj[i]["connectedNums"].asInt();
    // }
    for (auto& elem : value[type]) {
        uid = value.at("uid").get<int32_t>();
        uid = value.at("connectedNums").get<int32_t>();
        uidSet.emplace(uid);
        record[uid] = nums;
    }
    return true;
}

void MultiDeviceDetect::ClearData()
{
    callerRecords_.clear();
    calleeRecords_.clear();
}
}
}