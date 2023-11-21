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

#include <chrono>
#include <thread>
#include <iremote_broker.h>
#include "gtest/gtest.h"

#include "bgtaskmgr_inner_errors.h"
#include "background_mode.h"
#include "background_task_mgr_helper.h"
#include "background_task_subscriber.h"
#include "background_task_subscriber_stub.h"
#include "continuous_task_callback_info.h"
#include "continuous_task_param.h"
#include "delay_suspend_info.h"
#include "efficiency_resource_info.h"
#include "expired_callback.h"
#include "ibackground_task_mgr.h"
#include "resource_callback_info.h"
#include "resource_type.h"
#include "transient_task_app_info.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
constexpr int32_t DATA_TRANSFER_ID = 1;
constexpr int32_t AUDIO_PLAYBACK_ID = 2;
constexpr int32_t AUDIO_RECORDING_ID = 3;
constexpr int32_t LOCATION_ID = 4;
constexpr int32_t BLUETOOTH_INTERACTION_ID = 5;
constexpr int32_t MULTI_DEVICE_CONNECTION_ID = 6;
constexpr int32_t WIFI_INTERACTION_ID = 7;
constexpr int32_t VOIP_ID = 8;
constexpr int32_t TASK_KEEPING_ID = 9;
constexpr int32_t SLEEP_TIME = 500;
constexpr uint32_t CPU_TYPE = 1;
constexpr uint32_t COMMON_EVENT_TYPE = 2;
constexpr uint32_t TIMER_TYPE = 4;
constexpr uint32_t WORK_SCHEDULER_TYPE = 8;
constexpr uint32_t BLUETOOTH_TYPE = 16;
constexpr uint32_t GPS_TYPE = 32;
constexpr uint32_t AUDIO_TYPE = 64;
constexpr uint32_t RUNNING_LOCK = 128;
constexpr uint32_t SENSOR = 256;
}
class BgTaskClientUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    inline void SleepForFC()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    }

    std::shared_ptr<BackgroundTaskSubscriber> subscriber_ {nullptr};
    static std::string bgtaskSubscriberRet_;
    static std::string expiredCallbackRet_;
};

std::string BgTaskClientUnitTest::bgtaskSubscriberRet_ {""};
std::string BgTaskClientUnitTest::expiredCallbackRet_ {""};

void BgTaskClientUnitTest::SetUpTestCase() {}

void BgTaskClientUnitTest::TearDownTestCase() {}

void BgTaskClientUnitTest::SetUp() {}

void BgTaskClientUnitTest::TearDown() {}

class TestBackgroundTaskSubscriber : public BackgroundTaskSubscriber {
public:
    TestBackgroundTaskSubscriber() : BackgroundTaskSubscriber() {}

    void OnConnected() override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface1";
    }

    void OnDisconnected() override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface2";
    }

    void OnTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info) override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface3";
    }

    void OnTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface4";
    }

    void OnAppTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info) override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface5";
    }

    void OnAppTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface6";
    }

    void OnContinuousTaskStart(const std::shared_ptr<ContinuousTaskCallbackInfo>
        &continuousTaskCallbackInfo) override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface7";
    }

    void OnContinuousTaskStop(const std::shared_ptr<ContinuousTaskCallbackInfo>
        &continuousTaskCallbackInfo) override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface8";
    }

    void OnAppContinuousTaskStop(int32_t uid) override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface9";
    }

    void OnRemoteDied(const wptr<IRemoteObject> &object) override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface10";
    }

    void OnAppEfficiencyResourcesApply(const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface11";
    }

    void OnAppEfficiencyResourcesReset(const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface12";
    }

    void OnProcEfficiencyResourcesApply(const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface13";
    }

    void OnProcEfficiencyResourcesReset(const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface14";
    }
};

class TestExpiredCallback : public ExpiredCallback {
public:
    TestExpiredCallback() : ExpiredCallback() {}

    void OnExpired() override
    {
        BgTaskClientUnitTest::expiredCallbackRet_ = "OnExpired";
    }
};

/**
 * @tc.name: BackgroundMode_001
 * @tc.desc: test background mode constant.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, BackgroundMode_001, TestSize.Level1)
{
    EXPECT_EQ(DATA_TRANSFER_ID, (int32_t)BackgroundMode::DATA_TRANSFER);
    EXPECT_EQ(AUDIO_PLAYBACK_ID, (int32_t)BackgroundMode::AUDIO_PLAYBACK);
    EXPECT_EQ(AUDIO_RECORDING_ID, (int32_t)BackgroundMode::AUDIO_RECORDING);
    EXPECT_EQ(LOCATION_ID, (int32_t)BackgroundMode::LOCATION);
    EXPECT_EQ(BLUETOOTH_INTERACTION_ID, (int32_t)BackgroundMode::BLUETOOTH_INTERACTION);
    EXPECT_EQ(MULTI_DEVICE_CONNECTION_ID, (int32_t)BackgroundMode::MULTI_DEVICE_CONNECTION);
    EXPECT_EQ(WIFI_INTERACTION_ID, (int32_t)BackgroundMode::WIFI_INTERACTION);
    EXPECT_EQ(VOIP_ID, (int32_t)BackgroundMode::VOIP);
    EXPECT_EQ(TASK_KEEPING_ID, (int32_t)BackgroundMode::TASK_KEEPING);
}

/**
 * @tc.name: RequestStartBackgroundRunning_001
 * @tc.desc: test RequestStartBackgroundRunning interface.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, RequestStartBackgroundRunning_001, TestSize.Level1)
{
    ContinuousTaskParam taskParam = ContinuousTaskParam();
    EXPECT_NE(BackgroundTaskMgrHelper::RequestStartBackgroundRunning(taskParam), ERR_OK);
}

/**
 * @tc.name: RequestStopBackgroundRunning_001
 * @tc.desc: test RequestStopBackgroundRunning interface.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, RequestStopBackgroundRunning_001, TestSize.Level1)
{
    EXPECT_NE(BackgroundTaskMgrHelper::RequestStopBackgroundRunning("test", nullptr), ERR_OK);
}

/**
 * @tc.name: RequestBackgroundRunningForInner_001
 * @tc.desc: test RequestBackgroundRunningForInner interface.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, RequestBackgroundRunningForInner_001, TestSize.Level1)
{
    ContinuousTaskParamForInner taskParam = ContinuousTaskParamForInner();
    EXPECT_NE(BackgroundTaskMgrHelper::RequestBackgroundRunningForInner(taskParam), ERR_OK);
}

/**
 * @tc.name: SubscribeBackgroundTask_001
 * @tc.desc: test SubscribeBackgroundTask interface.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, SubscribeBackgroundTask_001, TestSize.Level1)
{
    subscriber_ = std::make_shared<TestBackgroundTaskSubscriber>();
    EXPECT_NE(subscriber_, nullptr);
    EXPECT_EQ(BackgroundTaskMgrHelper::SubscribeBackgroundTask(*subscriber_), ERR_OK);
    SleepForFC();
    EXPECT_EQ(BackgroundTaskMgrHelper::UnsubscribeBackgroundTask(*subscriber_), ERR_OK);
    SleepForFC();
}

/**
 * @tc.name: GetTransientTaskApps_001
 * @tc.desc: test GetTransientTaskApps interface.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, GetTransientTaskApps_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<TransientTaskAppInfo>> list;
    EXPECT_EQ(BackgroundTaskMgrHelper::GetTransientTaskApps(list), ERR_OK);
}

/**
 * @tc.name: GetContinuousTaskApps_001
 * @tc.desc: test GetContinuousTaskApps interface.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, GetContinuousTaskApps_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> list;
    EXPECT_EQ(BackgroundTaskMgrHelper::GetContinuousTaskApps(list), ERR_OK);
}

/**
 * @tc.name: GetEfficiencyResourcesInfos_001
 * @tc.desc: test GetEfficiencyResourcesInfos interface.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, GetEfficiencyResourcesInfos_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<ResourceCallbackInfo>> appList;
    std::vector<std::shared_ptr<ResourceCallbackInfo>> procList;
    EXPECT_EQ(BackgroundTaskMgrHelper::GetEfficiencyResourcesInfos(appList, procList), ERR_OK);
}

/**
 * @tc.name: ApplyEfficiencyResources_001
 * @tc.desc: test ApplyEfficiencyResources interface.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, ApplyEfficiencyResources_001, TestSize.Level1)
{
    EfficiencyResourceInfo resourceInfo = EfficiencyResourceInfo();
    EXPECT_NE(BackgroundTaskMgrHelper::ApplyEfficiencyResources(resourceInfo), ERR_OK);
}

/**
 * @tc.name: ResetAllEfficiencyResources_001
 * @tc.desc: test ResetAllEfficiencyResources interface.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, ResetAllEfficiencyResources_001, TestSize.Level1)
{
    EXPECT_NE((int32_t)BackgroundTaskMgrHelper::ResetAllEfficiencyResources(), (int32_t)ERR_OK);
}

/**
 * @tc.name: StopContinuousTask_001
 * @tc.desc: request stop target continuous task api test.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, StopContinuousTask_001, TestSize.Level1)
{
    EXPECT_EQ((int32_t)BackgroundTaskMgrHelper::StopContinuousTask(1, 1, 1), (int32_t)ERR_OK);
}

/**
 * @tc.name: BackgroundTaskSubscriber_001
 * @tc.desc: test BackgroundTaskSubscriber.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, BackgroundTaskSubscriber_001, TestSize.Level1)
{
    auto subscriber = TestBackgroundTaskSubscriber();
    auto subscriberImpl = std::make_shared<BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl>(subscriber);
    subscriberImpl->OnConnected();
    subscriberImpl->OnDisconnected();
    subscriberImpl->OnTransientTaskStart(nullptr);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface3");
    subscriberImpl->OnTransientTaskEnd(nullptr);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface4");
    subscriberImpl->OnAppTransientTaskStart(nullptr);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface5");
    subscriberImpl->OnAppTransientTaskEnd(nullptr);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface6");
    subscriberImpl->OnContinuousTaskStart(nullptr);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface7");
    subscriberImpl->OnContinuousTaskStop(nullptr);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface8");
    subscriberImpl->OnAppContinuousTaskStop(1);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface9");
    subscriberImpl->OnAppEfficiencyResourcesApply(nullptr);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface11");
    subscriberImpl->OnAppEfficiencyResourcesReset(nullptr);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface12");
    subscriberImpl->OnProcEfficiencyResourcesApply(nullptr);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface13");
    subscriberImpl->OnProcEfficiencyResourcesReset(nullptr);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface14");
}

/**
 * @tc.name: BackgroundTaskSubscriber_002
 * @tc.desc: test BackgroundTaskSubscriber.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, BackgroundTaskSubscriber_002, TestSize.Level1)
{
    auto subscriber = BackgroundTaskSubscriber();
    subscriber.OnConnected();
    subscriber.OnDisconnected();
    subscriber.OnTransientTaskStart(nullptr);
    subscriber.OnTransientTaskEnd(nullptr);
    subscriber.OnAppTransientTaskStart(nullptr);
    subscriber.OnAppTransientTaskEnd(nullptr);
    subscriber.OnContinuousTaskStart(nullptr);
    subscriber.OnContinuousTaskStop(nullptr);
    subscriber.OnAppContinuousTaskStop(1);
    subscriber.OnRemoteDied(nullptr);
    subscriber.OnAppEfficiencyResourcesApply(nullptr);
    subscriber.OnAppEfficiencyResourcesReset(nullptr);
    subscriber.OnProcEfficiencyResourcesApply(nullptr);
    subscriber.OnProcEfficiencyResourcesReset(nullptr);
    EXPECT_NE(subscriber.GetImpl(), nullptr);
}

/**
 * @tc.name: ContinuousTaskCallbackInfo_001
 * @tc.desc: test ContinuousTaskCallbackInfo.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, ContinuousTaskCallbackInfo_001, TestSize.Level1)
{
    sptr<ContinuousTaskCallbackInfo> info1 = sptr<ContinuousTaskCallbackInfo>(new ContinuousTaskCallbackInfo());
    sptr<ContinuousTaskCallbackInfo> info2 = sptr<ContinuousTaskCallbackInfo>(
        new ContinuousTaskCallbackInfo(1, 1, 1, "test"));

    Parcel parcel = Parcel();
    info2->Marshalling(parcel);
    sptr<ContinuousTaskCallbackInfo> info3 = sptr<ContinuousTaskCallbackInfo>(
        ContinuousTaskCallbackInfo::Unmarshalling(parcel));
    EXPECT_EQ(info3->GetTypeId(), 1);
    EXPECT_EQ(info3->GetCreatorUid(), 1);
    EXPECT_EQ(info3->GetCreatorPid(), 1);
    EXPECT_EQ(info3->GetAbilityName(), "test");
    EXPECT_EQ(info3->IsFromWebview(), false);
}

/**
 * @tc.name: ContinuousTaskParam_001
 * @tc.desc: test ContinuousTaskParam.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, ContinuousTaskParam_001, TestSize.Level1)
{
    sptr<ContinuousTaskParam> info1 = sptr<ContinuousTaskParam>(new ContinuousTaskParam());
    sptr<ContinuousTaskParam> info2 = sptr<ContinuousTaskParam>(
        new ContinuousTaskParam(true, 1, nullptr, "abilityName", nullptr, "appName"));

    Parcel parcel1 = Parcel();
    info2->Marshalling(parcel1);
    sptr<ContinuousTaskParam> info3 = sptr<ContinuousTaskParam>(new ContinuousTaskParam());
    info3->ReadFromParcel(parcel1);
    Parcel parcel2 = Parcel();
    info3->Marshalling(parcel2);
    sptr<ContinuousTaskParam> info4 = sptr<ContinuousTaskParam>(
        ContinuousTaskParam::Unmarshalling(parcel2));
    EXPECT_EQ(info4->isNewApi_, true);
    EXPECT_EQ(info4->bgModeId_, 1);
    EXPECT_EQ(info4->wantAgent_, nullptr);
    EXPECT_EQ(info4->abilityName_, "abilityName");
    EXPECT_EQ(info4->abilityToken_, nullptr);
    EXPECT_EQ(info4->appName_, "appName");
}

/**
 * @tc.name: ContinuousTaskParamForInner_001
 * @tc.desc: test ContinuousTaskParamForInner.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, ContinuousTaskParamForInner_001, TestSize.Level1)
{
    sptr<ContinuousTaskParamForInner> info1 = sptr<ContinuousTaskParamForInner>(new ContinuousTaskParamForInner());
    sptr<ContinuousTaskParamForInner> info2 = sptr<ContinuousTaskParamForInner>(
        new ContinuousTaskParamForInner(1, 1, true));

    Parcel parcel1 = Parcel();
    info2->Marshalling(parcel1);
    sptr<ContinuousTaskParamForInner> info3 = sptr<ContinuousTaskParamForInner>(new ContinuousTaskParamForInner());
    info3->ReadFromParcel(parcel1);
    Parcel parcel2 = Parcel();
    info3->Marshalling(parcel2);
    sptr<ContinuousTaskParamForInner> info4 = sptr<ContinuousTaskParamForInner>(
        ContinuousTaskParamForInner::Unmarshalling(parcel2));
    EXPECT_EQ(info4->uid_, 1);
    EXPECT_EQ(info4->bgModeId_, 1);
    EXPECT_EQ(info4->isStart_, true);
}

/**
 * @tc.name: DelaySuspendInfo_001
 * @tc.desc: test DelaySuspendInfo.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, DelaySuspendInfo_001, TestSize.Level1)
{
    sptr<DelaySuspendInfo> info1 = sptr<DelaySuspendInfo>(new DelaySuspendInfo());
    info1->SetRequestId(1);
    info1->SetActualDelayTime(1);

    Parcel parcel = Parcel();
    info1->Marshalling(parcel);

    auto info2 = DelaySuspendInfo::Unmarshalling(parcel);
    EXPECT_EQ(info2->GetRequestId(), 1);
    EXPECT_EQ(info2->GetActualDelayTime(), 1);
    EXPECT_EQ(info2->IsSameRequestId(1), true);
}

/**
 * @tc.name: EfficiencyResourceInfo_001
 * @tc.desc: test EfficiencyResourceInfo.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, EfficiencyResourceInfo_001, TestSize.Level1)
{
    sptr<EfficiencyResourceInfo> info1 = sptr<EfficiencyResourceInfo>(new EfficiencyResourceInfo());
    sptr<EfficiencyResourceInfo> info2 = sptr<EfficiencyResourceInfo>(
        new EfficiencyResourceInfo(0, true, 1, "test", true, false));
    info2->SetResourceNumber(1);
    info2->SetProcess(true);

    Parcel parcel = Parcel();
    info2->Marshalling(parcel);
    sptr<EfficiencyResourceInfo> info3 = EfficiencyResourceInfo::Unmarshalling(parcel);
    EXPECT_EQ(info2->GetResourceNumber(), 1);
    EXPECT_EQ(info2->IsApply(), true);
    EXPECT_EQ(info2->GetTimeOut(), 1);
    EXPECT_EQ(info2->GetReason(), "test");
    EXPECT_EQ(info2->IsPersist(), true);
    EXPECT_EQ(info2->IsProcess(), true);
}

/**
 * @tc.name: ExpiredCallback_001
 * @tc.desc: test ExpiredCallback.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, ExpiredCallback_001, TestSize.Level1)
{
    auto expiredCallback = std::make_shared<TestExpiredCallback>();
    expiredCallback->Init();
    auto expiredCallbackImpl = std::make_shared<ExpiredCallback::ExpiredCallbackImpl>(expiredCallback);
    expiredCallbackImpl->OnExpired();
    EXPECT_EQ(expiredCallbackRet_, "OnExpired");
}

/**
 * @tc.name: ResourceCallbackInfo_001
 * @tc.desc: test ResourceCallbackInfo.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, ResourceCallbackInfo_001, TestSize.Level1)
{
    sptr<ResourceCallbackInfo> info1 = sptr<ResourceCallbackInfo>(new ResourceCallbackInfo());
    sptr<ResourceCallbackInfo> info2 = sptr<ResourceCallbackInfo>(
        new ResourceCallbackInfo(1, 1, 1, "bundleName"));
    EXPECT_EQ(info2->GetResourceNumber(), 1);

    info2->SetResourceNumber(2);
    Parcel parcel = Parcel();
    info2->Marshalling(parcel);
    sptr<ResourceCallbackInfo> info3 = ResourceCallbackInfo::Unmarshalling(parcel);
    EXPECT_EQ(info3->GetUid(), 1);
    EXPECT_EQ(info3->GetPid(), 1);
    EXPECT_EQ(info3->GetResourceNumber(), 2);
    EXPECT_EQ(info3->GetBundleName(), "bundleName");
}

/**
 * @tc.name: ResourceType_001
 * @tc.desc: test ResourceType.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, ResourceType_001, TestSize.Level1)
{
    EXPECT_EQ(CPU_TYPE, (uint32_t)ResourceType::CPU);
    EXPECT_EQ(COMMON_EVENT_TYPE, (uint32_t)ResourceType::COMMON_EVENT);
    EXPECT_EQ(TIMER_TYPE, (uint32_t)ResourceType::TIMER);
    EXPECT_EQ(WORK_SCHEDULER_TYPE, (uint32_t)ResourceType::WORK_SCHEDULER);
    EXPECT_EQ(BLUETOOTH_TYPE, (uint32_t)ResourceType::BLUETOOTH);
    EXPECT_EQ(GPS_TYPE, (uint32_t)ResourceType::GPS);
    EXPECT_EQ(AUDIO_TYPE, (uint32_t)ResourceType::AUDIO);
    EXPECT_EQ(RUNNING_LOCK, (uint32_t)ResourceType::RUNNING_LOCK);
    EXPECT_EQ(SENSOR, (uint32_t)ResourceType::SENSOR);
}

/**
 * @tc.name: TransientTaskAppInfo_001
 * @tc.desc: test TransientTaskAppInfo.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, TransientTaskAppInfo_001, TestSize.Level1)
{
    std::shared_ptr<TransientTaskAppInfo> info1 = std::make_shared<TransientTaskAppInfo>();
    std::shared_ptr<TransientTaskAppInfo> info2 = std::make_shared<TransientTaskAppInfo>(
        "packageName", 1, 1);

    MessageParcel parcel = MessageParcel();
    info2->Marshalling(parcel);
    std::shared_ptr<TransientTaskAppInfo> info3 = TransientTaskAppInfo::Unmarshalling(parcel);
    EXPECT_EQ(info3->GetPackageName(), "packageName");
    EXPECT_EQ(info3->GetUid(), 1);
    EXPECT_EQ(info3->GetPid(), 1);
}
}
}