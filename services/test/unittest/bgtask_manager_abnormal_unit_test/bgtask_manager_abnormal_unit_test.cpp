/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#include "common_utils.h"

using namespace testing::ext;

namespace OHOS {

void BgMockTokenType(int32_t mockTokenType);

void BgMockIpcUid(int32_t mockUid);

void BgMockAtomicService(bool mockAtomicService);

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
    EXPECT_EQ(BackgroundTaskMgrService_->UnsubscribeBackgroundTask(nullptr), ERR_BGTASK_PERMISSION_DENIED);

    EXPECT_EQ(BackgroundTaskMgrService_->SuspendContinuousAudioTask(1), ERR_BGTASK_PERMISSION_DENIED);
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
    EXPECT_EQ(BackgroundTaskMgrService_->RequestSuspendDelay("test", nullptr, delayInfo), ERR_BGTASK_PERMISSION_DENIED);
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
    EXPECT_EQ(BackgroundTaskMgrService_->StopBackgroundRunning("test", nullptr, abilityId, continuousTaskId),
        ERR_BGTASK_SYS_NOT_READY);
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

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_010
 * @tc.desc: test BackgroundTaskMgrServiceAbnormal.
 * @tc.type: FUNC
 * @tc.require: 752
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_010, TestSize.Level3)
{
    uint32_t authResult = -1;
    BgMockTokenType(0);
    BgMockAtomicService(true);
    EXPECT_EQ(BackgroundTaskMgrService_->CheckSpecialScenarioAuth(0, authResult),
        ERR_BGTASK_PERMISSION_DENIED);

    BgMockAtomicService(false);
    auto ret = BackgroundTaskMgrService_->CheckSpecialScenarioAuth(0, authResult);
    EXPECT_EQ(ret, ERR_BGTASK_PERMISSION_DENIED);

    BgMockTokenType(1);
    auto ret2 = BackgroundTaskMgrService_->CheckSpecialScenarioAuth(0, authResult, 1);
    EXPECT_EQ(ret2, ERR_BGTASK_CONTINUOUS_API_VERSION_FAIL);

    auto ret3 = BackgroundTaskMgrService_->CheckSpecialScenarioAuth(0, authResult);
    EXPECT_NE(ret3, 0);

    BgMockAtomicService(true);
    BgMockTokenType(0);
}

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_011
 * @tc.desc: test BackgroundTaskMgrServiceAbnormal.
 * @tc.type: FUNC
 * @tc.require: 809
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_011, TestSize.Level3)
{
    BackgroundTaskMgrService_->dependsReady_ = 7;
    uint32_t flag = 1;
    BackgroundTaskMgrService_->SetReady(flag);
    BackgroundTaskMgrService_->dependsReady_ = 0;
    BackgroundTaskMgrService_->SetReady(flag);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_012
 * @tc.desc: test BackgroundTaskMgrServiceAbnormal.
 * @tc.type: FUNC
 * @tc.require: 809
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_012, TestSize.Level3)
{
    std::string extension = "backup";
    MessageParcel data;
    MessageParcel reply;
    MessageParcel options;
    std::u16string descriptor = u"test";
    EXPECT_NE(BackgroundTaskMgrService_->OnExtension(extension, data, reply), ERR_OK);

    extension = "restore";
    EXPECT_NE(BackgroundTaskMgrService_->OnExtension(extension, data, reply), ERR_OK);
}

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_013
 * @tc.desc: test BackgroundTaskMgrServiceAbnormal.
 * @tc.type: FUNC
 * @tc.require: 809
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_013, TestSize.Level3)
{
    ContinuousTaskParam taskParam = ContinuousTaskParam();
    taskParam.isByRequestObject_ = false;
    int32_t notificationId = -1;
    int32_t continuousTaskId = -1;
    EXPECT_NE(BackgroundTaskMgrService_->StartBackgroundRunning(
        taskParam, notificationId, continuousTaskId), ERR_OK);

    taskParam.isByRequestObject_ = true;
    taskParam.bgModeIds_.push_back(1);
    EXPECT_EQ(BackgroundTaskMgrService_->StartBackgroundRunning(
        taskParam, notificationId, continuousTaskId), ERR_BGTASK_PERMISSION_DENIED);

    taskParam.bgModeIds_.clear();
    taskParam.bgModeIds_ = {1, 2, 4};
    EXPECT_NE(BackgroundTaskMgrService_->StartBackgroundRunning(
        taskParam, notificationId, continuousTaskId), ERR_OK);
}

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_014
 * @tc.desc: test BackgroundTaskMgrServiceAbnormal.
 * @tc.type: FUNC
 * @tc.require: 809
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_014, TestSize.Level3)
{
    ContinuousTaskParam taskParam = ContinuousTaskParam();
    taskParam.isByRequestObject_ = false;
    int32_t notificationId = -1;
    int32_t continuousTaskId = -1;
    EXPECT_NE(BackgroundTaskMgrService_->UpdateBackgroundRunning(
        taskParam, notificationId, continuousTaskId), ERR_OK);

    taskParam.isByRequestObject_ = true;
    taskParam.bgModeIds_.push_back(1);
    EXPECT_EQ(BackgroundTaskMgrService_->UpdateBackgroundRunning(
        taskParam, notificationId, continuousTaskId), ERR_BGTASK_PERMISSION_DENIED);

    taskParam.bgModeIds_.clear();
    taskParam.bgModeIds_ = {1, 2, 4};
    EXPECT_NE(BackgroundTaskMgrService_->UpdateBackgroundRunning(
        taskParam, notificationId, continuousTaskId), ERR_OK);
}

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_015
 * @tc.desc: test BackgroundTaskMgrServiceAbnormal.
 * @tc.type: FUNC
 * @tc.require: 809
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_015, TestSize.Level3)
{
    std::string result = "";
    BackgroundTaskMgrService_->DumpUsage(result);
    ContinuousTaskParam taskParam = ContinuousTaskParam();
    EXPECT_EQ(BackgroundTaskMgrService_->IsModeSupported(taskParam), ERR_BGTASK_PERMISSION_DENIED);
    uint32_t authResult = 1;
    BackgroundTaskStateInfo stateInfo = BackgroundTaskStateInfo();
    EXPECT_EQ(BackgroundTaskMgrService_->CheckSpecialScenarioAuth(0, authResult), ERR_BGTASK_PERMISSION_DENIED);
    EXPECT_EQ(BackgroundTaskMgrService_->SetBackgroundTaskState(stateInfo), ERR_BGTASK_PERMISSION_DENIED);
    EXPECT_EQ(BackgroundTaskMgrService_->GetBackgroundTaskState(stateInfo, authResult), ERR_BGTASK_PERMISSION_DENIED);
    std::vector<std::shared_ptr<ContinuousTaskInfo>> list;
    EXPECT_EQ(BackgroundTaskMgrService_->GetAllContinuousTasksBySystem(list), ERR_BGTASK_PERMISSION_DENIED);
    std::set<std::string> bundleNameSet;
    bundleNameSet.insert("com.test.app");
    EXPECT_EQ(BackgroundTaskMgrService_->SetSpecialExemptedProcess(bundleNameSet), ERR_BGTASK_PERMISSION_DENIED);
    std::set<std::string> taskKeys;
    EXPECT_EQ(BackgroundTaskMgrService_->SendNotificationByDeteTask(taskKeys), ERR_BGTASK_PERMISSION_DENIED);
}

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_016
 * @tc.desc: test RemoveAuthRecord.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_016, TestSize.Level3)
{
    std::string result = "";
    BackgroundTaskMgrService_->DumpUsage(result);
    BgMockTokenType(0);
    ContinuousTaskParam taskParam = ContinuousTaskParam();
    EXPECT_EQ(BackgroundTaskMgrService_->RemoveAuthRecord(taskParam), ERR_BGTASK_PERMISSION_DENIED);

    BgMockTokenType(1);
    EXPECT_NE(BackgroundTaskMgrService_->RemoveAuthRecord(taskParam), 0);
    BgMockTokenType(0);
}

/**
 * @tc.name: CheckHapCalling_001
 * @tc.desc: test CheckHapCalling.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, CheckHapCalling_001, TestSize.Level3)
{
    BgMockTokenType(1);
    bool isHap = false;
    auto ret = BackgroundTaskMgrService_->CheckHapCalling(isHap, 1);
    EXPECT_FALSE(ret);

    auto ret2 = BackgroundTaskMgrService_->CheckHapCalling(isHap, SUBSCRIBER_BACKGROUND_TASK_STATE);
    EXPECT_TRUE(ret2);
    BgMockTokenType(0);
}

/**
 * @tc.name: CheckCallingProcess_001
 * @tc.desc: test CheckCallingProcess.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, CheckCallingProcess_001, TestSize.Level3)
{
    std::vector<std::shared_ptr<DelaySuspendInfo>> list;
    int32_t remainingQuota = 0;
    BackgroundTaskMgrService_->GetAllTransientTasks(remainingQuota, list);
    std::vector<ContinuousTaskInfo> list2;
    BackgroundTaskMgrService_->GetAllContinuousTasks(list2);
    std::vector<std::shared_ptr<ContinuousTaskInfo>> list3;
    BackgroundTaskMgrService_->GetAllContinuousTasks(list3, true);

    auto ret = BackgroundTaskMgrService_->CheckCallingProcess();
    EXPECT_FALSE(ret);

    int32_t resUid = 1096;
    BgMockIpcUid(resUid);
    auto ret2 = BackgroundTaskMgrService_->CheckCallingProcess();
    EXPECT_TRUE(ret2);
    BgMockIpcUid(-1);
}

/**
 * @tc.name: RequestGetContinuousTasksByUidForInner_001
 * @tc.desc: test RequestGetContinuousTasksByUidForInner.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, RequestGetContinuousTasksByUidForInner_001, TestSize.Level3)
{
    int uid = 1;
    std::vector<ContinuousTaskInfo> list;
    auto ret = BackgroundTaskMgrService_->RequestGetContinuousTasksByUidForInner(uid, list);
    EXPECT_NE(ret, 0);

    BgMockTokenType(2);
    std::vector<TransientTaskAppInfo> list2 {};
    BackgroundTaskMgrService_->GetTransientTaskApps(list2);
    std::vector<ContinuousTaskCallbackInfo> list3 {};
    BackgroundTaskMgrService_->GetContinuousTaskApps(list3);
    std::vector<EfficiencyResourceInfo> resourceInfoList {};
    BackgroundTaskMgrService_->GetAllEfficiencyResources(resourceInfoList);
    
    auto ret2 = BackgroundTaskMgrService_->RequestGetContinuousTasksByUidForInner(-1, list);
    EXPECT_NE(ret2, 0);

    BackgroundTaskMgrService_->RequestGetContinuousTasksByUidForInner(uid, list);
    BgMockTokenType(0);
}

/**
 * @tc.name: SetSupportedTaskKeepingProcesses_001
 * @tc.desc: test SetSupportedTaskKeepingProcesses.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, SetSupportedTaskKeepingProcesses_001, TestSize.Level3)
{
    std::set<std::string> processSet;
    processSet.insert("com.test.app");
    auto ret = BackgroundTaskMgrService_->SetSupportedTaskKeepingProcesses(processSet);
    EXPECT_NE(ret, 0);

    BgMockTokenType(2);
    auto ret2 = BackgroundTaskMgrService_->SetSupportedTaskKeepingProcesses(processSet);
    EXPECT_NE(ret2, 0);
    int32_t resUid = 1096;
    BgMockIpcUid(resUid);

    auto ret3 = BackgroundTaskMgrService_->SetSupportedTaskKeepingProcesses(processSet);
    EXPECT_EQ(ret3, 0);

    BgMockIpcUid(-1);
    BgMockTokenType(0);
}

/**
 * @tc.name: SetMaliciousAppConfig_001
 * @tc.desc: test SetMaliciousAppConfig.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, SetMaliciousAppConfig_001, TestSize.Level3)
{
    std::set<std::string> maliciousAppSet;
    auto ret = BackgroundTaskMgrService_->SetMaliciousAppConfig(maliciousAppSet);
    EXPECT_NE(ret, 0);

    BgMockTokenType(2);
    auto ret2 = BackgroundTaskMgrService_->SetMaliciousAppConfig(maliciousAppSet);
    EXPECT_NE(ret2, 0);
    int32_t resUid = 1096;
    BgMockIpcUid(resUid);

    auto ret3 = BackgroundTaskMgrService_->SetMaliciousAppConfig(maliciousAppSet);
    EXPECT_EQ(ret3, 0);

    BgMockIpcUid(-1);
    BgMockTokenType(0);
}

/**
 * @tc.name: RequestAuthFromUser_001
 * @tc.desc: test RequestAuthFromUser.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, RequestAuthFromUser_001, TestSize.Level3)
{
    int32_t notificationId = -1;
    ContinuousTaskParam taskParam = ContinuousTaskParam();
    auto ret = BackgroundTaskMgrService_->RequestAuthFromUser(taskParam, nullptr, notificationId);
    EXPECT_EQ(ret, ERR_BGTASK_PERMISSION_DENIED);

    BgMockAtomicService(false);
    auto ret2 = BackgroundTaskMgrService_->RequestAuthFromUser(taskParam, nullptr, notificationId);
    EXPECT_EQ(ret2, ERR_BGTASK_PERMISSION_DENIED);

    BgMockTokenType(1);
    taskParam.requestAuthApiVersion_ = 1;
    auto ret3 = BackgroundTaskMgrService_->RequestAuthFromUser(taskParam, nullptr, notificationId);
    EXPECT_EQ(ret3, ERR_BGTASK_CONTINUOUS_API_VERSION_FAIL);

    taskParam.requestAuthApiVersion_ = API_VERSION_REQUEST_SPECIAL_USER_AUTH_BY_DIALOG;
    auto ret4 = BackgroundTaskMgrService_->RequestAuthFromUser(taskParam, nullptr, notificationId);
    EXPECT_NE(ret4, 0);

    BgMockAtomicService(true);
    BgMockTokenType(0);
}

/**
 * @tc.name: CheckTaskAuthResult_001
 * @tc.desc: test CheckTaskAuthResult.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, CheckTaskAuthResult_001, TestSize.Level3)
{
    auto ret = BackgroundTaskMgrService_->CheckTaskAuthResult("", 1, 1);
    EXPECT_EQ(ret, ERR_BGTASK_PERMISSION_DENIED);

    BgMockTokenType(2);
    auto ret2 = BackgroundTaskMgrService_->CheckTaskAuthResult("", 1, 1);
    EXPECT_EQ(ret2, ERR_BGTASK_PERMISSION_DENIED);

    int32_t resUid = 1096;
    BgMockIpcUid(resUid);
    auto ret3 = BackgroundTaskMgrService_->CheckTaskAuthResult("", 1, 1);
    EXPECT_NE(ret3, 0);

    BgMockTokenType(0);
    BgMockIpcUid(-1);
}

/**
 * @tc.name: EnableContinuousTaskRequest_001
 * @tc.desc: test EnableContinuousTaskRequest.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, EnableContinuousTaskRequest_001, TestSize.Level3)
{
    auto ret = BackgroundTaskMgrService_->EnableContinuousTaskRequest(1, false);
    EXPECT_EQ(ret, ERR_BGTASK_PERMISSION_DENIED);

    BgMockTokenType(2);
    auto ret2 = BackgroundTaskMgrService_->EnableContinuousTaskRequest(1, false);
    EXPECT_EQ(ret2, ERR_BGTASK_PERMISSION_DENIED);

    int32_t resUid = 1096;
    BgMockIpcUid(resUid);
    auto ret3 = BackgroundTaskMgrService_->EnableContinuousTaskRequest(1, false);
    EXPECT_NE(ret3, 0);

    BgMockTokenType(0);
    BgMockIpcUid(-1);
}

/**
 * @tc.name: SetBackgroundTaskState_001
 * @tc.desc: test SetBackgroundTaskState.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, SetBackgroundTaskState_001, TestSize.Level3)
{
    BackgroundTaskStateInfo stateInfo = BackgroundTaskStateInfo();
    BgMockAtomicService(false);
    auto ret = BackgroundTaskMgrService_->SetBackgroundTaskState(stateInfo);
    EXPECT_EQ(ret, ERR_BGTASK_PERMISSION_DENIED);

    BgMockTokenType(1);
    auto ret2 = BackgroundTaskMgrService_->SetBackgroundTaskState(stateInfo);
    EXPECT_EQ(ret2, ERR_BGTASK_PERMISSION_DENIED);

    BgMockAtomicService(true);
    BgMockTokenType(0);
}

/**
 * @tc.name: GetBackgroundTaskState_001
 * @tc.desc: test GetBackgroundTaskState.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, GetBackgroundTaskState_001, TestSize.Level3)
{
    BackgroundTaskStateInfo stateInfo = BackgroundTaskStateInfo();
    BgMockAtomicService(false);
    uint32_t authResult = 0;
    auto ret = BackgroundTaskMgrService_->GetBackgroundTaskState(stateInfo, authResult);
    EXPECT_EQ(ret, ERR_BGTASK_PERMISSION_DENIED);

    BgMockTokenType(1);
    auto ret2 = BackgroundTaskMgrService_->GetBackgroundTaskState(stateInfo, authResult);
    EXPECT_EQ(ret2, ERR_BGTASK_PERMISSION_DENIED);

    BgMockAtomicService(true);
    BgMockTokenType(0);
}

/**
 * @tc.name: GetAllContinuousTasksBySystem_001
 * @tc.desc: test GetAllContinuousTasksBySystem.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, GetAllContinuousTasksBySystem_001, TestSize.Level3)
{
    BgMockAtomicService(false);
    std::vector<std::shared_ptr<ContinuousTaskInfo>> list;
    auto ret = BackgroundTaskMgrService_->GetAllContinuousTasksBySystem(list);
    EXPECT_EQ(ret, ERR_BGTASK_PERMISSION_DENIED);

    BgMockTokenType(1);
    auto ret2 = BackgroundTaskMgrService_->GetAllContinuousTasksBySystem(list);
    EXPECT_EQ(ret2, ERR_BGTASK_PERMISSION_DENIED);

    BgMockAtomicService(true);
    BgMockTokenType(0);
}

/**
 * @tc.name: GetAllContinuousTaskApps_001
 * @tc.desc: test GetAllContinuousTaskApps.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, GetAllContinuousTaskApps_001, TestSize.Level3)
{
    std::vector<ContinuousTaskCallbackInfo> list;
    auto ret = BackgroundTaskMgrService_->GetAllContinuousTaskApps(list);
    EXPECT_EQ(ret, ERR_BGTASK_PERMISSION_DENIED);

    BgMockTokenType(2);
    auto ret2 = BackgroundTaskMgrService_->GetAllContinuousTaskApps(list);
    EXPECT_EQ(ret2, ERR_BGTASK_PERMISSION_DENIED);

    int32_t resUid = 1096;
    BgMockIpcUid(resUid);

    auto ret3 = BackgroundTaskMgrService_->GetAllContinuousTaskApps(list);
    EXPECT_NE(ret3, 0);

    BgMockIpcUid(-1);
    BgMockTokenType(0);
}

/**
 * @tc.name: BackgroundTaskMgrServiceAbnormalTest_017
 * @tc.desc: test BackgroundTaskMgrServiceAbnormalTest.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskManagerAbnormalUnitTest, BackgroundTaskMgrServiceAbnormalTest_017, TestSize.Level3)
{
    BgMockTokenType(2);
    int32_t resUid = 1096;
    BgMockIpcUid(resUid);

    auto ret = BackgroundTaskMgrService_->StopContinuousTask(1, 1, 1, "");
    EXPECT_EQ(ret, 0);

    auto ret2 = BackgroundTaskMgrService_->SuspendContinuousTask(1, 1, 4, "");
    EXPECT_EQ(ret2, 0);

    auto ret3 = BackgroundTaskMgrService_->ActiveContinuousTask(1, 1, "");
    EXPECT_EQ(ret3, 0);

    auto ret4 = BackgroundTaskMgrService_->AVSessionNotifyUpdateNotification(1, 1, true);
    EXPECT_NE(ret4, 0);

    auto ret5 = BackgroundTaskMgrService_->SetBgTaskConfig("", 1);
    EXPECT_NE(ret5, 0);

    auto ret6 = BackgroundTaskMgrService_->SuspendContinuousAudioTask(1);
    EXPECT_EQ(ret6, 0);

    std::set<std::string> bundleNameSet;
    bundleNameSet.insert("com.test.app");
    auto ret7 = BackgroundTaskMgrService_->SetSpecialExemptedProcess(bundleNameSet);
    EXPECT_EQ(ret7, 0);

    std::set<std::string> taskKeys;
    taskKeys.insert("key");
    auto ret8 = BackgroundTaskMgrService_->SendNotificationByDeteTask(taskKeys);
    EXPECT_NE(ret8, 0);

    BgMockIpcUid(-1);
    BgMockTokenType(0);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS