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

#include "continuous_task_cancel_reason.h"
#include "background_mode.h"

namespace OHOS {
namespace BackgroundTaskMgr {
const std::unordered_map<uint32_t, std::string> PARAM_BACKGROUND_MODE_STR_MAP = {
    {BackgroundMode::DATA_TRANSFER, "dataTransfer"},
    {BackgroundMode::AUDIO_PLAYBACK, "audioPlayback"},
    {BackgroundMode::AUDIO_RECORDING, "audioRecording"},
    {BackgroundMode::LOCATION, "location"},
    {BackgroundMode::BLUETOOTH_INTERACTION, "bluetoothInteraction"},
    {BackgroundMode::MULTI_DEVICE_CONNECTION, "multiDeviceConnection"},
    {BackgroundMode::WIFI_INTERACTION, "wifiInteraction"},
    {BackgroundMode::VOIP, "voip"},
    {BackgroundMode::TASK_KEEPING, "taskKeeping"},
    {BackgroundMode::WORKOUT, "workout"},
    {BackgroundMode::SPECIAL_SCENARIO_PROCESSING, "specialScenarioProcessing"},
    {BackgroundMode::NEARLINK, "nearlink"},
    {BackgroundMode::END, "end"}
};

const std::unordered_map<uint32_t, uint32_t> BACKGROUND_MODE_TO_CANCEL_REASON_MAP = {
    {BackgroundMode::DATA_TRANSFER, ContinuousTaskCancelReason::SYSTEM_CANCEL_DATA_TRANSFER_LOW_SPEED},
    {BackgroundMode::AUDIO_PLAYBACK, ContinuousTaskCancelReason::SYSTEM_CANCEL_AUDIO_PLAYBACK_NOT_RUNNING},
    {BackgroundMode::AUDIO_RECORDING, ContinuousTaskCancelReason::SYSTEM_CANCEL_AUDIO_RECORDING_NOT_RUNNING},
    {BackgroundMode::LOCATION, ContinuousTaskCancelReason::SYSTEM_CANCEL_NOT_USE_LOCATION},
    {BackgroundMode::BLUETOOTH_INTERACTION, ContinuousTaskCancelReason::SYSTEM_CANCEL_NOT_USE_BLUETOOTH},
    {BackgroundMode::MULTI_DEVICE_CONNECTION, ContinuousTaskCancelReason::SYSTEM_CANCEL_NOT_USE_MULTI_DEVICE},
    {BackgroundMode::VOIP, ContinuousTaskCancelReason::SYSTEM_CANCEL_VOIP_NOT_RUNNING},
    {BackgroundMode::SPECIAL_SCENARIO_PROCESSING, ContinuousTaskCancelReason::SYSTEM_CANCEL_USER_UNAUTHORIZED},
    {BackgroundMode::END, ContinuousTaskCancelReason::INVALID_REASON}
};

std::string BackgroundMode::GetBackgroundModeStr(uint32_t mode)
{
    auto iter = PARAM_BACKGROUND_MODE_STR_MAP.find(mode);
    if (iter != PARAM_BACKGROUND_MODE_STR_MAP.end()) {
        return iter->second.c_str();
    }
    return "default";
}

int32_t BackgroundMode::GetDetailedCancelReasonFromMode(uint32_t mode)
{
    auto iter = BACKGROUND_MODE_TO_CANCEL_REASON_MAP.find(mode);
    if (iter != BACKGROUND_MODE_TO_CANCEL_REASON_MAP.end()) {
        return iter->second;
    }
    return ContinuousTaskCancelReason::INVALID_REASON;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS