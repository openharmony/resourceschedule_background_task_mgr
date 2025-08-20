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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_MODE_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_MODE_H

#include <string>

namespace OHOS {
namespace BackgroundTaskMgr {
class ContinuousTaskMode {
public:
    virtual ~ContinuousTaskMode() = default;
    enum Type : uint32_t {
        MODE_DATA_TRANSFER = 1,
        MODE_SHARE_POSITION = 4,
        MODE_ALLOW_BLUETOOTH_AWARE,
        MODE_MULTI_DEVICE_CONNECTION,
        MODE_ALLOW_WIFI_AWARE,
        MODE_TASK_KEEPING = 9,
        MODE_AV_PLAYBACK_AND_RECORD,
        END,
    };

    static std::string GetContinuousTaskModeStr(uint32_t mode);
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_MODE_H