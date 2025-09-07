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

#include "continuous_task_submode.h"
#include "continuous_task_mode.h"
#include "background_mode.h"

namespace OHOS {
namespace BackgroundTaskMgr {
const std::unordered_map<uint32_t, std::string> PARAM_CONTINUOUS_TASK_MODE_STR_MAP = {
    {ContinuousTaskMode::MODE_DATA_TRANSFER, "modeDataTransfer"},
    {ContinuousTaskMode::MODE_SHARE_POSITION, "modeSharePosition"},
    {ContinuousTaskMode::MODE_ALLOW_BLUETOOTH_AWARE, "modeAllowBluetoothAware"},
    {ContinuousTaskMode::MODE_MULTI_DEVICE_CONNECTION, "modeMultiDeviceConnection"},
    {ContinuousTaskMode::MODE_ALLOW_WIFI_AWARE, "modeAllowWifiAware"},
    {ContinuousTaskMode::MODE_TASK_KEEPING, "modeTaskKeeping"},
    {ContinuousTaskMode::MODE_AV_PLAYBACK_AND_RECORD, "modeAVPlaybackAndRecord"},
    {ContinuousTaskMode::END, "end"}
};

const std::unordered_map<uint32_t, uint32_t> PARAM_CONTINUOUS_TASK_MODE_CORRESPONDENCE_SUBMODE = {
    {ContinuousTaskSubmode::SUBMODE_LIVE_VIEW_NOTIFICATION, ContinuousTaskMode::MODE_DATA_TRANSFER},
    {ContinuousTaskSubmode::SUBMODE_CAR_KEY_NORMAL_NOTIFICATION, ContinuousTaskMode::MODE_ALLOW_BLUETOOTH_AWARE},
    {ContinuousTaskSubmode::SUBMODE_AUDIO_PLAYBACK_NORMAL_NOTIFICATION,
        ContinuousTaskMode::MODE_AV_PLAYBACK_AND_RECORD},
    {ContinuousTaskSubmode::SUBMODE_AVSESSION_AUDIO_PLAYBACK, ContinuousTaskMode::MODE_AV_PLAYBACK_AND_RECORD},
    {ContinuousTaskSubmode::SUBMODE_AUDIO_RECORD_NORMAL_NOTIFICATION, ContinuousTaskMode::MODE_AV_PLAYBACK_AND_RECORD},
    {ContinuousTaskSubmode::SUBMODE_VOICE_CHAT_NORMAL_NOTIFICATION, ContinuousTaskMode::MODE_AV_PLAYBACK_AND_RECORD}
};

const std::unordered_map<uint32_t, uint32_t> PARAM_CONTINUOUS_TASK_SUBMODE_CORRESPONDENCE_MODE = {
    {ContinuousTaskMode::MODE_SHARE_POSITION, ContinuousTaskSubmode::SUBMODE_NORMAL_NOTIFICATION},
    {ContinuousTaskMode::MODE_ALLOW_BLUETOOTH_AWARE, ContinuousTaskSubmode::SUBMODE_NORMAL_NOTIFICATION},
    {ContinuousTaskMode::MODE_MULTI_DEVICE_CONNECTION, ContinuousTaskSubmode::SUBMODE_NORMAL_NOTIFICATION},
    {ContinuousTaskMode::MODE_ALLOW_WIFI_AWARE, ContinuousTaskSubmode::SUBMODE_NORMAL_NOTIFICATION},
    {ContinuousTaskMode::MODE_TASK_KEEPING, ContinuousTaskSubmode::SUBMODE_NORMAL_NOTIFICATION}
};

const std::unordered_map<uint32_t, uint32_t> PARAM_CONTINUOUS_TASK_V9MODE_CORRESPONDENCE_V21MODE = {
    {ContinuousTaskMode::MODE_SHARE_POSITION, BackgroundMode::LOCATION},
    {ContinuousTaskMode::MODE_ALLOW_BLUETOOTH_AWARE, BackgroundMode::BLUETOOTH_INTERACTION},
    {ContinuousTaskMode::MODE_MULTI_DEVICE_CONNECTION, BackgroundMode::MULTI_DEVICE_CONNECTION},
    {ContinuousTaskMode::MODE_ALLOW_WIFI_AWARE, BackgroundMode::WIFI_INTERACTION},
    {ContinuousTaskMode::MODE_TASK_KEEPING, BackgroundMode::TASK_KEEPING},
};

const std::unordered_map<uint32_t, uint32_t> PARAM_CONTINUOUS_TASK_V9MODE_CORRESPONDENCE_V21SUBMODE = {
    {ContinuousTaskSubmode::SUBMODE_LIVE_VIEW_NOTIFICATION, BackgroundMode::DATA_TRANSFER},
    {ContinuousTaskSubmode::SUBMODE_CAR_KEY_NORMAL_NOTIFICATION, BackgroundMode::BLUETOOTH_INTERACTION},
    {ContinuousTaskSubmode::SUBMODE_AUDIO_PLAYBACK_NORMAL_NOTIFICATION, BackgroundMode::AUDIO_PLAYBACK},
    {ContinuousTaskSubmode::SUBMODE_AVSESSION_AUDIO_PLAYBACK, BackgroundMode::AUDIO_PLAYBACK},
    {ContinuousTaskSubmode::SUBMODE_AUDIO_RECORD_NORMAL_NOTIFICATION, BackgroundMode::AUDIO_RECORDING},
    {ContinuousTaskSubmode::SUBMODE_VOICE_CHAT_NORMAL_NOTIFICATION, BackgroundMode::VOIP},
};

std::string ContinuousTaskMode::GetContinuousTaskModeStr(uint32_t mode)
{
    auto iter = PARAM_CONTINUOUS_TASK_MODE_STR_MAP.find(mode);
    if (iter != PARAM_CONTINUOUS_TASK_MODE_STR_MAP.end()) {
        return iter->second;
    }
    return "default";
}

uint32_t ContinuousTaskMode::GetSubModeTypeMatching(const uint32_t continuousTaskSubMode)
{
    auto iter = PARAM_CONTINUOUS_TASK_MODE_CORRESPONDENCE_SUBMODE.find(continuousTaskSubMode);
    if (iter != PARAM_CONTINUOUS_TASK_MODE_CORRESPONDENCE_SUBMODE.end()) {
        return iter->second;
    } else {
        return ContinuousTaskMode::END;
    }
}

bool ContinuousTaskMode::IsModeTypeMatching(const uint32_t continuousTaskMode)
{
    auto iter = PARAM_CONTINUOUS_TASK_SUBMODE_CORRESPONDENCE_MODE.find(continuousTaskMode);
    return iter != PARAM_CONTINUOUS_TASK_SUBMODE_CORRESPONDENCE_MODE.end();
}

uint32_t ContinuousTaskMode::GetV9BackgroundModeByMode(const uint32_t continuousTaskMode)
{
    auto iter = PARAM_CONTINUOUS_TASK_V9MODE_CORRESPONDENCE_V21MODE.find(continuousTaskMode);
    if (iter != PARAM_CONTINUOUS_TASK_V9MODE_CORRESPONDENCE_V21MODE.end()) {
        return iter->second;
    } else {
        return BackgroundMode::END;
    }
}

uint32_t ContinuousTaskMode::GetV9BackgroundModeBySubMode(const uint32_t continuousTaskSubMode)
{
    auto iter = PARAM_CONTINUOUS_TASK_V9MODE_CORRESPONDENCE_V21SUBMODE.find(continuousTaskSubMode);
    if (iter != PARAM_CONTINUOUS_TASK_V9MODE_CORRESPONDENCE_V21SUBMODE.end()) {
        return iter->second;
    } else {
        return BackgroundMode::END;
    }
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS