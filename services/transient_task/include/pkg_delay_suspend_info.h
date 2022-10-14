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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_PKG_DELAY_SUSPEND_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_PKG_DELAY_SUSPEND_INFO_H

#include <memory>
#include <string>

#include "bgtask_common.h"
#include "delay_suspend_info_ex.h"
#include "timer_manager.h"

namespace OHOS {
namespace BackgroundTaskMgr {
using std::shared_ptr;
using std::vector;
using std::string;

class PkgDelaySuspendInfo {
public:
    PkgDelaySuspendInfo(const string& pkg, const int32_t& uid, const shared_ptr<TimerManager>& timerManager)
        : pkg_(pkg), uid_(uid), timerManager_(timerManager) {}
    ~PkgDelaySuspendInfo() = default;
    ErrCode IsAllowRequest();
    void AddRequest(const shared_ptr<DelaySuspendInfoEx>& delayInfo, const int32_t delayTime);
    void RemoveRequest(const int32_t requestId);
    int32_t GetRemainDelayTime(const int32_t requestId);
    void StartAccounting(const int32_t requestId = -1);
    void StopAccounting(const int32_t requestId);
    void StopAccountingAll();
    void UpdateQuota(bool result = false);

    inline const string& GetPkg() const
    {
        return pkg_;
    }

    inline int32_t GetUid() const
    {
        return uid_;
    }

    inline bool IsRequestEmpty() const
    {
        return requestList_.empty();
    }

    inline size_t GetRequestSize() const
    {
        return requestList_.size();
    }

    inline int32_t GetQuota() const
    {
        return quota_;
    }

private:
    string pkg_ {""};
    int32_t uid_ {-1};
    int32_t quota_ {INIT_QUOTA};
    int32_t spendTime_ {0};
    int32_t baseTime_ {0};
    bool isCounting_ {false};
    shared_ptr<TimerManager> timerManager_ {nullptr};
    vector<shared_ptr<DelaySuspendInfoEx>> requestList_;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_PKG_DELAY_SUSPEND_INFO_H