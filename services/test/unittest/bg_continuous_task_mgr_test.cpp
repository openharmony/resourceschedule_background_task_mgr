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

#include "gtest/gtest.h"

#define private public

#include "bgtaskmgr_inner_errors.h"
#include "background_task_subscriber.h"
#include "bg_continuous_task_mgr.h"
#include "continuous_task_detection.h"
#include "want_agent.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr int32_t SLEEP_TIME = 500;
}
class BgContinuousTaskMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    inline void SleepForFC()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    }

    static std::shared_ptr<BgContinuousTaskMgr> bgContinuousTaskMgr_;
};

std::shared_ptr<BgContinuousTaskMgr> BgContinuousTaskMgrTest::bgContinuousTaskMgr_ = nullptr;

void BgContinuousTaskMgrTest::SetUpTestCase()
{
    bgContinuousTaskMgr_ = BgContinuousTaskMgr::GetInstance();
    bgContinuousTaskMgr_->dataStorage_ = std::make_shared<DataStorage>();
    bgContinuousTaskMgr_->InitRequiredResourceInfo();
}

void BgContinuousTaskMgrTest::TearDownTestCase() {}

void BgContinuousTaskMgrTest::SetUp() {}

void BgContinuousTaskMgrTest::TearDown()
{
    std::vector<string> dumpOption;
    dumpOption.emplace_back("-C");
    dumpOption.emplace_back("--cancel_all");
    std::vector<string> dumpInfo;
    bgContinuousTaskMgr_->ShellDump(dumpOption, dumpInfo);
    ContinuousTaskDetection::GetInstance()->ClearAllData();
}

class TestBackgroundTaskSubscriber : public BackgroundTaskSubscriber {
public:
    void OnContinuousTaskStart(const std::shared_ptr<ContinuousTaskCallbackInfo>
        &continuousTaskCallbackInfo) override {}

    void OnContinuousTaskStop(const std::shared_ptr<ContinuousTaskCallbackInfo>
        &continuousTaskCallbackInfo) override {}
};

/**
 * @tc.name: StartBackgroundRunning_001
 * @tc.desc: start background runnging use new api test.
 * @tc.type: FUNC
 * @tc.require: SR000GGT7U AR000GH6ER AR000GH6EM AR000GH6EN AR000GH6EO
 */
HWTEST_F(BgContinuousTaskMgrTest, StartBackgroundRunning_001, TestSize.Level1)
{
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(nullptr), (int32_t)ERR_BGTASK_INVALID_PARAM);
    sptr<ContinuousTaskParam> taskParam = new (std::nothrow) ContinuousTaskParam();
    EXPECT_NE(taskParam, nullptr);
    taskParam->appName_ = "Entry";
    taskParam->isNewApi_ = true;
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), (int32_t)ERR_BGTASK_INVALID_PARAM);
    taskParam->wantAgent_ = std::make_shared<AbilityRuntime::WantAgent::WantAgent>();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), (int32_t)ERR_BGTASK_INVALID_PARAM);
    taskParam->abilityName_ = "";
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), (int32_t)ERR_BGTASK_INVALID_PARAM);
    taskParam->abilityName_ = "ability1";
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), (int32_t)ERR_BGTASK_INVALID_BGMODE);
    taskParam->bgModeId_ = 9;
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), (int32_t)ERR_BGTASK_INVALID_BGMODE);
    taskParam->bgModeId_ = 1;
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), (int32_t)ERR_BGTASK_INVALID_BGMODE);
    taskParam->bgModeId_ = 4;
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), (int32_t)ERR_OK);
}

/**
 * @tc.name: StartBackgroundRunning_002
 * @tc.desc: start background runnging use old api test.
 * @tc.type: FUNC
 * @tc.require: SR000GGT7T AR000GH6ER AR000GH6EP AR000GJ9PR AR000GH6G8
 */
HWTEST_F(BgContinuousTaskMgrTest, StartBackgroundRunning_002, TestSize.Level1)
{
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(nullptr), (int32_t)ERR_BGTASK_INVALID_PARAM);
    sptr<ContinuousTaskParam> taskParam = new (std::nothrow) ContinuousTaskParam();
    EXPECT_NE(taskParam, nullptr);
    taskParam->appName_ = "Entry";
    taskParam->isNewApi_ = false;
    taskParam->bgModeId_ = 0;
    taskParam->abilityName_ = "";
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), (int32_t)ERR_BGTASK_INVALID_PARAM);
    taskParam->abilityName_ = "ability1";
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), (int32_t)ERR_OK);
}

/**
 * @tc.name: StopBackgroundRunning_001
 * @tc.desc: stop background runnging test.
 * @tc.type: FUNC
 * @tc.require: SR000GGT7V AR000GH6ES AR000GH6EM AR000GH6G9 AR000GH56K
 */
HWTEST_F(BgContinuousTaskMgrTest, StopBackgroundRunning_001, TestSize.Level1)
{
    sptr<ContinuousTaskParam> taskParam = new (std::nothrow) ContinuousTaskParam();
    EXPECT_NE(taskParam, nullptr);
    taskParam->appName_ = "Entry";
    taskParam->wantAgent_ = std::make_shared<AbilityRuntime::WantAgent::WantAgent>();
    taskParam->abilityName_ = "ability1";
    taskParam->bgModeId_ = 4;
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning(""), (int32_t)ERR_BGTASK_INVALID_PARAM);
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("ability1"), (int32_t)ERR_BGTASK_INVALID_PARAM);
    SleepForFC();
    ContinuousTaskDetection::GetInstance()->isLocationSwitchOn_ = true;
    ContinuousTaskDetection::GetInstance()->locationUsingRecords_.emplace_back(std::make_pair(1, 1));
    bgContinuousTaskMgr_->StartBackgroundRunning(taskParam);
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("ability1"), (int32_t)ERR_OK);
}

/**
 * @tc.name: SubscribeContinuousTask_001
 * @tc.desc: subscribe continuous task event callback test.
 * @tc.type: FUNC
 * @tc.require: SR000GGT81 AR000GH6EM AR000GH6G9 AR000GH6ET
 */
HWTEST_F(BgContinuousTaskMgrTest, SubscribeContinuousTask_001, TestSize.Level1)
{
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->AddSubscriber(nullptr), (int32_t)ERR_BGTASK_INVALID_PARAM);
    SleepForFC();
    auto subscriber = new (std::nothrow) TestBackgroundTaskSubscriber();
    EXPECT_NE(subscriber, nullptr);
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->AddSubscriber(subscriber->GetImpl()), (int32_t)ERR_OK);
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->AddSubscriber(subscriber->GetImpl()), (int32_t)ERR_BGTASK_OBJECT_EXISTS);
}

/**
 * @tc.name: UnsubscribeContinuousTask_001
 * @tc.desc: unsubscribe continuous task event callback test.
 * @tc.type: FUNC
 * @tc.require: SR000GGT7U AR000GH6EM AR000GH6G9 AR000GH6ET
 */
HWTEST_F(BgContinuousTaskMgrTest, UnsubscribeContinuousTask_001, TestSize.Level1)
{
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->RemoveSubscriber(nullptr), (int32_t)ERR_BGTASK_INVALID_PARAM);
    SleepForFC();
    auto subscriber = new (std::nothrow) TestBackgroundTaskSubscriber();
    EXPECT_NE(subscriber, nullptr);
    bgContinuousTaskMgr_->AddSubscriber(subscriber->GetImpl());
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->RemoveSubscriber(subscriber->GetImpl()), (int32_t)ERR_OK);
}

/**
 * @tc.name: AudioPlaybackDetection
 * @tc.desc: detect audio playback background mode.
 * @tc.type: FUNC
 * @tc.require: SR000GGT7U AR000GH6EM AR000GH6G9 AR000GH6ET
 */
HWTEST_F(BgContinuousTaskMgrTest, AudioPlaybackDetection, TestSize.Level1)
{
    sptr<ContinuousTaskParam> taskParam = new (std::nothrow) ContinuousTaskParam();
    EXPECT_NE(taskParam, nullptr);
    taskParam->appName_ = "Entry";
    taskParam->wantAgent_ = std::make_shared<AbilityRuntime::WantAgent::WantAgent>();
    taskParam->abilityName_ = "ability1";
    taskParam->bgModeId_ = 2;
    bgContinuousTaskMgr_->StartBackgroundRunning(taskParam);
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("ability1"), (int32_t)ERR_BGTASK_INVALID_PARAM);
    ContinuousTaskDetection::GetInstance()->audioPlayerInfos_.emplace_back(std::make_shared<AudioInfo>(1, 111));
    ContinuousTaskDetection::GetInstance()->avSessionInfos_.emplace_back(std::make_shared<AVSessionInfo>(1, 1, "111"));
    SleepForFC();
    bgContinuousTaskMgr_->StartBackgroundRunning(taskParam);
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("ability1"), (int32_t)ERR_OK);
}

/**
 * @tc.name: AudioRecordingDetection
 * @tc.desc: detect audio recording background mode.
 * @tc.type: FUNC
 * @tc.require: SR000GGT7U AR000GH6EM AR000GH6G9 AR000GH6ET
 */
HWTEST_F(BgContinuousTaskMgrTest, AudioRecordingDetection, TestSize.Level1)
{
    sptr<ContinuousTaskParam> taskParam = new (std::nothrow) ContinuousTaskParam();
    EXPECT_NE(taskParam, nullptr);
    taskParam->appName_ = "Entry";
    taskParam->wantAgent_ = std::make_shared<AbilityRuntime::WantAgent::WantAgent>();
    taskParam->abilityName_ = "ability1";
    taskParam->bgModeId_ = 3;
    bgContinuousTaskMgr_->StartBackgroundRunning(taskParam);
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("ability1"), (int32_t)ERR_BGTASK_INVALID_PARAM);
    ContinuousTaskDetection::GetInstance()->audioRecorderInfos_.emplace_back(std::make_shared<AudioInfo>(1, 111));
    SleepForFC();
    bgContinuousTaskMgr_->StartBackgroundRunning(taskParam);
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("ability1"), (int32_t)ERR_OK);
}

/**
 * @tc.name: LocationDetection
 * @tc.desc: detect audio recording background mode.
 * @tc.type: FUNC
 * @tc.require: SR000GGT7U AR000GH6EM AR000GH6G9 AR000GH6ET
 */
HWTEST_F(BgContinuousTaskMgrTest, LocationDetection, TestSize.Level1)
{
    sptr<ContinuousTaskParam> taskParam = new (std::nothrow) ContinuousTaskParam();
    EXPECT_NE(taskParam, nullptr);
    taskParam->appName_ = "Entry";
    taskParam->wantAgent_ = std::make_shared<AbilityRuntime::WantAgent::WantAgent>();
    taskParam->abilityName_ = "ability1";
    taskParam->bgModeId_ = 4;
    bgContinuousTaskMgr_->StartBackgroundRunning(taskParam);
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("ability1"), (int32_t)ERR_BGTASK_INVALID_PARAM);
    ContinuousTaskDetection::GetInstance()->isLocationSwitchOn_ = true;
    ContinuousTaskDetection::GetInstance()->locationUsingRecords_.emplace_back(std::make_pair(1, 1));
    SleepForFC();
    bgContinuousTaskMgr_->StartBackgroundRunning(taskParam);
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("ability1"), (int32_t)ERR_OK);
}

/**
 * @tc.name: BluetoothInteractionDetection
 * @tc.desc: detect audio recording background mode.
 * @tc.type: FUNC
 * @tc.require: SR000GGT7U AR000GH6EM AR000GH6G9 AR000GH6ET
 */
HWTEST_F(BgContinuousTaskMgrTest, BluetoothInteractionDetection, TestSize.Level1)
{
    sptr<ContinuousTaskParam> taskParam = new (std::nothrow) ContinuousTaskParam();
    EXPECT_NE(taskParam, nullptr);
    taskParam->appName_ = "Entry";
    taskParam->wantAgent_ = std::make_shared<AbilityRuntime::WantAgent::WantAgent>();
    taskParam->abilityName_ = "ability1";
    taskParam->bgModeId_ = 5;
    bgContinuousTaskMgr_->StartBackgroundRunning(taskParam);
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("ability1"), (int32_t)ERR_BGTASK_INVALID_PARAM);
    ContinuousTaskDetection::GetInstance()->isBrSwitchOn_ = true;
    ContinuousTaskDetection::GetInstance()->isBleSwitchOn_ = true;
    ContinuousTaskDetection::GetInstance()->sppConnectRecords_.emplace_back(
        std::make_shared<SppConnectStateReocrd>(1, 1, 1));
    SleepForFC();
    bgContinuousTaskMgr_->StartBackgroundRunning(taskParam);
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("ability1"), (int32_t)ERR_OK);
}

/**
 * @tc.name: MultiDeviceConnectionDetection
 * @tc.desc: detect audio recording background mode.
 * @tc.type: FUNC
 * @tc.require: SR000GGT7U AR000GH6EM AR000GH6G9 AR000GH6ET
 */
HWTEST_F(BgContinuousTaskMgrTest, MultiDeviceConnectionDetection, TestSize.Level1)
{
    sptr<ContinuousTaskParam> taskParam = new (std::nothrow) ContinuousTaskParam();
    EXPECT_NE(taskParam, nullptr);
    taskParam->appName_ = "Entry";
    taskParam->wantAgent_ = std::make_shared<AbilityRuntime::WantAgent::WantAgent>();
    taskParam->abilityName_ = "ability1";
    taskParam->bgModeId_ = 6;
    bgContinuousTaskMgr_->StartBackgroundRunning(taskParam);
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("ability1"), (int32_t)ERR_BGTASK_INVALID_PARAM);
    ContinuousTaskDetection::GetInstance()->callerRecords_[1] = 1;
    ContinuousTaskDetection::GetInstance()->calleeRecords_[1] = 1;
    SleepForFC();
    bgContinuousTaskMgr_->StartBackgroundRunning(taskParam);
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("ability1"), (int32_t)ERR_OK);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS