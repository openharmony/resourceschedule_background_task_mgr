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
    {BackgroundMode::VOIP, ContinuousTaskSuspendReason::SYSTEM_SUSPEND_AUDIO_PLAYBACK_NOT_RUNNING},
    {ContinuousTaskSuspendReason::ALL_MODE, ContinuousTaskSuspendReason::SYSTEM_SUSPEND_USED_ILLEGALLY}
};

uint32_t ContinuousTaskSuspendReason::GetSuspendReasonValue(const uint32_t mode)
{
    auto iter = PARAM_SUSPEND_REASON.find(mode);
    if (iter != PARAM_SUSPEND_REASON.end()) {
        return iter->second;
    }
    return 0;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS