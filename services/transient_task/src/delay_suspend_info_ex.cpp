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

#include "delay_suspend_info_ex.h"

#include "transient_task_log.h"
#include "time_provider.h"

namespace OHOS {
namespace BackgroundTaskMgr {
DelaySuspendInfoEx::DelaySuspendInfoEx(const int32_t& pid, const int32_t& requestId, const int32_t& delaytime)
{
    SetPid(pid);
    SetRequestId(requestId);
    SetActualDelayTime(delaytime);
}

int32_t DelaySuspendInfoEx::GetRemainDelayTime()
{
    int64_t spendTime = (baseTime_ > 0) ? (spendTime_ + (TimeProvider::GetCurrentTime() - baseTime_)) : spendTime_;
    int32_t remainTime = GetActualDelayTime() - spendTime;
    return (remainTime < 0) ? 0 : remainTime;
}

int32_t DelaySuspendInfoEx::GetAdvanceCallbackTime()
{
    int32_t advanceCallbackTime = GetRemainDelayTime() - ADVANCE_TIME;
    return (advanceCallbackTime > 0) ? advanceCallbackTime : 0;
}

void DelaySuspendInfoEx::StartAccounting()
{
    if (baseTime_ == 0) {
        baseTime_ = TimeProvider::GetCurrentTime();
    }
}

void DelaySuspendInfoEx::StopAccounting()
{
    if (baseTime_ != 0) {
        spendTime_ += TimeProvider::GetCurrentTime() - baseTime_;
        baseTime_ = 0;
    }
}

void DelaySuspendInfoEx::Dump(std::string& result)
{
    result.append("{ requestId: " + std::to_string(GetRequestId()));
    result.append(", reaminTime: " + std::to_string(GetRemainDelayTime()));
    result.append(", advanceCallbackTime: " + std::to_string(GetAdvanceCallbackTime()) + "}");
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS