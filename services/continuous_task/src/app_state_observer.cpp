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

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "bg_continuous_task_mgr.h"
#include "continuous_task_log.h"
namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
const std::string TASK_ON_PROCESS_DIED = "OnProcessDiedTask";
}

AppStateObserver::AppStateObserver()
{
    appMgrDeathRecipient_ = new RemoteDeathRecipient(std::bind(&AppStateObserver::OnRemoteDied,
        this, std::placeholders::_1));
}

AppStateObserver::~AppStateObserver()
{
    std::lock_guard<std::mutex> lock(mutex_);
    Disconnect();
}

void AppStateObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    BGTASK_LOGI("process died, pid : %{public}d", processData.pid);
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

    auto task = [=]() { bgContinuousTaskMgr->OnProcessDied(processData.pid); };
    handler->PostTask(task, TASK_ON_PROCESS_DIED);
}

void AppStateObserver::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler)
{
    handler_ = handler;
}

void AppStateObserver::SetBgContinuousTaskMgr(const std::shared_ptr<BgContinuousTaskMgr> &bgContinuousTaskMgr)
{
    bgContinuousTaskMgr_ = bgContinuousTaskMgr;
}

bool AppStateObserver::Subscribe()
{
    BGTASK_LOGI("Subscribe called");
    std::lock_guard<std::mutex> lock(mutex_);

    if (!Connect()) {
        return false;
    }
    appMgrProxy_->RegisterApplicationStateObserver(iface_cast<AppExecFwk::IApplicationStateObserver>(this));
    return true;
}

bool AppStateObserver::Unsubscribe()
{
    BGTASK_LOGI("UnSubscribe called");
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
    if (appMgrProxy_ != nullptr) {
        appMgrProxy_->AsObject()->RemoveDeathRecipient(appMgrDeathRecipient_);
        appMgrProxy_ = nullptr;
    }
}

void AppStateObserver::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    Disconnect();
}
}
}