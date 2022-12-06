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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_BUNDLE_MANAGER_HELPER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_BUNDLE_MANAGER_HELPER_H

#include "bundle_mgr_interface.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "resource_manager.h"
#include "singleton.h"

#include "remote_death_recipient.h"

namespace OHOS {
namespace BackgroundTaskMgr {
using OHOS::AppExecFwk::Constants::UNSPECIFIED_USERID;
class BundleManagerHelper : public DelayedSingleton<BundleManagerHelper> {
public:
    std::string GetClientBundleName(int32_t uid);
    bool CheckPermission(const std::string &permission);
    bool IsSystemApp(uint64_t fullTokenId);
    bool GetBundleInfo(const std::string &bundleName, const AppExecFwk::BundleFlag flag,
        AppExecFwk::BundleInfo &bundleInfo, int32_t userId = UNSPECIFIED_USERID);
    bool GetApplicationInfo(const std::string &appName, const AppExecFwk::ApplicationFlag flag,
        const int userId, AppExecFwk::ApplicationInfo &appInfo);
    bool QueryAbilityInfo(const AAFwk::Want &want, int32_t flags, int32_t userId,
        AppExecFwk::AbilityInfo &abilityInfo);

private:
    bool Connect();
    void Disconnect();
    void OnRemoteDied(const wptr<IRemoteObject> &object);

private:
    sptr<AppExecFwk::IBundleMgr> bundleMgr_ = nullptr;
    std::mutex connectionMutex_;
    sptr<RemoteDeathRecipient> bundleMgrDeathRecipient_ = nullptr;

    DECLARE_DELAYED_SINGLETON(BundleManagerHelper)
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_BUNDLE_MANAGER_HELPER_H