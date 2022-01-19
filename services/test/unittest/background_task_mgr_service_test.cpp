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

#include <functional>
#include <gtest/gtest.h>

#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "background_task_mgr_service.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundTaskMgrServiceTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: BackgroundTaskMgrServiceTest_001
 * @tc.desc: Test BackgroundTaskMgrService service ready.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(BackgroundTaskMgrServiceTest, BackgroundTaskMgrServiceTest_001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr) << "BackgroundTaskMgrServiceTest fail to get GetSystemAbilityManager";
    sptr<IRemoteObject> remoteObject = sm->CheckSystemAbility(BACKGROUND_TASK_MANAGER_SERVICE_ID);
    EXPECT_TRUE(remoteObject != nullptr) << "GetSystemAbility failed";
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
