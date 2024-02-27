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
#include "iservice_registry.h"
#include "resource_callback_info.h"
#include "singleton.h"
#include "transient_task_app_info.h"

using namespace testing::ext;

namespace OHOS {
extern void BgTaskFwkAbnormalSetWriteRemoteObjectFlag(bool flag);
extern void BgTaskFwkAbnormalSetWriteReadRemoteObjectFlag(bool flag);
extern void BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(bool flag);
extern void BgTaskFwkAbnormalSetReadInterfaceTokenFlag(bool flag);
extern void BgTaskFwkAbnormalSetWriteString16Flag(bool flag);
extern void BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(bool flag);
extern void BgTaskFwkAbnormalSetWriteInt32WithParamFlag(bool flag);
extern void BgTaskFwkAbnormalSetWriteParcelableFlag(bool flag);
extern void BgTaskFwkAbnormalSetWriteStringFlag(bool flag);
extern void BgTaskFwkAbnormalSetWriteUint32Flag(bool flag);
extern void BgTaskFwkAbnormalSetReadInt32Flag(bool flag);

namespace BackgroundTaskMgr {
extern void BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(int32_t flag);

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
    void OnConnected() override {}
    void OnDisconnected() override {}
    void OnTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info) override {}
    void OnAppTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info) override {}
    void OnAppTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) override {}
    void OnTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) override {}
    void OnContinuousTaskStart(
        const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) override {}
    void OnContinuousTaskUpdate(
        const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) override {}
    void OnContinuousTaskStop(
        const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) override {}
    void OnAppContinuousTaskStop(int32_t uid) override {}
    void OnAppEfficiencyResourcesApply(const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) override {}
    void OnAppEfficiencyResourcesReset(const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) override {}
    void OnProcEfficiencyResourcesApply(const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) override {}
    void OnProcEfficiencyResourcesReset(const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) override {}
};

class TestExpiredCallbackStub : public ExpiredCallbackStub {
public:
    void OnExpired() override {}
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
    std::shared_ptr<DelaySuspendInfo> delayInfo = std::make_shared<DelaySuspendInfo>();
    sptr<TestExpiredCallbackStub> expiredCallbackStub = sptr<TestExpiredCallbackStub>(new TestExpiredCallbackStub());

    EXPECT_EQ(backgroundTaskMgrProxy.RequestSuspendDelay(u"reason", nullptr, delayInfo),
        ERR_CALLBACK_NULL_OR_TYPE_ERR);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestSuspendDelay(u"reason", expiredCallbackStub, delayInfo),
        ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskFwkAbnormalSetWriteString16Flag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestSuspendDelay(u"reason", expiredCallbackStub, delayInfo),
        ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteString16Flag(true);
    BgTaskFwkAbnormalSetWriteRemoteObjectFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestSuspendDelay(u"reason", expiredCallbackStub, delayInfo),
        ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteRemoteObjectFlag(true);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestSuspendDelay(u"reason", expiredCallbackStub, delayInfo),
        ERR_BGTASK_TRANSACT_FAILED);
    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestSuspendDelay(u"reason", expiredCallbackStub, delayInfo),
        ERR_BGTASK_TRANSACT_FAILED);
    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(true);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestSuspendDelay(u"reason", expiredCallbackStub, delayInfo),
        ERR_BGTASK_TRANSACT_FAILED);
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

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.CancelSuspendDelay(1), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskFwkAbnormalSetWriteInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.CancelSuspendDelay(1), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteInt32WithParamFlag(true);
    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.CancelSuspendDelay(1), ERR_BGTASK_TRANSACT_FAILED);

    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.CancelSuspendDelay(1), ERR_BGTASK_TRANSACT_FAILED);
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

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetRemainingDelayTime(1, delayTime), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskFwkAbnormalSetWriteInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetRemainingDelayTime(1, delayTime), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteInt32WithParamFlag(true);
    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.GetRemainingDelayTime(1, delayTime), ERR_BGTASK_TRANSACT_FAILED);

    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetRemainingDelayTime(1, delayTime), ERR_BGTASK_TRANSACT_FAILED);
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
    EXPECT_EQ(backgroundTaskMgrProxy.StartBackgroundRunning(nullptr), ERR_BGTASK_INVALID_PARAM);

    sptr<ContinuousTaskParam> taskParam = sptr<ContinuousTaskParam>(new ContinuousTaskParam());

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StartBackgroundRunning(taskParam), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskFwkAbnormalSetWriteParcelableFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StartBackgroundRunning(taskParam), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteParcelableFlag(true);
    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.StartBackgroundRunning(taskParam), ERR_BGTASK_TRANSACT_FAILED);

    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StartBackgroundRunning(taskParam), ERR_BGTASK_TRANSACT_FAILED);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_005
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_005, TestSize.Level1)
{
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StopBackgroundRunning("abilityName", nullptr), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskFwkAbnormalSetWriteStringFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StopBackgroundRunning("abilityName", nullptr), ERR_BGTASK_TRANSACT_FAILED);

    BgTaskFwkAbnormalSetWriteStringFlag(true);
    BgTaskFwkAbnormalSetWriteRemoteObjectFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StopBackgroundRunning("abilityName", nullptr), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteRemoteObjectFlag(true);
    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.StopBackgroundRunning("abilityName", nullptr), ERR_BGTASK_TRANSACT_FAILED);

    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StopBackgroundRunning("abilityName", nullptr), ERR_BGTASK_TRANSACT_FAILED);
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
    EXPECT_EQ(backgroundTaskMgrProxy.SubscribeBackgroundTask(nullptr), ERR_BGTASK_PARCELABLE_FAILED);

    sptr<TestBackgroundTaskSubscriberStub> subscribe =
        sptr<TestBackgroundTaskSubscriberStub>(new TestBackgroundTaskSubscriberStub());

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.SubscribeBackgroundTask(subscribe), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskFwkAbnormalSetWriteRemoteObjectFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.SubscribeBackgroundTask(subscribe), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteRemoteObjectFlag(true);
    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.SubscribeBackgroundTask(subscribe), ERR_BGTASK_TRANSACT_FAILED);

    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.SubscribeBackgroundTask(subscribe), ERR_BGTASK_TRANSACT_FAILED);
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
    EXPECT_EQ(backgroundTaskMgrProxy.UnsubscribeBackgroundTask(nullptr), ERR_BGTASK_PARCELABLE_FAILED);

    sptr<TestBackgroundTaskSubscriberStub> subscribe =
        sptr<TestBackgroundTaskSubscriberStub>(new TestBackgroundTaskSubscriberStub());

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.UnsubscribeBackgroundTask(subscribe), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskFwkAbnormalSetWriteRemoteObjectFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.UnsubscribeBackgroundTask(subscribe), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteRemoteObjectFlag(true);
    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.UnsubscribeBackgroundTask(subscribe), ERR_BGTASK_TRANSACT_FAILED);

    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.UnsubscribeBackgroundTask(subscribe), ERR_BGTASK_TRANSACT_FAILED);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_008
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_008, TestSize.Level1)
{
    std::vector<std::shared_ptr<TransientTaskAppInfo>> list;
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetTransientTaskApps(list), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.GetTransientTaskApps(list), ERR_BGTASK_TRANSACT_FAILED);

    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetTransientTaskApps(list), ERR_BGTASK_TRANSACT_FAILED);

    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(true);
    EXPECT_EQ(backgroundTaskMgrProxy.GetTransientTaskApps(list), ERR_BGTASK_TRANSACT_FAILED);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_009
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_009, TestSize.Level1)
{
    std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> list;
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetContinuousTaskApps(list), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.GetContinuousTaskApps(list), ERR_BGTASK_TRANSACT_FAILED);

    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetContinuousTaskApps(list), ERR_BGTASK_TRANSACT_FAILED);

    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(true);
    EXPECT_EQ(backgroundTaskMgrProxy.GetContinuousTaskApps(list), ERR_BGTASK_TRANSACT_FAILED);
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

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StopContinuousTask(1, 1, 1), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskFwkAbnormalSetWriteInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StopContinuousTask(1, 1, 1), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteInt32WithParamFlag(true);
    BgTaskFwkAbnormalSetWriteUint32Flag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StopContinuousTask(1, 1, 1), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteUint32Flag(true);
    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.StopContinuousTask(1, 1, 1), ERR_BGTASK_TRANSACT_FAILED);

    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.StopContinuousTask(1, 1, 1), ERR_BGTASK_TRANSACT_FAILED);
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
    EXPECT_EQ(backgroundTaskMgrProxy.ApplyEfficiencyResources(nullptr), ERR_BGTASK_INVALID_PARAM);

    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo();

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.ApplyEfficiencyResources(resourceInfo), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskFwkAbnormalSetWriteParcelableFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.ApplyEfficiencyResources(resourceInfo), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteParcelableFlag(true);
    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.ApplyEfficiencyResources(resourceInfo), ERR_BGTASK_TRANSACT_FAILED);

    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.ApplyEfficiencyResources(resourceInfo), ERR_BGTASK_TRANSACT_FAILED);
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

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.ResetAllEfficiencyResources(), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.ResetAllEfficiencyResources(), ERR_BGTASK_TRANSACT_FAILED);

    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.ResetAllEfficiencyResources(), ERR_BGTASK_TRANSACT_FAILED);
}

/**
 * @tc.name: BackgroundTaskMgrProxyAbnormalTest_013
 * @tc.desc: test BackgroundTaskMgrProxy abnormal.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskFrameworkAbnormalUnitTest, BackgroundTaskMgrProxyAbnormalTest_013, TestSize.Level1)
{
    std::vector<std::shared_ptr<ResourceCallbackInfo>> appList;
    std::vector<std::shared_ptr<ResourceCallbackInfo>> procList;
    BackgroundTaskMgrProxy backgroundTaskMgrProxy = BackgroundTaskMgrProxy(nullptr);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetEfficiencyResourcesInfos(appList, procList), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.GetEfficiencyResourcesInfos(appList, procList), ERR_BGTASK_TRANSACT_FAILED);

    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.GetEfficiencyResourcesInfos(appList, procList), ERR_BGTASK_TRANSACT_FAILED);

    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(true);
    EXPECT_EQ(backgroundTaskMgrProxy.GetEfficiencyResourcesInfos(appList, procList), ERR_BGTASK_TRANSACT_FAILED);
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
    EXPECT_EQ(backgroundTaskMgrProxy.RequestBackgroundRunningForInner(nullptr), ERR_BGTASK_INVALID_PARAM);

    sptr<ContinuousTaskParamForInner> taskParam = sptr<ContinuousTaskParamForInner>(new ContinuousTaskParamForInner());

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestBackgroundRunningForInner(taskParam), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(true);
    BgTaskFwkAbnormalSetWriteParcelableFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestBackgroundRunningForInner(taskParam), ERR_BGTASK_PARCELABLE_FAILED);

    BgTaskFwkAbnormalSetWriteParcelableFlag(true);
    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(0);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestBackgroundRunningForInner(taskParam), ERR_BGTASK_TRANSACT_FAILED);

    BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(1);
    BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(false);
    EXPECT_EQ(backgroundTaskMgrProxy.RequestBackgroundRunningForInner(taskParam), ERR_BGTASK_TRANSACT_FAILED);
}
}
}
