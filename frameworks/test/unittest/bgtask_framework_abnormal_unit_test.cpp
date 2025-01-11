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
#include <chrono>
#include <thread>
#include <message_parcel.h>

#include "gtest/gtest.h"

#include "background_task_mgr_proxy.h"

#include "background_task_manager.h"
#include "background_task_subscriber.h"
#include "background_task_subscriber_stub.h"
#include "background_task_subscriber_proxy.h"
#include "bgtaskmgr_inner_errors.h"
#include "bgtaskmgr_log_wrapper.h"
#include "continuous_task_callback_info.h"
#include "continuous_task_param.h"
#include "delay_suspend_info.h"
#include "efficiency_resource_info.h"
#include "expired_callback.h"
#include "expired_callback_proxy.h"
#include "expired_callback_stub.h"
#include "mock_background_task_mgr_proxy_helper.h"
#include "mock_message_parcel_helper.h"
#include "iservice_registry.h"
#include "resource_callback_info.h"
#include "singleton.h"
#include "transient_task_app_info.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {
class BgTaskFrameworkAbnormalUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

class TestExpiredCallback : public ExpiredCallback {
public:
    void OnExpired() override {}
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
    ErrCode OnAppContinuousTaskStop(int32_t uid) override {return ERR_OK;}
    ErrCode OnAppEfficiencyResourcesApply(const ResourceCallbackInfo &resourceInfo) override {return ERR_OK;}
    ErrCode OnAppEfficiencyResourcesReset(const ResourceCallbackInfo &resourceInfo) override {return ERR_OK;}
    ErrCode OnProcEfficiencyResourcesApply(const ResourceCallbackInfo &resourceInfo) override {return ERR_OK;}
    ErrCode OnProcEfficiencyResourcesReset(const ResourceCallbackInfo &resourceInfo) override {return ERR_OK;}
};

class TestExpiredCallbackStub : public ExpiredCallbackStub {
public:
    ErrCode OnExpired() override {return ERR_OK;}
};

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_001
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_001, TestSize.Level1)
{
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);
    DelaySuspendInfo delayInfo;
    sptr<TestExpiredCallbackStub> expiredCallbackStub = sptr<TestExpiredCallbackStub>(new TestExpiredCallbackStub());

    EXPECT_EQ(backgroundTaskMgrProxy.RequestSuspendDelay("reason", nullptr, delayInfo),
        ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestSuspendDelay("reason", expiredCallbackStub, delayInfo),
        ERR_INVALID_VALUE);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteString16Flag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestSuspendDelay("reason", expiredCallbackStub, delayInfo),
        ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteString16Flag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteRemoteObjectFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestSuspendDelay("reason", expiredCallbackStub, delayInfo),
        ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteRemoteObjectFlag(true);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestSuspendDelay("reason", expiredCallbackStub, delayInfo),
        ERR_INVALID_DATA);
    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestSuspendDelay("reason", expiredCallbackStub, delayInfo),
        ERR_INVALID_DATA);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(true);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestSuspendDelay("reason", expiredCallbackStub, delayInfo),
        ERR_INVALID_DATA);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_002
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_002, TestSize.Level1)
{
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.CancelSuspendDelay(1), ERR_INVALID_VALUE);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.CancelSuspendDelay(1), ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInt32WithParamFlag(true);
    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.CancelSuspendDelay(1), ERR_INVALID_DATA);

    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.CancelSuspendDelay(1), ERR_INVALID_DATA);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_003
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_003, TestSize.Level1)
{
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);
    int32_t delayTime;

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetRemainingDelayTime(1, delayTime), ERR_INVALID_VALUE);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetRemainingDelayTime(1, delayTime), ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInt32WithParamFlag(true);
    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.GetRemainingDelayTime(1, delayTime), ERR_INVALID_DATA);

    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetRemainingDelayTime(1, delayTime), ERR_INVALID_DATA);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_004
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_004, TestSize.Level1)
{
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);
    int32_t notificationId = -1;
    int32_t continuousTaskId = -1;
    EXPECT_EQ(backgroundTaskMgrProxy.StartBackgroundRunning(
        *new ContinuousTaskParam(), notificationId, continuousTaskId), ERR_INVALID_DATA);

    sptr<ContinuousTaskParam> taskParam(new ContinuousTaskParam());

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StartBackgroundRunning(
        *taskParam, notificationId, continuousTaskId), ERR_INVALID_VALUE);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteParcelableFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StartBackgroundRunning(
        *taskParam, notificationId, continuousTaskId), ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteParcelableFlag(true);
    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.StartBackgroundRunning(
        *taskParam, notificationId, continuousTaskId), ERR_INVALID_DATA);

    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StartBackgroundRunning(
        *taskParam, notificationId, continuousTaskId), ERR_INVALID_DATA);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_005
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V issueI99HSB
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_005, TestSize.Level1)
{
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StopBackgroundRunning("abilityName", nullptr, -1), ERR_INVALID_VALUE);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteStringFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StopBackgroundRunning("abilityName", nullptr, -1), ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteStringFlag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteRemoteObjectFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StopBackgroundRunning("abilityName", nullptr, -1), ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteRemoteObjectFlag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StopBackgroundRunning("abilityName", nullptr, -1), ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInt32WithParamFlag(true);
    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.StopBackgroundRunning("abilityName", nullptr, -1), ERR_INVALID_DATA);

    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StopBackgroundRunning("abilityName", nullptr, -1), ERR_INVALID_DATA);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_006
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_006, TestSize.Level1)
{
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);
    EXPECT_EQ(backgroundTaskMgrProxy.SubscribeBackgroundTask(nullptr), ERR_INVALID_DATA);

    sptr<TestBackgroundTaskSubscriberStub> subscribe =
        sptr<TestBackgroundTaskSubscriberStub>(new TestBackgroundTaskSubscriberStub());

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.SubscribeBackgroundTask(subscribe), ERR_INVALID_VALUE);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteRemoteObjectFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.SubscribeBackgroundTask(subscribe), ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteRemoteObjectFlag(true);
    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.SubscribeBackgroundTask(subscribe), ERR_INVALID_DATA);

    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.SubscribeBackgroundTask(subscribe), ERR_INVALID_DATA);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_007
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_007, TestSize.Level1)
{
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);
    EXPECT_EQ(backgroundTaskMgrProxy.UnsubscribeBackgroundTask(nullptr), ERR_INVALID_DATA);

    sptr<TestBackgroundTaskSubscriberStub> subscribe =
        sptr<TestBackgroundTaskSubscriberStub>(new TestBackgroundTaskSubscriberStub());

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.UnsubscribeBackgroundTask(subscribe), ERR_INVALID_VALUE);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteRemoteObjectFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.UnsubscribeBackgroundTask(subscribe), ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteRemoteObjectFlag(true);
    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.UnsubscribeBackgroundTask(subscribe), ERR_INVALID_DATA);

    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.UnsubscribeBackgroundTask(subscribe), ERR_INVALID_DATA);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_008
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_008, TestSize.Level1)
{
    std::vector<TransientTaskAppInfo> list;
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetTransientTaskApps(list), ERR_INVALID_VALUE);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.GetTransientTaskApps(list), ERR_INVALID_DATA);

    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetTransientTaskApps(list), ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(true);
    EXPECT_EQ(backgroundTaskMgrProxy.GetTransientTaskApps(list), ERR_INVALID_DATA);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_009
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_009, TestSize.Level1)
{
    std::vector<ContinuousTaskCallbackInfo> list;
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetContinuousTaskApps(list), ERR_INVALID_VALUE);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.GetContinuousTaskApps(list), ERR_INVALID_DATA);

    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetContinuousTaskApps(list), ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(true);
    EXPECT_EQ(backgroundTaskMgrProxy.GetContinuousTaskApps(list), ERR_INVALID_DATA);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_010
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_010, TestSize.Level1)
{
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StopContinuousTask(1, 1, 1, ""), ERR_INVALID_VALUE);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StopContinuousTask(1, 1, 1, ""), ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInt32WithParamFlag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteUint32Flag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StopContinuousTask(1, 1, 1, ""), ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteUint32Flag(true);
    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.StopContinuousTask(1, 1, 1, ""), ERR_INVALID_DATA);

    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StopContinuousTask(1, 1, 1, ""), ERR_INVALID_DATA);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_011
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_011, TestSize.Level1)
{
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);
    EXPECT_EQ(backgroundTaskMgrProxy.ApplyEfficiencyResources(*new EfficiencyResourceInfo()), ERR_INVALID_DATA);

    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo();

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.ApplyEfficiencyResources(*resourceInfo), ERR_INVALID_VALUE);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteParcelableFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.ApplyEfficiencyResources(*resourceInfo), ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteParcelableFlag(true);
    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.ApplyEfficiencyResources(*resourceInfo), ERR_INVALID_DATA);

    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.ApplyEfficiencyResources(*resourceInfo), ERR_INVALID_DATA);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_012
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_012, TestSize.Level1)
{
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.ResetAllEfficiencyResources(), ERR_INVALID_VALUE);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.ResetAllEfficiencyResources(), ERR_INVALID_DATA);

    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.ResetAllEfficiencyResources(), ERR_INVALID_DATA);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_013
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_013, TestSize.Level1)
{
    std::vector<ResourceCallbackInfo> appList;
    std::vector<ResourceCallbackInfo> procList;
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetEfficiencyResourcesInfos(appList, procList), ERR_INVALID_VALUE);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.GetEfficiencyResourcesInfos(appList, procList), ERR_INVALID_DATA);

    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetEfficiencyResourcesInfos(appList, procList), ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(true);
    EXPECT_EQ(backgroundTaskMgrProxy.GetEfficiencyResourcesInfos(appList, procList), ERR_INVALID_DATA);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_014
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_014, TestSize.Level1)
{
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestBackgroundRunningForInner(
        *new ContinuousTaskParamForInner()), ERR_INVALID_DATA);

    sptr<ContinuousTaskParamForInner> taskParam = sptr<ContinuousTaskParamForInner>(new ContinuousTaskParamForInner());

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestBackgroundRunningForInner(*taskParam), ERR_INVALID_VALUE);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteParcelableFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestBackgroundRunningForInner(*taskParam), ERR_INVALID_DATA);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteParcelableFlag(true);
    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestBackgroundRunningForInner(*taskParam), ERR_INVALID_DATA);

    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestBackgroundRunningForInner(*taskParam), ERR_INVALID_DATA);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_015
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI936BL
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_015, TestSize.Level1)
{
    int32_t uid = -1;
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);
    
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_NE(backgroundTaskMgrProxy.PauseTransientTaskTimeForInner(uid), 0);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInt32WithParamFlag(false);
    EXPECT_NE(backgroundTaskMgrProxy.PauseTransientTaskTimeForInner(uid), 0);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInt32WithParamFlag(false);
    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_NE(backgroundTaskMgrProxy.PauseTransientTaskTimeForInner(uid), 0);

    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_NE(backgroundTaskMgrProxy.PauseTransientTaskTimeForInner(uid), 0);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_016
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI936BL
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_016, TestSize.Level1)
{
    int32_t uid = -1;
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);
    
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_NE(backgroundTaskMgrProxy.StartTransientTaskTimeForInner(uid), 0);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInt32WithParamFlag(false);
    EXPECT_NE(backgroundTaskMgrProxy.StartTransientTaskTimeForInner(uid), 0);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInt32WithParamFlag(true);
    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_NE(backgroundTaskMgrProxy.StartTransientTaskTimeForInner(uid), 0);

    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_NE(backgroundTaskMgrProxy.StartTransientTaskTimeForInner(uid), 0);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_017
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issueIAULHW
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_017, TestSize.Level1)
{
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);
    
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_NE(backgroundTaskMgrProxy.SetBgTaskConfig("", 1), 0);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteStringFlag(false);
    EXPECT_NE(backgroundTaskMgrProxy.SetBgTaskConfig("", 1), 0);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteStringFlag(true);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInt32WithParamFlag(false);
    EXPECT_NE(backgroundTaskMgrProxy.SetBgTaskConfig("", 1), 0);

    MessageParcelHelper::BgTaskFwkAbnormalSetWriteInt32WithParamFlag(true);
    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_NE(backgroundTaskMgrProxy.SetBgTaskConfig("", 1), 0);

    BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    MessageParcelHelper::BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_NE(backgroundTaskMgrProxy.SetBgTaskConfig("", 1), 0);
}
}
}
