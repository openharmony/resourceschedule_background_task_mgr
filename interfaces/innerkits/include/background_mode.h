/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_MODE_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_MODE_H

namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundMode {
public:
    virtual ~BackgroundMode() = default;
    enum Type : uint32_t {
        DATA_TRANSFER = 1,
        AUDIO_PLAYBACK,
        AUDIO_RECORDING,
        LOCATION,
        BLUETOOTH_INTERACTION,
        MULTI_DEVICE_CONNECTION,
        WIFI_INTERACTION,
        VOIP,
        TASK_KEEPING,
    };
};
}
}
#endif