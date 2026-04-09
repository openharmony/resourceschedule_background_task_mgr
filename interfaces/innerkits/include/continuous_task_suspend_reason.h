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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_SUSPEND_REASON_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_SUSPEND_REASON_H
 
namespace OHOS {
namespace BackgroundTaskMgr {
class ContinuousTaskSuspendReason {
public:
    virtual ~ContinuousTaskSuspendReason() = default;
    enum Type : uint32_t {
        SYSTEM_SUSPEND_DATA_TRANSFER_LOW_SPEED = 4,
        SYSTEM_SUSPEND_AUDIO_PLAYBACK_NOT_USE_AVSESSION = 5,
        SYSTEM_SUSPEND_AUDIO_PLAYBACK_NOT_RUNNING = 6,
        SYSTEM_SUSPEND_AUDIO_RECORDING_NOT_RUNNING = 7,
        SYSTEM_SUSPEND_LOCATION_NOT_USED = 8,
        SYSTEM_SUSPEND_BLUETOOTH_NOT_USED = 9,
        SYSTEM_SUSPEND_MULTI_DEVICE_NOT_USED = 10,
        SYSTEM_SUSPEND_USED_ILLEGALLY = 11,
        SYSTEM_SUSPEND_SYSTEM_LOAD_WARNING = 12,
        SYSTEM_SUSPEND_VOIP_NOT_USED = 13,
        SYSTEM_SUSPEND_BLUETOOTH_DATA_NOT_EXIST = 14,
        SYSTEM_SUSPEND_POSITION_NOT_MOVED = 15,
        SYSTEM_SUSPEND_AUDIO_PLAYBACK_MUTE = 16,
        SYSTEM_SUSPEND_NEARLINK_NOT_USED = 17,
        SYSTEM_SUSPEND_NEARLINK_DATA_NOT_EXIST = 18,
        SYSTEM_SUSPEND_USER_UNAUTHORIZED = 19
    };

    static uint32_t GetSuspendReasonValue(const uint32_t mode, , bool isStandby);
    static std::string GetSuspendReasonMessage(const uint32_t suspendReason);
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif