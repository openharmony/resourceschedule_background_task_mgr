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
#include "common_utils.h"
#include "continuous_task_log.h"
#include "task_detection_manager.h"

namespace OHOS {
namespace BackgroundTaskMgr {
void LocationDetect::HandleLocationSysEvent(const nlohmann::json &root)
{
    if (!CommonUtils::CheckJsonValue(root, {"name_"})) {
        BGTASK_LOGE("hisysevent data domain info lost");
        return;
    }
    std::string eventName = root.at("name_").get<std::string>();
    if (eventName == "GNSS_STATE") {
        if (!CommonUtils::CheckJsonValue(root, { "STATE", "UID", "PID" })) {
            return;
        }
        std::string state = root.at("STATE").get<std::string>();
        int32_t uid = root.at("UID").get<int32_t>();
        int32_t pid = root.at("PID").get<int32_t>();
        
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
                TaskDetectionManager::GetInstance()->ReportNeedRecheckTask(uid, CommonUtils::LOCATION_BGMODE_ID);
            }
        }
    } else if (eventName == "LOCATION_SWITCH_STATE") {
        if (!CommonUtils::CheckJsonValue(root, {"STATE"})) {
            return;
        }
        std::string switchState = root.at("STATE").get<std::string>();
        if (switchState == "enable") {
            isLocationSwitchOn_ = true;
        } else if (switchState == "disable") {
            isLocationSwitchOn_ = false;
            BgContinuousTaskMgr::GetInstance()->ReportNeedRecheckTask(CommonUtils::UNSET_UID,
                CommonUtils::LOCATION_BGMODE_ID);
        }
    }
}

bool LocationDetect::CheckLocationCondition(int32_t uid)
{
    if (uid == CommonUtils::UNSET_UID) {
        return isLocationSwitchOn_;
    }
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

void LocationDetect::ParseLocationRecordToStr(nlohmann::json &value)
{
    nlohmann::json locationInfo;
    locationInfo["location switch"] = isLocationSwitchOn_;
    auto arrayInfo = nlohmann::json::array();
    for (auto var : locationUsingRecords_) {
        nlohmann::json locationUsingRecord;
        locationUsingRecord["uid"] = var.first;
        locationUsingRecord["pid"] = var.second;
        arrayInfo.push_back(locationUsingRecord);
    }
    locationInfo["locationRecords"] = arrayInfo;
    value["location"] = locationInfo;
}

bool LocationDetect::ParseLocationRecordFromJson(const nlohmann::json &value, std::set<int32_t> &uidSet)
{
    if (!CommonUtils::CheckJsonValue(value, {"location"})) {
        return false;
    }

    nlohmann::json locationInfo = value["location"];
    this->isLocationSwitchOn_ = locationInfo.at("location switch").get<bool>();
    int32_t uid;
    int32_t pid;
    for (auto &elem : locationInfo["locationRecords"]) {
        uid = elem["uid"].get<int32_t>();
        pid = elem["pid"].get<int32_t>();
        uidSet.emplace(uid);
        auto pair = std::make_pair(uid, pid);
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