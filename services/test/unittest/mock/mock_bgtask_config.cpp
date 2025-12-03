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

#include "bgtask_config.h"

namespace OHOS {
namespace BackgroundTaskMgr {
bool BgtaskConfig::CheckRequestCpuLevelBundleNameConfigured(const std::string &bundleName)
{
    return bundleName == "bundleTest" || bundleName == "false-test" || bundleName == "bundleTestSign";
}

bool BgtaskConfig::CheckRequestCpuLevelAppSignatures(const std::string &bundleName, const std::string &appId,
    const std::string &appIdentifier)
{
    return bundleName == "bundleTestSign";
}

bool BgtaskConfig::CheckRequestCpuLevel(const std::string &bundleName, int32_t cpuLevel)
{
    return cpuLevel <= EfficiencyResourcesCpuLevel::MEDIUM_CPU;
}
}
}