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

#include "continuous_task_mode.h"

namespace OHOS {
namespace BackgroundTaskMgr {
const std::unordered_map<uint32_t, std::string> PARAM_CONTINUOUS_TASK_MODE_STR_MAP = {
    {ContinuousTaskMode::MODE_DATA_TRANSFER, "modeDataTransfer"},
    {ContinuousTaskMode::MODE_SHARE_POSITION, "modeSharePosition"},
    {ContinuousTaskMode::MODE_ALLOW_BLUETOOTH_AWARE, "modeAllowBluetoothAware"},
    {ContinuousTaskMode::MODE_MULTI_DEVICE_CONNECTION, "modeMultiDeviceConnection"},
    {ContinuousTaskMode::MODE_ALLOW_WIFI_AWARE, "modeAllowWifiAware"},
    {ContinuousTaskMode::MODE_TASK_KEEPING, "modeTaskKeeping"},
    {ContinuousTaskMode::MODE_AV_PLAYBACK_AND_RECORD, "modeAvPlaybackAndRecord"},
    {ContinuousTaskMode::END, "end"}
}

std::string ContinuousTaskMode::GetContinuousTaskModeStr(uint32_t mode)
{
    auto iter = PARAM_CONTINUOUS_TASK_MODE_STR_MAP.find(mode);
    if (iter != PARAM_CONTINUOUS_TASK_MODE_STR_MAP.end()) {
        return iter->second.c_str();
    }
    return "default";
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS