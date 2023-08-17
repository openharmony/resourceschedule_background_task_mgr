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
#include "tokenid_kit.h"

#include "continuous_task_log.h"
#include "gmock_bundle_manager_helper.h"

namespace OHOS {
namespace BackgroundTaskMgr {
std::shared_ptr<IBundleManagerHelper> bundleManagerHelperMock;

bool WEAK_FUNC BundleManagerHelper::GetApplicationInfo(const std::string &appName, const AppExecFwk::ApplicationFlag flag,
    const int userId, AppExecFwk::ApplicationInfo &appInfo)
{
    bool ret {false};
    if (bundleManagerHelperMock) {
        ret = bundleManagerHelperMock->GetApplicationInfo(appName, flag, userId, appInfo);
    }
    return ret;
}

void SetBundleManagerHelper(std::shared_ptr<IBundleManagerHelper> mock)
{
    bundleManagerHelperMock = mock;
}

void CleanBundleManagerHelper(std::shared_ptr<IBundleManagerHelper> mock)
{
    bundleManagerHelperMock.reset();
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS