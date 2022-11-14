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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_DEVICE_INFO_MANAGER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_DEVICE_INFO_MANAGER_H

#include "input_manager.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class DeviceInfoManager : public IEventListener {
public:
    DeviceInfoManager() = default;
    ~DeviceInfoManager() override = default;
    void OnInputEvent(const EventInfo& eventInfo) override;

    inline bool IsScreenOn() const
    {
        return isScreenOn_;
    }

    inline bool IsScreenUnlock() const
    {
        return isScreenUnlock_;
    }

    inline bool IsLowPowerMode() const
    {
        return isLowPowerMode_;
    }

    inline void SetDump(bool isDump)
    {
        isDump_ = isDump;
    }

private:
    bool isDump_ {false};
    bool isScreenOn_ {true};
    bool isScreenUnlock_ {false};
    bool isLowPowerMode_ {false};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_DEVICE_INFO_MANAGER_H