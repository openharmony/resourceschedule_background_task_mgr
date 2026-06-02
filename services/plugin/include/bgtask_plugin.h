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
 
#ifndef BACKGROUND_TASK_MGR_SERVICES_PLUGIN_INCLUDE_BGTASK_PLUGIN_H
#define BACKGROUND_TASK_MGR_SERVICES_PLUGIN_INCLUDE_BGTASK_PLUGIN_H
 
#include <unordered_map>
#include <functional>
#include "nlohmann/json.hpp"
#include "bgtaskmgr_log_wrapper.h"
 
namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
constexpr int32_t RES_VALUE_FOR_ALL = -1;
}
 
class BgtaskPlugin {
public:
    virtual void Init() = 0;
    virtual void Uninit() = 0;
    virtual std::string GetPluginName() const;
    virtual std::vector<int32_t> GetPluginValue(const uint32_t resType) const;
    void DispatchResource(const uint32_t resType, const int32_t stateType, const nlohmann::json& payload);
protected:
    using CallBackMap = std::unordered_map<uint32_t, std::unordered_map<int32_t, std::function<
        void(const int32_t, const nlohmann::json&)>>>;
    CallBackMap cbMap_{};
    virtual void InitCbMap(CallBackMap& cbMap) = 0;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // BACKGROUND_TASK_MGR_SERVICES_PLUGIN_INCLUDE_BGTASK_PLUGIN_H