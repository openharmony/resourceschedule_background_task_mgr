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

#include "location_detect.h"

#include "bg_continuous_task_mgr.h"
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr int32_t UNSET_UID = -1;
static constexpr int32_t UNSET_PID = -1;
static constexpr uint32_t LOCATION_BGMODE_ID = 4;
}

void LocationDetect::HandleLocationSysEvent(const Json::Value &root)
{
    if (!root.isMember("name_") || root["name_"].isNull()) {
        BGTASK_LOGE("hisysevent data domain info lost");
        return;
    }
    if (root["name_"].asString() == "GNSS_STATE") {
        if (!root.isMember("STATE") || !root.isMember("UID") || !root.isMember("PID")) {
            return;
        }
        std::string state = root["STATE"].asString();
        int32_t uid = atoi(root["UID"].asString().c_str());
        int32_t pid = atoi(root["PID"].asString().c_str());
        BGTASK_LOGI("GNSS_STATE: %{public}s", state.c_str());
        auto pair = std::make_pair(uid, pid);
        auto iter = find_if(locationUsingRecords_.begin(), locationUsingRecords_.end(),
            [pair](const auto &target) { return pair.first == target.first && pair.second == target.second; });
        if (state == "start") {
            if (iter == locationUsingRecords_.end()) {
                locationUsingRecords_.emplace_back(pair);
            }
        } else if (state == "stop") {
            if (iter != locationUsingRecords_.end()) {
                locationUsingRecords_.erase(iter);
                BgContinuousTaskMgr::GetInstance()->ReportTaskRunningStateUnmet(uid, pid, LOCATION_BGMODE_ID);
            }
        }
    } else if (root["name_"].asString() == "LOCATION_SWITCH_STATE") {
        if (!root.isMember("STATE")) {
            return;
        }
        std::string switchState = root["STATE"].asString();
        if (switchState == "enable") {
            isLocationSwitchOn_ = true;
        } else if (switchState == "disable") {
            isLocationSwitchOn_ = false;
            BgContinuousTaskMgr::GetInstance()->ReportTaskRunningStateUnmet(UNSET_UID,
                UNSET_PID, LOCATION_BGMODE_ID);
        }
    }
}

bool LocationDetect::CheckLocationCondition(int32_t uid)
{
    if (!isLocationSwitchOn_) {
            return false;
        }
    for (auto var : locationUsingRecords_) {
        if (var.first == uid) {
            return true;
        }
    }
    return false;
}

void LocationDetect::ParseLocationRecordToStr(Json::Value &value)
{
    Json::Value locationInfo;
    locationInfo["location switch"] = isLocationSwitchOn_;
    for (auto var : locationUsingRecords_) {
        Json::Value locationUsingRecord;
        locationUsingRecord["uid"] = var.first;
        locationUsingRecord["pid"] = var.second;
        locationInfo["locationRecords"].append(locationUsingRecord);
    }
    value["location"] = locationInfo;
}

bool LocationDetect::ParseLocationRecordFromJson(const Json::Value &value, std::set<int32_t> &uidSet)
{
    if (!value.isMember("location")) {
        return false;
    }

    Json::Value locationInfo = value["location"];
    this->isLocationSwitchOn_ = locationInfo["location switch"].asBool();
    Json::Value arrayObj = locationInfo["locationRecords"];
    for (uint32_t i = 0; i < arrayObj.size(); i++) {
        int32_t uid = arrayObj[i]["uid"].asInt();
        uidSet.emplace(uid);
        auto pair = std::make_pair(uid, arrayObj[i]["pid"].asInt());
        locationUsingRecords_.emplace_back(pair);
    }
    return true;
}

void LocationDetect::ClearData()
{
    locationUsingRecords_.clear();
}
}
}