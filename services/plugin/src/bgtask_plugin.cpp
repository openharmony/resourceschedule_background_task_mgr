/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "bgtask_plugin.h"
#include "bgtaskmgr_inner_errors.h"

namespace OHOS {
namespace BackgroundTaskMgr {

void BgtaskPlugin::DispatchResource(const uint32_t resType, const int32_t stateType, const nlohmann::json& payload)
{
    auto itResType = cbMap_.find(resType);
    if (itResType == cbMap_.end()) {
        BGTASK_LOGE("can not found resType, type: %{public}u", resType);
        return;
    }

    std::unordered_map<int32_t, std::function<void(const int32_t, const nlohmann::json&)>> &stateTypeCbMap =
        itResType->second;
    if ((stateTypeCbMap.size() == 1) && (stateTypeCbMap.find(RES_VALUE_FOR_ALL) != stateTypeCbMap.end())) {
        stateTypeCbMap[RES_VALUE_FOR_ALL](stateType, payload);
        return;
    }

    auto itStateType = stateTypeCbMap.find(stateType);
    if (itStateType == stateTypeCbMap.end()) {
        BGTASK_LOGE("unknown resType:%{public}u stateType:%{public}d", resType, stateType);
        return;
    }
    itStateType->second(stateType, payload);
}

std::string BgtaskPlugin::GetPluginName() const
{
    return "BgtaskPlugin";
}

std::vector<int32_t> BgtaskPlugin::GetPluginValue(const uint32_t resType) const
{
    auto itResType = cbMap_.find(resType);
    if (itResType == cbMap_.end()) {
        BGTASK_LOGE("can not found resType, type: %{public}u, get value failed!", resType);
        return {};
    }

    std::vector<int32_t> pluginValues;
    for (const auto& item : itResType->second) {
        pluginValues.emplace_back(item.first);
    }
    return pluginValues;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS