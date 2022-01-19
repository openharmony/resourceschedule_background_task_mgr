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

#include "device_info_manager.h"

#include "transient_task_log.h"

using namespace std;

namespace OHOS {
namespace BackgroundTaskMgr {
void DeviceInfoManager::OnInputEvent(const EventInfo& eventInfo)
{
    if (isDump_) {
        std::vector<std::string> strArg = eventInfo.GetStringArgs();
        if (strArg[0] != "dump") {
            return;
        }
    }
    
    switch (eventInfo.GetEventId()) {
        case EVENT_SCREEN_ON:
            isScreenOn_ = true;
            break;
        case EVENT_SCREEN_OFF:
            isScreenOn_ = false;
            isScreenUnlock_ = false;
            break;
        case EVENT_SCREEN_UNLOCK:
            isScreenUnlock_ = true;
            break;
        case EVENT_BATTERY_LOW:
            isLowPowerMode_ = true;
            break;
        case EVENT_BATTERY_OKAY:
            isLowPowerMode_ = false;
            break;
        default:
            break;
    }
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS