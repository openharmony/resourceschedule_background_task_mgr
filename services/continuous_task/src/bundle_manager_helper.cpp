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

#include "bundle_manager_helper.h"

#include "accesstoken_kit.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
using OHOS::AppExecFwk::Constants::PERMISSION_GRANTED;

BundleManagerHelper::BundleManagerHelper()
{
    bundleMgrDeathRecipient_ = new (std::nothrow) RemoteDeathRecipient(
        [this](const wptr<IRemoteObject> &object) { this->OnRemoteDied(object); });
}

BundleManagerHelper::~BundleManagerHelper()
{
    std::lock_guard<std::mutex> lock(connectionMutex_);
    Disconnect();
}

std::string BundleManagerHelper::GetClientBundleName(int32_t uid)
{
    std::string bundle {""};
    std::lock_guard<std::mutex> lock(connectionMutex_);
    Connect();
    if (bundleMgr_ != nullptr) {
        bundleMgr_->GetBundleNameForUid(uid, bundle);
    }
    BGTASK_LOGD("get client Bundle Name: %{public}s", bundle.c_str());
    return bundle;
}

bool BundleManagerHelper::CheckPermission(const std::string &permission)
{
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permission);
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        BGTASK_LOGE("CheckPermission: %{public}s failed", permission.c_str());
        return false;
    }
    return true;
}

bool BundleManagerHelper::IsSystemApp(int32_t uid)
{
    bool isSystemApp = false;
    std::lock_guard<std::mutex> lock(connectionMutex_);
    Connect();
    if (bundleMgr_ != nullptr) {
        isSystemApp = bundleMgr_->CheckIsSystemAppByUid(uid);
    }
    return isSystemApp;
}

bool BundleManagerHelper::GetBundleInfo(const std::string &bundleName, const AppExecFwk::BundleFlag flag,
    AppExecFwk::BundleInfo &bundleInfo, int32_t userId)
{
    std::lock_guard<std::mutex> lock(connectionMutex_);

    Connect();

    if (bundleMgr_ != nullptr && bundleMgr_->GetBundleInfo(bundleName, flag, bundleInfo, userId)) {
        return true;
    }
    return false;
}

bool BundleManagerHelper::Connect()
{
    if (bundleMgr_ != nullptr) {
        return true;
    }

    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        BGTASK_LOGE("get SystemAbilityManager failed");
        return false;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        BGTASK_LOGE("get Bundle Manager failed");
        return false;
    }

    bundleMgr_ = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (bundleMgr_ != nullptr && bundleMgrDeathRecipient_ != nullptr) {
        bundleMgr_->AsObject()->AddDeathRecipient(bundleMgrDeathRecipient_);
        return true;
    }
    BGTASK_LOGE("get bundleMgr failed");
    return false;
}

void BundleManagerHelper::Disconnect()
{
    if (bundleMgr_ != nullptr && bundleMgr_->AsObject() != nullptr) {
        bundleMgr_->AsObject()->RemoveDeathRecipient(bundleMgrDeathRecipient_);
        bundleMgr_ = nullptr;
    }
}

void BundleManagerHelper::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(connectionMutex_);
    Disconnect();
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS