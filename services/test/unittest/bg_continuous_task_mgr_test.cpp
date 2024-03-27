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

#include "bgtask_common.h"
#include "background_task_subscriber.h"
#include "background_task_subscriber_proxy.h"
#include "bgtaskmgr_inner_errors.h"
#include "background_task_subscriber.h"
#include "bg_continuous_task_mgr.h"
#include "common_event_support.h"
#include "want_agent.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr int32_t SLEEP_TIME = 500;
static constexpr int32_t PROMPT_NUMS = 10;
static constexpr uint32_t LOCATION_BGMODE = 8;
static constexpr uint32_t BLUETOOTH_INTERACTION = 16;
static constexpr uint32_t BGMODE_WIFI_INTERACTION = 64;
static constexpr uint32_t BGMODE_VOIP = 128;
static constexpr uint32_t PC_BGMODE_TASK_KEEPING = 256;
static constexpr uint32_t LOCATION_BGMODE_ID = 4;
static constexpr uint32_t BGMODE_WIFI_INTERACTION_ID = 7;
static constexpr uint32_t BGMODE_VOIP_ID = 8;
static constexpr uint32_t BGMODE_TASK_KEEPING_ID = 9;
static constexpr uint32_t INVALID_BGMODE_ID = 11;
static constexpr uint64_t NO_SYSTEM_APP_TOKEN_ID = -100;
static constexpr int32_t DEFAULT_USERID = 100;
static constexpr int32_t TEST_NUM_ONE = 1;
static constexpr int32_t TEST_NUM_TWO = 2;
static constexpr int32_t TEST_NUM_THREE = 3;
static constexpr uint32_t CONFIGURE_ALL_MODES = 0x1FF;
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
    std::fill_n(std::back_inserter(bgContinuousTaskMgr_->continuousTaskText_), PROMPT_NUMS, "bgmode_test");
    bgContinuousTaskMgr_->isSysReady_.store(true);
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
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(nullptr), (int32_t)ERR_BGTASK_CHECK_TASK_PARAM);
    sptr<ContinuousTaskParam> taskParam = new (std::nothrow) ContinuousTaskParam();
    EXPECT_NE(taskParam, nullptr);
    taskParam->appName_ = "Entry";
    taskParam->isNewApi_ = true;
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), (int32_t)ERR_BGTASK_CHECK_TASK_PARAM);
    taskParam->wantAgent_ = std::make_shared<AbilityRuntime::WantAgent::WantAgent>();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), (int32_t)ERR_BGTASK_CHECK_TASK_PARAM);
    taskParam->abilityName_ = "";
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), (int32_t)ERR_BGTASK_CHECK_TASK_PARAM);
    taskParam->abilityName_ = "ability1";
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), (int32_t)ERR_BGTASK_INVALID_BGMODE);
    taskParam->bgModeId_ = 9;
    taskParam->bgModeIds_.clear();
    EXPECT_NE((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), -1);
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
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(nullptr), (int32_t)ERR_BGTASK_CHECK_TASK_PARAM);
    sptr<ContinuousTaskParam> taskParam = new (std::nothrow) ContinuousTaskParam();
    EXPECT_NE(taskParam, nullptr);
    taskParam->appName_ = "Entry";
    taskParam->isNewApi_ = false;
    taskParam->bgModeId_ = 0;
    taskParam->abilityName_ = "";
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), (int32_t)ERR_BGTASK_CHECK_TASK_PARAM);
    taskParam->abilityName_ = "ability1";
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), (int32_t)ERR_OK);
}

/**
 * @tc.name: StartAndUpdateBackgroundRunning_001
 * @tc.desc: use batch api.
 * @tc.type: FUNC
 * @tc.require: issueI94UH9 issueI99HSB
 */
