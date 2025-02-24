/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "application_state_observer_stub.h"
#include "event_handler.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class AppStateObserver : public AppExecFwk::ApplicationStateObserverStub {
public:
    void OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData) override;
    void OnProcessDied(const AppExecFwk::ProcessData &processData) override;
    void OnAppStopped(const AppExecFwk::AppStateData &appStateData) override;
    void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler);

private:
    bool ValidateAppStateData(const AppExecFwk::AppStateData &appStateData);
    void OnProcessDiedEfficiencyRes(const AppExecFwk::ProcessData &processData);

private:
    std::shared_ptr<AppExecFwk::EventHandler> handler_ {};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_APP_STATE_OBSERVER_H