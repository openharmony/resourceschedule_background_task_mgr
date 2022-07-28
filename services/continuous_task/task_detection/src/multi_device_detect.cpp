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
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::stringstream is(info.c_str());
    if (parseFromStream(reader, is, &root, &errors)) {
        if (!root.isMember("componentType") || root["componentType"].isNull()
            || !root.isMember("deviceType") || root["deviceType"].isNull()
            || !root.isMember("changeType") || root["changeType"].isNull()
            || !root.isMember("uid") || root["uid"].isNull()) {
            BGTASK_LOGE("reported distributed component change event lost important info");
            return;
        }
        std::string componentType = root["componentType"].asString();
        std::string deviceType = root["deviceType"].asString();
        std::string changeType = root["changeType"].asString();
        int32_t uid = atoi(root["uid"].asString().c_str());

        if (deviceType == "0") { // caller
            UpdateDisCallerInfo(uid, changeType);
        } else if (deviceType == "1") { // callee
            UpdateDisCalleeInfo(uid, changeType);
        }
    }
}

void MultiDeviceDetect::UpdateDisCallerInfo(int32_t uid, const std::string &changeType)
{
    auto findIter = callerRecords_.find(uid);
    if (changeType == "1") { // add
        if (findIter != callerRecords_.end()) {
            callerRecords_[uid]++;
        } else {
            callerRecords_.emplace(uid, INIT_CONNECTED_NUM);
        }
    } else if (changeType == "2") { // remove
        if (findIter == callerRecords_.end()) {
            return;
        }
        if (callerRecords_[uid] == INIT_CONNECTED_NUM) {
            callerRecords_.erase(findIter);
            if (!CheckIsDisSchedScene(uid)) {
                BgContinuousTaskMgr::GetInstance()->ReportTaskRunningStateUnmet(uid,
                    UNSET_PID, MULTIDEVICE_CONNECTION_BGMODE_ID);
            }
        } else {
            callerRecords_[uid]--;
        }
    }
}

void MultiDeviceDetect::UpdateDisCalleeInfo(int32_t uid, const std::string &changeType)
{
    auto findIter = calleeRecords_.find(uid);
    if (changeType == "1") { // add
        if (findIter != calleeRecords_.end()) {
            calleeRecords_[uid]++;
        } else {
            calleeRecords_.emplace(uid, INIT_CONNECTED_NUM);
        }
    } else if (changeType == "2") { // remove
        if (findIter == calleeRecords_.end()) {
            return;
        }
        if (calleeRecords_[uid] == INIT_CONNECTED_NUM) {
            calleeRecords_.erase(findIter);
            if (!CheckIsDisSchedScene(uid)) {
                BgContinuousTaskMgr::GetInstance()->ReportTaskRunningStateUnmet(uid,
                    UNSET_PID, MULTIDEVICE_CONNECTION_BGMODE_ID);
            }
        } else {
            calleeRecords_[uid]--;
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

void MultiDeviceDetect::ParseDisSchedRecordToStr(Json::Value &value)
{
    Json::Value disScheduleInfo;

    for (auto var : callerRecords_) {
        Json::Value callerRecord;
        callerRecord["uid"] = var.first;
        callerRecord["connectedNums"] = var.second;
        disScheduleInfo["callerRecords"].append(callerRecord);
    }

    for (auto var : calleeRecords_) {
        Json::Value calleeRecord;
        calleeRecord["uid"] = var.first;
        calleeRecord["connectedNums"] = var.second;
        disScheduleInfo["calleeRecords"].append(calleeRecord);
    }
    value["disSchedule"] = disScheduleInfo;
}

bool MultiDeviceDetect::ParseDisSchedRecordFromJson(const Json::Value &value, std::set<int32_t> &uidSet)
{
    if (!value.isMember("disSchedule")) {
        return false;
    }

    Json::Value disScheduleInfo = value["disSchedule"];
    Json::Value arrayObj = disScheduleInfo["callerRecords"];
    int32_t uid;
    for (uint32_t i = 0; i < arrayObj.size(); i++) {
        uid = arrayObj[i]["uid"].asInt();
        uidSet.emplace(uid);
        callerRecords_[uid] = arrayObj[i]["connectedNums"].asInt();
    }

    arrayObj = disScheduleInfo["calleeRecords"];
    for (uint32_t i = 0; i < arrayObj.size(); i++) {
        uid = arrayObj[i]["uid"].asInt();
        uidSet.emplace(uid);
        calleeRecords_[uid] = arrayObj[i]["connectedNums"].asInt();
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