/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_TASK_MODE_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_TASK_MODE_H

#include <string>

namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundTaskMode {
public:
    virtual ~BackgroundTaskMode() = default;
    enum Type : uint32_t {
        MODE_DATA_TRANSFER = 1,
        MODE_AUDIO_PLAYBACK = 2,
        MODE_AUDIO_RECORDING = 3,
        MODE_LOCATION = 4,
        MODE_BLUETOOTH_INTERACTION = 5,
        MODE_MULTI_DEVICE_CONNECTION = 6,
        MODE_ALLOW_WIFI_AWARE = 7,
        MODE_VOIP = 8,
        MODE_TASK_KEEPING = 9,
        MODE_AV_PLAYBACK_AND_RECORD = 12,
        MODE_SPECIAL_SCENARIO_PROCESSING = 13,
        END,
    };

    static std::string GetBackgroundTaskModeStr(uint32_t mode);
    static uint32_t GetSubModeTypeMatching(const uint32_t backgroundTaskSubMode);
    static bool IsModeTypeMatching(const uint32_t backgroundTaskMode);
    static uint32_t GetV9BackgroundModeByMode(const uint32_t backgroundTaskMode);
    static uint32_t GetV9BackgroundModeBySubMode(const uint32_t backgroundTaskSubMode);
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_TASK_MODE_H