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

#include <chrono>
#include <thread>
#include <iremote_broker.h>
#include "gtest/gtest.h"

#include "accesstoken_kit.h"
#include "bgtaskmgr_inner_errors.h"
#include "background_mode.h"
#include "background_sub_mode.h"
#include "background_task_mgr_helper.h"
#include "background_task_subscriber.h"
#include "background_task_subscriber_stub.h"
#include "continuous_task_callback_info.h"
#include "continuous_task_cancel_reason.h"
#include "background_task_mode.h"
#include "background_task_state_info.h"
#include "continuous_task_param.h"
#include "continuous_task_request.h"
#include "background_task_submode.h"
#include "continuous_task_suspend_reason.h"
#include "delay_suspend_info.h"
#include "efficiency_resource_info.h"
#include "expired_callback.h"
#include "ibackground_task_mgr.h"
#include "resource_callback_info.h"
#include "resource_type.h"
#include "token_setproc.h"
#include "transient_task_app_info.h"
#include "user_auth_result.h"

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
constexpr int32_t WORKOUT = 10;
constexpr int32_t SPECIAL_SCENARIO_PROCESSING = 13;
constexpr int32_t END = 14;
constexpr int32_t SLEEP_TIME = 500;
constexpr int32_t RSS_UID = 1096;
constexpr uint32_t CPU_TYPE = 1;
constexpr uint32_t COMMON_EVENT_TYPE = 2;
constexpr uint32_t TIMER_TYPE = 4;
constexpr uint32_t WORK_SCHEDULER_TYPE = 8;
constexpr uint32_t BLUETOOTH_TYPE = 16;
constexpr uint32_t GPS_TYPE = 32;
constexpr uint32_t AUDIO_TYPE = 64;
constexpr uint32_t RUNNING_LOCK = 128;
constexpr uint32_t SENSOR = 256;
constexpr uint32_t USER_CANCEL = 1;
constexpr uint32_t SYSTEM_CANCEL = 2;
constexpr uint32_t USER_CANCEL_REMOVE_NOTIFICATION = 3;
constexpr uint32_t SYSTEM_CANCEL_DATA_TRANSFER_LOW_SPEED = 4;
constexpr uint32_t SYSTEM_CANCEL_AUDIO_PLAYBACK_NOT_USE_AVSESSION = 5;
constexpr uint32_t SYSTEM_CANCEL_AUDIO_PLAYBACK_NOT_RUNNING = 6;
constexpr uint32_t SYSTEM_CANCEL_AUDIO_RECORDING_NOT_RUNNING = 7;
constexpr uint32_t SYSTEM_CANCEL_NOT_USE_LOCATION = 8;
constexpr uint32_t SYSTEM_CANCEL_NOT_USE_BLUETOOTH = 9;
constexpr uint32_t SYSTEM_CANCEL_NOT_USE_MULTI_DEVICE = 10;
constexpr uint32_t SYSTEM_CANCEL_USE_ILLEGALLY = 11;
constexpr uint32_t CAR_KEY = 1;
constexpr uint32_t SYSTEM_SUSPEND_DATA_TRANSFER_LOW_SPEED = 4;
constexpr uint32_t SYSTEM_SUSPEND_AUDIO_PLAYBACK_NOT_USE_AVSESSION = 5;
constexpr uint32_t SYSTEM_SUSPEND_AUDIO_PLAYBACK_NOT_RUNNING = 6;
constexpr uint32_t SYSTEM_SUSPEND_AUDIO_RECORDING_NOT_RUNNING = 7;
constexpr uint32_t SYSTEM_SUSPEND_LOCATION_NOT_USED = 8;
constexpr uint32_t SYSTEM_SUSPEND_BLUETOOTH_NOT_USED = 9;
constexpr uint32_t SYSTEM_SUSPEND_MULTI_DEVICE_NOT_USED = 10;
constexpr uint32_t SYSTEM_SUSPEND_USED_ILLEGALLY = 11;
constexpr uint32_t SYSTEM_SUSPEND_SYSTEM_LOAD_WARNING = 12;
constexpr uint32_t MODE_DATA_TRANSFER = 1;
constexpr uint32_t MODE_AUDIO_PLAYBACK = 2;
constexpr uint32_t MODE_AUDIO_RECORDING = 3;
constexpr uint32_t MODE_LOCATION = 4;
constexpr uint32_t MODE_BLUETOOTH_INTERACTION = 5;
constexpr uint32_t MODE_MULTI_DEVICE_CONNECTION = 6;
constexpr uint32_t MODE_ALLOW_WIFI_AWARE = 7;
constexpr uint32_t MODE_VOIP = 8;
constexpr uint32_t MODE_TASK_KEEPING = 9;
constexpr uint32_t MODE_AV_PLAYBACK_AND_RECORD = 12;
constexpr uint32_t MODE_SPECIAL_SCENARIO_PROCESSING = 13;
constexpr uint32_t MODE_END = 14;
constexpr uint32_t SUBMODE_CAR_KEY_NORMAL_NOTIFICATION = 1;
constexpr uint32_t SUBMODE_NORMAL_NOTIFICATION = 2;
constexpr uint32_t SUBMODE_LIVE_VIEW_NOTIFICATION = 3;
constexpr uint32_t SUBMODE_AUDIO_PLAYBACK_NORMAL_NOTIFICATION = 4;
constexpr uint32_t SUBMODE_AVSESSION_AUDIO_PLAYBACK = 5;
constexpr uint32_t SUBMODE_AUDIO_RECORD_NORMAL_NOTIFICATION = 6;
constexpr uint32_t SUBMODE_SCREEN_RECORD_NORMAL_NOTIFICATION = 7;
constexpr uint32_t SUBMODE_VOICE_CHAT_NORMAL_NOTIFICATION = 8;
constexpr uint32_t SUBMODE_MEDIA_PROCESS_NORMAL_NOTIFICATION = 9;
constexpr uint32_t SUBMODE_VIDEO_BROADCAST_NORMAL_NOTIFICATION = 10;
constexpr uint32_t SUBMODE_WORK_OUT_NORMAL_NOTIFICATION = 11;
constexpr uint32_t SUBMODE_END = 12;
constexpr uint32_t INVALID_MODE_OR_SUBMODE = 0;
constexpr uint32_t NOT_SUPPORTED = 0;
constexpr uint32_t NOT_DETERMINED = 1;
constexpr uint32_t DENIED = 2;
constexpr uint32_t GRANTED_ONCE = 3;
constexpr uint32_t GRANTED_ALWAYS = 4;
constexpr uint32_t AUTH_END = 5;

static const std::string RSS_NAME = "resource_schedule_service";
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

void GetNativeToken(const std::string &name, const int32_t uid)
{
    auto tokenId = Security::AccessToken::AccessTokenKit::GetNativeTokenId(name);
    setuid(uid);
    SetSelfTokenID(tokenId);
}

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

    void OnTransientTaskErr(const std::shared_ptr<TransientTaskAppInfo>& info) override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface16";
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

    void OnContinuousTaskSuspend(const std::shared_ptr<ContinuousTaskCallbackInfo>
        &continuousTaskCallbackInfo) override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface17";
    }

    void OnContinuousTaskActive(const std::shared_ptr<ContinuousTaskCallbackInfo>
        &continuousTaskCallbackInfo) override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface18";
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

    void OnContinuousTaskUpdate(const std::shared_ptr<ContinuousTaskCallbackInfo>
        &continuousTaskCallbackInfo) override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface15";
    }

    void GetFlag(int32_t &flag) override
    {
        BgTaskClientUnitTest::bgtaskSubscriberRet_ = "interface16";
    }
};

class TestExpiredCallback : public ExpiredCallback {
public:
    TestExpiredCallback() : ExpiredCallback() {}

    void OnExpired() override
    {
        BgTaskClientUnitTest::expiredCallbackRet_ = "OnExpired";
    }

    void OnExpiredAuth(int32_t authResult) override
    {
        BgTaskClientUnitTest::expiredCallbackRet_ = "OnExpiredAuth";
    }
};

/**
 * @tc.name: BackgroundMode_001
 * @tc.desc: test background mode constant.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, BackgroundMode_001, TestSize.Level0)
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
    EXPECT_EQ(WORKOUT, (int32_t)BackgroundMode::WORKOUT);
    EXPECT_EQ(SPECIAL_SCENARIO_PROCESSING, (int32_t)BackgroundMode::SPECIAL_SCENARIO_PROCESSING);
    EXPECT_EQ(END, (int32_t)BackgroundMode::END);
}

/**
* @tc.name: ContinuousTaskCancelReason_001
* @tc.desc: test continuous task cancel reason constant.
* @tc.type: FUNC
* @tc.require: issueIBNOEQ
*/
HWTEST_F(BgTaskClientUnitTest, ContinuousTaskCancelReason_001, TestSize.Level1)
{
    EXPECT_EQ(USER_CANCEL, (int32_t)ContinuousTaskCancelReason::USER_CANCEL);
    EXPECT_EQ(SYSTEM_CANCEL, (int32_t)ContinuousTaskCancelReason::SYSTEM_CANCEL);
    EXPECT_EQ(USER_CANCEL_REMOVE_NOTIFICATION, (int32_t)ContinuousTaskCancelReason::USER_CANCEL_REMOVE_NOTIFICATION);
    EXPECT_EQ(SYSTEM_CANCEL_DATA_TRANSFER_LOW_SPEED,
        (int32_t)ContinuousTaskCancelReason::SYSTEM_CANCEL_DATA_TRANSFER_LOW_SPEED);
    EXPECT_EQ(SYSTEM_CANCEL_AUDIO_PLAYBACK_NOT_USE_AVSESSION,
        (int32_t)ContinuousTaskCancelReason::SYSTEM_CANCEL_AUDIO_PLAYBACK_NOT_USE_AVSESSION);
    EXPECT_EQ(SYSTEM_CANCEL_AUDIO_PLAYBACK_NOT_RUNNING,
        (int32_t)ContinuousTaskCancelReason::SYSTEM_CANCEL_AUDIO_PLAYBACK_NOT_RUNNING);
    EXPECT_EQ(SYSTEM_CANCEL_AUDIO_RECORDING_NOT_RUNNING,
        (int32_t)ContinuousTaskCancelReason::SYSTEM_CANCEL_AUDIO_RECORDING_NOT_RUNNING);
    EXPECT_EQ(SYSTEM_CANCEL_NOT_USE_LOCATION, (int32_t)ContinuousTaskCancelReason::SYSTEM_CANCEL_NOT_USE_LOCATION);
    EXPECT_EQ(SYSTEM_CANCEL_NOT_USE_BLUETOOTH, (int32_t)ContinuousTaskCancelReason::SYSTEM_CANCEL_NOT_USE_BLUETOOTH);
    EXPECT_EQ(SYSTEM_CANCEL_NOT_USE_MULTI_DEVICE,
        (int32_t)ContinuousTaskCancelReason::SYSTEM_CANCEL_NOT_USE_MULTI_DEVICE);
    EXPECT_EQ(SYSTEM_CANCEL_USE_ILLEGALLY, (int32_t)ContinuousTaskCancelReason::SYSTEM_CANCEL_USE_ILLEGALLY);
}

/**
* @tc.name: ContinuousTaskSuspendReason_001
* @tc.desc: test continuous task suspend reason constant.
* @tc.type: FUNC
* @tc.require: issueIC6B53
*/
HWTEST_F(BgTaskClientUnitTest, ContinuousTaskSuspendReason_001, TestSize.Level1)
{
    EXPECT_EQ(SYSTEM_SUSPEND_DATA_TRANSFER_LOW_SPEED,
        (uint32_t)ContinuousTaskSuspendReason::SYSTEM_SUSPEND_DATA_TRANSFER_LOW_SPEED);
    EXPECT_EQ(SYSTEM_SUSPEND_AUDIO_PLAYBACK_NOT_USE_AVSESSION,
        (uint32_t)ContinuousTaskSuspendReason::SYSTEM_SUSPEND_AUDIO_PLAYBACK_NOT_USE_AVSESSION);
    EXPECT_EQ(SYSTEM_SUSPEND_AUDIO_PLAYBACK_NOT_RUNNING,
        (uint32_t)ContinuousTaskSuspendReason::SYSTEM_SUSPEND_AUDIO_PLAYBACK_NOT_RUNNING);
    EXPECT_EQ(SYSTEM_SUSPEND_AUDIO_RECORDING_NOT_RUNNING,
        (uint32_t)ContinuousTaskSuspendReason::SYSTEM_SUSPEND_AUDIO_RECORDING_NOT_RUNNING);
    EXPECT_EQ(SYSTEM_SUSPEND_LOCATION_NOT_USED,
        (uint32_t)ContinuousTaskSuspendReason::SYSTEM_SUSPEND_LOCATION_NOT_USED);
    EXPECT_EQ(SYSTEM_SUSPEND_BLUETOOTH_NOT_USED,
        (uint32_t)ContinuousTaskSuspendReason::SYSTEM_SUSPEND_BLUETOOTH_NOT_USED);
    EXPECT_EQ(SYSTEM_SUSPEND_MULTI_DEVICE_NOT_USED,
        (uint32_t)ContinuousTaskSuspendReason::SYSTEM_SUSPEND_MULTI_DEVICE_NOT_USED);
    EXPECT_EQ(SYSTEM_SUSPEND_USED_ILLEGALLY, (uint32_t)ContinuousTaskSuspendReason::SYSTEM_SUSPEND_USED_ILLEGALLY);
    EXPECT_EQ(SYSTEM_SUSPEND_SYSTEM_LOAD_WARNING,
        (uint32_t)ContinuousTaskSuspendReason::SYSTEM_SUSPEND_SYSTEM_LOAD_WARNING);
}

/**
* @tc.name: BackgroundSubMode_001
* @tc.desc: test continuous task sub mode constant.
* @tc.type: FUNC
* @tc.require: issueIBNOEQ
*/
HWTEST_F(BgTaskClientUnitTest, BackgroundSubMode_001, TestSize.Level1)
{
    EXPECT_EQ(CAR_KEY, (int32_t)BackgroundSubMode::CAR_KEY);
}

/**
 * @tc.name: RequestStartBackgroundRunning_001
 * @tc.desc: test RequestStartBackgroundRunning interface.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, RequestStartBackgroundRunning_001, TestSize.Level0)
{
    ContinuousTaskParam taskParam = ContinuousTaskParam();
    EXPECT_NE(BackgroundTaskMgrHelper::RequestStartBackgroundRunning(taskParam), ERR_OK);
}

/**
 * @tc.name: RequestUpdateBackgroundRunning_001
 * @tc.desc: test RequestUpdateBackgroundRunning interface.
 * @tc.type: FUNC
 * @tc.require: issueI94UH9
 */
HWTEST_F(BgTaskClientUnitTest, RequestUpdateBackgroundRunning_001, TestSize.Level0)
{
    ContinuousTaskParam taskParam = ContinuousTaskParam();
    EXPECT_NE(BackgroundTaskMgrHelper::RequestUpdateBackgroundRunning(taskParam), ERR_OK);
}

/**
 * @tc.name: RequestStopBackgroundRunning_001
 * @tc.desc: test RequestStopBackgroundRunning interface.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI99HSB
 */
HWTEST_F(BgTaskClientUnitTest, RequestStopBackgroundRunning_001, TestSize.Level0)
{
    EXPECT_NE(BackgroundTaskMgrHelper::RequestStopBackgroundRunning("test", nullptr, -1), ERR_OK);
}

/**
 * @tc.name: RequestBackgroundRunningForInner_001
 * @tc.desc: test RequestBackgroundRunningForInner interface.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, RequestBackgroundRunningForInner_001, TestSize.Level0)
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
    BackgroundTaskMgrHelper::SubscribeBackgroundTask(*subscriber_);
    SleepForFC();
    BackgroundTaskMgrHelper::UnsubscribeBackgroundTask(*subscriber_);
    SleepForFC();
    EXPECT_NE(subscriber_, nullptr);
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
    BackgroundTaskMgrHelper::GetTransientTaskApps(list);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: PauseTransientTaskTimeForInner_001
 * @tc.desc: test PauseTransientTaskTimeForInner interface.
 * @tc.type: FUNC
 * @tc.require: issueI936BL
 */
HWTEST_F(BgTaskClientUnitTest, PauseTransientTaskTimeForInner_001, TestSize.Level1)
{
    int32_t uid = -1;
    EXPECT_NE(BackgroundTaskMgrHelper::PauseTransientTaskTimeForInner(uid), ERR_OK);
}

/**
 * @tc.name: StartTransientTaskTimeForInner_001
 * @tc.desc: test StartTransientTaskTimeForInner interface.
 * @tc.type: FUNC
 * @tc.require: issueI936BL
 */
HWTEST_F(BgTaskClientUnitTest, StartTransientTaskTimeForInner_001, TestSize.Level1)
{
    int32_t uid = -1;
    EXPECT_NE(BackgroundTaskMgrHelper::StartTransientTaskTimeForInner(uid), ERR_OK);
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
    GetNativeToken(RSS_NAME, RSS_UID);
    EXPECT_EQ((int32_t)BackgroundTaskMgrHelper::StopContinuousTask(1, 1, 1, ""), (int32_t)ERR_OK);
}

