/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr char TEST_DEFAULT_BUNDLE[]  = "bundleName";
}

BundleManagerHelper::BundleManagerHelper()
{}

BundleManagerHelper::~BundleManagerHelper()
{}

std::string BundleManagerHelper::GetClientBundleName(uid_t uid)
{
    return TEST_DEFAULT_BUNDLE;
}

bool BundleManagerHelper::CheckPermission(const std::string &bundleName,
    const std::string &permission)
{
    return true;
}

bool BundleManagerHelper::IsSystemApp(uid_t uid)
{
    return true;
}

bool BundleManagerHelper::GetBundleInfo(const std::string &bundleName, const AppExecFwk::BundleFlag flag,
    AppExecFwk::BundleInfo &bundleInfo, int32_t userId)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.name = "ability1";
    abilityInfo.backgroundModes = 1;
    bundleInfo.abilityInfos.emplace_back(abilityInfo);
    bundleInfo.name = TEST_DEFAULT_BUNDLE;
    return true;
}

bool BundleManagerHelper::Connect()
{
    return true;
}

void BundleManagerHelper::Disconnect()
{}

void BundleManagerHelper::OnRemoteDied(const wptr<IRemoteObject> &object)
{}
}
}