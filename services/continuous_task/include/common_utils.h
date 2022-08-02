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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_COMMON_UTILS_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_COMMON_UTILS_H

#include "nlohmann/json.hpp"

namespace OHOS {
namespace BackgroundTaskMgr {
class CommonUtils {
public:
    static bool CheckJsonValue(const nlohmann::json &value, std::initializer_list<std::string> params)
    {
        for (auto param : params) {
            if (value.find(param) == value.end()) {
                return false;
            }
        }
        return true;
    }

    template<typename T>
    static bool CheckIsUidExist(int32_t uid, const T &t)
    {
        for (auto var : t) {
            if (var->uid_ == uid) {
                return true;
            }
        }
        return false;
    }

public:
    static constexpr int32_t UNSET_PID = -1;
    static constexpr int32_t UNSET_UID = -1;
    static constexpr int32_t JSON_FORMAT = 4;
    static constexpr int32_t AUDIO_RUNNING_STATE = 2;
    static constexpr int32_t GATT_ROLE_MASTER = 0;
    static constexpr int32_t GATT_ROLE_SLAVE = 1;
    static constexpr int32_t INIT_CONNECTED_NUM = 1;
    static constexpr int32_t GATT_CONNECT = 0;
    static constexpr int32_t GATT_RECONNECT = 4;
    static constexpr int32_t BT_SWITCH_TURN_ON = 1;
    static constexpr int32_t BT_SWITCH_TURN_OFF = 3;
    static constexpr int32_t DIS_TYPE_CALLER = 0;
    static constexpr int32_t DIS_TYPE_CALLEE = 1;
    static constexpr int32_t DIS_ACTION_ADD = 1;
    static constexpr int32_t DIS_ACTION_REMOVE = 2;
    static constexpr uint32_t AUDIO_PLAYBACK_BGMODE_ID = 2;
    static constexpr uint32_t AUDIO_RECORDING_BGMODE_ID = 3;
    static constexpr uint32_t LOCATION_BGMODE_ID = 4;
    static constexpr uint32_t BLUETOOTH_INTERACTION_BGMODE_ID = 5;
    static constexpr uint32_t MULTIDEVICE_CONNECTION_BGMODE_ID = 6;
    static constexpr uint32_t SOFTBUS_SA_UID = 1024;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_COMMON_UTILS_H