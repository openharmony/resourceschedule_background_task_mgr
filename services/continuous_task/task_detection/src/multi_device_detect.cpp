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

#include "bg_continuous_task_mgr.h"
#include "common_utils.h"
#include "continuous_task_log.h"
#include "task_detection_manager.h"

namespace OHOS {
namespace BackgroundTaskMgr {
void MultiDeviceDetect::HandleDisComponentChange(const std::string &info)
{
    nlohmann::json root = nlohmann::json::parse(info, nullptr, false);
    if (root.is_discarded()) {
        BGTASK_LOGE("Parse json value From stream failed");
        return;
    }
    if (!CommonUtils::CheckJsonValue(root, { "deviceType", "changeType", "uid" })) {
        BGTASK_LOGE("reported distributed component change event lost important info");
        return;
    }
    int32_t deviceType = root.at("deviceType").get<int32_t>();
    int32_t changeType = root.at("changeType").get<int32_t>();
    int32_t uid = root.at("uid").get<int32_t>();

    if (deviceType == CommonUtils::DIS_TYPE_CALLER) { // caller
        UpdateDisComponentInfo(uid, changeType, callerRecords_);
    } else if (deviceType == CommonUtils::DIS_TYPE_CALLEE) { // callee
        UpdateDisComponentInfo(uid, changeType, calleeRecords_);
    }
}

void MultiDeviceDetect::UpdateDisComponentInfo(int32_t uid, int32_t changeType,
    std::map<int32_t, uint32_t> &record)
{
    auto findIter = record.find(uid);
    if (changeType == CommonUtils::DIS_ACTION_ADD) { // add
        if (findIter != record.end()) {
            record[uid]++;
        } else {
            record.emplace(uid, CommonUtils::INIT_CONNECTED_NUM);
        }
    } else if (changeType == CommonUtils::DIS_ACTION_REMOVE) { // remove
        if (findIter == record.end()) {
            return;
        }
        if (record[uid] != CommonUtils::INIT_CONNECTED_NUM) {
            record[uid]--;
            return;
        }
        record.erase(findIter);
        if (!CheckIsDisSchedScene(uid)) {
            TaskDetectionManager::GetInstance()->ReportNeedRecheckTask(uid,
                CommonUtils::MULTIDEVICE_CONNECTION_BGMODE_ID);
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
    if (!CommonUtils::CheckJsonValue(value, {"disSchedule"})) {
        return false;
    }

    nlohmann::json disScheduleInfo = value["disSchedule"];
    ParseRecordFromJsonByType(disScheduleInfo, uidSet, "callerRecords", callerRecords_);
    ParseRecordFromJsonByType(disScheduleInfo, uidSet, "calleeRecords", calleeRecords_);
    return true;
}

bool MultiDeviceDetect::ParseRecordFromJsonByType(const nlohmann::json &value, std::set<int32_t> &uidSet,
    const std::string &type, std::map<int32_t, uint32_t> &record)
{
    int32_t uid;
    uint32_t nums;
    for (auto& elem : value[type]) {
        uid = elem.at("uid").get<int32_t>();
        nums = elem.at("connectedNums").get<uint32_t>();
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