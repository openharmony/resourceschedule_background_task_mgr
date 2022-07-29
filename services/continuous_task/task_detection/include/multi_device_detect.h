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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_MULTI_DEVICE_DETECT_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_MULTI_DEVICE_DETECT_H

#include <list>
#include <set>

#include "json/json.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class MultiDeviceDetect {
public:
    void HandleDisComponentChange(const std::string &info);
    bool CheckIsDisSchedScene(int32_t uid);
    void ParseDisSchedRecordToStr(Json::Value &value);
    bool ParseDisSchedRecordFromJson(const Json::Value &value, std::set<int32_t> &uidSet);
    void ClearData();

private:
    void UpdateDisComponentInfo(int32_t uid, const std::string &changeType,
        std::map<int32_t, uint32_t> &record);
    void ParseRecordToStrByType(Json::Value &value, const std::string &type,
        const std::map<int32_t, uint32_t> &record);
    bool ParseRecordFromJsonByType(const Json::Value &value, std::set<int32_t> &uidSet,
        const std::string &type, std::map<int32_t, uint32_t> &record);

private:
    std::map<int32_t, uint32_t> callerRecords_ {};
    std::map<int32_t, uint32_t> calleeRecords_ {};

    friend class TaskDetectionManager;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_MULTI_DEVICE_DETECT_H