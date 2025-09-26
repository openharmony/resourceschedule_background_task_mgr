/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "app_mgr_helper.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
AppMgrHelper::AppMgrHelper()
{
    appMgrProxyDeathRecipient_ = new (std::nothrow) RemoteDeathRecipient(
        [this](const wptr<IRemoteObject> &object) { this->OnRemoteDied(object); });
}

AppMgrHelper::~AppMgrHelper()
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    Disconnect();
}

std::shared_ptr<AppMgrHelper> AppMgrHelper::GetInstance()
{
    return DelayedSingleton<AppMgrHelper>::GetInstance();
}

bool WEAK_FUNC AppMgrHelper::GetAllRunningProcesses(std::vector<AppExecFwk::RunningProcessInfo>& allAppProcessInfos)
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    if (!Connect()) {
        return false;
    }
    if (appMgrProxy_->GetAllRunningProcesses(allAppProcessInfos) != ERR_OK) {
        BGTASK_LOGE("failed to get all running process.");
        return false;
    }
    return true;
}

bool WEAK_FUNC AppMgrHelper::GetForegroundApplications(std::vector<AppExecFwk::AppStateData> &fgApps)
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    if (!Connect()) {
        return false;
    }
    if (appMgrProxy_->GetForegroundApplications(fgApps) != ERR_OK) {
        return false;
    }
    return true;
}

bool WEAK_FUNC AppMgrHelper::SubscribeConfigurationObserver(const sptr<AppExecFwk::IConfigurationObserver> &observer)
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    if (!Connect()) {
        return false;
    }
    int32_t res = appMgrProxy_->RegisterConfigurationObserver(observer);
    if (res != ERR_OK) {
        BGTASK_LOGE("failed to register configuration state observer, ret: %{public}d", res);
        return false;
    }
    return true;
}

bool WEAK_FUNC AppMgrHelper::SubscribeObserver(const sptr<AppExecFwk::IApplicationStateObserver> &observer)
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    if (!Connect()) {
        return false;
    }
    int32_t res = appMgrProxy_->RegisterApplicationStateObserver(observer);
    if (res != ERR_OK) {
        BGTASK_LOGE("failed to register application state observer, ret: %{public}d", res);
        return false;
    }
    return true;
}

bool WEAK_FUNC AppMgrHelper::UnsubscribeObserver(const sptr<AppExecFwk::IApplicationStateObserver> &observer)
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    if (!Connect()) {
        return false;
    }
    if (appMgrProxy_->UnregisterApplicationStateObserver(observer) != ERR_OK) {
        return false;
    }
    return true;
}

bool WEAK_FUNC AppMgrHelper::Connect()
{
    if (appMgrProxy_ != nullptr) {
        return true;
    }

    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        BGTASK_LOGE("failed to get SystemAbilityManager");
        return false;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        BGTASK_LOGE("failed to get App Manager Service");
        return false;
    }

    appMgrProxy_ = iface_cast<AppExecFwk::IAppMgr>(remoteObject);
    if (!appMgrProxy_ || !appMgrProxy_->AsObject()) {
        BGTASK_LOGE("failed to get app mgr proxy");
        return false;
    }
    return true;
}

void AppMgrHelper::Disconnect()
{
    if (appMgrProxy_ != nullptr && appMgrProxy_->AsObject() != nullptr) {
        appMgrProxy_->AsObject()->RemoveDeathRecipient(appMgrProxyDeathRecipient_);
        appMgrProxy_ = nullptr;
    }
}

void AppMgrHelper::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    Disconnect();
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS