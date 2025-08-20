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

namespace OHOS {
namespace BackgroundTaskMgr {
const std::unordered_map<uint32_t, std::string> PARAM_CONTINUOUS_TASK_SUBMODE_STR_MAP = {
    {ContinuousTaskSubmode::SUBMODE_CAR_KEY_NORMAL_NOTIFICATION, "submodeCarKeyNormalNotification"},
    {ContinuousTaskSubmode::SUBMODE_NORMAL_NOTIFICATION, "submodeNormalNotification"},
    {ContinuousTaskSubmode::SUBMODE_LIVE_VIEW_NOTIFICATION, "submodeLiveViewNotification"},
    {ContinuousTaskSubmode::SUBMODE_AUDIO_PLAYBACK_NORMAL_NOTIFICATION, "submodeAudioPlaybackNormalNotification"},
    {ContinuousTaskSubmode::SUBMODE_AVSESSION_AUDIO_PLAYBACK, "submodeAvsessionAudioPlayback"},
    {ContinuousTaskSubmode::SUBMODE_AUDIO_RECORD_NORMAL_NOTIFICATION, "submodeAudioRecordNormalNotification"},
    {ContinuousTaskSubmode::SUBMODE_SCREEN_RECORD_NORMAL_NOTIFICATION, "submodeScreenRecordNormalNotification"},
    {ContinuousTaskSubmode::SUBMODE_VOICE_CHAT_NORMAL_NOTIFICATION, "submodeVoiceChatNormalNotification"},
    {ContinuousTaskSubmode::END, "end"}
};

std::string ContinuousTaskSubmode::GetContinuousTaskSubmodeStr(uint32_t mode)
{
    auto iter = PARAM_CONTINUOUS_TASK_SUBMODE_STR_MAP.find(mode);
    if (iter != PARAM_CONTINUOUS_TASK_SUBMODE_STR_MAP.end()) {
        return iter->second.c_str();
    }
    return "default";
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS