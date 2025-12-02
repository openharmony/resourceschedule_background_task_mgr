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

#include <unordered_map>

#include "background_task_submode.h"
#include "background_task_mode.h"
#include "background_mode.h"

namespace OHOS {
namespace BackgroundTaskMgr {
const std::unordered_map<uint32_t, std::string> PARAM_BACKGROUND_TASK_MODE_STR_MAP = {
    {BackgroundTaskMode::MODE_DATA_TRANSFER, "modeDataTransfer"},
    {BackgroundTaskMode::MODE_AUDIO_PLAYBACK, "modeAudioPlayback"},
    {BackgroundTaskMode::MODE_AUDIO_RECORDING, "modeAudioRecording"},
    {BackgroundTaskMode::MODE_LOCATION, "modeLocation"},
    {BackgroundTaskMode::MODE_BLUETOOTH_INTERACTION, "modeBluetoothInteraction"},
    {BackgroundTaskMode::MODE_MULTI_DEVICE_CONNECTION, "modeMultiDeviceConnection"},
    {BackgroundTaskMode::MODE_ALLOW_WIFI_AWARE, "modeAllowWifiAware"},
    {BackgroundTaskMode::MODE_VOIP, "modeVoip"},
    {BackgroundTaskMode::MODE_TASK_KEEPING, "modeTaskKeeping"},
    {BackgroundTaskMode::MODE_AV_PLAYBACK_AND_RECORD, "modeAVPlaybackAndRecord"},
    {BackgroundTaskMode::MODE_SPECIAL_SCENARIO_PROCESSING, "modeSpecialScenarioProcessing"},
    {BackgroundTaskMode::END, "end"}
};

const std::unordered_map<uint32_t, uint32_t> PARAM_BACKGROUND_TASK_MODE_CORRESPONDENCE_SUBMODE = {
    {BackgroundTaskSubmode::SUBMODE_LIVE_VIEW_NOTIFICATION, BackgroundTaskMode::MODE_DATA_TRANSFER},
    {BackgroundTaskSubmode::SUBMODE_CAR_KEY_NORMAL_NOTIFICATION, BackgroundTaskMode::MODE_BLUETOOTH_INTERACTION},
    {BackgroundTaskSubmode::SUBMODE_AUDIO_PLAYBACK_NORMAL_NOTIFICATION,
        BackgroundTaskMode::MODE_AV_PLAYBACK_AND_RECORD},
    {BackgroundTaskSubmode::SUBMODE_AVSESSION_AUDIO_PLAYBACK, BackgroundTaskMode::MODE_AV_PLAYBACK_AND_RECORD},
    {BackgroundTaskSubmode::SUBMODE_AUDIO_RECORD_NORMAL_NOTIFICATION, BackgroundTaskMode::MODE_AV_PLAYBACK_AND_RECORD},
    {BackgroundTaskSubmode::SUBMODE_VOICE_CHAT_NORMAL_NOTIFICATION, BackgroundTaskMode::MODE_AV_PLAYBACK_AND_RECORD},
    {BackgroundTaskSubmode::SUBMODE_MEDIA_PROCESS_NORMAL_NOTIFICATION,
        BackgroundTaskMode::MODE_SPECIAL_SCENARIO_PROCESSING},
    {BackgroundTaskSubmode::SUBMODE_VIDEO_BROADCAST_NORMAL_NOTIFICATION,
        BackgroundTaskMode::MODE_SPECIAL_SCENARIO_PROCESSING},
    {BackgroundTaskSubmode::SUBMODE_WORK_OUT_NORMAL_NOTIFICATION, BackgroundTaskMode::MODE_SPECIAL_SCENARIO_PROCESSING},
};

const std::unordered_map<uint32_t, uint32_t> PARAM_BACKGROUND_TASK_SUBMODE_CORRESPONDENCE_MODE = {
    {BackgroundTaskMode::MODE_AUDIO_PLAYBACK, BackgroundTaskSubmode::SUBMODE_NORMAL_NOTIFICATION},
    {BackgroundTaskMode::MODE_AUDIO_RECORDING, BackgroundTaskSubmode::SUBMODE_NORMAL_NOTIFICATION},
    {BackgroundTaskMode::MODE_LOCATION, BackgroundTaskSubmode::SUBMODE_NORMAL_NOTIFICATION},
    {BackgroundTaskMode::MODE_BLUETOOTH_INTERACTION, BackgroundTaskSubmode::SUBMODE_NORMAL_NOTIFICATION},
    {BackgroundTaskMode::MODE_MULTI_DEVICE_CONNECTION, BackgroundTaskSubmode::SUBMODE_NORMAL_NOTIFICATION},
    {BackgroundTaskMode::MODE_ALLOW_WIFI_AWARE, BackgroundTaskSubmode::SUBMODE_NORMAL_NOTIFICATION},
    {BackgroundTaskMode::MODE_VOIP, BackgroundTaskSubmode::SUBMODE_NORMAL_NOTIFICATION},
    {BackgroundTaskMode::MODE_TASK_KEEPING, BackgroundTaskSubmode::SUBMODE_NORMAL_NOTIFICATION}
};

const std::unordered_map<uint32_t, uint32_t> PARAM_CONTINUOUS_TASK_V9MODE_CORRESPONDENCE_V21MODE = {
    {BackgroundTaskMode::MODE_AUDIO_PLAYBACK, BackgroundMode::AUDIO_PLAYBACK},
    {BackgroundTaskMode::MODE_AUDIO_RECORDING, BackgroundMode::AUDIO_RECORDING},
    {BackgroundTaskMode::MODE_LOCATION, BackgroundMode::LOCATION},
    {BackgroundTaskMode::MODE_BLUETOOTH_INTERACTION, BackgroundMode::BLUETOOTH_INTERACTION},
    {BackgroundTaskMode::MODE_MULTI_DEVICE_CONNECTION, BackgroundMode::MULTI_DEVICE_CONNECTION},
    {BackgroundTaskMode::MODE_ALLOW_WIFI_AWARE, BackgroundMode::WIFI_INTERACTION},
    {BackgroundTaskMode::MODE_VOIP, BackgroundMode::VOIP},
    {BackgroundTaskMode::MODE_TASK_KEEPING, BackgroundMode::TASK_KEEPING},
    {BackgroundTaskMode::MODE_SPECIAL_SCENARIO_PROCESSING, BackgroundMode::SPECIAL_SCENARIO_PROCESSING},
};

const std::unordered_map<uint32_t, uint32_t> PARAM_CONTINUOUS_TASK_V9MODE_CORRESPONDENCE_V21SUBMODE = {
    {BackgroundTaskSubmode::SUBMODE_LIVE_VIEW_NOTIFICATION, BackgroundMode::DATA_TRANSFER},
    {BackgroundTaskSubmode::SUBMODE_CAR_KEY_NORMAL_NOTIFICATION, BackgroundMode::BLUETOOTH_INTERACTION},
    {BackgroundTaskSubmode::SUBMODE_AUDIO_PLAYBACK_NORMAL_NOTIFICATION, BackgroundMode::AUDIO_PLAYBACK},
    {BackgroundTaskSubmode::SUBMODE_AVSESSION_AUDIO_PLAYBACK, BackgroundMode::AUDIO_PLAYBACK},
    {BackgroundTaskSubmode::SUBMODE_AUDIO_RECORD_NORMAL_NOTIFICATION, BackgroundMode::AUDIO_RECORDING},
    {BackgroundTaskSubmode::SUBMODE_VOICE_CHAT_NORMAL_NOTIFICATION, BackgroundMode::VOIP},
    {BackgroundTaskSubmode::SUBMODE_MEDIA_PROCESS_NORMAL_NOTIFICATION, BackgroundMode::SPECIAL_SCENARIO_PROCESSING},
    {BackgroundTaskSubmode::SUBMODE_VIDEO_BROADCAST_NORMAL_NOTIFICATION, BackgroundMode::SPECIAL_SCENARIO_PROCESSING},
    {BackgroundTaskSubmode::SUBMODE_WORK_OUT_NORMAL_NOTIFICATION, BackgroundMode::SPECIAL_SCENARIO_PROCESSING},
};

std::string BackgroundTaskMode::GetBackgroundTaskModeStr(uint32_t mode)
{
    auto iter = PARAM_BACKGROUND_TASK_MODE_STR_MAP.find(mode);
    if (iter != PARAM_BACKGROUND_TASK_MODE_STR_MAP.end()) {
        return iter->second;
    }
    return "default";
}

uint32_t BackgroundTaskMode::GetSubModeTypeMatching(const uint32_t backgroundTaskSubMode)
{
    auto iter = PARAM_BACKGROUND_TASK_MODE_CORRESPONDENCE_SUBMODE.find(backgroundTaskSubMode);
    if (iter != PARAM_BACKGROUND_TASK_MODE_CORRESPONDENCE_SUBMODE.end()) {
        return iter->second;
    } else {
        return BackgroundTaskMode::END;
    }
}

bool BackgroundTaskMode::IsModeTypeMatching(const uint32_t backgroundTaskMode)
{
    auto iter = PARAM_BACKGROUND_TASK_SUBMODE_CORRESPONDENCE_MODE.find(backgroundTaskMode);
    return iter != PARAM_BACKGROUND_TASK_SUBMODE_CORRESPONDENCE_MODE.end();
}

uint32_t BackgroundTaskMode::GetV9BackgroundModeByMode(const uint32_t backgroundTaskMode)
{
    auto iter = PARAM_CONTINUOUS_TASK_V9MODE_CORRESPONDENCE_V21MODE.find(backgroundTaskMode);
    if (iter != PARAM_CONTINUOUS_TASK_V9MODE_CORRESPONDENCE_V21MODE.end()) {
        return iter->second;
    } else {
        return BackgroundMode::END;
    }
}

uint32_t BackgroundTaskMode::GetV9BackgroundModeBySubMode(const uint32_t backgroundTaskSubMode)
{
    auto iter = PARAM_CONTINUOUS_TASK_V9MODE_CORRESPONDENCE_V21SUBMODE.find(backgroundTaskSubMode);
    if (iter != PARAM_CONTINUOUS_TASK_V9MODE_CORRESPONDENCE_V21SUBMODE.end()) {
        return iter->second;
    } else {
        return BackgroundMode::END;
    }
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS