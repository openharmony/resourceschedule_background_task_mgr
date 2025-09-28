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
#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_APP_MGR_HELPER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_APP_MGR_HELPER_H

#ifdef BGTASK_MGR_UNIT_TEST
#define WEAK_FUNC __attribute__((weak))
#else
#define WEAK_FUNC
#endif

#include "app_mgr_interface.h"
#include "app_mgr_proxy.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "remote_death_recipient.h"
#include "singleton.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class AppMgrHelper : public DelayedSingleton<AppMgrHelper> {
public:
    /**
     * @brief Get the All Running Processes info.
     *
     * @param allAppProcessInfos process info of applications.
     * @return true if subscribe successfully.
     */
    bool GetAllRunningProcesses(std::vector<AppExecFwk::RunningProcessInfo>& allAppProcessInfos);

    /**
     * Get Foreground Applications.
     */
    bool GetForegroundApplications(std::vector<AppExecFwk::AppStateData> &fgApps);

    /**
     * @brief Subscribe AppStateObserver.
     *
     * @param observer app state observer.
     * @return true if subscribe successfully.
     */
    bool SubscribeObserver(const sptr<AppExecFwk::IApplicationStateObserver> &observer);

    /**
     * @brief Unubscribe AppStateObserver.
     *
     * @param observer app state observer.
     * @return true if unsubscribe successfully.
     */
    bool UnsubscribeObserver(const sptr<AppExecFwk::IApplicationStateObserver> &observer);

    bool SubscribeConfigurationObserver(const sptr<AppExecFwk::IConfigurationObserver> &observer);

private:
    bool Connect();

private:
    sptr<AppExecFwk::IAppMgr> appMgrProxy_ {nullptr};
    std::mutex connectMutex_ {};
    DECLARE_DELAYED_SINGLETON(AppMgrHelper);
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS

#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_APP_MGR_HELPER_H