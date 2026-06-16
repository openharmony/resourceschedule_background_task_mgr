/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_APP_STATE_OBSERVER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_APP_STATE_OBSERVER_H

#include "event_handler.h"
#include "app_state_data.h"
#include "ability_state_data.h"
#include "process_data.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class AppStateObserver {
public:
    void OnAppCacheStateChanged(const AppExecFwk::AppStateData &appStateData);
    void OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData);
#ifdef GAME_PRE_LAUNCH_ENABLE
    void OnProcessCreated(const AppExecFwk::ProcessData &processData);
#endif
    void OnProcessDied(const AppExecFwk::ProcessData &processData);
    void OnAppStopped(const AppExecFwk::AppStateData &appStateData);
    void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler);
    void OnAppStateChanged(const AppExecFwk::AppStateData &appStateData);

private:
    bool ValidateAppStateData(const AppExecFwk::AppStateData &appStateData);
    void OnProcessDiedEfficiencyRes(const AppExecFwk::ProcessData &processData);

private:
    std::shared_ptr<AppExecFwk::EventHandler> handler_ {};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_APP_STATE_OBSERVER_H