/**
 * @tc.name: SuspendContinuousTask_001
 * @tc.desc: request suspend target continuous task api test.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, SuspendContinuousTask_001, TestSize.Level1)
{
    GetNativeToken(RSS_NAME, RSS_UID);
    EXPECT_EQ((int32_t)BackgroundTaskMgrHelper::SuspendContinuousTask(1, 1, 4, ""), (int32_t)ERR_OK);
}

/**
 * @tc.name: ActiveContinuousTask_001
 * @tc.desc: request active target continuous task api test.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, ActiveContinuousTask_001, TestSize.Level1)
{
    GetNativeToken(RSS_NAME, RSS_UID);
    EXPECT_EQ((int32_t)BackgroundTaskMgrHelper::ActiveContinuousTask(1, 1, ""), (int32_t)ERR_OK);
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
    OHOS::BackgroundTaskMgr::TransientTaskAppInfo taskInfo;
    OHOS::BackgroundTaskMgr::ContinuousTaskCallbackInfo continousInfo;
    OHOS::BackgroundTaskMgr::ResourceCallbackInfo resourceInfo;
    subscriberImpl->OnConnected();
    subscriberImpl->OnDisconnected();
    subscriberImpl->OnTransientTaskStart(taskInfo);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface3");
    subscriberImpl->OnTransientTaskEnd(taskInfo);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface4");
    subscriberImpl->OnTransientTaskErr(taskInfo);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface16");
    subscriberImpl->OnAppTransientTaskStart(taskInfo);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface5");
    subscriberImpl->OnAppTransientTaskEnd(taskInfo);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface6");
    subscriberImpl->OnContinuousTaskStart(continousInfo);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface7");
    subscriberImpl->OnContinuousTaskStop(continousInfo);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface8");
    subscriberImpl->OnContinuousTaskSuspend(continousInfo);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface17");
    subscriberImpl->OnContinuousTaskActive(continousInfo);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface18");
    subscriberImpl->OnAppContinuousTaskStop(1);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface9");
    subscriberImpl->OnAppEfficiencyResourcesApply(resourceInfo);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface11");
    subscriberImpl->OnAppEfficiencyResourcesReset(resourceInfo);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface12");
    subscriberImpl->OnProcEfficiencyResourcesApply(resourceInfo);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface13");
    subscriberImpl->OnProcEfficiencyResourcesReset(resourceInfo);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface14");
    subscriberImpl->OnContinuousTaskUpdate(continousInfo);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface15");
    int32_t flag = 1;
    subscriberImpl->GetFlag(flag);
    EXPECT_EQ(bgtaskSubscriberRet_, "interface16");
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
    subscriber.OnTransientTaskErr(nullptr);
    subscriber.OnAppTransientTaskStart(nullptr);
    subscriber.OnAppTransientTaskEnd(nullptr);
    subscriber.OnContinuousTaskStart(nullptr);
    subscriber.OnContinuousTaskUpdate(nullptr);
    subscriber.OnContinuousTaskStop(nullptr);
    subscriber.OnContinuousTaskSuspend(nullptr);
    subscriber.OnContinuousTaskActive(nullptr);
    subscriber.OnAppContinuousTaskStop(1);
    subscriber.OnRemoteDied(nullptr);
    subscriber.OnAppEfficiencyResourcesApply(nullptr);
    subscriber.OnAppEfficiencyResourcesReset(nullptr);
    subscriber.OnProcEfficiencyResourcesApply(nullptr);
    subscriber.OnProcEfficiencyResourcesReset(nullptr);
    int32_t flag = 1;
    subscriber.GetFlag(flag);
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
    info2->SetContinuousTaskId(1);
    info2->SetCancelReason(1);
    info2->SetByRequestObject(true);
    info2->SetBundleName("bundleName");
    info2->SetUserId(1);
    info2->SetAppIndex(1);
    info2->SetSuspendReason(-1);
    info2->SetSuspendState(false);
    std::vector<uint32_t> backgroundSubModes {1};
    info2->SetBackgroundSubModes(backgroundSubModes);
    info2->SetNotificationId(1);
    info2->SetWantAgentBundleName("wantAgentBundleName");
    info2->SetWantAgentAbilityName("wantAgentAbilityName");
    info2->SetCancelCallBackSelf(true);
    Parcel parcel = Parcel();
    info2->Marshalling(parcel);
    sptr<ContinuousTaskCallbackInfo> info3 = sptr<ContinuousTaskCallbackInfo>(
        ContinuousTaskCallbackInfo::Unmarshalling(parcel));
    EXPECT_EQ(info3->GetTypeId(), 1);
    EXPECT_EQ(info3->GetCreatorUid(), 1);
    EXPECT_EQ(info3->GetCreatorPid(), 1);
    EXPECT_EQ(info3->GetAbilityName(), "test");
    EXPECT_EQ(info3->IsFromWebview(), false);
    EXPECT_EQ(info3->GetTokenId(), 0);
    EXPECT_EQ(info3->GetContinuousTaskId(), 1);
    EXPECT_EQ(info3->GetCancelReason(), 1);
    EXPECT_EQ(info3->GetSuspendState(), false);
    EXPECT_EQ(info3->GetSuspendReason(), -1);
    EXPECT_EQ(info3->IsByRequestObject(), true);
    EXPECT_EQ(info3->GetBundleName(), "bundleName");
    EXPECT_EQ(info3->GetUserId(), 1);
    EXPECT_EQ(info3->GetAppIndex(), 1);
    EXPECT_EQ(info3->GetBackgroundSubModes().size(), 1);
    EXPECT_EQ(info3->GetNotificationId(), 1);
    EXPECT_EQ(info3->GetWantAgentBundleName(), "wantAgentBundleName");
    EXPECT_EQ(info3->GetWantAgentAbilityName(), "wantAgentAbilityName");
    EXPECT_EQ(info3->IsCancelCallBackSelf(), true);
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
    info2->SetPid(1);
    EXPECT_EQ(info2->GetPid(), 1);

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
    expiredCallbackImpl->OnExpiredAuth(1);
    EXPECT_EQ(expiredCallbackRet_, "OnExpiredAuth");
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
    sptr<TransientTaskAppInfo> info3 = TransientTaskAppInfo::Unmarshalling(parcel);
    EXPECT_EQ(info3->GetPackageName(), "packageName");
    EXPECT_EQ(info3->GetUid(), 1);
    EXPECT_EQ(info3->GetPid(), 1);
}

/**
 * @tc.name: SetBgTaskConfig_001
 * @tc.desc: test SetBgTaskConfig interface.
 * @tc.type: FUNC
 * @tc.require: issueIAULHW
 */
HWTEST_F(BgTaskClientUnitTest, SetBgTaskConfig_001, TestSize.Level1)
{
    const std::string configData = "";
    EXPECT_NE(BackgroundTaskMgrHelper::SetBgTaskConfig(configData, 1), ERR_OK);
}

/**
 * @tc.name: ContinuousTaskInfo_001
 * @tc.desc: test ContinuousTaskInfo.
 * @tc.type: FUNC
 * @tc.require: issueIBY0DN
 */
HWTEST_F(BgTaskClientUnitTest, ContinuousTaskInfo_001, TestSize.Level1)
{
    std::shared_ptr<ContinuousTaskInfo> info1 = std::make_shared<ContinuousTaskInfo>();
    std::vector<uint32_t> backgroundModes {1};
    std::vector<uint32_t> backgroundSubModes {1};
    std::shared_ptr<ContinuousTaskInfo> info2 = std::make_shared<ContinuousTaskInfo>(
        "abilityName", 1, 1, false, backgroundModes, backgroundSubModes, 1, 1, 1, "wantAgentBundleName",
        "wantAgentAbilityName");
    info2->SetBundleName("bundleName");
    info2->SetAppIndex(1);
    info2->SetSuspendState(true);
    MessageParcel parcel = MessageParcel();
    info2->Marshalling(parcel);
    sptr<ContinuousTaskInfo> info3 = ContinuousTaskInfo::Unmarshalling(parcel);
    EXPECT_EQ(info3->GetAbilityName(), "abilityName");
    EXPECT_EQ(info3->GetWantAgentBundleName(), "wantAgentBundleName");
    EXPECT_EQ(info3->GetWantAgentAbilityName(), "wantAgentAbilityName");
    EXPECT_EQ(info3->GetUid(), 1);
    EXPECT_EQ(info3->GetPid(), 1);
    EXPECT_EQ(info3->GetNotificationId(), 1);
    EXPECT_EQ(info3->GetContinuousTaskId(), 1);
    EXPECT_EQ(info3->GetAbilityId(), 1);
    EXPECT_EQ(info3->IsFromWebView(), false);
    EXPECT_EQ(info3->GetBackgroundModes().size(), 1);
    EXPECT_EQ(info3->GetBackgroundSubModes().size(), 1);
    EXPECT_NE(info3->ToString(backgroundModes), "");
    EXPECT_EQ(info3->GetAppIndex(), 1);
    EXPECT_EQ(info3->GetBundleName(), "bundleName");
    EXPECT_EQ(info3->GetSuspendState(), true);
}

/**
 * @tc.name: RequestGetAllContinuousTasks_001
 * @tc.desc: test RequestGetAllContinuousTasks interface.
 * @tc.type: FUNC
 * @tc.require: issueIBY0DN
 */
HWTEST_F(BgTaskClientUnitTest, RequestGetAllContinuousTasks_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<ContinuousTaskInfo>> list;
    EXPECT_NE(BackgroundTaskMgrHelper::RequestGetAllContinuousTasks(list), ERR_OK);
}

/**
 * @tc.name: RequestGetAllContinuousTasks_002
 * @tc.desc: test RequestGetAllContinuousTasks interface.
 * @tc.type: FUNC
 * @tc.require: issuesICRZHF
 */
HWTEST_F(BgTaskClientUnitTest, RequestGetAllContinuousTasks_002, TestSize.Level1)
{
    std::vector<std::shared_ptr<ContinuousTaskInfo>> list;
    EXPECT_NE(BackgroundTaskMgrHelper::RequestGetAllContinuousTasks(list, true), ERR_OK);
}

/**
 * @tc.name: RequestGetContinuousTasksByUidForInner_001
 * @tc.desc: test RequestGetContinuousTasksByUidForInner interface.
 * @tc.type: FUNC
 * @tc.require: issueIBY0DN
 */
HWTEST_F(BgTaskClientUnitTest, RequestGetContinuousTasksByUidForInner_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<ContinuousTaskInfo>> list;
    int32_t uid = 1;
    EXPECT_EQ(BackgroundTaskMgrHelper::RequestGetContinuousTasksByUidForInner(uid, list),
        ERR_OK);
}

/**
 * @tc.name: AVSessionNotifyUpdateNotification_001
 * @tc.desc: test AVSessionNotifyUpdateNotification interface.
 * @tc.type: FUNC
 * @tc.require: issueIC9VN9
 */
HWTEST_F(BgTaskClientUnitTest, AVSessionNotifyUpdateNotification_001, TestSize.Level1)
{
    int32_t uid = 1;
    int32_t pid = 1;
    EXPECT_NE(BackgroundTaskMgrHelper::AVSessionNotifyUpdateNotification(uid, pid, true), ERR_OK);
    EXPECT_NE(BackgroundTaskMgrHelper::AVSessionNotifyUpdateNotification(uid, pid, false), ERR_OK);
}

/**
 * @tc.name: SuspendContinuousAudioTask_001
 * @tc.desc: test SuspendContinuousAudioTask interface.
 * @tc.type: FUNC
 * @tc.require: issueICT1ZV
 */
HWTEST_F(BgTaskClientUnitTest, SuspendContinuousAudioTask_001, TestSize.Level1)
{
    int32_t uid = 1;
    EXPECT_EQ(BackgroundTaskMgrHelper::SuspendContinuousAudioTask(uid), ERR_OK);
}

/**
 * @tc.name: BackgroundTaskMode_001
 * @tc.desc: test background task mode.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, BackgroundTaskMode_001, TestSize.Level0)
{
    EXPECT_EQ(MODE_DATA_TRANSFER, (int32_t)BackgroundTaskMode::MODE_DATA_TRANSFER);
    EXPECT_EQ(MODE_AUDIO_PLAYBACK, (int32_t)BackgroundTaskMode::MODE_AUDIO_PLAYBACK);
    EXPECT_EQ(MODE_AUDIO_RECORDING, (int32_t)BackgroundTaskMode::MODE_AUDIO_RECORDING);
    EXPECT_EQ(MODE_LOCATION, (int32_t)BackgroundTaskMode::MODE_LOCATION);
    EXPECT_EQ(MODE_BLUETOOTH_INTERACTION, (int32_t)BackgroundTaskMode::MODE_BLUETOOTH_INTERACTION);
    EXPECT_EQ(MODE_MULTI_DEVICE_CONNECTION, (int32_t)BackgroundTaskMode::MODE_MULTI_DEVICE_CONNECTION);
    EXPECT_EQ(MODE_ALLOW_WIFI_AWARE, (int32_t)BackgroundTaskMode::MODE_ALLOW_WIFI_AWARE);
    EXPECT_EQ(MODE_VOIP, (int32_t)BackgroundTaskMode::MODE_VOIP);
    EXPECT_EQ(MODE_TASK_KEEPING, (int32_t)BackgroundTaskMode::MODE_TASK_KEEPING);
    EXPECT_EQ(MODE_AV_PLAYBACK_AND_RECORD, (int32_t)BackgroundTaskMode::MODE_AV_PLAYBACK_AND_RECORD);
    EXPECT_EQ(MODE_SPECIAL_SCENARIO_PROCESSING, (int32_t)BackgroundTaskMode::MODE_SPECIAL_SCENARIO_PROCESSING);
    EXPECT_EQ(MODE_END, (int32_t)BackgroundTaskMode::END);
    BackgroundTaskMode::GetBackgroundTaskModeStr(MODE_DATA_TRANSFER);
    BackgroundTaskMode::GetBackgroundTaskModeStr(INVALID_MODE_OR_SUBMODE);
    EXPECT_TRUE(BackgroundTaskMode::IsModeTypeMatching(MODE_AUDIO_PLAYBACK));
    EXPECT_TRUE(BackgroundTaskMode::IsModeTypeMatching(MODE_AUDIO_RECORDING));
    EXPECT_TRUE(BackgroundTaskMode::IsModeTypeMatching(MODE_LOCATION));
    EXPECT_TRUE(BackgroundTaskMode::IsModeTypeMatching(MODE_VOIP));
    EXPECT_NE(BackgroundTaskMode::GetSubModeTypeMatching(SUBMODE_LIVE_VIEW_NOTIFICATION),
        BackgroundTaskMode::END);
    EXPECT_EQ(BackgroundTaskMode::GetSubModeTypeMatching(INVALID_MODE_OR_SUBMODE),
        BackgroundTaskMode::END);
    EXPECT_NE(BackgroundTaskMode::GetV9BackgroundModeByMode(MODE_AUDIO_PLAYBACK),
        BackgroundMode::END);
    EXPECT_NE(BackgroundTaskMode::GetV9BackgroundModeByMode(MODE_AUDIO_RECORDING),
        BackgroundMode::END);
    EXPECT_NE(BackgroundTaskMode::GetV9BackgroundModeByMode(MODE_LOCATION),
        BackgroundMode::END);
    EXPECT_EQ(BackgroundTaskMode::GetV9BackgroundModeByMode(INVALID_MODE_OR_SUBMODE),
        BackgroundMode::END);
    EXPECT_NE(BackgroundTaskMode::GetV9BackgroundModeBySubMode(SUBMODE_LIVE_VIEW_NOTIFICATION),
        BackgroundMode::END);
    EXPECT_EQ(BackgroundTaskMode::GetV9BackgroundModeBySubMode(INVALID_MODE_OR_SUBMODE),
        BackgroundMode::END);
}

/**
 * @tc.name: BackgroundTaskSubmode_001
 * @tc.desc: test continuous task sub mode.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, BackgroundTaskSubmode_001, TestSize.Level0)
{
    EXPECT_EQ(SUBMODE_CAR_KEY_NORMAL_NOTIFICATION, (int32_t)BackgroundTaskSubmode::SUBMODE_CAR_KEY_NORMAL_NOTIFICATION);
    EXPECT_EQ(SUBMODE_NORMAL_NOTIFICATION, (int32_t)BackgroundTaskSubmode::SUBMODE_NORMAL_NOTIFICATION);
    EXPECT_EQ(SUBMODE_LIVE_VIEW_NOTIFICATION, (int32_t)BackgroundTaskSubmode::SUBMODE_LIVE_VIEW_NOTIFICATION);
    EXPECT_EQ(SUBMODE_AUDIO_PLAYBACK_NORMAL_NOTIFICATION,
        (int32_t)BackgroundTaskSubmode::SUBMODE_AUDIO_PLAYBACK_NORMAL_NOTIFICATION);
    EXPECT_EQ(SUBMODE_AVSESSION_AUDIO_PLAYBACK, (int32_t)BackgroundTaskSubmode::SUBMODE_AVSESSION_AUDIO_PLAYBACK);
    EXPECT_EQ(SUBMODE_AUDIO_RECORD_NORMAL_NOTIFICATION,
        (int32_t)BackgroundTaskSubmode::SUBMODE_AUDIO_RECORD_NORMAL_NOTIFICATION);
    EXPECT_EQ(SUBMODE_SCREEN_RECORD_NORMAL_NOTIFICATION,
        (int32_t)BackgroundTaskSubmode::SUBMODE_SCREEN_RECORD_NORMAL_NOTIFICATION);
    EXPECT_EQ(SUBMODE_VOICE_CHAT_NORMAL_NOTIFICATION,
        (int32_t)BackgroundTaskSubmode::SUBMODE_VOICE_CHAT_NORMAL_NOTIFICATION);
    EXPECT_EQ(SUBMODE_MEDIA_PROCESS_NORMAL_NOTIFICATION,
        (int32_t)BackgroundTaskSubmode::SUBMODE_MEDIA_PROCESS_NORMAL_NOTIFICATION);
    EXPECT_EQ(SUBMODE_VIDEO_BROADCAST_NORMAL_NOTIFICATION,
        (int32_t)BackgroundTaskSubmode::SUBMODE_VIDEO_BROADCAST_NORMAL_NOTIFICATION);
    EXPECT_EQ(SUBMODE_WORK_OUT_NORMAL_NOTIFICATION,
        (int32_t)BackgroundTaskSubmode::SUBMODE_WORK_OUT_NORMAL_NOTIFICATION);
    EXPECT_EQ(SUBMODE_END, (int32_t)BackgroundTaskSubmode::END);
    BackgroundTaskSubmode::GetBackgroundTaskSubmodeStr(SUBMODE_CAR_KEY_NORMAL_NOTIFICATION);
    BackgroundTaskSubmode::GetBackgroundTaskSubmodeStr(INVALID_MODE_OR_SUBMODE);
}

/**
 * @tc.name: GetBackgroundModeStr_001
 * @tc.desc: test GetBackgroundModeStr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskClientUnitTest, GetBackgroundModeStr_001, TestSize.Level0)
{
    EXPECT_NE(BackgroundMode::GetBackgroundModeStr(MODE_LOCATION), "");
    EXPECT_EQ(BackgroundMode::GetBackgroundModeStr(INVALID_MODE_OR_SUBMODE), "default");
}

/**
 * @tc.name: GetBackgroundSubModeStr_001
 * @tc.desc: test GetBackgroundSubModeStr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskClientUnitTest, GetBackgroundSubModeStr_001, TestSize.Level0)
{
    EXPECT_NE(BackgroundSubMode::GetBackgroundSubModeStr(CAR_KEY), "");
    EXPECT_EQ(BackgroundSubMode::GetBackgroundSubModeStr(INVALID_MODE_OR_SUBMODE), "default");
}

/**
 * @tc.name: ContinuousTaskRequest_001
 * @tc.desc: test ContinuousTaskRequest.
 * @tc.type: FUNC
 * @tc.require: issueIBY0DN
 */
HWTEST_F(BgTaskClientUnitTest, ContinuousTaskRequest_001, TestSize.Level1)
{
    std::shared_ptr<ContinuousTaskRequest> info1 = std::make_shared<ContinuousTaskRequest>();
    std::vector<uint32_t> backgroundModes {1};
    std::vector<uint32_t> backgroundSubModes {1};
    std::shared_ptr<ContinuousTaskRequest> info2 = std::make_shared<ContinuousTaskRequest>(
        backgroundModes, backgroundSubModes, std::make_shared<AbilityRuntime::WantAgent::WantAgent>(), true, 10, true);

    MessageParcel parcel = MessageParcel();
    info2->Marshalling(parcel);
    sptr<ContinuousTaskRequest> info3 = ContinuousTaskRequest::Unmarshalling(parcel);
    info3->GetWantAgent();
    info3->GetBackgroundTaskModes();
    info3->GetBackgroundTaskSubmodes();
    info3->IsBuildByRequest();
    info3->GetWantAgent();
    info3->GetContinuousTaskId();
    info3->SetContinuousTaskId(1);
    info3->SetCombinedTaskNotification(false);
    std::vector<uint32_t> backgroundTaskSubMode {1};
    std::vector<uint32_t> backgroundTaskMode {1};
    info3->SetBackgroundTaskSubMode(backgroundTaskSubMode);
    info3->SetBackgroundTaskMode(backgroundTaskMode);
    info3->SetWantAgent(std::make_shared<AbilityRuntime::WantAgent::WantAgent>());
    EXPECT_EQ(info3->GetContinuousTaskId(), 1);
}

/**
 * @tc.name: IsModeSupported_001
 * @tc.desc: test IsModeSupported interface.
 * @tc.type: FUNC
 * @tc.require: issueICWQV5
 */
HWTEST_F(BgTaskClientUnitTest, IsModeSupported_001, TestSize.Level1)
{
    ContinuousTaskParam taskParam = ContinuousTaskParam();
    EXPECT_NE(BackgroundTaskMgrHelper::IsModeSupported(taskParam), ERR_OK);
}

/**
 * @tc.name: SetSupportedTaskKeepingProcesses_001
 * @tc.desc: test SetSupportedTaskKeepingProcesses.
 * @tc.type: FUNC
 * @tc.require: issueICVQZF
 */
HWTEST_F(BgTaskClientUnitTest, SetSupportedTaskKeepingProcesses_001, TestSize.Level1)
{
    std::set<std::string> processSet;
    EXPECT_EQ(BackgroundTaskMgrHelper::SetSupportedTaskKeepingProcesses(processSet), ERR_OK);
}

/**
 * @tc.name: SetMaliciousAppConfig_001
 * @tc.desc: test SetMaliciousAppConfig.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskClientUnitTest, SetMaliciousAppConfig_001, TestSize.Level1)
{
    std::set<std::string> maliciousAppSet;
    EXPECT_EQ(BackgroundTaskMgrHelper::SetMaliciousAppConfig(maliciousAppSet), ERR_OK);
}

/**
 * @tc.name: UserAuthResult_001
 * @tc.desc: test UserAuthResult.
 * @tc.type: FUNC
 * @tc.require: 752
 */
HWTEST_F(BgTaskClientUnitTest, UserAuthResult_001, TestSize.Level1)
{
    EXPECT_EQ(NOT_SUPPORTED, (int32_t)UserAuthResult::NOT_SUPPORTED);
    EXPECT_EQ(NOT_DETERMINED, (int32_t)UserAuthResult::NOT_DETERMINED);
    EXPECT_EQ(DENIED, (int32_t)UserAuthResult::DENIED);
    EXPECT_EQ(GRANTED_ONCE, (int32_t)UserAuthResult::GRANTED_ONCE);
    EXPECT_EQ(GRANTED_ALWAYS, (int32_t)UserAuthResult::GRANTED_ALWAYS);
    EXPECT_EQ(AUTH_END, (int32_t)UserAuthResult::END);
}

/**
 * @tc.name: CheckSpecialScenarioAuth_001
 * @tc.desc: test CheckSpecialScenarioAuth.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskClientUnitTest, CheckSpecialScenarioAuth_001, TestSize.Level1)
{
    uint32_t authResult = 0;
    EXPECT_EQ(BackgroundTaskMgrHelper::CheckSpecialScenarioAuth(0, authResult), ERR_BGTASK_PERMISSION_DENIED);
}

/**
 * @tc.name: CheckTaskAuthResult_001
 * @tc.desc: test CheckTaskAuthResult.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskClientUnitTest, CheckTaskAuthResult_001, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    int32_t userId = 100;
    int32_t appIndex = 0;
    EXPECT_EQ(BackgroundTaskMgrHelper::CheckTaskAuthResult(bundleName, userId, appIndex),
        ERR_BGTASK_CONTINUOUS_NOT_APPLY_AUTH_RECORD);
}

/**
 * @tc.name: EnableContinuousTaskRequest_001
 * @tc.desc: test EnableContinuousTaskRequest.
 * @tc.type: FUNC
 * @tc.require: 783
 */
HWTEST_F(BgTaskClientUnitTest, EnableContinuousTaskRequest_001, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    int32_t uid = 100;
    bool isEnable = true;
    EXPECT_EQ(BackgroundTaskMgrHelper::EnableContinuousTaskRequest(uid, isEnable), 0);
}

/**
 * @tc.name: BackgroundTaskStateInfo_001
 * @tc.desc: test BackgroundTaskStateInfo.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskClientUnitTest, BackgroundTaskStateInfo_001, TestSize.Level1)
{
    std::shared_ptr<BackgroundTaskStateInfo> info1 = std::make_shared<BackgroundTaskStateInfo>();
    std::shared_ptr<BackgroundTaskStateInfo> info2 = std::make_shared<BackgroundTaskStateInfo>(
        1, "bundleName", 1, 1);
    info1->SetUserId(2);
    info1->SetBundleName("bundleName2");
    info1->SetAppIndex(2);
    info1->SetUserAuthResult(2);
    MessageParcel parcel = MessageParcel();
    info2->Marshalling(parcel);
    sptr<BackgroundTaskStateInfo> info3 = BackgroundTaskStateInfo::Unmarshalling(parcel);
    EXPECT_EQ(info3->GetUserId(), 1);
    EXPECT_EQ(info3->GetBundleName(), "bundleName");
    EXPECT_EQ(info3->GetAppIndex(), 1);
    EXPECT_EQ(info3->GetUserAuthResult(), 1);
}

/**
 * @tc.name: SetBackgroundTaskState_001
 * @tc.desc: test SetBackgroundTaskState.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskClientUnitTest, SetBackgroundTaskState_001, TestSize.Level1)
{
    std::shared_ptr<BackgroundTaskStateInfo> taskParam = std::make_shared<BackgroundTaskStateInfo>();
    EXPECT_EQ(BackgroundTaskMgrHelper::SetBackgroundTaskState(taskParam), ERR_BGTASK_PERMISSION_DENIED);
}

/**
 * @tc.name: GetBackgroundTaskState_001
 * @tc.desc: test GetBackgroundTaskState.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskClientUnitTest, GetBackgroundTaskState_001, TestSize.Level1)
{
    std::shared_ptr<BackgroundTaskStateInfo> taskParam = std::make_shared<BackgroundTaskStateInfo>();
    EXPECT_EQ(BackgroundTaskMgrHelper::GetBackgroundTaskState(taskParam), ERR_BGTASK_PERMISSION_DENIED);
}

/**
 * @tc.name: GetAllContinuousTasksBySystem_001
 * @tc.desc: test GetAllContinuousTasksBySystem.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskClientUnitTest, GetAllContinuousTasksBySystem_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<ContinuousTaskInfo>> list;
    EXPECT_EQ(BackgroundTaskMgrHelper::GetAllContinuousTasksBySystem(list), ERR_BGTASK_PERMISSION_DENIED);
}

/**
 * @tc.name: GetSuspendReasonValue_001
 * @tc.desc: test GetSuspendReasonValue.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskClientUnitTest, GetSuspendReasonValue_001, TestSize.Level1)
{
    EXPECT_EQ(ContinuousTaskSuspendReason::GetSuspendReasonValue(INVALID_MODE_OR_SUBMODE), 0);
}

/**
 * @tc.name: SetSpecialExemptedProcess_001
 * @tc.desc: test SetSpecialExemptedProcess.
 * @tc.type: FUNC
 * @tc.require: issueICVQZF
 */
HWTEST_F(BgTaskClientUnitTest, SetSpecialExemptedProcess_001, TestSize.Level1)
{
    std::set<std::string> bundleNameSet;
    EXPECT_EQ(BackgroundTaskMgrHelper::SetSpecialExemptedProcess(bundleNameSet), ERR_OK);
}

/**
 * @tc.name: GetAllContinuousTaskApps_001
 * @tc.desc: test GetAllContinuousTaskApps interface.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, GetAllContinuousTaskApps_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> list;
    EXPECT_EQ(BackgroundTaskMgrHelper::GetAllContinuousTaskApps(list), ERR_OK);
}
}
}