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

#include "pkg_delay_suspend_info.h"
#include "bgtask_config.h"
#include "transient_task_log.h"
#include "time_provider.h"
#include "bgtaskmgr_inner_errors.h"
#include "errors.h"

#include <sstream>

using namespace std;

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
    constexpr int32_t MAX_REQUEST_ID = 3;
    constexpr int32_t MIN_ALLOW_QUOTA_TIME = 16 * MSEC_PER_SEC; // 16s
    constexpr int32_t WATCHDOG_DELAY_TIME = 6 * MSEC_PER_SEC;
}

ErrCode PkgDelaySuspendInfo::IsAllowRequest()
{
    if (requestList_.size() >= MAX_REQUEST_ID) {
        return ERR_BGTASK_EXCEEDS_THRESHOLD;
    }

    UpdateQuota();
    if (quota_ >= MIN_ALLOW_QUOTA_TIME) {
        return ERR_OK;
    }

    return ERR_BGTASK_TIME_INSUFFICIENT;
}

void PkgDelaySuspendInfo::AddRequest(const shared_ptr<DelaySuspendInfoEx>& delayInfo,
    const int32_t delayTime, const bool needSetTime)
{
    if (needSetTime) {
        int32_t exempted_quota = DelayedSingleton<BgtaskConfig>::GetInstance()->GetTransientTaskExemptedQuato();
        BGTASK_LOGD("pkgname: %{public}s, requestId: %{public}d exempted_quota %{public}d", pkg_.c_str(),
            delayInfo->GetRequestId(), exempted_quota);
        delayInfo->SetActualDelayTime(exempted_quota + WATCHDOG_DELAY_TIME);
    } else {
        delayInfo->SetActualDelayTime((quota_ < delayTime) ? quota_ : delayTime);
    }
    requestList_.push_back(delayInfo);
}

void PkgDelaySuspendInfo::RemoveRequest(const int32_t requestId)
{
    for (auto iter = requestList_.begin(); iter != requestList_.end(); iter++) {
        if (!(*iter)->IsSameRequestId(requestId)) {
            continue;
        }
        StopAccounting(requestId);
        requestList_.erase(iter);
        if (requestList_.empty()) {
            UpdateQuota();
            isCounting_ = false;
        }
        break;
    }
}

int32_t PkgDelaySuspendInfo::GetRemainDelayTime(const int32_t requestId)
{
    for (auto &info : requestList_) {
        if (info->IsSameRequestId(requestId)) {
            return info->GetRemainDelayTime();
        }
    }
    return 0;
}

void PkgDelaySuspendInfo::StartAccounting(const int32_t requestId)
{
    for (auto &info : requestList_) {
        if ((requestId != -1) && !info->IsSameRequestId(requestId)) {
            continue;
        }
        if (!isCounting_) {
            UpdateQuota();
            isCounting_ = true;
        }
        if (info->GetBaseTime() == 0) {
            info->StartAccounting();
            timerManager_->AddTimer(info->GetRequestId(), info->GetAdvanceCallbackTime());
            BGTASK_LOGD("StartAccounting pkgname: %{public}s, requestId: %{public}d, pid: %{public}d",
                pkg_.c_str(), info->GetRequestId(), info->GetPid());
        }
    }
}

void PkgDelaySuspendInfo::StopAccounting(const int32_t requestId)
{
    for (auto &info : requestList_) {
        if (!info->IsSameRequestId(requestId) || (info->GetBaseTime() == 0)) {
            continue;
        }
        info->StopAccounting();
        timerManager_->RemoveTimer(info->GetRequestId());
        BGTASK_LOGD("StopAccounting pkgname: %{public}s, requestId: %{public}d, pid: %{public}d",
            pkg_.c_str(), info->GetRequestId(), info->GetPid());
    }
}

void PkgDelaySuspendInfo::StopAccountingAll()
{
    BGTASK_LOGD("StopAccountingAll %{public}s", pkg_.c_str());
    for (auto &info : requestList_) {
        if (info->GetBaseTime() == 0) {
            continue;
        }
        info->StopAccounting();
        timerManager_->RemoveTimer(info->GetRequestId());
        BGTASK_LOGD("StopAccountingAll pkgname: %{public}s, requestId: %{public}d, pid: %{public}d",
            pkg_.c_str(), info->GetRequestId(), info->GetPid());
    }
    UpdateQuota();
    isCounting_ = false;
}

void PkgDelaySuspendInfo::UpdateQuota(bool reset)
{
    spendTime_ = isCounting_ ? GetModifiedTime() : 0;
    quota_ -= spendTime_;
    if (quota_ < 0) {
        quota_ = 0;
    }
    baseTime_ = static_cast<int32_t>(TimeProvider::GetCurrentTime());
    if (reset) {
        quota_ = INIT_QUOTA;
    }
    BGTASK_LOGD("%{public}s Lastest quota: %{public}d, spendTime: %{public}d, isCounting: %{public}d",
        pkg_.c_str(), quota_, spendTime_, isCounting_);
}

int32_t PkgDelaySuspendInfo::GetModifiedTime()
{
    bool isExemptedApp = DelayedSingleton<BgtaskConfig>::GetInstance()->
        IsTransientTaskExemptedQuatoApp(pkg_);
    int32_t exempted_quota = 0;
    if (isExemptedApp) {
        exempted_quota = DelayedSingleton<BgtaskConfig>::GetInstance()->GetTransientTaskExemptedQuato();
    }
    BGTASK_LOGD("bundleName: %{public}s exempted: %{public}d exempted_quota: %{public}d",
        pkg_.c_str(), isExemptedApp, exempted_quota);
    int32_t time = static_cast<int32_t>(TimeProvider::GetCurrentTime()) - baseTime_ - exempted_quota;
    return (time < 0) ? 0 : time;
}

}  // namespace BackgroundTaskMgr
}  // namespace OHOS
