/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "background_mode.h"
#include "continuous_task_suspend_reason.h"

namespace OHOS {
namespace BackgroundTaskMgr {
const std::unordered_map<uint32_t, uint32_t> PARAM_SUSPEND_REASON = {
    {BackgroundMode::DATA_TRANSFER, ContinuousTaskSuspendReason::SYSTEM_SUSPEND_DATA_TRANSFER_LOW_SPEED},
    {BackgroundMode::AUDIO_PLAYBACK, ContinuousTaskSuspendReason::SYSTEM_SUSPEND_AUDIO_PLAYBACK_NOT_RUNNING},
    {BackgroundMode::AUDIO_RECORDING, ContinuousTaskSuspendReason::SYSTEM_SUSPEND_AUDIO_RECORDING_NOT_RUNNING},
    {BackgroundMode::LOCATION, ContinuousTaskSuspendReason::SYSTEM_SUSPEND_LOCATION_NOT_USED},
    {BackgroundMode::BLUETOOTH_INTERACTION, ContinuousTaskSuspendReason::SYSTEM_SUSPEND_BLUETOOTH_NOT_USED},
    {BackgroundMode::MULTI_DEVICE_CONNECTION, ContinuousTaskSuspendReason::SYSTEM_SUSPEND_MULTI_DEVICE_NOT_USED},
    {BackgroundMode::VOIP, ContinuousTaskSuspendReason::SYSTEM_SUSPEND_VOIP_NOT_USED},
    {BackgroundMode::SPECIAL_SCENARIO_PROCESSING, ContinuousTaskSuspendReason::SYSTEM_SUSPEND_USER_UNAUTHORIZED},
    {BackgroundMode::NEARLINK, ContinuousTaskSuspendReason::SYSTEM_SUSPEND_NEARLINK_NOT_USED}
};

const std::unordered_map<uint32_t, uint32_t> STANDBY_SUSPEND_REASON = {
    {BackgroundMode::AUDIO_PLAYBACK, ContinuousTaskSuspendReason::SYSTEM_SUSPEND_AUDIO_PLAYBACK_MUTE},
    {BackgroundMode::LOCATION, ContinuousTaskSuspendReason::SYSTEM_SUSPEND_POSITION_NOT_MOVED},
    {BackgroundMode::BLUETOOTH_INTERACTION, ContinuousTaskSuspendReason::SYSTEM_SUSPEND_BLUETOOTH_DATA_NOT_EXIST},
    {BackgroundMode::NEARLINK, ContinuousTaskSuspendReason::SYSTEM_SUSPEND_NEARLINK_DATA_NOT_EXIST}
};

const std::unordered_map<uint32_t, std::string> SUSPEND_REASON_TO_MESSAGE = {
    {ContinuousTaskSuspendReason::SYSTEM_SUSPEND_DATA_TRANSFER_LOW_SPEED,
        "For a period of time, the application did not use data transfer or the data transfer rate was too low."},
    {ContinuousTaskSuspendReason::SYSTEM_SUSPEND_AUDIO_PLAYBACK_NOT_USE_AVSESSION,
        "The application did use avsession when request audio playback mode."},
    {ContinuousTaskSuspendReason::SYSTEM_SUSPEND_AUDIO_PLAYBACK_NOT_RUNNING,
        "Audio is not running when request audio playback mode."},
    {ContinuousTaskSuspendReason::SYSTEM_SUSPEND_AUDIO_RECORDING_NOT_RUNNING,
        "No recording stream detected when applying for audio recording mode"},
    {ContinuousTaskSuspendReason::SYSTEM_SUSPEND_LOCATION_NOT_USED,
        "Not use location when request location mode."},
    {ContinuousTaskSuspendReason::SYSTEM_SUSPEND_BLUETOOTH_NOT_USED,
        "Bluetooth connection not detected when request bluetooth interaction mode."},
    {ContinuousTaskSuspendReason::SYSTEM_SUSPEND_MULTI_DEVICE_NOT_USED,
        "Not use multi device when request multi-device connection mode."},
    {ContinuousTaskSuspendReason::SYSTEM_SUSPEND_USED_ILLEGALLY,
        "Detected that the application is using an unapproved mode"},
    {ContinuousTaskSuspendReason::SYSTEM_SUSPEND_SYSTEM_LOAD_WARNING,
        "System load warning."},
    {ContinuousTaskSuspendReason::SYSTEM_SUSPEND_VOIP_NOT_USED,
        "No audio stream or recording stream detected when applying for voip mode"},
    {ContinuousTaskSuspendReason::SYSTEM_SUSPEND_BLUETOOTH_DATA_NOT_EXIST,
        "Not bluetooth data for a period of time when request bluetooth interaction mode."},
    {ContinuousTaskSuspendReason::SYSTEM_SUSPEND_POSITION_NOT_MOVED,
        "The location has not moved for a period of time when request location mode."},
    {ContinuousTaskSuspendReason::SYSTEM_SUSPEND_AUDIO_PLAYBACK_MUTE,
        "The system muted for a period of time when request audio playback mode."},
    {ContinuousTaskSuspendReason::SYSTEM_SUSPEND_NEARLINK_NOT_USED,
        "No nearlink connection for a period of time when request nearlink mode."},
    {ContinuousTaskSuspendReason::SYSTEM_SUSPEND_NEARLINK_DATA_NOT_EXIST,
        "No nearlink data for a period of time when request nearlink mode."},
    {ContinuousTaskSuspendReason::SYSTEM_SUSPEND_USER_UNAUTHORIZED,
        "User not authorized when request MODE_SPECIAL_SCENARIO_PROCESSING."}
};

uint32_t ContinuousTaskSuspendReason::GetSuspendReasonValue(const uint32_t mode, bool isStandby)
{
    auto reasonMap = isStandby ? STANDBY_SUSPEND_REASON : PARAM_SUSPEND_REASON;
    auto iter = reasonMap.find(mode);
    if (iter != reasonMap.end()) {
        return iter->second;
    }
    return 0;
}

std::string ContinuousTaskSuspendReason::GetSuspendReasonMessage(const uint32_t suspendReason)
{
    auto iter = SUSPEND_REASON_TO_MESSAGE.find(suspendReason);
    if (iter != SUSPEND_REASON_TO_MESSAGE.end()) {
        return iter->second;
    }
    return "unknown suspendReason";
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS