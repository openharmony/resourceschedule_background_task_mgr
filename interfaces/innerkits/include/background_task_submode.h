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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_TASK_SUBMODE_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_TASK_SUBMODE_H

#include <string>

namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundTaskSubmode {
public:
    virtual ~BackgroundTaskSubmode() = default;
    enum Type : uint32_t {
        SUBMODE_CAR_KEY_NORMAL_NOTIFICATION = 1,
        SUBMODE_NORMAL_NOTIFICATION,
        SUBMODE_LIVE_VIEW_NOTIFICATION,
        SUBMODE_AUDIO_PLAYBACK_NORMAL_NOTIFICATION,
        SUBMODE_AVSESSION_AUDIO_PLAYBACK,
        SUBMODE_AUDIO_RECORD_NORMAL_NOTIFICATION,
        SUBMODE_SCREEN_RECORD_NORMAL_NOTIFICATION,
        SUBMODE_VOICE_CHAT_NORMAL_NOTIFICATION,
        END,
    };

    static std::string GetBackgroundTaskSubmodeStr(uint32_t mode);
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_TASK_SUBMODE_H