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

#include "report_hisysevent_data.h"
#include <sys/time.h>

namespace OHOS {
namespace BackgroundTaskMgr {
constexpr int64_t SEC_TO_MILLISEC = 1000;

bool EfficiencyResourceApplyReportHisysEvent::AddData(const sptr<EfficiencyResourceInfo> &resourceInfo,
    const std::shared_ptr<ResourceCallbackInfo> &callbackInfo)
{
    if (length_ == MAX_EFFICIENTCY_RESOURCE_HISYSEVCENT_DATA_LENGTH) {
        return false;
    }
    appUid_.push_back(callbackInfo->GetUid());
    appPid_.push_back(callbackInfo->GetPid());
    appName_.push_back(callbackInfo->GetBundleName() + "|" + std::to_string(GetCurrentTimestamp()));
    uiabilityIdentity_.push_back(-1);
    resourceType_.push_back(callbackInfo->GetResourceNumber());
    timeout_.push_back(resourceInfo->GetTimeOut());
    persist_.push_back(resourceInfo->IsPersist());
    process_.push_back(resourceInfo->IsProcess());
    length_++;
    return true;
}

void EfficiencyResourceApplyReportHisysEvent::ClearData()
{
    appUid_.clear();
    appPid_.clear();
    appName_.clear();
    uiabilityIdentity_.clear();
    resourceType_.clear();
    timeout_.clear();
    persist_.clear();
    process_.clear();
    length_ = 0;
}

bool EfficiencyResourceResetReportHisysEvent::AddData(const std::shared_ptr<ResourceCallbackInfo> &callbackInfo,
    EfficiencyResourcesEventType type)
{
    if (length_ == MAX_EFFICIENTCY_RESOURCE_HISYSEVCENT_DATA_LENGTH) {
        return false;
    }
    appUid_.push_back(callbackInfo->GetUid());
    appPid_.push_back(callbackInfo->GetPid());
    appName_.push_back(callbackInfo->GetBundleName());
    uiabilityIdentity_.push_back(-1);
    resourceType_.push_back(callbackInfo->GetResourceNumber());
    if (type == EfficiencyResourcesEventType::APP_RESOURCE_RESET) {
        process_.push_back(false);
    } else {
        process_.push_back(true);
    }
    quota_.push_back(0);
    allQuota_.push_back(0);
    length_++;
    return true;
}

void EfficiencyResourceResetReportHisysEvent::ClearData()
{
    appUid_.clear();
    appPid_.clear();
    appName_.clear();
    uiabilityIdentity_.clear();
    resourceType_.clear();
    process_.clear();
    quota_.clear();
    allQuota_.clear();
    length_ = 0;
}

int64_t EfficiencyResourceApplyReportHisysEvent::GetCurrentTimestamp()
{
    // get current time,precision is ms + us
    struct timeval currentTime;
    gettimeofdy(&currentTime, nullptr);
    return static_cast<int64_t>(currentTime.tv_sec) * SEC_TO_MILLISEC + currentTime.tv_usec /SEC_TO_MILLISEC;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS