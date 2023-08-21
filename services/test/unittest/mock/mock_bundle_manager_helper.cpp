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

#include "ability_info.h"
#include "bundle_info.h"
#include "bundle_manager_helper.h"

#include "continuous_task_log.h"
#include "include/ibundle_manager_helper.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr char TEST_DEFAULT_BUNDLE[]  = "bundleName";
static constexpr uint32_t ALL_NEED_CHECK_BGMODE = 62;
static constexpr uint64_t NO_SYSTEM_APP_TOKEN_ID = -100;
std::shared_ptr<IBundleManagerHelper> bundleManagerHelperMock;
}

BundleManagerHelper::BundleManagerHelper() {}

BundleManagerHelper::~BundleManagerHelper() {}

void SetBundleManagerHelper(std::shared_ptr<IBundleManagerHelper> mock)
{
    bundleManagerHelperMock = mock;
}

void CleanBundleManagerHelper()
{
    bundleManagerHelperMock.reset();
}

bool BundleManagerHelper::GetApplicationInfo(const std::string &appName, const AppExecFwk::ApplicationFlag flag,
    const int userId, AppExecFwk::ApplicationInfo &appInfo)
{
    bool ret {false};
    if (bundleManagerHelperMock) {
        ret = bundleManagerHelperMock->GetApplicationInfo(appName, flag, userId, appInfo);
    }
    return ret;
}

std::string BundleManagerHelper::GetClientBundleName(int32_t uid)
{
    return TEST_DEFAULT_BUNDLE;
}

bool BundleManagerHelper::CheckPermission(const std::string &permission)
{
    return true;
}

bool BundleManagerHelper::IsSystemApp(uint64_t fullTokenId)
{
    if (fullTokenId == NO_SYSTEM_APP_TOKEN_ID) {
        return false;
    }
    return true;
}

bool BundleManagerHelper::GetBundleInfo(const std::string &bundleName, const AppExecFwk::BundleFlag flag,
    AppExecFwk::BundleInfo &bundleInfo, int32_t userId)
{
    if (bundleName == "false-test") {
        return false;
    }
    if (bundleName == "empty-info") {
        AppExecFwk::AbilityInfo abilityInfo;
        abilityInfo.backgroundModes = 0;
        bundleInfo.abilityInfos.emplace_back(abilityInfo);
        return true;
    }
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.name = "ability1";
    abilityInfo.backgroundModes = ALL_NEED_CHECK_BGMODE;
    bundleInfo.abilityInfos.emplace_back(abilityInfo);
    bundleInfo.name = TEST_DEFAULT_BUNDLE;
    return true;
}

bool BundleManagerHelper::QueryAbilityInfo(const AAFwk::Want &want, int32_t flags, int32_t userId,
    AppExecFwk::AbilityInfo &abilityInfo)
{
    abilityInfo.labelId = 1;
    abilityInfo.label = "ability";
    return true;
}

bool BundleManagerHelper::Connect()
{
    return true;
}

void BundleManagerHelper::Disconnect() {}

void BundleManagerHelper::OnRemoteDied(const wptr<IRemoteObject> &object) {}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS