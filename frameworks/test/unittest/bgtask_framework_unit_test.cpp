/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include <chrono>
#include <thread>
#include <message_parcel.h>

#include "gtest/gtest.h"

#include "background_task_manager.h"
#include "background_task_subscriber.h"
#include "background_task_subscriber_stub.h"
#include "background_task_subscriber_proxy.h"
#include "bgtaskmgr_inner_errors.h"
#include "bgtaskmgr_log_wrapper.h"
#include "continuous_task_callback_info.h"
#include "continuous_task_info.h"
#include "continuous_task_param.h"
#include "delay_suspend_info.h"
#include "efficiency_resource_info.h"
#include "expired_callback.h"
#include "expired_callback_proxy.h"
#include "expired_callback_stub.h"
#include "iservice_registry.h"
#include "resource_callback_info.h"
#include "singleton.h"
#include "transient_task_app_info.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
constexpr int32_t SLEEP_TIME = 500;
constexpr uint32_t ON_CONNECTED = 1;
constexpr uint32_t ON_DISCONNECTED = 2;
constexpr uint32_t ON_TRANSIENT_TASK_START = 3;
constexpr uint32_t ON_TRANSIENT_TASK_END = 4;
constexpr uint32_t ON_TRANSIENT_TASK_ERR = 15;
constexpr uint32_t ON_APP_TRANSIENT_TASK_START = 5;
constexpr uint32_t ON_APP_TRANSIENT_TASK_END = 6;
constexpr uint32_t ON_CONTINUOUS_TASK_START = 7;
constexpr uint32_t ON_CONTINUOUS_TASK_STOP = 8;
constexpr uint32_t ON_APP_CONTINUOUS_TASK_STOP = 9;
}
class BgTaskFrameworkUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
    inline void SleepForFC()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    }
};

class TestExpiredCallback : public ExpiredCallback {
public:
    void OnExpired() override {}
    void OnExpiredAuth(int32_t authResult) override {}
};

class TestBackgroundTaskSubscriber : public BackgroundTaskSubscriber {};

class TestBackgroundTaskSubscriberStub : public BackgroundTaskSubscriberStub {
    ErrCode OnConnected() override {return ERR_OK;}
    ErrCode OnDisconnected() override {return ERR_OK;}
    ErrCode OnTransientTaskStart(const TransientTaskAppInfo& info) override {return ERR_OK;}
    ErrCode OnAppTransientTaskStart(const TransientTaskAppInfo& info) override {return ERR_OK;}
    ErrCode OnAppTransientTaskEnd(const TransientTaskAppInfo& info) override {return ERR_OK;}
    ErrCode OnTransientTaskEnd(const TransientTaskAppInfo& info) override {return ERR_OK;}
    ErrCode OnTransientTaskErr(const TransientTaskAppInfo& info) override {return ERR_OK;}
    ErrCode OnContinuousTaskStart(
        const ContinuousTaskCallbackInfo &continuousTaskCallbackInfo) override {return ERR_OK;}
    ErrCode OnContinuousTaskUpdate(
        const ContinuousTaskCallbackInfo &continuousTaskCallbackInfo) override {return ERR_OK;}
    ErrCode OnContinuousTaskStop(
        const ContinuousTaskCallbackInfo &continuousTaskCallbackInfo) override {return ERR_OK;}
    ErrCode OnContinuousTaskSuspend(
        const ContinuousTaskCallbackInfo &continuousTaskCallbackInfo) override {return ERR_OK;}
    ErrCode OnContinuousTaskActive(
        const ContinuousTaskCallbackInfo &continuousTaskCallbackInfo) override {return ERR_OK;}
    ErrCode OnAppContinuousTaskStop(int32_t uid) override {return ERR_OK;}
    ErrCode OnAppEfficiencyResourcesApply(const ResourceCallbackInfo &resourceInfo) override {return ERR_OK;}
    ErrCode OnAppEfficiencyResourcesReset(const ResourceCallbackInfo &resourceInfo) override {return ERR_OK;}
    ErrCode OnProcEfficiencyResourcesApply(const ResourceCallbackInfo &resourceInfo) override {return ERR_OK;}
    ErrCode OnProcEfficiencyResourcesReset(const ResourceCallbackInfo &resourceInfo) override {return ERR_OK;}
    ErrCode GetFlag(int32_t &flag) override {return ERR_OK;}
};

class TestExpiredCallbackStub : public ExpiredCallbackStub {
public:
    ErrCode OnExpired() override {return ERR_OK;}
    ErrCode OnExpiredAuth(int32_t authResult) override {return ERR_OK;}
};

/**
 * @tc.name: BgTaskFrameworkUnitTest_001
 * @tc.desc: test RequestSuspendDelay.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_001, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    std::shared_ptr<DelaySuspendInfo> delayInfo = std::make_shared<DelaySuspendInfo>();
    auto expiredCallback = std::make_shared<TestExpiredCallback>();

    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestSuspendDelay(
        u"test", *expiredCallback, delayInfo), ERR_BGTASK_SERVICE_NOT_CONNECTED);
    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestSuspendDelay(
        u"test", *expiredCallback, delayInfo), ERR_OK);
    expiredCallback->Init();
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestSuspendDelay(
        u"test", *expiredCallback, delayInfo), ERR_OK);
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_002
 * @tc.desc: test GetRemainingDelayTime.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_002, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    int32_t delayTime = -1;
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetRemainingDelayTime(-1, delayTime),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);
    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetRemainingDelayTime(-1, delayTime), ERR_OK);
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_003
 * @tc.desc: test CancelSuspendDelay.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_003, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->CancelSuspendDelay(-1),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);
    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->CancelSuspendDelay(-1), ERR_OK);
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_004
 * @tc.desc: test RequestStartBackgroundRunning.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_004, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    ContinuousTaskParam taskParam = ContinuousTaskParam();
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestStartBackgroundRunning(taskParam),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);
    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestStartBackgroundRunning(taskParam),
        ERR_OK);
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_005
 * @tc.desc: test RequestStopBackgroundRunning.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V issueI99HSB
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_005, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestStopBackgroundRunning("test", nullptr, -1),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);
    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestStopBackgroundRunning("test", nullptr, -1),
        ERR_OK);
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_006
 * @tc.desc: test SubscribeBackgroundTask.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_006, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    TestBackgroundTaskSubscriber taskSubscriber = TestBackgroundTaskSubscriber();
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->SubscribeBackgroundTask(taskSubscriber),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->UnsubscribeBackgroundTask(taskSubscriber),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);
    SystemAbilityManagerClient::GetInstance().action_ = "";
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_007
 * @tc.desc: test GetTransientTaskApps.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_007, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    std::vector<std::shared_ptr<TransientTaskAppInfo>> list;
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetTransientTaskApps(list),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);
    SystemAbilityManagerClient::GetInstance().action_ = "";
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetTransientTaskApps(list);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_008
 * @tc.desc: test GetContinuousTaskApps.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_008, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;

    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> list;
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetContinuousTaskApps(list),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);
    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetContinuousTaskApps(list), ERR_OK);
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_009
 * @tc.desc: test ApplyEfficiencyResources.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_009, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    EfficiencyResourceInfo resourceInfo = EfficiencyResourceInfo();
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->ApplyEfficiencyResources(
        resourceInfo), ERR_BGTASK_SERVICE_NOT_CONNECTED);
    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->ApplyEfficiencyResources(
        resourceInfo), ERR_OK);
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_010
 * @tc.desc: test ResetAllEfficiencyResources.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_0010, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->ResetAllEfficiencyResources(),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);
    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->ResetAllEfficiencyResources(), ERR_OK);
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_011
 * @tc.desc: test GetEfficiencyResourcesInfos.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_011, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    std::vector<std::shared_ptr<ResourceCallbackInfo>> appList;
    std::vector<std::shared_ptr<ResourceCallbackInfo>> procList;
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetEfficiencyResourcesInfos(appList, procList),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);
    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetEfficiencyResourcesInfos(appList, procList),
        ERR_OK);
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_012
 * @tc.desc: test StopContinuousTask.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_012, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->StopContinuousTask(1, 1, 1, ""),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);

    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->StopContinuousTask(1, 1, 1, ""),
        ERR_BGTASK_PERMISSION_DENIED);
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_013
 * @tc.desc: test GetBackgroundTaskManagerProxy.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_013, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "";
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetBackgroundTaskManagerProxy();
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_, nullptr);
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->ResetBackgroundTaskManagerProxy();
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_, nullptr);
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_014
 * @tc.desc: test RequestBackgroundRunningForInner.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_014, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    ContinuousTaskParamForInner taskParam = ContinuousTaskParamForInner();
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestBackgroundRunningForInner(taskParam),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);
    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestBackgroundRunningForInner(taskParam),
        ERR_OK);
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_015
 * @tc.desc: test PauseTransientTaskTimeForInner.
 * @tc.type: FUNC
 * @tc.require: issueI936BL
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_015, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    int32_t uid = -1;
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->PauseTransientTaskTimeForInner(uid),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);
    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->PauseTransientTaskTimeForInner(uid),
        ERR_OK);
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_016
 * @tc.desc: test StartTransientTaskTimeForInner.
 * @tc.type: FUNC
 * @tc.require: issueI936BL
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_016, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    int32_t uid = -1;
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->StartTransientTaskTimeForInner(uid),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);
    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->StartTransientTaskTimeForInner(uid),
        ERR_OK);
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_017
 * @tc.desc: test GetAllEfficiencyResources.
 * @tc.type: FUNC
 * @tc.require: issuesIC3JY3
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_017, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    std::vector<std::shared_ptr<EfficiencyResourceInfo>> resourceInfoList;
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetAllEfficiencyResources(resourceInfoList),
        ERR_BGTASK_RESOURCES_SERVICE_NOT_CONNECTED);
    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetAllEfficiencyResources(resourceInfoList),
        ERR_OK);
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_018
 * @tc.desc: test SuspendContinuousTask.
 * @tc.type: FUNC
 * @tc.require: issueIC6B53
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_018, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->SuspendContinuousTask(1, 1, 4, ""),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);

    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->SuspendContinuousTask(1, 1, 4, ""),
        ERR_BGTASK_PERMISSION_DENIED);
}

/**
 * @tc.name: BgTaskFrameworkUnitTest_019
 * @tc.desc: test ActiveContinuousTask.
 * @tc.type: FUNC
 * @tc.require: issueIC6B53
 */
HWTEST_F(BgTaskFrameworkUnitTest, BgTaskFrameworkUnitTest_019, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->ActiveContinuousTask(1, 1, ""),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);

    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->ActiveContinuousTask(1, 1, ""),
        ERR_BGTASK_PERMISSION_DENIED);
}

/**
 * @tc.name: BackgroundTaskSubscriberProxyTest_001
 * @tc.desc: test BackgroundTaskSubscriberProxy.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BackgroundTaskSubscriberProxyTest_001, TestSize.Level1)
{
    sptr<TestBackgroundTaskSubscriberStub> subscirberStub
        = sptr<TestBackgroundTaskSubscriberStub>(new TestBackgroundTaskSubscriberStub());
    BackgroundTaskSubscriberProxy subscirberProxy1 = BackgroundTaskSubscriberProxy(nullptr);
    BackgroundTaskSubscriberProxy subscirberProxy2 = BackgroundTaskSubscriberProxy(subscirberStub->AsObject());
    subscirberProxy1.OnConnected();
    subscirberProxy2.OnConnected();
    subscirberProxy1.OnDisconnected();
    subscirberProxy2.OnDisconnected();
    EXPECT_NE(subscirberStub, nullptr);
}

/**
 * @tc.name: BackgroundTaskSubscriberProxyTest_002
 * @tc.desc: test BackgroundTaskSubscriberProxy.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BackgroundTaskSubscriberProxyTest_002, TestSize.Level1)
{
    sptr<TestBackgroundTaskSubscriberStub> subscirberStub
        = sptr<TestBackgroundTaskSubscriberStub>(new TestBackgroundTaskSubscriberStub());
    BackgroundTaskSubscriberProxy subscirberProxy1 = BackgroundTaskSubscriberProxy(nullptr);
    BackgroundTaskSubscriberProxy subscirberProxy2 = BackgroundTaskSubscriberProxy(subscirberStub->AsObject());
    std::shared_ptr<TransientTaskAppInfo> info = std::make_shared<TransientTaskAppInfo>();
    subscirberProxy2.OnTransientTaskStart(*info);
    subscirberProxy2.OnTransientTaskEnd(*info);
    subscirberProxy2.OnTransientTaskErr(*info);
    subscirberProxy2.OnAppTransientTaskStart(*info);
    subscirberProxy2.OnAppTransientTaskEnd(*info);
    EXPECT_NE(subscirberStub, nullptr);
}

/**
 * @tc.name: BackgroundTaskSubscriberProxyTest_003
 * @tc.desc: test BackgroundTaskSubscriberProxy.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BackgroundTaskSubscriberProxyTest_003, TestSize.Level1)
{
    sptr<TestBackgroundTaskSubscriberStub> subscirberStub
        = sptr<TestBackgroundTaskSubscriberStub>(new TestBackgroundTaskSubscriberStub());
    BackgroundTaskSubscriberProxy subscirberProxy1 = BackgroundTaskSubscriberProxy(nullptr);
    BackgroundTaskSubscriberProxy subscirberProxy2 = BackgroundTaskSubscriberProxy(subscirberStub->AsObject());
    subscirberProxy1.OnAppContinuousTaskStop(-1);
    subscirberProxy2.OnAppContinuousTaskStop(-1);
    std::shared_ptr<ContinuousTaskCallbackInfo> info = std::make_shared<ContinuousTaskCallbackInfo>();
    subscirberProxy2.OnContinuousTaskStart(*info);
    subscirberProxy2.OnContinuousTaskStop(*info);
    EXPECT_NE(subscirberStub, nullptr);
}

/**
 * @tc.name: BackgroundTaskSubscriberProxyTest_004
 * @tc.desc: test BackgroundTaskSubscriberProxy.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BackgroundTaskSubscriberProxyTest_004, TestSize.Level1)
{
    sptr<TestBackgroundTaskSubscriberStub> subscirberStub
        = sptr<TestBackgroundTaskSubscriberStub>(new TestBackgroundTaskSubscriberStub());
    BackgroundTaskSubscriberProxy subscirberProxy1 = BackgroundTaskSubscriberProxy(nullptr);
    BackgroundTaskSubscriberProxy subscirberProxy2 = BackgroundTaskSubscriberProxy(subscirberStub->AsObject());
    std::shared_ptr<ResourceCallbackInfo> info = std::make_shared<ResourceCallbackInfo>();
    subscirberProxy2.OnAppEfficiencyResourcesApply(*info);
    subscirberProxy2.OnAppEfficiencyResourcesReset(*info);
    subscirberProxy2.OnProcEfficiencyResourcesApply(*info);
    subscirberProxy2.OnProcEfficiencyResourcesReset(*info);
    EXPECT_NE(subscirberStub, nullptr);
}

/**
 * @tc.name: BackgroundTaskSubscriberProxyTest_005
 * @tc.desc: test BackgroundTaskSubscriberProxy.
 * @tc.type: FUNC
 * @tc.require: issueIC6B53
 */
HWTEST_F(BgTaskFrameworkUnitTest, BackgroundTaskSubscriberProxyTest_005, TestSize.Level1)
{
    sptr<TestBackgroundTaskSubscriberStub> subscirberStub
        = sptr<TestBackgroundTaskSubscriberStub>(new TestBackgroundTaskSubscriberStub());
    BackgroundTaskSubscriberProxy subscirberProxy1 = BackgroundTaskSubscriberProxy(nullptr);
    BackgroundTaskSubscriberProxy subscirberProxy2 = BackgroundTaskSubscriberProxy(subscirberStub->AsObject());
    std::shared_ptr<ContinuousTaskCallbackInfo> info = std::make_shared<ContinuousTaskCallbackInfo>();
    subscirberProxy2.OnContinuousTaskSuspend(*info);
    subscirberProxy2.OnContinuousTaskActive(*info);
    EXPECT_NE(subscirberStub, nullptr);
}

/**
 * @tc.name: BackgroundTaskSubscriberProxyTest_006
 * @tc.desc: test BackgroundTaskSubscriberProxy.
 * @tc.type: FUNC
 * @tc.require: issueIC6B53
 */
HWTEST_F(BgTaskFrameworkUnitTest, BackgroundTaskSubscriberProxyTest_006, TestSize.Level1)
{
    sptr<TestBackgroundTaskSubscriberStub> subscirberStub
        = sptr<TestBackgroundTaskSubscriberStub>(new TestBackgroundTaskSubscriberStub());
    BackgroundTaskSubscriberProxy subscirberProxy1 = BackgroundTaskSubscriberProxy(nullptr);
    BackgroundTaskSubscriberProxy subscirberProxy2 = BackgroundTaskSubscriberProxy(subscirberStub->AsObject());
    int32_t flag = 1;
    subscirberProxy1.GetFlag(flag);
    flag = 2;
    subscirberProxy2.GetFlag(flag);
    EXPECT_NE(subscirberStub, nullptr);
}

