/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "ipc_skeleton.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bundle_mgr_interface.h"

namespace OHOS {
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif
namespace {
constexpr int32_t DEFAULT_UID = 100;
}
int32_t DebugGetUidByBundleName(const std::string &bundleName, const int32_t userId)
    {
        sptr<ISystemAbilityManager> systemMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemMgr == nullptr) {
            return -1;
        }

        sptr<IRemoteObject> remoteObject = systemMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (remoteObject == nullptr) {
            return -1;
        }

        sptr<OHOS::AppExecFwk::IBundleMgr> bundleMgrProxy = iface_cast<OHOS::AppExecFwk::IBundleMgr>(remoteObject);
        if (bundleMgrProxy == nullptr) {
            return -1;
        }

        return bundleMgrProxy->GetUidByBundleName(bundleName, userId);
    }

pid_t IPCSkeleton::GetCallingPid()
{
    return 1;
}

pid_t IPCSkeleton::GetCallingUid()
{
    int32_t uid = DebugGetUidByBundleName("com.ohos.launcher", DEFAULT_UID);
    if (uid == -1) {
        uid = DebugGetUidByBundleName("com.ohos.sceneboard", DEFAULT_UID);
    }
    return uid;
}

std::string IPCSkeleton::ResetCallingIdentity()
{
    return "";
}

bool IPCSkeleton::SetCallingIdentity(std::string &identity, bool flag)
{
    return true;
}
}  // namespace OHOS
