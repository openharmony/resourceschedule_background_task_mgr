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

#include "bg_task_config_file_info.h"

#include <algorithm>

#include "bgtaskmgr_log_wrapper.h"

namespace OHOS {
namespace BackgroundTaskMgr {
bool BgTaskConfigFileInfo::AddCpuLevelConfigInfo(const CpuLevelConfigInfo &cpuLevelConfigInfo)
{
    if (cpuLevelConfigInfo.bundleName.empty()) {
        return false;
    }
    allowApplyCpuBundleInfoMap_[cpuLevelConfigInfo.bundleName] = cpuLevelConfigInfo;
    return true;
}

bool BgTaskConfigFileInfo::CheckCpuLevel(const std::string &bundleName, int32_t cpuLevel)
{
    auto iter = allowApplyCpuBundleInfoMap_.find(bundleName);
    if (iter == allowApplyCpuBundleInfoMap_.end()) {
        BGTASK_LOGE("%{public}s bundle name %{public}s not configured", __func__, bundleName.c_str());
        return false;
    }

    if (cpuLevel <= static_cast<int32_t>(iter->second.cpuLevel)) {
        return true;
    }

    BGTASK_LOGE("%{public}s bundle name %{public}s cpuLevel invalid, cpuLevel: req %{public}d, config %{public}d",
        __func__, bundleName.c_str(), cpuLevel, iter->second.cpuLevel);
    return false;
}

bool BgTaskConfigFileInfo::CheckBundleName(const std::string &bundleName)
{
    return allowApplyCpuBundleInfoMap_.count(bundleName);
}

bool BgTaskConfigFileInfo::CheckAppSignatures(const std::string &bundleName, const std::string &appId,
    const std::string &appIdentifier)
{
    auto iter = allowApplyCpuBundleInfoMap_.find(bundleName);
    if (iter == allowApplyCpuBundleInfoMap_.end()) {
        BGTASK_LOGE("%{public}s bundle name %{public}s not configured", __func__, bundleName.c_str());
        return false;
    }

    const std::vector<std::string> &appSignatures = iter->second.appSignatures;
    if (std::find(appSignatures.begin(), appSignatures.end(), appId) == appSignatures.end() &&
        std::find(appSignatures.begin(), appSignatures.end(), appIdentifier) == appSignatures.end()) {
        BGTASK_LOGE("%{public}s bundle name %{public}s signatures info invalid", __func__, bundleName.c_str());
        return false;
    }
    return true;
}

std::unordered_map<std::string, CpuLevelConfigInfo> BgTaskConfigFileInfo::GetAllowApplyCpuBundleInfoMap()
{
    return allowApplyCpuBundleInfoMap_;
}
} // namespace BackgroundTaskMgr
} // namespace OHOS