/**
 * @tc.name: BackgroundTaskSubscriberStubTest_001
 * @tc.desc: test BackgroundTaskSubscriberStub.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BackgroundTaskSubscriberStubTest_001, TestSize.Level1)
{
    TestBackgroundTaskSubscriberStub subscirberStub = TestBackgroundTaskSubscriberStub();
    MessageParcel data1;
    MessageParcel data2;
    MessageParcel reply;
    MessageOption option;
    std::u16string descriptor = u"test";
    data1.WriteInterfaceToken(descriptor);
    EXPECT_NE(subscirberStub.OnRemoteRequest(1, data1, reply, option), ERR_OK);
    data2.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    EXPECT_NE(subscirberStub.OnRemoteRequest(100, data2, reply, option), ERR_OK);
}

/**
 * @tc.name: BackgroundTaskSubscriberStubTest_002
 * @tc.desc: test BackgroundTaskSubscriberStub.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BackgroundTaskSubscriberStubTest_002, TestSize.Level1)
{
    TestBackgroundTaskSubscriberStub subscirberStub = TestBackgroundTaskSubscriberStub();
    MessageParcel reply;
    MessageOption option;
    MessageParcel data1;
    data1.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    EXPECT_EQ(subscirberStub.OnRemoteRequest(ON_CONNECTED, data1, reply, option), ERR_OK);
    MessageParcel data2;
    data2.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    EXPECT_NE(subscirberStub.OnRemoteRequest(ON_DISCONNECTED, data1, reply, option), ERR_OK);
}

/**
 * @tc.name: BackgroundTaskSubscriberStubTest_003
 * @tc.desc: test BackgroundTaskSubscriberStub.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BackgroundTaskSubscriberStubTest_003, TestSize.Level1)
{
    TestBackgroundTaskSubscriberStub subscirberStub = TestBackgroundTaskSubscriberStub();
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<TransientTaskAppInfo> info = std::make_shared<TransientTaskAppInfo>();
    MessageParcel data1;
    data1.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    EXPECT_NE(subscirberStub.OnRemoteRequest(ON_TRANSIENT_TASK_START, data1, reply, option), ERR_OK);
    MessageParcel data2;
    data2.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    data2.WriteParcelable(info.get());
    EXPECT_EQ(subscirberStub.OnRemoteRequest(ON_TRANSIENT_TASK_START, data2, reply, option), ERR_OK);

    MessageParcel data3;
    data3.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    EXPECT_NE(subscirberStub.OnRemoteRequest(ON_TRANSIENT_TASK_END, data3, reply, option), ERR_OK);
    MessageParcel data4;
    data4.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    data4.WriteParcelable(info.get());
    EXPECT_EQ(subscirberStub.OnRemoteRequest(ON_TRANSIENT_TASK_END, data4, reply, option), ERR_OK);

    MessageParcel data9;
    data9.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    EXPECT_NE(subscirberStub.OnRemoteRequest(ON_TRANSIENT_TASK_ERR, data9, reply, option), ERR_OK);
    MessageParcel data10;
    data10.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    data10.WriteParcelable(info.get());
    EXPECT_NE(subscirberStub.OnRemoteRequest(ON_TRANSIENT_TASK_ERR, data10, reply, option), ERR_OK);

    MessageParcel data5;
    data5.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    EXPECT_NE(subscirberStub.OnRemoteRequest(ON_APP_TRANSIENT_TASK_START, data5, reply, option), ERR_OK);
    MessageParcel data6;
    data6.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    data6.WriteParcelable(info.get());
    EXPECT_EQ(subscirberStub.OnRemoteRequest(ON_APP_TRANSIENT_TASK_START, data6, reply, option), ERR_OK);

    MessageParcel data7;
    data7.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    EXPECT_NE(subscirberStub.OnRemoteRequest(ON_APP_TRANSIENT_TASK_END, data7, reply, option), ERR_OK);
    MessageParcel data8;
    data8.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    data8.WriteParcelable(info.get());
    EXPECT_EQ(subscirberStub.OnRemoteRequest(ON_APP_TRANSIENT_TASK_END, data8, reply, option), ERR_OK);
}

/**
 * @tc.name: BackgroundTaskSubscriberStubTest_004
 * @tc.desc: test BackgroundTaskSubscriberStub.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BackgroundTaskSubscriberStubTest_004, TestSize.Level1)
{
    TestBackgroundTaskSubscriberStub subscirberStub = TestBackgroundTaskSubscriberStub();
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<ContinuousTaskCallbackInfo> info = std::make_shared<ContinuousTaskCallbackInfo>();
    MessageParcel data1;
    data1.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    EXPECT_NE(subscirberStub.OnRemoteRequest(ON_CONTINUOUS_TASK_START, data1, reply, option), ERR_OK);
    MessageParcel data2;
    data2.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    data2.WriteParcelable(info.get());
    EXPECT_EQ(subscirberStub.OnRemoteRequest(ON_CONTINUOUS_TASK_START, data2, reply, option), ERR_OK);

    MessageParcel data3;
    data3.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    EXPECT_NE(subscirberStub.OnRemoteRequest(ON_CONTINUOUS_TASK_STOP, data3, reply, option), ERR_OK);
    MessageParcel data4;
    data4.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    data4.WriteParcelable(info.get());
    EXPECT_EQ(subscirberStub.OnRemoteRequest(ON_CONTINUOUS_TASK_STOP, data4, reply, option), ERR_OK);

    MessageParcel data5;
    data5.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    data5.WriteInt32(-1);
    EXPECT_EQ(subscirberStub.OnRemoteRequest(ON_APP_CONTINUOUS_TASK_STOP, data5, reply, option),
        ERR_INVALID_DATA);
}

/**
 * @tc.name: BackgroundTaskSubscriberStubTest_005
 * @tc.desc: test BackgroundTaskSubscriberStub.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BackgroundTaskSubscriberStubTest_005, TestSize.Level1)
{
    TestBackgroundTaskSubscriberStub subscirberStub = TestBackgroundTaskSubscriberStub();
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<ResourceCallbackInfo> info = std::make_shared<ResourceCallbackInfo>();
    MessageParcel data1;
    data1.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    EXPECT_NE(subscirberStub.OnRemoteRequest(static_cast<uint32_t>(OHOS::BackgroundTaskMgr::
        IBackgroundTaskSubscriberIpcCode::COMMAND_ON_APP_EFFICIENCY_RESOURCES_APPLY), data1, reply, option), ERR_OK);
    MessageParcel data2;
    data2.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    data2.WriteParcelable(info.get());
    EXPECT_EQ(subscirberStub.OnRemoteRequest(static_cast<uint32_t>(OHOS::BackgroundTaskMgr::
        IBackgroundTaskSubscriberIpcCode::COMMAND_ON_APP_EFFICIENCY_RESOURCES_APPLY), data2, reply, option), ERR_OK);

    MessageParcel data3;
    data3.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    EXPECT_NE(subscirberStub.OnRemoteRequest(static_cast<uint32_t>(OHOS::BackgroundTaskMgr::
        IBackgroundTaskSubscriberIpcCode::COMMAND_ON_APP_EFFICIENCY_RESOURCES_RESET), data3, reply, option), ERR_OK);
    MessageParcel data4;
    data4.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    data4.WriteParcelable(info.get());
    EXPECT_EQ(subscirberStub.OnRemoteRequest(static_cast<uint32_t>(OHOS::BackgroundTaskMgr::
        IBackgroundTaskSubscriberIpcCode::COMMAND_ON_APP_EFFICIENCY_RESOURCES_RESET), data4, reply, option), ERR_OK);

    MessageParcel data5;
    data5.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    EXPECT_NE(subscirberStub.OnRemoteRequest(static_cast<uint32_t>(OHOS::BackgroundTaskMgr::
        IBackgroundTaskSubscriberIpcCode::COMMAND_ON_PROC_EFFICIENCY_RESOURCES_APPLY), data5, reply, option), ERR_OK);
    MessageParcel data6;
    data6.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    data6.WriteParcelable(info.get());

    MessageParcel data7;
    data7.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    EXPECT_NE(subscirberStub.OnRemoteRequest(static_cast<uint32_t>(OHOS::BackgroundTaskMgr::
        IBackgroundTaskSubscriberIpcCode::COMMAND_ON_PROC_EFFICIENCY_RESOURCES_RESET), data7, reply, option), ERR_OK);
    MessageParcel data8;
    data8.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    data8.WriteParcelable(info.get());
    EXPECT_EQ(subscirberStub.OnRemoteRequest(static_cast<uint32_t>(OHOS::BackgroundTaskMgr::
        IBackgroundTaskSubscriberIpcCode::COMMAND_ON_PROC_EFFICIENCY_RESOURCES_RESET), data8, reply, option), ERR_OK);
}

/**
 * @tc.name: BackgroundTaskSubscriberStubTest_006
 * @tc.desc: test BackgroundTaskSubscriberStub.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, BackgroundTaskSubscriberStubTest_006, TestSize.Level1)
{
    TestBackgroundTaskSubscriberStub subscirberStub = TestBackgroundTaskSubscriberStub();
    MessageParcel reply;
    MessageOption option;
    MessageParcel data1;
    data1.WriteInterfaceToken(TestBackgroundTaskSubscriberStub::GetDescriptor());
    EXPECT_EQ(subscirberStub.OnRemoteRequest(static_cast<uint32_t>(OHOS::BackgroundTaskMgr::
        IBackgroundTaskSubscriberIpcCode::COMMAND_GET_FLAG), data1, reply, option), ERR_OK);
}

/**
 * @tc.name: ExpiredCallbackProxyTest_001
 * @tc.desc: test ExpiredCallbackProxy.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, ExpiredCallbackProxyTest_001, TestSize.Level1)
{
    sptr<TestExpiredCallbackStub> expiredCallbackStub = sptr<TestExpiredCallbackStub>(new TestExpiredCallbackStub());
    ExpiredCallbackProxy proxy1 = ExpiredCallbackProxy(nullptr);
    ExpiredCallbackProxy proxy2 = ExpiredCallbackProxy(expiredCallbackStub->AsObject());
    proxy1.OnExpired();
    proxy1.OnExpiredAuth(1);
    proxy2.OnExpired();
    proxy2.OnExpiredAuth(1);
    EXPECT_NE(expiredCallbackStub, nullptr);
}

/**
 * @tc.name: ExpiredCallbackStubTest_001
 * @tc.desc: test ExpiredCallbackStub.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkUnitTest, ExpiredCallbackStubTest_001, TestSize.Level1)
{
    TestExpiredCallbackStub expiredCallbackStub = TestExpiredCallbackStub();
    MessageParcel reply;
    MessageOption option;
    std::u16string descriptor = u"test";
    MessageParcel data1;
    data1.WriteInterfaceToken(descriptor);
    EXPECT_NE(expiredCallbackStub.OnRemoteRequest(FIRST_CALL_TRANSACTION, data1, reply, option), ERR_OK);
    MessageParcel data2;
    data2.WriteInterfaceToken(TestExpiredCallbackStub::GetDescriptor());
    EXPECT_EQ(expiredCallbackStub.OnRemoteRequest(FIRST_CALL_TRANSACTION + 1, data2, reply, option), ERR_OK);
    MessageParcel data3;
    data3.WriteInterfaceToken(TestExpiredCallbackStub::GetDescriptor());
    EXPECT_EQ(expiredCallbackStub.OnRemoteRequest(FIRST_CALL_TRANSACTION, data3, reply, option), ERR_OK);
}

/**
 * @tc.name: SetBgTaskConfig_001
 * @tc.desc: test SetBgTaskConfig
 * @tc.type: FUNC
 * @tc.require: issueIAULHW
 */
HWTEST_F(BgTaskFrameworkUnitTest, SetBgTaskConfig_001, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->SetBgTaskConfig("", 1),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);
    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->SetBgTaskConfig("", 1),
        ERR_OK);
}

/**
 * @tc.name: RequestGetContinuousTasksByUidForInner_001
 * @tc.desc: test RequestGetContinuousTasksByUidForInner
 * @tc.type: FUNC
 * @tc.require: issueIBY0DN
 */
HWTEST_F(BgTaskFrameworkUnitTest, RequestGetContinuousTasksByUidForInner_001, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    std::vector<std::shared_ptr<ContinuousTaskInfo>> list;
    int32_t uid = -1;
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        RequestGetContinuousTasksByUidForInner(uid, list), ERR_BGTASK_SERVICE_NOT_CONNECTED);

    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        RequestGetContinuousTasksByUidForInner(uid, list), ERR_BGTASK_INVALID_UID);

    uid = 1;
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        RequestGetContinuousTasksByUidForInner(uid, list), ERR_OK);
}

/**
 * @tc.name: RequestGetAllContinuousTasks_001
 * @tc.desc: test RequestGetAllContinuousTasks
 * @tc.type: FUNC
 * @tc.require: issueIBY0DN
 */
HWTEST_F(BgTaskFrameworkUnitTest, RequestGetAllContinuousTasks_001, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    std::vector<std::shared_ptr<ContinuousTaskInfo>> list;
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestGetAllContinuousTasks(list),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);

    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestGetAllContinuousTasks(list),
        ERR_BGTASK_PERMISSION_DENIED);
}

/**
 * @tc.name: RequestGetAllContinuousTasks_002
 * @tc.desc: test RequestGetAllContinuousTasks
 * @tc.type: FUNC
 * @tc.require: issuesICRZHF
 */
HWTEST_F(BgTaskFrameworkUnitTest, RequestGetAllContinuousTasks_002, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    std::vector<std::shared_ptr<ContinuousTaskInfo>> list;
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestGetAllContinuousTasks(list, false),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);

    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestGetAllContinuousTasks(list, false),
        ERR_BGTASK_PERMISSION_DENIED);
}

/**
 * @tc.name: GetAllTransientTasks_001
 * @tc.desc: test GetAllTransientTasks.
 * @tc.type: FUNC
 * @tc.require: issueIC1HDY
 */
HWTEST_F(BgTaskFrameworkUnitTest, GetAllTransientTasks_001, TestSize.Level0)
{
    int32_t remainingQuota = -1;
    std::vector<std::shared_ptr<DelaySuspendInfo>> list;
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetAllTransientTasks(remainingQuota, list),
        ERR_BGTASK_TRANSIENT_SERVICE_NOT_CONNECTED);

    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetAllTransientTasks(remainingQuota, list),
        ERR_BGTASK_SERVICE_INNER_ERROR);
}

/**
 * @tc.name: AVSessionNotifyUpdateNotification_001
 * @tc.desc: test AVSessionNotifyUpdateNotification.
 * @tc.type: FUNC
 * @tc.require: issueIC9VN9
 */
