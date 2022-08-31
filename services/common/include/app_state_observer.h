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

#include "singleton.h"
#include "app_mgr_interface.h"
#include "application_state_observer_stub.h"
#include "event_handler.h"
#include "iremote_object.h"

#include "remote_death_recipient.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class BgEfficiencyResourcesMgr;
class BgContinuousTaskMgr;

class AppStateObserver : public AppExecFwk::ApplicationStateObserverStub {
    DECLARE_DELAYED_SINGLETON(AppStateObserver);
public:
    void OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData) override;
    void OnProcessDied(const AppExecFwk::ProcessData &processData) override;
    void OnApplicationStateChanged(const AppExecFwk::AppStateData &appStateData) override;
    void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler);
    void SetBgContinuousTaskMgr(const std::shared_ptr<BgContinuousTaskMgr> &bgContinuousTaskMgr);
    void SetBgEfficiencyResourcesMgr(const std::shared_ptr<BgEfficiencyResourcesMgr> &resourceMgr);
    bool Subscribe();
    bool Unsubscribe();

private:
    bool Connect();
    void Disconnect();
    void OnRemoteDied(const wptr<IRemoteObject> &object);
    inline bool ValidateAppStateData(const AppExecFwk::AppStateData &appStateData);
    void OnProcessDiedContinuousTask(const AppExecFwk::ProcessData &processData);
    void OnProcessDiedEfficiencyRes(const AppExecFwk::ProcessData &processData);

private:
    std::mutex mutex_ {};
    std::weak_ptr<AppExecFwk::EventHandler> handler_ {};
    std::weak_ptr<BgContinuousTaskMgr> bgContinuousTaskMgr_ {};
    sptr<AppExecFwk::IAppMgr> appMgrProxy_ {nullptr};
    sptr<RemoteDeathRecipient> appMgrDeathRecipient_ {nullptr};
    std::weak_ptr<BgEfficiencyResourcesMgr> bgEfficiencyResourcesMgr_ {};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_APP_STATE_OBSERVER_H