HWTEST_F(BgContinuousTaskMgrTest, StartAndUpdateBackgroundRunning_001, TestSize.Level1)
{
    // 1 modes is empty
    sptr<ContinuousTaskParam> taskParam1 = new (std::nothrow) ContinuousTaskParam(true, 0,
        std::make_shared<AbilityRuntime::WantAgent::WantAgent>(),
        "ability1", nullptr, "Entry", true, {});
    EXPECT_NE(taskParam1, nullptr);
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam1), (int32_t)ERR_BGTASK_CHECK_TASK_PARAM);

    // 2 set configure mode is CONFIGURE_ALL_MODES
    bgContinuousTaskMgr_->cachedBundleInfos_.clear();
    EXPECT_EQ(bgContinuousTaskMgr_->GetBackgroundModeInfo(1, "abilityName"), 0u);
    CachedBundleInfo info = CachedBundleInfo();
    info.abilityBgMode_["ability1"] = CONFIGURE_ALL_MODES;
    info.appName_ = "Entry";
    bgContinuousTaskMgr_->cachedBundleInfos_.emplace(1, info);

    // 3 start ok
    sptr<ContinuousTaskParam> taskParam2 = new (std::nothrow) ContinuousTaskParam(true, 0,
        std::make_shared<AbilityRuntime::WantAgent::WantAgent>(),
        "ability1", nullptr, "Entry", true, {1, 2, 3}, 1);
    EXPECT_NE(taskParam2, nullptr);
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam2), (int32_t)ERR_OK);

    // 4 update ok
    sptr<ContinuousTaskParam> taskParam3 = new (std::nothrow) ContinuousTaskParam(true, 0,
        std::make_shared<AbilityRuntime::WantAgent::WantAgent>(),
        "ability1", nullptr, "Entry", true, {4}, 1);
    EXPECT_NE(taskParam3, nullptr);
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->UpdateBackgroundRunning(taskParam3), (int32_t)ERR_OK);

    // 5 update invalid
    sptr<ContinuousTaskParam> taskParam4 = new (std::nothrow) ContinuousTaskParam(true, 0,
        std::make_shared<AbilityRuntime::WantAgent::WantAgent>(),
        "ability1", nullptr, "Entry", true, {10}, 1);
    EXPECT_NE(taskParam4, nullptr);
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->UpdateBackgroundRunning(taskParam4), (int32_t)ERR_BGTASK_INVALID_BGMODE);

    // 6 stop ok
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning(taskParam2->abilityName_, 1), (int32_t)ERR_OK);

    // 7 no start then update error
    sptr<ContinuousTaskParam> taskParam5 = new (std::nothrow) ContinuousTaskParam(true, 0,
        std::make_shared<AbilityRuntime::WantAgent::WantAgent>(),
        "ability1", nullptr, "Entry", true, {1, 2});
    EXPECT_NE(taskParam5, nullptr);
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->UpdateBackgroundRunning(taskParam5), (int32_t)ERR_BGTASK_OBJECT_NOT_EXIST);
}

/**
 * @tc.name: StopBackgroundRunning_001
 * @tc.desc: stop background runnging test.
 * @tc.type: FUNC
 * @tc.require: SR000GGT7V AR000GH6ES AR000GH6EM AR000GH6G9 AR000GH56K issueI99HSB
 */
HWTEST_F(BgContinuousTaskMgrTest, StopBackgroundRunning_001, TestSize.Level1)
{
    sptr<ContinuousTaskParam> taskParam = new (std::nothrow) ContinuousTaskParam();
    EXPECT_NE(taskParam, nullptr);
    taskParam->appName_ = "Entry";
    taskParam->wantAgent_ = std::make_shared<AbilityRuntime::WantAgent::WantAgent>();
    taskParam->abilityName_ = "ability1";
    taskParam->bgModeId_ = 4;
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("", 1), (int32_t)ERR_BGTASK_INVALID_PARAM);
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("ability1", -1), (int32_t)ERR_BGTASK_INVALID_PARAM);
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("ability1", 1),
        (int32_t)ERR_BGTASK_OBJECT_NOT_EXIST);
    SleepForFC();
    bgContinuousTaskMgr_->StartBackgroundRunning(taskParam);
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("ability1", 1), (int32_t)ERR_OK);
}

/**
 * @tc.name: StopBackgroundRunning_002
 * @tc.desc: stop background runnging test.
 * @tc.type: FUNC
 * @tc.require: issues#I8FWJH issueI99HSB
 */
HWTEST_F(BgContinuousTaskMgrTest, StopBackgroundRunning_002, TestSize.Level1)
{
    sptr<ContinuousTaskParam> taskParam = new (std::nothrow) ContinuousTaskParam();
    EXPECT_NE(taskParam, nullptr);
    taskParam->appName_ = "Entry";
    taskParam->wantAgent_ = std::make_shared<AbilityRuntime::WantAgent::WantAgent>();
    taskParam->abilityName_ = "ability1";
    taskParam->bgModeId_ = 2;
    taskParam->abilityId_ = 1;
    bgContinuousTaskMgr_->StartBackgroundRunning(taskParam);
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("ability1", 1), (int32_t)ERR_OK);
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
}

/**
 * @tc.name: UnsubscribeContinuousTask_001
 * @tc.desc: unsubscribe continuous task event callback test.
 * @tc.type: FUNC
 * @tc.require: SR000GGT7U AR000GH6EM AR000GH6G9 AR000GH6ET
 */
HWTEST_F(BgContinuousTaskMgrTest, UnsubscribeContinuousTask_001, TestSize.Level1)
{
    auto subscriber = new (std::nothrow) TestBackgroundTaskSubscriber();
    EXPECT_NE(subscriber, nullptr);
    bgContinuousTaskMgr_->AddSubscriber(subscriber->GetImpl());
    SleepForFC();
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->RemoveSubscriber(subscriber->GetImpl()), (int32_t)ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_002
 * @tc.desc: test SetCachedBundleInfo.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_002, TestSize.Level1)
{
    EXPECT_FALSE(bgContinuousTaskMgr_->SetCachedBundleInfo(1, 1, "false-test", "test"));
    EXPECT_FALSE(bgContinuousTaskMgr_->SetCachedBundleInfo(1, 1, "empty-info", "test"));
    EXPECT_TRUE(bgContinuousTaskMgr_->SetCachedBundleInfo(1, 1, "valid", "test"));
}

/**
 * @tc.name: BgTaskManagerUnitTest_003
 * @tc.desc: test CheckBgmodeType.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_003, TestSize.Level1)
{
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(0, 1, false, 1ULL), ERR_BGMODE_NULL_OR_TYPE_ERR);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(1, 1, false, 1ULL), ERR_OK);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(BGMODE_WIFI_INTERACTION, BGMODE_WIFI_INTERACTION_ID,
        true, NO_SYSTEM_APP_TOKEN_ID), ERR_BGTASK_NOT_SYSTEM_APP);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(BGMODE_VOIP, BGMODE_VOIP_ID, true, NO_SYSTEM_APP_TOKEN_ID),
        ERR_BGTASK_NOT_SYSTEM_APP);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(BGMODE_WIFI_INTERACTION, BGMODE_WIFI_INTERACTION_ID,
        true, 1ULL), ERR_OK);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(BGMODE_VOIP, BGMODE_VOIP_ID, true, 1ULL), ERR_OK);
    if (SUPPORT_TASK_KEEPING) {
        EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(PC_BGMODE_TASK_KEEPING, BGMODE_TASK_KEEPING_ID, true, 1ULL),
            ERR_OK);
    } else {
        EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(PC_BGMODE_TASK_KEEPING, BGMODE_TASK_KEEPING_ID, true, 1ULL),
            ERR_BGTASK_KEEPING_TASK_VERIFY_ERR);
    }
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(BGMODE_VOIP, BGMODE_VOIP_ID, true, 1ULL), ERR_OK);
    bgContinuousTaskMgr_->deviceType_ = "default";
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(LOCATION_BGMODE, LOCATION_BGMODE_ID, true, 1ULL), ERR_OK);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(BLUETOOTH_INTERACTION, LOCATION_BGMODE_ID, true, 1ULL),
        ERR_BGTASK_INVALID_BGMODE);
}

/**
 * @tc.name: BgTaskManagerUnitTest_004
 * @tc.desc: test GetBackgroundModeInfo.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_004, TestSize.Level1)
{
    bgContinuousTaskMgr_->cachedBundleInfos_.clear();
    EXPECT_EQ(bgContinuousTaskMgr_->GetBackgroundModeInfo(1, "abilityName"), 0u);

    CachedBundleInfo info = CachedBundleInfo();
    info.abilityBgMode_["abilityName"] = 1;
    info.appName_ = "appName";
    bgContinuousTaskMgr_->cachedBundleInfos_.emplace(1, info);

    EXPECT_EQ(bgContinuousTaskMgr_->GetBackgroundModeInfo(1, "test"), 0u);
    EXPECT_EQ(bgContinuousTaskMgr_->GetBackgroundModeInfo(1, "abilityName"), 1u);
    bgContinuousTaskMgr_->cachedBundleInfos_.clear();
}

/**
 * @tc.name: BgTaskManagerUnitTest_005
 * @tc.desc: test SendContinuousTaskNotification.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_005, TestSize.Level1)
{
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord->bgModeId_ = 1;
    continuousTaskRecord->bgModeIds_.clear();
    continuousTaskRecord->bgModeIds_.push_back(continuousTaskRecord->bgModeId_);
    continuousTaskRecord->isNewApi_ = false;
    EXPECT_EQ(bgContinuousTaskMgr_->SendContinuousTaskNotification(continuousTaskRecord),
        ERR_BGTASK_NOTIFICATION_VERIFY_FAILED);
    continuousTaskRecord->isNewApi_ = true;
    EXPECT_EQ(bgContinuousTaskMgr_->SendContinuousTaskNotification(continuousTaskRecord),
        ERR_BGTASK_NOTIFICATION_VERIFY_FAILED);
    continuousTaskRecord->bgModeId_ = INVALID_BGMODE_ID;
    continuousTaskRecord->bgModeIds_.clear();
    continuousTaskRecord->bgModeIds_.push_back(continuousTaskRecord->bgModeId_);
    EXPECT_EQ(bgContinuousTaskMgr_->SendContinuousTaskNotification(continuousTaskRecord),
        ERR_BGTASK_NOTIFICATION_VERIFY_FAILED);

    CachedBundleInfo info = CachedBundleInfo();
    info.abilityBgMode_["abilityName"] = 1;
    info.appName_ = "appName";
    bgContinuousTaskMgr_->cachedBundleInfos_.emplace(1, info);
    continuousTaskRecord->uid_ = 1;
    EXPECT_EQ(bgContinuousTaskMgr_->SendContinuousTaskNotification(continuousTaskRecord),
        ERR_BGTASK_NOTIFICATION_VERIFY_FAILED);
    continuousTaskRecord->bgModeId_ = 1;
    continuousTaskRecord->bgModeIds_.clear();
    continuousTaskRecord->bgModeIds_.push_back(continuousTaskRecord->bgModeId_);
    EXPECT_EQ(bgContinuousTaskMgr_->SendContinuousTaskNotification(continuousTaskRecord), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_006
 * @tc.desc: test StopContinuousTask.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_006, TestSize.Level1)
{
    bgContinuousTaskMgr_->isSysReady_.store(false);
    bgContinuousTaskMgr_->StopContinuousTask(1, 1, 1, "");
    bgContinuousTaskMgr_->isSysReady_.store(true);
    bgContinuousTaskMgr_->StopContinuousTask(-1, 1, 1, "");
    SleepForFC();
    bgContinuousTaskMgr_->StopContinuousTask(1, 1, 1, "");
    SleepForFC();

    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord1 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->uid_ = TEST_NUM_ONE;
    continuousTaskRecord1->bgModeId_ = TEST_NUM_TWO;
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord2 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord2->uid_ = TEST_NUM_ONE;
    continuousTaskRecord2->bgModeId_ = TEST_NUM_THREE;
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord3 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord3->uid_ = TEST_NUM_TWO;
    continuousTaskRecord3->bgModeId_ = TEST_NUM_TWO;
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord4 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord4->uid_ = TEST_NUM_TWO;
    continuousTaskRecord4->bgModeId_ = TEST_NUM_THREE;

    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key2"] = continuousTaskRecord2;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key3"] = continuousTaskRecord3;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key4"] = continuousTaskRecord4;
    bgContinuousTaskMgr_->StopContinuousTask(-1, 1, 1, "");
    SleepForFC();
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();

    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key2"] = continuousTaskRecord2;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key3"] = continuousTaskRecord3;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key4"] = continuousTaskRecord4;
    bgContinuousTaskMgr_->StopContinuousTask(1, 1, 1, "");
    SleepForFC();
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_007
 * @tc.desc: test GetContinuousTaskApps.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_007, TestSize.Level1)
{
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    bgContinuousTaskMgr_->isSysReady_.store(false);
    std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> list;
    EXPECT_EQ(bgContinuousTaskMgr_->GetContinuousTaskApps(list), ERR_BGTASK_SYS_NOT_READY);
    bgContinuousTaskMgr_->isSysReady_.store(true);
    EXPECT_EQ(bgContinuousTaskMgr_->GetContinuousTaskApps(list), ERR_OK);


    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key"] = continuousTaskRecord;
    EXPECT_EQ(bgContinuousTaskMgr_->GetContinuousTaskApps(list), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_008
 * @tc.desc: test ShellDump.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_008, TestSize.Level1)
{
    bgContinuousTaskMgr_->isSysReady_.store(false);
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    std::vector<std::string> dumpOption;
    dumpOption.emplace_back("-C");
    std::vector<std::string> dumpInfo;
    EXPECT_EQ(bgContinuousTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_BGTASK_SYS_NOT_READY);
    bgContinuousTaskMgr_->isSysReady_.store(true);
    dumpOption.emplace_back("--all");
    EXPECT_EQ(bgContinuousTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_OK);
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key"] = continuousTaskRecord;
    EXPECT_EQ(bgContinuousTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_OK);
    dumpOption.pop_back();
    dumpOption.emplace_back("--cancel_all");
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    EXPECT_EQ(bgContinuousTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_OK);
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key"] = continuousTaskRecord;
    EXPECT_EQ(bgContinuousTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_OK);
    dumpOption.pop_back();
    dumpOption.emplace_back("--cancel");
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    EXPECT_EQ(bgContinuousTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_OK);
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key"] = continuousTaskRecord;
    EXPECT_EQ(bgContinuousTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_OK);
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    dumpOption.pop_back();
    dumpOption.emplace_back("invalid");
    EXPECT_EQ(bgContinuousTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_009
 * @tc.desc: test RemoveContinuousTaskRecord.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_009, TestSize.Level1)
{
    EXPECT_FALSE(bgContinuousTaskMgr_->RemoveContinuousTaskRecord("key"));
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key"] = continuousTaskRecord;
    EXPECT_TRUE(bgContinuousTaskMgr_->RemoveContinuousTaskRecord("key"));
}

/**
 * @tc.name: BgTaskManagerUnitTest_010
 * @tc.desc: test StopContinuousTaskByUser.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_010, TestSize.Level1)
{
    bgContinuousTaskMgr_->isSysReady_.store(false);
    EXPECT_FALSE(bgContinuousTaskMgr_->StopContinuousTaskByUser("key"));
    bgContinuousTaskMgr_->isSysReady_.store(true);
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key"] = continuousTaskRecord;
    EXPECT_TRUE(bgContinuousTaskMgr_->StopContinuousTaskByUser("key"));
}

/**
 * @tc.name: BgTaskManagerUnitTest_011
 * @tc.desc: test OnRemoteSubscriberDied.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_011, TestSize.Level1)
{
    bgContinuousTaskMgr_->isSysReady_.store(false);
    bgContinuousTaskMgr_->OnRemoteSubscriberDied(nullptr);
    bgContinuousTaskMgr_->isSysReady_.store(true);
    bgContinuousTaskMgr_->OnRemoteSubscriberDied(nullptr);
    TestBackgroundTaskSubscriber subscriber1 = TestBackgroundTaskSubscriber();
    TestBackgroundTaskSubscriber subscriber2 = TestBackgroundTaskSubscriber();
    bgContinuousTaskMgr_->bgTaskSubscribers_.emplace_back(subscriber1.GetImpl());
    bgContinuousTaskMgr_->bgTaskSubscribers_.emplace_back(subscriber2.GetImpl());
    bgContinuousTaskMgr_->OnRemoteSubscriberDied(subscriber1.GetImpl());
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_012
 * @tc.desc: test OnAbilityStateChanged.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V issueI99HSB
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_012, TestSize.Level1)
{
    bgContinuousTaskMgr_->isSysReady_.store(false);
    bgContinuousTaskMgr_->OnAbilityStateChanged(-1, "test", -1);
    bgContinuousTaskMgr_->isSysReady_.store(true);
    bgContinuousTaskMgr_->OnAbilityStateChanged(-1, "test", -1);

    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    bgContinuousTaskMgr_->bgTaskSubscribers_.clear();
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord->uid_ = 1;
    continuousTaskRecord->abilityName_ = "test";
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key"] = continuousTaskRecord;
    bgContinuousTaskMgr_->OnAbilityStateChanged(-1, "test", -1);
    bgContinuousTaskMgr_->OnAbilityStateChanged(1, "test1", -1);
    bgContinuousTaskMgr_->OnAbilityStateChanged(-1, "test1", -1);
    bgContinuousTaskMgr_->OnAbilityStateChanged(1, "test", -1);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_013
 * @tc.desc: test OnProcessDied.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_013, TestSize.Level1)
{
    bgContinuousTaskMgr_->isSysReady_.store(false);
    bgContinuousTaskMgr_->OnProcessDied(-1, 1);
    bgContinuousTaskMgr_->isSysReady_.store(true);
    bgContinuousTaskMgr_->OnProcessDied(-1, 1);

    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord->pid_ = 1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key"] = continuousTaskRecord;
    bgContinuousTaskMgr_->OnProcessDied(-1, -1);
    bgContinuousTaskMgr_->OnProcessDied(-1, 1);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_014
 * @tc.desc: test OnContinuousTaskChanged.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_014, TestSize.Level1)
{
    bgContinuousTaskMgr_->OnContinuousTaskChanged(nullptr, ContinuousTaskEventTriggerType::TASK_START);
    bgContinuousTaskMgr_->bgTaskSubscribers_.clear();
    auto continuousTaskInfo =  std::make_shared<ContinuousTaskRecord>();
    bgContinuousTaskMgr_->OnContinuousTaskChanged(continuousTaskInfo, ContinuousTaskEventTriggerType::TASK_START);
    TestBackgroundTaskSubscriber subscriber = TestBackgroundTaskSubscriber();
    bgContinuousTaskMgr_->bgTaskSubscribers_.emplace_back(subscriber.GetImpl());
    bgContinuousTaskMgr_->OnContinuousTaskChanged(continuousTaskInfo, ContinuousTaskEventTriggerType::TASK_START);
    bgContinuousTaskMgr_->OnContinuousTaskChanged(continuousTaskInfo, ContinuousTaskEventTriggerType::TASK_UPDATE);
    bgContinuousTaskMgr_->OnContinuousTaskChanged(continuousTaskInfo, ContinuousTaskEventTriggerType::TASK_CANCEL);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_015
 * @tc.desc: test OnBundleInfoChanged.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_015, TestSize.Level1)
{
    bgContinuousTaskMgr_->isSysReady_.store(false);
    bgContinuousTaskMgr_->OnBundleInfoChanged("action", "bundleName", -1);
    bgContinuousTaskMgr_->isSysReady_.store(true);
    bgContinuousTaskMgr_->OnBundleInfoChanged(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED,
        "bundleName", -1);

    bgContinuousTaskMgr_->OnBundleInfoChanged(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED,
        "bundleName", -1);

    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    bgContinuousTaskMgr_->bgTaskSubscribers_.clear();
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord1 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->uid_ = TEST_NUM_ONE;
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord2 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord2->uid_ = TEST_NUM_TWO;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key2"] = continuousTaskRecord2;
    bgContinuousTaskMgr_->OnBundleInfoChanged(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED,
        "bundleName", 1);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_016
 * @tc.desc: test OnAccountsStateChanged.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_016, TestSize.Level1)
{
    bgContinuousTaskMgr_->OnAccountsStateChanged(1);
    bgContinuousTaskMgr_->OnAccountsStateChanged(1);

    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord1 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->userId_ = 1;
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord2 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord2->userId_ = DEFAULT_USERID;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key2"] = continuousTaskRecord2;
    bgContinuousTaskMgr_->OnAccountsStateChanged(1);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_017
 * @tc.desc: test HandleAppContinuousTaskStop.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_017, TestSize.Level1)
{
    bgContinuousTaskMgr_->HandleAppContinuousTaskStop(1);
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord->uid_ = 1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key"] = continuousTaskRecord;
    bgContinuousTaskMgr_->HandleAppContinuousTaskStop(1);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_033
 * @tc.desc: test CheckPersistenceData.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_033, TestSize.Level1)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord1 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->pid_ = TEST_NUM_ONE;
    continuousTaskRecord1->notificationLabel_ = "label1";
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord2 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->pid_ = TEST_NUM_TWO;
    continuousTaskRecord1->notificationLabel_ = "label1";
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord3 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->pid_ = TEST_NUM_ONE;
    continuousTaskRecord1->notificationLabel_ = "label2";
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord4 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->pid_ = TEST_NUM_TWO;
    continuousTaskRecord1->notificationLabel_ = "label2";

    std::vector<AppExecFwk::RunningProcessInfo> allProcesses;
    AppExecFwk::RunningProcessInfo processInfo1;
    processInfo1.pid_ = TEST_NUM_ONE;
    std::set<std::string> allLabels;
    allLabels.emplace("label1");

    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key2"] = continuousTaskRecord2;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key3"] = continuousTaskRecord3;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key4"] = continuousTaskRecord4;
    bgContinuousTaskMgr_->CheckPersistenceData(allProcesses, allLabels);
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key2"] = continuousTaskRecord2;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key3"] = continuousTaskRecord3;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key4"] = continuousTaskRecord4;
    bgContinuousTaskMgr_->CheckPersistenceData(allProcesses, allLabels);
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key2"] = continuousTaskRecord2;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key3"] = continuousTaskRecord3;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key4"] = continuousTaskRecord4;
    bgContinuousTaskMgr_->CheckPersistenceData(allProcesses, allLabels);
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key2"] = continuousTaskRecord2;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key3"] = continuousTaskRecord3;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key4"] = continuousTaskRecord4;
    bgContinuousTaskMgr_->CheckPersistenceData(allProcesses, allLabels);
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->continuousTaskInfosMap_.size(), 0);
#endif
}

/**
 * @tc.name: BgTaskManagerUnitTest_034
 * @tc.desc: test HandleStopContinuousTask.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_034, TestSize.Level1)
{
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord1 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->uid_ = TEST_NUM_ONE;
    continuousTaskRecord1->bgModeId_ = TEST_NUM_ONE;
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord2 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->uid_ = TEST_NUM_ONE;
    continuousTaskRecord1->bgModeId_ = TEST_NUM_TWO;

    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key2"] = continuousTaskRecord2;
    bgContinuousTaskMgr_->HandleStopContinuousTask(TEST_NUM_ONE, TEST_NUM_ONE, TEST_NUM_ONE, "");
    EXPECT_NE((int32_t)bgContinuousTaskMgr_->continuousTaskInfosMap_.size(), 0);
}

/**
 * @tc.name: BgTaskManagerUnitTest_035
 * @tc.desc: test HandleStopContinuousTask.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_035, TestSize.Level1)
{
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    bgContinuousTaskMgr_->HandleStopContinuousTask(0, 0, 0, "");
    EXPECT_EQ(bgContinuousTaskMgr_->continuousTaskInfosMap_.size(), 0);

    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord1 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->uid_ = 1;
    continuousTaskRecord1->bgModeIds_ = {1};
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    EXPECT_EQ(bgContinuousTaskMgr_->continuousTaskInfosMap_.size(), 1);

    bgContinuousTaskMgr_->HandleStopContinuousTask(1, 0, 1, "");
    EXPECT_EQ(bgContinuousTaskMgr_->continuousTaskInfosMap_.size(), 0);

    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    EXPECT_EQ(bgContinuousTaskMgr_->continuousTaskInfosMap_.size(), 1);
    bgContinuousTaskMgr_->HandleStopContinuousTask(1, 0, 0xFF, "");
    EXPECT_EQ(bgContinuousTaskMgr_->continuousTaskInfosMap_.size(), 0);

    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    EXPECT_EQ(bgContinuousTaskMgr_->continuousTaskInfosMap_.size(), 1);
    bgContinuousTaskMgr_->HandleStopContinuousTask(1, 0, 0, "key1");
    EXPECT_EQ(bgContinuousTaskMgr_->continuousTaskInfosMap_.size(), 0);
}

/**
 * @tc.name: BgTaskManagerUnitTest_036
 * @tc.desc: test SubscriberChange.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_036, TestSize.Level1)
{
    TestBackgroundTaskSubscriber subscriber1 = TestBackgroundTaskSubscriber();
    bgContinuousTaskMgr_->AddSubscriberInner(subscriber1.GetImpl());
    EXPECT_EQ(bgContinuousTaskMgr_->AddSubscriberInner(subscriber1.GetImpl()), ERR_BGTASK_OBJECT_EXISTS);
    sptr<BackgroundTaskSubscriberProxy> subscirberProxy1
        = sptr<BackgroundTaskSubscriberProxy>(new BackgroundTaskSubscriberProxy(nullptr));
    EXPECT_EQ(bgContinuousTaskMgr_->AddSubscriberInner(subscirberProxy1), ERR_BGTASK_INVALID_PARAM);

    bgContinuousTaskMgr_->bgTaskSubscribers_.clear();
    EXPECT_EQ(bgContinuousTaskMgr_->RemoveSubscriberInner(subscirberProxy1), ERR_BGTASK_INVALID_PARAM);
    EXPECT_EQ(bgContinuousTaskMgr_->RemoveSubscriberInner(subscriber1.GetImpl()), ERR_BGTASK_INVALID_PARAM);
}

/**
 * @tc.name: BgTaskManagerUnitTest_037
 * @tc.desc: test DumpAllTaskInfo.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_037, TestSize.Level1)
{
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord1 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->pid_ = TEST_NUM_ONE;
    continuousTaskRecord1->notificationLabel_ = "label1";

    std::shared_ptr<WantAgentInfo> info = std::make_shared<WantAgentInfo>();
    info->bundleName_ = "wantAgentBundleName";
    info->abilityName_ = "wantAgentAbilityName";
    continuousTaskRecord1->wantAgentInfo_ = info;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    std::vector<std::string> dumpInfo;
    bgContinuousTaskMgr_->DumpAllTaskInfo(dumpInfo);
    EXPECT_NE((int32_t)dumpInfo.size(), 0);
}

/**
 * @tc.name: BgTaskManagerUnitTest_038
 * @tc.desc: test DumpCancelTask.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_038, TestSize.Level1)
{
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord1 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->pid_ = TEST_NUM_ONE;
    continuousTaskRecord1->notificationLabel_ = "label1";

    std::vector<std::string> dumpOption;
    dumpOption.emplace_back("param1");
    dumpOption.emplace_back("param2");
    dumpOption.emplace_back("key1");

    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    bgContinuousTaskMgr_->DumpCancelTask(dumpOption, false);
    dumpOption.pop_back();
    dumpOption.emplace_back("key2");
    bgContinuousTaskMgr_->DumpCancelTask(dumpOption, false);
    EXPECT_NE((int32_t)dumpOption.size(), 0);
}

/**
 * @tc.name: BgTaskManagerUnitTest_039
 * @tc.desc: test OnConfigurationChanged.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_039, TestSize.Level1)
{
    AppExecFwk::Configuration configuration;
    bgContinuousTaskMgr_->isSysReady_.store(false);
    bgContinuousTaskMgr_->OnConfigurationChanged(configuration);
    bgContinuousTaskMgr_->isSysReady_.store(true);

    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord1 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->bgModeId_ = TEST_NUM_ONE;
    continuousTaskRecord1->isNewApi_ = true;

    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord2 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord2->bgModeId_ = INVALID_BGMODE_ID;
    continuousTaskRecord1->isNewApi_ = true;

    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key2"] = continuousTaskRecord2;
    EXPECT_NE((int32_t)bgContinuousTaskMgr_->continuousTaskInfosMap_.size(), 0);
}

/**
 * @tc.name: BgTaskManagerUnitTest_043
 * @tc.desc: test RequestBackgroundRunningForInner.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_043, TestSize.Level1)
{
    bgContinuousTaskMgr_->isSysReady_.store(false);
    EXPECT_EQ(bgContinuousTaskMgr_->RequestBackgroundRunningForInner(nullptr), ERR_BGTASK_SYS_NOT_READY);
    bgContinuousTaskMgr_->isSysReady_.store(true);
    EXPECT_EQ(bgContinuousTaskMgr_->RequestBackgroundRunningForInner(nullptr), ERR_BGTASK_CHECK_TASK_PARAM);
    sptr<ContinuousTaskParamForInner> taskParam = sptr<ContinuousTaskParamForInner>(
        new ContinuousTaskParamForInner(1, 1, true));
    EXPECT_EQ(bgContinuousTaskMgr_->StartBackgroundRunningForInner(taskParam), ERR_OK);
    taskParam->isStart_ = false;
    EXPECT_EQ(bgContinuousTaskMgr_->StopBackgroundRunningForInner(taskParam), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_044
 * @tc.desc: test CheckBgmodeTypeForInner.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_044, TestSize.Level1)
{
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeTypeForInner(0), ERR_BGTASK_INVALID_BGMODE);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeTypeForInner(1), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_045
 * @tc.desc: test CheckProcessUidInfo.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_045, TestSize.Level1)
{
    std::vector<AppExecFwk::RunningProcessInfo> allProcesses;

    AppExecFwk::RunningProcessInfo info1;
    info1.uid_ = TEST_NUM_ONE;
    allProcesses.push_back(info1);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckProcessUidInfo(allProcesses, TEST_NUM_TWO), false);
    AppExecFwk::RunningProcessInfo info2;
    info2.uid_ = TEST_NUM_TWO;
    allProcesses.push_back(info2);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckProcessUidInfo(allProcesses, TEST_NUM_TWO), true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_046
 * @tc.desc: test SendContinuousTaskNotification.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_046, TestSize.Level1)
{
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord->bgModeId_ = 2;
    continuousTaskRecord->isNewApi_ = true;
    continuousTaskRecord->uid_ = 1;

    CachedBundleInfo info = CachedBundleInfo();
    info.abilityBgMode_["abilityName"] = 2;
    info.appName_ = "appName";
    bgContinuousTaskMgr_->cachedBundleInfos_.emplace(1, info);

    EXPECT_EQ(bgContinuousTaskMgr_->SendContinuousTaskNotification(continuousTaskRecord), ERR_OK);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS