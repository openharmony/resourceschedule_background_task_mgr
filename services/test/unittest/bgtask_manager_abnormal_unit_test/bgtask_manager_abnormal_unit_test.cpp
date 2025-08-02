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
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_001, TestSize.Level3)
{
    BackgroundTaskMgrService_->state_ = ServiceRunningState::STATE_RUNNING;
    BackgroundTaskMgrService_->OnStart();
    BackgroundTaskMgrService_->state_ = ServiceRunningState::STATE_NOT_START;

    BackgroundTaskMgrService_->OnAddSystemAbility(-1, "");
    BackgroundTaskMgrService_->OnRemoveSystemAbility(-1, "");
    EXPECT_EQ(BackgroundTaskMgrService_->state_, ServiceRunningState::STATE_NOT_START);
}

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_002
 * @tc.desc: test BackgroundTaskMgrServiceAbnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_002, TestSize.Level3)
{
    std::vector<TransientTaskAppInfo> list1;
    EXPECT_EQ(BackgroundTaskMgrService_->GetTransientTaskApps(list1), ERR_BGTASK_PERMISSION_DENIED);

    std::vector<ContinuousTaskCallbackInfo> list2;
    EXPECT_EQ(BackgroundTaskMgrService_->GetContinuousTaskApps(list2), ERR_BGTASK_SYS_NOT_READY);

    std::vector<ResourceCallbackInfo> list3;
    std::vector<ResourceCallbackInfo> list4;
    EXPECT_EQ(BackgroundTaskMgrService_->GetEfficiencyResourcesInfos(list3, list4), ERR_BGTASK_PERMISSION_DENIED);

    EXPECT_EQ(BackgroundTaskMgrService_->StopContinuousTask(1, 1, 1, ""), ERR_BGTASK_PERMISSION_DENIED);
    EXPECT_EQ(BackgroundTaskMgrService_->SuspendContinuousTask(1, 1, 4, ""), ERR_BGTASK_PERMISSION_DENIED);
    EXPECT_EQ(BackgroundTaskMgrService_->ActiveContinuousTask(1, 1, ""), ERR_BGTASK_PERMISSION_DENIED);

    EXPECT_EQ(BackgroundTaskMgrService_->SubscribeBackgroundTask(nullptr, 0), ERR_BGTASK_PERMISSION_DENIED);
    EXPECT_EQ(BackgroundTaskMgrService_->UnsubscribeBackgroundTask(nullptr, 0), ERR_BGTASK_PERMISSION_DENIED);
}

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_003
 * @tc.desc: test BackgroundTaskMgrServiceAbnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_003, TestSize.Level3)
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
    EXPECT_EQ(BackgroundTaskMgrService_->Dump(-1, args1), ERR_BGTASK_PERMISSION_DENIED);
}

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_004
 * @tc.desc: test BackgroundTaskMgrServiceAbnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_004, TestSize.Level3)
{
    DelaySuspendInfo delayInfo;
    EXPECT_EQ(BackgroundTaskMgrService_->RequestSuspendDelay("test", nullptr, delayInfo), ERR_BGTASK_SYS_NOT_READY);
    int32_t requestId = 1;
    int32_t delayTime = 1;
    EXPECT_EQ(BackgroundTaskMgrService_->GetRemainingDelayTime(requestId, delayTime), ERR_BGTASK_SYS_NOT_READY);
    BackgroundTaskMgrService_->ForceCancelSuspendDelay(requestId);
    EXPECT_EQ(BackgroundTaskMgrService_->CancelSuspendDelay(requestId), ERR_BGTASK_SYS_NOT_READY);
}

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_005
 * @tc.desc: test BackgroundTaskMgrServiceAbnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_005, TestSize.Level3)
{
    int32_t notificationId = -1;
    int32_t continuousTaskId = -1;
    ContinuousTaskParam taskParam = ContinuousTaskParam();
    EXPECT_EQ(BackgroundTaskMgrService_->StartBackgroundRunning(
        taskParam, notificationId, continuousTaskId), ERR_BGTASK_SYS_NOT_READY);

    ContinuousTaskParamForInner taskParamInner = ContinuousTaskParamForInner();
    EXPECT_EQ(BackgroundTaskMgrService_->RequestBackgroundRunningForInner(taskParamInner), ERR_BGTASK_SYS_NOT_READY);

    EXPECT_EQ(BackgroundTaskMgrService_->UpdateBackgroundRunning(
        taskParam, notificationId, continuousTaskId), ERR_BGTASK_SYS_NOT_READY);
    int32_t abilityId = -1;
    EXPECT_EQ(BackgroundTaskMgrService_->StopBackgroundRunning("test", nullptr, abilityId), ERR_BGTASK_SYS_NOT_READY);
}

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_006
 * @tc.desc: test BackgroundTaskMgrServiceAbnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_006, TestSize.Level3)
{
    int32_t requestId = 1;
    BackgroundTaskMgrService_->HandleRequestExpired(requestId);
    BackgroundTaskMgrService_->HandleExpiredCallbackDeath(nullptr);
    BackgroundTaskMgrService_->HandleSubscriberDeath(nullptr);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_007
 * @tc.desc: test BackgroundTaskMgrServiceAbnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_007, TestSize.Level3)
{
    int32_t uid = 0;
    EXPECT_EQ(BackgroundTaskMgrService_->StartTransientTaskTimeForInner(uid), ERR_BGTASK_SYS_NOT_READY);
    EXPECT_EQ(BackgroundTaskMgrService_->PauseTransientTaskTimeForInner(uid), ERR_BGTASK_SYS_NOT_READY);

    EfficiencyResourceInfo resources = EfficiencyResourceInfo();
    EXPECT_EQ(BackgroundTaskMgrService_->ApplyEfficiencyResources(resources), ERR_BGTASK_SYS_NOT_READY);
    EXPECT_EQ(BackgroundTaskMgrService_->ResetAllEfficiencyResources(), ERR_BGTASK_SYS_NOT_READY);
    
    EXPECT_EQ(BackgroundTaskMgrService_->SetBgTaskConfig("", 1), ERR_BGTASK_PERMISSION_DENIED);
}

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_008
 * @tc.desc: test BackgroundTaskMgrServiceAbnormal.
 * @tc.type: FUNC
 * @tc.require: issueIC9VN9
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_008, TestSize.Level3)
{
    int32_t uid = 0;
    int32_t pid = 0;
    EXPECT_EQ(BackgroundTaskMgrService_->AVSessionNotifyUpdateNotification(uid, pid, true),
        ERR_BGTASK_PERMISSION_DENIED);
    EXPECT_EQ(BackgroundTaskMgrService_->AVSessionNotifyUpdateNotification(uid, pid, false),
        ERR_BGTASK_PERMISSION_DENIED);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS