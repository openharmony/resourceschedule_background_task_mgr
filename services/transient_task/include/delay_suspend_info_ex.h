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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_SERVICES_TRANSIENT_TASK_INCLUDE_DELAY_SUSPEND_INFO_EX_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_SERVICES_TRANSIENT_TASK_INCLUDE_DELAY_SUSPEND_INFO_EX_H

#include <string>

#include "delay_suspend_info.h"
#include "time_provider.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class DelaySuspendInfoEx : public DelaySuspendInfo {
public:
    DelaySuspendInfoEx(const int32_t& pid, const int32_t& requestId = -1, const int32_t& delaytime = 0);
    ~DelaySuspendInfoEx() override = default;
    int32_t GetRemainDelayTime();
    int32_t GetAdvanceCallbackTime();
    void StartAccounting();
    void StopAccounting();
    void Dump(std::string& result);
    inline int32_t GetPid() const
    {
        return pid_;
    }

    inline void SetPid(int32_t pid)
    {
        pid_ = pid;
    }

    inline int64_t GetBaseTime() const
    {
        return baseTime_;
    }

private:
    const int32_t ADVANCE_TIME = 6 * MSEC_PER_SEC; //6s
    int32_t pid_ {-1};
    int64_t baseTime_ {0};
    int64_t spendTime_ {0};
};
} // namespace BackgroundTaskMgr
} // namespace OHOS
#endif