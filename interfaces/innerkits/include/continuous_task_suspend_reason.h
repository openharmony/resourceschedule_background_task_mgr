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
         SYSTEM_SUSPEND_AUDIO_PLAYBACK_NOT_USE_AVSESSION,
         SYSTEM_SUSPEND_AUDIO_PLAYBACK_NOT_RUNNING,
         SYSTEM_SUSPEND_AUDIO_RECORDING_NOT_RUNNING,
         SYSTEM_SUSPEND_LOCATION_NOT_USED,
         SYSTEM_SUSPEND_BLUETOOTH_NOT_USED,
         SYSTEM_SUSPEND_MULTI_DEVICE_NOT_USED,
         SYSTEM_SUSPEND_USE_ILLEGALLY,
         SYSTEM_SUSPEND_SYSTEM_LOAD_WARNING,
     };
 };
 }  // namespace BackgroundTaskMgr
 }  // namespace OHOS
 #endif