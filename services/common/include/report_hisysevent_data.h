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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_COMMON_REPORT_HISYSTEM_DATA_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_COMMON_REPORT_HISYSTEM_DATA_H

#include <string>

#include "efficiency_resource_info.h"
#include "resource_callback_info.h"
#include "resources_subscriber_mgr.h"

namespace OHOS {
namespace BackgroundTaskMgr {
const int32_t MAX_EFFICIENTCY_RESOURCE_HISYSEVCENT_DATA_LENGTH = 20;

struct EfficiencyResourceApplyReportHisysEvent {
    int32_t length_ {0};
    std::vector<int32_t> appUid_ {};
    std::vector<int32_t> appPid_ {};
    std::vector<std::string> appName_ {};
    std::vector<int32_t> uiabilityIdentity_ {};
    std::vector<int32_t> resourceType_ {};
    std::vector<int64_t> timeout_ {};
    std::vector<bool> persist_ {};
    std::vector<bool> process_ {};
    EfficiencyResourceApplyReportHisysEvent() = default;

    bool AddData(const sptr<EfficiencyResourceInfo> &resourceInfo,
        const std::shared_ptr<ResourceCallbackInfo> &callbackInfo);

    int64_t GetCurrentTimestamp();

    void ClearData();
};

struct EfficiencyResourceResetReportHisysEvent {
    int32_t length_ {0};
    std::vector<int32_t> appUid_ {};
    std::vector<int32_t> appPid_ {};
    std::vector<std::string> appName_ {};
    std::vector<int32_t> uiabilityIdentity_ {};
    std::vector<int32_t> resourceType_ {};
    std::vector<bool> process_ {};
    std::vector<int64_t> quota_ {};
    std::vector<int64_t> allQuota_ {};
    EfficiencyResourceResetReportHisysEvent() = default;

    bool AddData(const std::shared_ptr<ResourceCallbackInfo> &callbackInfo,
        EfficiencyResourcesEventType type);

    void ClearData();
};

}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_PARAMS_H