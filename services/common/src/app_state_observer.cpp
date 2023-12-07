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

#include "app_state_observer.h"

#include "app_mgr_constants.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "bg_continuous_task_mgr.h"
#include "continuous_task_log.h"
#include "bg_efficiency_resources_mgr.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
const std::string TASK_ON_PROCESS_DIED = "OnProcessDiedTask";
const std::string TASK_ON_ABILITY_STATE_CHANGED = "OnAbilityStateChangedTask";
}

#ifdef BGTASK_MGR_UNIT_TEST
#define WEAK_FUNC __attribute__((weak))
#else
#define WEAK_FUNC
#endif

AppStateObserver::AppStateObserver() {}

AppStateObserver::~AppStateObserver() {}

bool AppStateObserver::CheckParamValid()
{
    if (handler_.expired()) {
        BGTASK_LOGE("AppStateObserver handler is null");
        return false;
    }
    if (bgContinuousTaskMgr_.expired()) {
        BGTASK_LOGE("AppStateObserver bgContinuousTaskMgr is null");
        return false;
    }
    return true;
}

void AppStateObserver::OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData)
{
    if (abilityStateData.abilityState != static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_TERMINATED)) {
        return;
    }
    BGTASK_LOGD("ability state changed, uid: %{public}d abilityName: %{public}s, abilityState: %{public}d",
        abilityStateData.uid, abilityStateData.abilityName.c_str(), abilityStateData.abilityState);
    if (!CheckParamValid()) {
        return;
    }
    int32_t uid = abilityStateData.uid;
    std::string abilityName = abilityStateData.abilityName;
    auto task = [this, uid, abilityName]() {
        this->bgContinuousTaskMgr_.lock()->OnAbilityStateChanged(uid, abilityName);
    };

    auto handler = handler_.lock();
    if (handler == nullptr) {
        return;
    }
    handler->PostTask(task, TASK_ON_ABILITY_STATE_CHANGED);
}

void AppStateObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    BGTASK_LOGD("process died, uid : %{public}d, pid : %{public}d", processData.uid, processData.pid);
    OnProcessDiedContinuousTask(processData);
    OnProcessDiedEfficiencyRes(processData);
}

void AppStateObserver::OnProcessDiedContinuousTask(const AppExecFwk::ProcessData &processData)
{
    if (!CheckParamValid()) {
        return;
    }
    auto task = [this, processData]() {
        this->bgContinuousTaskMgr_.lock()->OnProcessDied(processData.uid, processData.pid);
    };
    handler_.lock()->PostTask(task, TASK_ON_PROCESS_DIED);
}

void AppStateObserver::OnProcessDiedEfficiencyRes(const AppExecFwk::ProcessData &processData)
{
    auto bgEfficiencyResourcesMgr = bgEfficiencyResourcesMgr_.lock();
    if (!bgEfficiencyResourcesMgr) {
        BGTASK_LOGE("bgEfficiencyResourcesMgr is null");
        return;
    }
    bgEfficiencyResourcesMgr->RemoveProcessRecord(processData.uid, processData.pid, processData.bundleName);
}

void AppStateObserver::OnAppStopped(const AppExecFwk::AppStateData &appStateData)
{
    if (!ValidateAppStateData(appStateData)) {
        BGTASK_LOGE("%{public}s : validate app state data failed!", __func__);
        return;
    }
    auto uid = appStateData.uid;
    auto bundleName = appStateData.bundleName;
    auto bgEfficiencyResourcesMgr = bgEfficiencyResourcesMgr_.lock();
    if (!bgEfficiencyResourcesMgr) {
        BGTASK_LOGE("bgEfficiencyResourcesMgr is null");
        return;
    }
    bgEfficiencyResourcesMgr->RemoveAppRecord(uid, bundleName, false);
}

inline bool AppStateObserver::ValidateAppStateData(const AppExecFwk::AppStateData &appStateData)
{
    return appStateData.uid > 0 && appStateData.bundleName.size() > 0;
}

void AppStateObserver::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler)
{
    handler_ = handler;
}

void AppStateObserver::SetBgContinuousTaskMgr(const std::shared_ptr<BgContinuousTaskMgr> &bgContinuousTaskMgr)
{
    bgContinuousTaskMgr_ = bgContinuousTaskMgr;
}

void WEAK_FUNC AppStateObserver::SetBgEfficiencyResourcesMgr(
    const std::shared_ptr<BgEfficiencyResourcesMgr> &resourceMgr)
{
    bgEfficiencyResourcesMgr_ = resourceMgr;
}

bool WEAK_FUNC AppStateObserver::Subscribe()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!Connect()) {
        return false;
    }
    appMgrProxy_->RegisterApplicationStateObserver(iface_cast<AppExecFwk::IApplicationStateObserver>(this));
    return true;
}

bool WEAK_FUNC AppStateObserver::Unsubscribe()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!Connect()) {
        return false;
    }
    appMgrProxy_->UnregisterApplicationStateObserver(iface_cast<AppExecFwk::IApplicationStateObserver>(this));
    return true;
}

bool AppStateObserver::Connect()
{
    if (appMgrProxy_ != nullptr) {
        return true;
    }

    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        BGTASK_LOGE("get SystemAbilityManager failed");
        return false;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        BGTASK_LOGE("get App Manager Service failed");
        return false;
    }

    appMgrProxy_ = iface_cast<AppExecFwk::IAppMgr>(remoteObject);
    if (!appMgrProxy_ || !appMgrProxy_->AsObject()) {
        BGTASK_LOGE("get app mgr proxy failed!");
        return false;
    }
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS