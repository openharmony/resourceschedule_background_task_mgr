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
 
#ifndef BACKGROUND_TASK_MGR_SERVICES_PLUGIN_INCLUDE_APP_STATE_OBSERVER_PLUGIN_ADAPTER_H
#define BACKGROUND_TASK_MGR_SERVICES_PLUGIN_INCLUDE_APP_STATE_OBSERVER_PLUGIN_ADAPTER_H
 
#include <functional>
#include "singleton.h"
#include "nlohmann/json.hpp"
#include "bgtask_plugin.h"
#include "app_state_observer.h"
#include "decision_maker.h"
 
namespace OHOS {
namespace BackgroundTaskMgr {
class AppStateObserverPluginAdapter final : public BgtaskPlugin,
                                            public DelayedSingleton<AppStateObserverPluginAdapter> {
public:
    static bool SelfRegister();
    void Init() override;
    void Uninit() override;
    std::string GetPluginName() const override;
 
private:
    bool UnmarshallingProcessData(const nlohmann::json& payload, AppExecFwk::ProcessData& processData);
    void OnProcessDied(const nlohmann::json &payload);
    void OnProcessStateChanged(const nlohmann::json& payload);
 
    bool UnmarshallingAppStateData(const nlohmann::json& payload, AppExecFwk::AppStateData& appStateData);
    void OnAppStateChanged(const nlohmann::json& payload);
    void OnAppStopped(const nlohmann::json &payload);
    void OnAppCacheStateChanged(const nlohmann::json &payload);
 
    bool UnmarshallingAbilityStateData(const nlohmann::json &payload, AppExecFwk::AbilityStateData &data);
    void OnAbilityStateChanged(const nlohmann::json &payload);
 
    void InitCbMap(CallBackMap &cbMap) override;
    void InitAbilityCbMap(CallBackMap &cbMap);
 
    std::shared_ptr<BackgroundTaskMgr::AppStateObserver> appStateObserver_ = nullptr;
    std::shared_ptr<BackgroundTaskMgr::DecisionMaker> decisionMaker_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> handler_ {nullptr};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // BACKGROUND_TASK_MGR_SERVICES_PLUGIN_INCLUDE_APP_STATE_OBSERVER_PLUGIN_ADAPTER_H