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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_COMMON_BG_TASK_CONFIG_FILE_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_COMMON_BG_TASK_CONFIG_FILE_INFO_H

#include <string>
#include <unordered_map>
#include <vector>

#include "efficiency_resources_cpu_level.h"

namespace OHOS {
namespace BackgroundTaskMgr {
struct CpuLevelConfigInfo {
    std::string bundleName {""};
    std::vector<std::string> appSignatures {};
    int32_t cpuLevel = static_cast<int32_t>(EfficiencyResourcesCpuLevel::DEFAULT);

    CpuLevelConfigInfo() = default;

    CpuLevelConfigInfo(std::string bundleName, const std::vector<std::string> &appSignatures, int32_t cpuLevel)
        : bundleName(std::move(bundleName)), appSignatures(appSignatures), cpuLevel(cpuLevel) {}
};

class BgTaskConfigFileInfo {
public:
    BgTaskConfigFileInfo() = default;
    ~BgTaskConfigFileInfo() = default;

    bool AddCpuLevelConfigInfo(const CpuLevelConfigInfo &cpuLevelConfigInfo);
    bool CheckCpuLevel(const std::string &bundleName, int32_t cpuLevel);
    bool CheckBundleName(const std::string &bundleName);
    bool CheckAppSignatures(const std::string &bundleName, const std::string &appId, const std::string &appIdentifier);
    std::unordered_map<std::string, CpuLevelConfigInfo>& GetAllowApplyCpuBundleInfoMap();

private:
    std::unordered_map<std::string, CpuLevelConfigInfo> allowApplyCpuBundleInfoMap_ {};
};
} // namespace BackgroundTaskMgr
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_COMMON_BG_TASK_CONFIG_FILE_INFO_H