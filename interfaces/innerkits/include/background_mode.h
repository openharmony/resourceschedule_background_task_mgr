/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_MODE_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_MODE_H

#include <string>

namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundMode {
public:
    virtual ~BackgroundMode() = default;
    enum Type : uint32_t {
        DATA_TRANSFER = 1,
        AUDIO_PLAYBACK = 2,
        AUDIO_RECORDING = 3,
        LOCATION = 4,
        BLUETOOTH_INTERACTION = 5,
        MULTI_DEVICE_CONNECTION = 6,
        WIFI_INTERACTION = 7,
        VOIP = 8,
        TASK_KEEPING = 9,
        WORKOUT = 10,
        SPECIAL_SCENARIO_PROCESSING = 13,
        END,
    };

    static std::string GetBackgroundModeStr(uint32_t mode);
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_MODE_H