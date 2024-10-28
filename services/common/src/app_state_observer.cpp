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
#include "bg_transient_task_mgr.h"
#include "continuous_task_log.h"
#include "bg_efficiency_resources_mgr.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
const std::string TASK_ON_PROCESS_DIED = "OnProcessDiedTask";
const std::string TASK_ON_ABILITY_STATE_CHANGED = "OnAbilityStateChangedTask";
const std::string TASK_ON_APP_CACHE_STATE_CHANGED = "OnAppCacheStateChangedTask";
const std::string TASK_ON_APP_DIED = "OnAppDiedTask";
}

void AppStateObserver::OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData)
{
    if (abilityStateData.abilityState != static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_TERMINATED)) {
        return;
    }
    BGTASK_LOGI("ability state changed, uid: %{public}d abilityName: %{public}s, abilityState: %{public}d, "
        "abilityId: %{public}d",
        abilityStateData.uid, abilityStateData.abilityName.c_str(), abilityStateData.abilityState,
        abilityStateData.abilityRecordId);
    int32_t uid = abilityStateData.uid;
    int32_t abilityId = abilityStateData.abilityRecordId;
    std::string abilityName = abilityStateData.abilityName;
    auto task = [uid, abilityName, abilityId]() {
        DelayedSingleton<BgContinuousTaskMgr>::GetInstance()->OnAbilityStateChanged(uid, abilityName, abilityId);
    };
    if (!handler_) {
        BGTASK_LOGE("handler_ null");
        return;
    }
    handler_->PostTask(task, TASK_ON_ABILITY_STATE_CHANGED);
}

void AppStateObserver::OnAppCacheStateChanged(const AppExecFwk::AppStateData &appStateData)
{
    if (appStateData.state != static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_CACHED)) {
        BGTASK_LOGE("state is invalid");
        return;
    }
    if (!handler_) {
        BGTASK_LOGE("handler_ null");
        return;
    }
    BGTASK_LOGI("app cache, name : %{public}s,  uid : %{public}d, pid : %{public}d, state : %{public}d,",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.pid, appStateData.state);
    int32_t uid = appStateData.uid;
    int32_t pid = appStateData.pid;
    std::string bundleName = appStateData.bundleName;
    auto task = [uid, pid, bundleName]() {
        DelayedSingleton<BgTransientTaskMgr>::GetInstance()->OnAppCacheStateChanged(uid, pid, bundleName);
    };
    handler_->PostTask(task, TASK_ON_APP_CACHE_STATE_CHANGED);
}

void AppStateObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    BGTASK_LOGD("process died, uid : %{public}d, pid : %{public}d", processData.uid, processData.pid);
    OnProcessDiedEfficiencyRes(processData);
}

void AppStateObserver::OnProcessDiedEfficiencyRes(const AppExecFwk::ProcessData &processData)
{
    DelayedSingleton<BgEfficiencyResourcesMgr>::GetInstance()->
        RemoveProcessRecord(processData.uid, processData.pid, processData.bundleName);
}

void AppStateObserver::OnAppStopped(const AppExecFwk::AppStateData &appStateData)
{
    BGTASK_LOGD("app stopped, uid : %{public}d", appStateData.uid);
    if (!ValidateAppStateData(appStateData)) {
        BGTASK_LOGE("%{public}s : validate app state data failed!", __func__);
        return;
    }
    auto uid = appStateData.uid;
    auto bundleName = appStateData.bundleName;
    auto task = [uid]() {
        DelayedSingleton<BgContinuousTaskMgr>::GetInstance()->OnAppStopped(uid);
    };
    if (!handler_) {
        BGTASK_LOGE("handler_ null.");
    } else {
        handler_->PostTask(task, TASK_ON_APP_DIED);
    }
    DelayedSingleton<BgEfficiencyResourcesMgr>::GetInstance()->RemoveAppRecord(uid, bundleName, false);
}

inline bool AppStateObserver::ValidateAppStateData(const AppExecFwk::AppStateData &appStateData)
{
    return appStateData.uid > 0 && appStateData.bundleName.size() > 0;
}

void AppStateObserver::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler)
{
    handler_ = handler;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS