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

#include "json/json.h"
// #include "singleton.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class CommonUtils {
public:
    static bool CheckJsonValue(const Json::Value &value, std::initializer_list<std::string> params)
    {
        for (auto param : params) {
            if (!value.isMember(param) || value[param].isNull()) {
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

    // template<typename T>
    // static void ParseAudioStreamInfoToStr(Json::Value &value, const std::string &type, const T &record)
    // {
    //     Json::Value streamInfos;
    //     for (auto var : record) {
    //         Json::Value info;
    //         info["uid"] = var->uid_;
    //         info["sessionId"] = var->sessionId_;
    //         streamInfos.append(info);
    //     }
    //     value[type] = streamInfos;
    // }

    // template<typename T>
    // static void ParseAudioStreamInfoFromJson(int32_t uid, const T &t)

    // DECLARE_DELAYED_SINGLETON(CommonUtils);

public:
    static constexpr int32_t UNSET_PID = -1;
    static constexpr int32_t UNSET_UID = -1;
    static constexpr int32_t AUDIO_RUNNING_STATE = 2;
    static constexpr uint32_t AUDIO_PLAYBACK_BGMODE_ID = 2;
    static constexpr uint32_t AUDIO_RECORDING_BGMODE_ID = 3;
    static constexpr char GATT_CONNECT[] = "0";
    static constexpr char GATT_RECONNECT[] = "4";
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_COMMON_UTILS_H