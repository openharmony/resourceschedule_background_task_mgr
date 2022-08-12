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
#include "task_detection_manager.h"
#include "continuous_task_log.h"
#include "bg_efficiency_resources_mgr.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
const std::string TASK_ON_PROCESS_DIED = "OnProcessDiedTask";
const std::string TASK_ON_ABILITY_STATE_CHANGED = "OnAbilityStateChangedTask";
}

AppStateObserver::AppStateObserver()
{
    appMgrDeathRecipient_ = new (std::nothrow) RemoteDeathRecipient(std::bind(&AppStateObserver::OnRemoteDied,
        this, std::placeholders::_1));
}

AppStateObserver::~AppStateObserver()
{
    std::lock_guard<std::mutex> lock(mutex_);
    Disconnect();
}

void AppStateObserver::OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData)
{
    if ((int32_t) AppExecFwk::AbilityState::ABILITY_STATE_TERMINATED != abilityStateData.abilityState) {
        return;
    }
    BGTASK_LOGD("ability state changed, uid: %{public}d abilityName: %{public}s, abilityState: %{public}d",
        abilityStateData.uid, abilityStateData.abilityName.c_str(), abilityStateData.abilityState);
    auto handler = handler_.lock();
    if (!handler) {
        BGTASK_LOGE("handler is null");
        return;
    }
    auto bgContinuousTaskMgr = bgContinuousTaskMgr_.lock();
    if (!bgContinuousTaskMgr) {
        BGTASK_LOGE("bgContinuousTaskMgr is null");
        return;
    }
    auto task = [=]() {
        bgContinuousTaskMgr->OnAbilityStateChanged(abilityStateData.uid, abilityStateData.abilityName);
    };
    handler->PostTask(task, TASK_ON_ABILITY_STATE_CHANGED);
}

void AppStateObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    BGTASK_LOGD("process died, pid : %{public}d", processData.pid);
    auto handler = handler_.lock();
    if (!handler) {
        BGTASK_LOGE("handler is null");
        return;
    }
    auto bgContinuousTaskMgr = bgContinuousTaskMgr_.lock();
    if (!bgContinuousTaskMgr) {
        BGTASK_LOGE("bgContinuousTaskMgr is null");
        return;
    }
    auto bgEfficiencyResourcesMgr = bgEfficiencyResourcesMgr_.lock();

    auto task = [=]() {
        bgContinuousTaskMgr->OnProcessDied(processData.pid);
        TaskDetectionManager::GetInstance()->HandleProcessDied(processData.uid, processData.pid);
    };
    handler->PostTask(task, TASK_ON_PROCESS_DIED);

    if(bgEfficiencyResourcesMgr == nullptr) {
        BGTASK_LOGE("bgEfficiencyResourcesMgr is null");
        return;
    }
    bgEfficiencyResourcesMgr->RemoveProcessRecord(processData.pid);
}

void AppStateObserver::OnApplicationStateChanged(const AppExecFwk::AppStateData &appStateData)
{
    if (!ValidateAppStateData(appStateData)) {
        BGTASK_LOGD("%{public}s : validate app state data failed!", __func__);
        return;
    }
    auto uid = appStateData.uid;
    auto bundleName = appStateData.bundleName;
    auto state = appStateData.state;
    if (state == static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_TERMINATED) || state == 
        static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_END)) {
        auto bgEfficiencyResourcesMgr = bgEfficiencyResourcesMgr_.lock();
        if(bgEfficiencyResourcesMgr == nullptr) {
            BGTASK_LOGE("bgEfficiencyResourcesMgr is null");
            return;
        }
        bgEfficiencyResourcesMgr->RemoveAppRecord(uid);
    }
}

inline bool AppStateObserver::ValidateAppStateData(const AppExecFwk::AppStateData &appStateData)
{
    return appStateData.uid > 0
        && appStateData.bundleName.size() > 0;
}

void AppStateObserver::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler)
{
    handler_ = handler;
}

void AppStateObserver::SetBgContinuousTaskMgr(const std::shared_ptr<BgContinuousTaskMgr> &bgContinuousTaskMgr)
{
    bgContinuousTaskMgr_ = bgContinuousTaskMgr;
}

void AppStateObserver::SetBgEfficiencyResourcesMgr(const std::shared_ptr<BgEfficiencyResourcesMgr> &resourceMgr)
{
    bgEfficiencyResourcesMgr_ = resourceMgr;
}

bool AppStateObserver::Subscribe()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!Connect()) {
        return false;
    }
    appMgrProxy_->RegisterApplicationStateObserver(iface_cast<AppExecFwk::IApplicationStateObserver>(this));
    return true;
}

bool AppStateObserver::Unsubscribe()
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
    appMgrProxy_->AsObject()->AddDeathRecipient(appMgrDeathRecipient_);
    return true;
}

void AppStateObserver::Disconnect()
{
    if (appMgrProxy_ != nullptr && appMgrProxy_->AsObject() != nullptr) {
        appMgrProxy_->AsObject()->RemoveDeathRecipient(appMgrDeathRecipient_);
        appMgrProxy_ = nullptr;
    }
}

void AppStateObserver::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    Disconnect();
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS