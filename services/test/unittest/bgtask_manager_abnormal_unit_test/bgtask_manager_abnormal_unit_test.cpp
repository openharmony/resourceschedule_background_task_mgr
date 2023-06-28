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

#include "background_task_mgr_service.h"
#include "transient_task_app_info.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {
class BgTaskManagerAbnormalUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}

    static std::shared_ptr<BackgroundTaskMgrService> BackgroundTaskMgrService_;
};

std::shared_ptr<BackgroundTaskMgrService> BgTaskManagerAbnormalUnitTest::BackgroundTaskMgrService_
    = std::make_shared<BackgroundTaskMgrService>();

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_001
 * @tc.desc: test BackgroundTaskMgrServiceAbnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_001, TestSize.Level1)
{
    BackgroundTaskMgrService_->state_ = ServiceRunningState::STATE_RUNNING;
    BackgroundTaskMgrService_->OnStart();
    BackgroundTaskMgrService_->state_ = ServiceRunningState::STATE_NOT_START;

    BackgroundTaskMgrService_->OnRemoveSystemAbility(-1, "");
    EXPECT_EQ(BackgroundTaskMgrService_->state_, ServiceRunningState::STATE_NOT_START);
}

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_002
 * @tc.desc: test BackgroundTaskMgrServiceAbnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_002, TestSize.Level1)
{
    std::vector<std::shared_ptr<TransientTaskAppInfo>> list1;
    EXPECT_EQ(BackgroundTaskMgrService_->GetTransientTaskApps(list1), ERR_BGTASK_PERMISSION_DENIED);

    std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> list2;
    EXPECT_EQ(BackgroundTaskMgrService_->GetContinuousTaskApps(list2), ERR_BGTASK_PERMISSION_DENIED);

    std::vector<std::shared_ptr<ResourceCallbackInfo>> list3;
    std::vector<std::shared_ptr<ResourceCallbackInfo>> list4;
    EXPECT_EQ(BackgroundTaskMgrService_->GetEfficiencyResourcesInfos(list3, list4), ERR_BGTASK_PERMISSION_DENIED);

    EXPECT_EQ(BackgroundTaskMgrService_->StopContinuousTask(1, 1, 1), ERR_BGTASK_PERMISSION_DENIED);

    EXPECT_EQ(BackgroundTaskMgrService_->SubscribeBackgroundTask(nullptr), ERR_BGTASK_PERMISSION_DENIED);
    EXPECT_EQ(BackgroundTaskMgrService_->UnsubscribeBackgroundTask(nullptr), ERR_BGTASK_PERMISSION_DENIED);
}

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_003
 * @tc.desc: test BackgroundTaskMgrServiceAbnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_003, TestSize.Level1)
{
    std::vector<std::u16string> args1;
    BackgroundTaskMgrService_->Dump(-1, args1);
    args1.emplace_back(Str8ToStr16("-h"));
    BackgroundTaskMgrService_->Dump(-1, args1);
    args1.clear();
    args1.emplace_back(Str8ToStr16("-T"));
    BackgroundTaskMgrService_->Dump(-1, args1);
    args1.clear();
    args1.emplace_back(Str8ToStr16("-C"));
    BackgroundTaskMgrService_->Dump(-1, args1);
    args1.clear();
    args1.emplace_back(Str8ToStr16("-E"));
    BackgroundTaskMgrService_->Dump(-1, args1);
    args1.clear();
    args1.emplace_back(Str8ToStr16("Invalid"));
    EXPECT_EQ(BackgroundTaskMgrService_->Dump(-1, args1), ERR_BGTASK_METHOD_CALLED_FAILED);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS