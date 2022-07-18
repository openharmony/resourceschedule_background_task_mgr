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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_LOCATION_DETECT_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_LOCATION_DETECT_H

#include <list>
#include <set>

#include "json/json.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class LocationDetect {
public:
    bool CheckLocationCondition(int32_t uid);
    void ParseLocationRecordToStr(Json::Value &value);
    bool ParseLocationRecordFromJson(const Json::Value &value, std::set<int32_t> &uidSet);
    void HandleLocationSysEvent(const Json::Value &root);
    void ClearData();

private:
    bool isLocationSwitchOn_ {false};
    std::list<std::pair<int32_t, int32_t>> locationUsingRecords_ {};

    friend class TaskDetectionManager;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_LOCATION_DETECT_H