HWTEST_F(BgTaskFrameworkUnitTest, AVSessionNotifyUpdateNotification_001, TestSize.Level1)
{
    int32_t uid = -1;
    int32_t pid = -1;
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        AVSessionNotifyUpdateNotification(uid, pid, true), ERR_BGTASK_SERVICE_NOT_CONNECTED);

    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        AVSessionNotifyUpdateNotification(uid, pid, true), ERR_BGTASK_CHECK_TASK_PARAM);
}

/**
 * @tc.name: SuspendContinuousAudioTask_001
 * @tc.desc: test SuspendContinuousAudioTask.
 * @tc.type: FUNC
 * @tc.require: issueICT1ZV
 */
HWTEST_F(BgTaskFrameworkUnitTest, SuspendContinuousAudioTask_001, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->SuspendContinuousAudioTask(1),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);
    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->SuspendContinuousAudioTask(1),
        ERR_OK);
}

/**
 * @tc.name: IsModeSupported_001
 * @tc.desc: test IsModeSupported.
 * @tc.type: FUNC
 * @tc.require: issueICWQV5
 */
HWTEST_F(BgTaskFrameworkUnitTest, IsModeSupported_001, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    ContinuousTaskParam taskParam = ContinuousTaskParam();
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->IsModeSupported(taskParam),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);

    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->IsModeSupported(taskParam),
        ERR_OK);
}

/**
 * @tc.name: SetSupportedTaskKeepingProcesses_001
 * @tc.desc: test SetSupportedTaskKeepingProcesses.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskFrameworkUnitTest, SetSupportedTaskKeepingProcesses_001, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    std::set<std::string> processSet;
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->SetSupportedTaskKeepingProcesses(processSet),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);

    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->SetSupportedTaskKeepingProcesses(processSet),
        ERR_OK);
}

/**
 * @tc.name: SetMaliciousAppConfig_001
 * @tc.desc: test SetMaliciousAppConfig.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskFrameworkUnitTest, SetMaliciousAppConfig_001, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    std::set<std::string> maliciousAppSet;
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->SetMaliciousAppConfig(maliciousAppSet),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);

    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->SetMaliciousAppConfig(maliciousAppSet),
        ERR_OK);
}

/**
 * @tc.name: RequestAuthFromUser_001
 * @tc.desc: test RequestAuthFromUser.
 * @tc.type: FUNC
 * @tc.require: 752
 */
HWTEST_F(BgTaskFrameworkUnitTest, RequestAuthFromUser_001, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    ContinuousTaskParam taskParam = ContinuousTaskParam();
    int32_t notificationId = 1;
    auto expiredCallback = std::make_shared<TestExpiredCallback>();
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestAuthFromUser(taskParam,
        *expiredCallback, notificationId), ERR_BGTASK_SERVICE_NOT_CONNECTED);

    expiredCallback->Init();
    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestAuthFromUser(taskParam,
        *expiredCallback, notificationId), ERR_OK);
}

/**
 * @tc.name: CheckSpecialScenarioAuth_001
 * @tc.desc: test CheckSpecialScenarioAuth.
 * @tc.type: FUNC
 * @tc.require: 752
 */
HWTEST_F(BgTaskFrameworkUnitTest, CheckSpecialScenarioAuth_001, TestSize.Level1)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->proxy_ = nullptr;
    SystemAbilityManagerClient::GetInstance().action_ = "set_null";
    uint32_t authResult = -1;
    EXPECT_EQ(DelayedSingleton<BackgroundTaskManager>::GetInstance()->CheckSpecialScenarioAuth(authResult),
        ERR_BGTASK_SERVICE_NOT_CONNECTED);

    SystemAbilityManagerClient::GetInstance().action_ = "";
    EXPECT_NE(DelayedSingleton<BackgroundTaskManager>::GetInstance()->CheckSpecialScenarioAuth(authResult),
        ERR_OK);
}
}
}
