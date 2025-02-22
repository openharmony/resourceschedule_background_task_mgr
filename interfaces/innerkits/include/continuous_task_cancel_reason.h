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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_CANCEL_REASON_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_CANCEL_REASON_H

namespace OHOS {
namespace BackgroundTaskMgr {
class ContinuousTaskCancelReason {
public:
    virtual ~ContinuousTaskCancelReason() = default;
    enum Type : uint32_t {
        USER_CANCEL = 1,
        SYSTEM_CANCEL,
        USER_CANCEL_REMOVE_NOTIFICATION,
        SYSTEM_CANCEL_DATA_TRANSFER_LOW_SPEED,
        SYSTEM_CANCEL_AUDIO_PLAYBACK_NOT_USE_AVSESSION,
        SYSTEM_CANCEL_AUDIO_PLAYBACK_NOT_RUNNING,
        SYSTEM_CANCEL_AUDIO_RECORDING_NOT_RUNNING,
        SYSTEM_CANCEL_NOT_USE_LOCATION,
        SYSTEM_CANCEL_NOT_USE_BLUETOOTH,
        SYSTEM_CANCEL_NOT_USE_MULTI_DEVICE,
        SYSTEM_CANCEL_USE_ILLEGALLY,
    };
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif