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
 
#ifndef BACKGROUND_TASK_MGR_SERVICES_PLUGIN_INCLUDE_BGTASK_PLUGIN_MGR_H
#define BACKGROUND_TASK_MGR_SERVICES_PLUGIN_INCLUDE_BGTASK_PLUGIN_MGR_H
 
#include <unordered_map>
#include "res_data.h"
#include "bgtask_plugin.h"
#include "single_instance.h"
#include "plugin.h"
 
namespace OHOS {
namespace BackgroundTaskMgr {
class BgtaskPluginMgr : public ResourceSchedule::Plugin {
    DECLARE_SINGLE_INSTANCE(BgtaskPluginMgr)
public:
    void Init() override;
    void Disable() override;
    void DispatchResource(const std::shared_ptr<ResourceSchedule::ResData>& resData) override;
    void RegisterAsyncPlugin(const uint32_t resType, const std::shared_ptr<BgtaskPlugin> plugin);
    void RegisterAsyncPluginByValue(const uint32_t resType, const std::shared_ptr<BgtaskPlugin> plugin);
 
private:
    std::atomic<bool> pluginEnable {false};
    std::mutex cbMapMutex_;
    std::unordered_map<uint32_t, std::shared_ptr<BgtaskPlugin>> asyncCbMap_;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // BACKGROUND_TASK_MGR_SERVICES_PLUGIN_INCLUDE_BGTASK_PLUGIN_MGR_H