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

#include "gtest/gtest.h"
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
static constexpr char BG_TASK_SUB_MODE_TYPE[] = "subMode";
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
    std::fill_n(std::back_inserter(bgContinuousTaskMgr_->continuousTaskSubText_), PROMPT_NUMS, "bgmsubmode_test");
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
    taskParam->abilityId_ = 1;
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
    CachedBundleInfo info = CachedBundleInfo();
    info.abilityBgMode_["ability1"] = CONFIGURE_ALL_MODES;
    info.appName_ = "Entry";
    bgContinuousTaskMgr_->cachedBundleInfos_.clear();
    bgContinuousTaskMgr_->cachedBundleInfos_.emplace(1, info);
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
    CachedBundleInfo info = CachedBundleInfo();
    info.abilityBgMode_["ability1"] = CONFIGURE_ALL_MODES;
    info.appName_ = "Entry";
    bgContinuousTaskMgr_->cachedBundleInfos_.clear();
    bgContinuousTaskMgr_->cachedBundleInfos_.emplace(1, info);
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), (int32_t)ERR_OK);
}

/**
 * @tc.name: StartBackgroundRunning_003
 * @tc.desc: start background runnging by abilityIds test.
 * @tc.type: FUNC
 * @tc.require: issueI99HSB
 */
HWTEST_F(BgContinuousTaskMgrTest, StartBackgroundRunning_003, TestSize.Level1)
{
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    int taskSize = 0;
    bgContinuousTaskMgr_->cachedBundleInfos_.clear();
    CachedBundleInfo info = CachedBundleInfo();
    info.abilityBgMode_["ability1"] = CONFIGURE_ALL_MODES;
    info.appName_ = "Entry";
    bgContinuousTaskMgr_->cachedBundleInfos_.emplace(1, info);

    // start one task by abilityId is 1
    sptr<ContinuousTaskParam> taskParam1 = new (std::nothrow) ContinuousTaskParam(true, 0,
        std::make_shared<AbilityRuntime::WantAgent::WantAgent>(),
        "ability1", nullptr, "Entry", true, {1, 2, 3}, 1);
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam1), (int32_t)ERR_OK);
    
    taskSize = bgContinuousTaskMgr_->continuousTaskInfosMap_.size();
    EXPECT_EQ(taskSize, 1);

    int32_t abilityId = -1;
    std::unordered_map<std::string, std::shared_ptr<ContinuousTaskRecord>>::iterator iter;
    for (iter = bgContinuousTaskMgr_->continuousTaskInfosMap_.begin();
        iter != bgContinuousTaskMgr_->continuousTaskInfosMap_.end(); ++iter) {
        abilityId = iter->second->GetAbilityId();
    }
    EXPECT_EQ(abilityId, 1);

    // start one task by abilityId is 2
    sptr<ContinuousTaskParam> taskParam2 = new (std::nothrow) ContinuousTaskParam(true, 0,
        std::make_shared<AbilityRuntime::WantAgent::WantAgent>(),
        "ability1", nullptr, "Entry", true, {1, 2, 3}, 2);
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam2), (int32_t)ERR_OK);
    
    taskSize = bgContinuousTaskMgr_->continuousTaskInfosMap_.size();
    EXPECT_EQ(taskSize, 2);

    // agent start one task by abilityId is 2
    sptr<ContinuousTaskParam> taskParam3 = new (std::nothrow) ContinuousTaskParam(true, 0,
        std::make_shared<AbilityRuntime::WantAgent::WantAgent>(),
        "ability1", nullptr, "Entry", true, {1, 2, 3}, 2);
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StartBackgroundRunning(taskParam3),
        (int32_t)ERR_BGTASK_OBJECT_EXISTS);
    
    taskSize = bgContinuousTaskMgr_->continuousTaskInfosMap_.size();
    EXPECT_EQ(taskSize, 2);

    // stop one task by abilityId is 1
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning(taskParam1->abilityName_, 1), (int32_t)ERR_OK);

    taskSize = bgContinuousTaskMgr_->continuousTaskInfosMap_.size();
    EXPECT_EQ(taskSize, 1);

    // stop one task by abilityId is 2
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning(taskParam2->abilityName_, 2), (int32_t)ERR_OK);

    taskSize = bgContinuousTaskMgr_->continuousTaskInfosMap_.size();
    EXPECT_EQ(taskSize, 0);

    // agent stop one task by abilityId is 2
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning(taskParam2->abilityName_, 2),
        (int32_t)ERR_BGTASK_OBJECT_NOT_EXIST);

    taskSize = bgContinuousTaskMgr_->continuousTaskInfosMap_.size();
    EXPECT_EQ(taskSize, 0);
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
    taskParam->abilityId_ = 1;
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->StopBackgroundRunning("", 1), (int32_t)ERR_BGTASK_INVALID_PARAM);
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
    auto info = std::make_shared<SubscriberInfo>(subscriber->GetImpl(), 1, 1, 0);
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->AddSubscriber(info), (int32_t)ERR_OK);
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
    auto info = std::make_shared<SubscriberInfo>(subscriber->GetImpl(), 1, 1, 0);
    bgContinuousTaskMgr_->AddSubscriber(info);
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
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord->bundleName_ = "bundleName";
    continuousTaskRecord->isNewApi_ = false;
    continuousTaskRecord->fullTokenId_ = 1ULL;
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(0, 1, false, continuousTaskRecord), ERR_BGMODE_NULL_OR_TYPE_ERR);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(1, 1, false, continuousTaskRecord), ERR_OK);
    continuousTaskRecord->fullTokenId_ = NO_SYSTEM_APP_TOKEN_ID;
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(BGMODE_WIFI_INTERACTION, BGMODE_WIFI_INTERACTION_ID,
        true, continuousTaskRecord), ERR_BGTASK_NOT_SYSTEM_APP);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(BGMODE_VOIP, BGMODE_VOIP_ID, true, continuousTaskRecord), ERR_OK);
    continuousTaskRecord->fullTokenId_ = 1ULL;
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(BGMODE_WIFI_INTERACTION, BGMODE_WIFI_INTERACTION_ID,
        true, continuousTaskRecord), ERR_OK);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(BGMODE_VOIP, BGMODE_VOIP_ID, true, continuousTaskRecord), ERR_OK);
    if (SUPPORT_TASK_KEEPING) {
        EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(PC_BGMODE_TASK_KEEPING, BGMODE_TASK_KEEPING_ID, true,
        continuousTaskRecord), ERR_OK);
    } else {
        EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(PC_BGMODE_TASK_KEEPING, BGMODE_TASK_KEEPING_ID, true,
        continuousTaskRecord), ERR_BGTASK_KEEPING_TASK_VERIFY_ERR);
    }
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(BGMODE_VOIP, BGMODE_VOIP_ID, true, continuousTaskRecord), ERR_OK);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(LOCATION_BGMODE, LOCATION_BGMODE_ID, true, continuousTaskRecord),
        ERR_OK);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeType(BLUETOOTH_INTERACTION, LOCATION_BGMODE_ID, true,
        continuousTaskRecord), ERR_BGTASK_INVALID_BGMODE);
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
    auto info1 = std::make_shared<SubscriberInfo>(subscriber1.GetImpl(), 1, 1, 0);
    TestBackgroundTaskSubscriber subscriber2 = TestBackgroundTaskSubscriber();
    auto info2 = std::make_shared<SubscriberInfo>(subscriber2.GetImpl(), 1, 1, 0);
    bgContinuousTaskMgr_->bgTaskSubscribers_.emplace_back(info1);
    bgContinuousTaskMgr_->bgTaskSubscribers_.emplace_back(info2);
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
 * @tc.desc: test OnAppStopped.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V issueIALCBZ
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_013, TestSize.Level1)
{
    bgContinuousTaskMgr_->isSysReady_.store(false);
    bgContinuousTaskMgr_->OnAppStopped(1);
    bgContinuousTaskMgr_->isSysReady_.store(true);
    bgContinuousTaskMgr_->OnAppStopped(1);

    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord->uid_ = 1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key"] = continuousTaskRecord;
    bgContinuousTaskMgr_->OnAppStopped(-1);
    bgContinuousTaskMgr_->OnAppStopped(1);
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
    auto info = std::make_shared<SubscriberInfo>(subscriber.GetImpl(), 1, 1, 0);
    bgContinuousTaskMgr_->bgTaskSubscribers_.emplace_back(info);
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
    continuousTaskRecord2->pid_ = TEST_NUM_TWO;
    continuousTaskRecord2->notificationLabel_ = "label1";
    continuousTaskRecord2->notificationId_ = 100;

    std::vector<AppExecFwk::RunningProcessInfo> allProcesses;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key2"] = continuousTaskRecord2;
    bgContinuousTaskMgr_->CheckPersistenceData(allProcesses);
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->continuousTaskInfosMap_.size(), 0);

    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key2"] = continuousTaskRecord2;
    AppExecFwk::RunningProcessInfo processInfo1;
    processInfo1.pid_ = TEST_NUM_ONE;
    AppExecFwk::RunningProcessInfo processInfo2;
    processInfo2.pid_ = TEST_NUM_TWO;
    allProcesses.push_back(processInfo1);
    allProcesses.push_back(processInfo2);
    bgContinuousTaskMgr_->CheckPersistenceData(allProcesses);
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->continuousTaskInfosMap_.size(), 2);
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
    auto info = std::make_shared<SubscriberInfo>(subscriber1.GetImpl(), 1, 1, 0);
    bgContinuousTaskMgr_->AddSubscriberInner(info);
    EXPECT_EQ(bgContinuousTaskMgr_->AddSubscriberInner(info), ERR_BGTASK_OBJECT_EXISTS);
    bgContinuousTaskMgr_->bgTaskSubscribers_.clear();
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
    EXPECT_EQ(bgContinuousTaskMgr_->CheckBgmodeTypeForInner(10), ERR_OK);
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

/**
 * @tc.name: BgTaskManagerUnitTest_047
 * @tc.desc: test GetNotificationText.
 * @tc.type: FUNC
 * @tc.require: issueIBOIHY
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_047, TestSize.Level1)
{
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord->bgModeIds_.push_back(2);
    EXPECT_NE(bgContinuousTaskMgr_->GetNotificationText(continuousTaskRecord), "");

    continuousTaskRecord->bgModeIds_.push_back(1);
    EXPECT_NE(bgContinuousTaskMgr_->GetNotificationText(continuousTaskRecord), "");
    
    continuousTaskRecord->bgSubModeIds_.push_back(1);
    EXPECT_NE(bgContinuousTaskMgr_->GetNotificationText(continuousTaskRecord), "");
}

/**
 * @tc.name: BgTaskManagerUnitTest_048
 * @tc.desc: test CheckSubMode.
 * @tc.type: FUNC
 * @tc.require: issueIBOIHY
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_048, TestSize.Level1)
{
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    EXPECT_EQ(bgContinuousTaskMgr_->CheckSubMode(want, continuousTaskRecord), ERR_OK);

    want->SetParam(BG_TASK_SUB_MODE_TYPE, 0);
    continuousTaskRecord->bgModeIds_.push_back(1);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckSubMode(want, continuousTaskRecord), ERR_BGTASK_CHECK_TASK_PARAM);

    want->SetParam(BG_TASK_SUB_MODE_TYPE, 1);
    continuousTaskRecord->bgModeIds_.push_back(1);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckSubMode(want, continuousTaskRecord), ERR_BGTASK_CHECK_TASK_PARAM);

    continuousTaskRecord->bgModeIds_.push_back(5);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckSubMode(want, continuousTaskRecord), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_049
 * @tc.desc: test CheckNotificationText.
 * @tc.type: FUNC
 * @tc.require: issueIBOIHY
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_049, TestSize.Level1)
{
    std::string notificationText {""};
    std::shared_ptr<ContinuousTaskRecord> record = std::make_shared<ContinuousTaskRecord>();
    record->bgModeIds_.clear();
    record->bgModeIds_.push_back(2);
    EXPECT_EQ(bgContinuousTaskMgr_->CheckNotificationText(notificationText, record), ERR_OK);

    record->bgModeIds_.clear();
    record->bgModeIds_.push_back(100);
    bgContinuousTaskMgr_->continuousTaskText_.clear();
    EXPECT_EQ(bgContinuousTaskMgr_->CheckNotificationText(notificationText, record),
        ERR_BGTASK_NOTIFICATION_VERIFY_FAILED);

    record->bgModeIds_.clear();
    record->bgModeIds_.push_back(5);
    record->bgSubModeIds_.push_back(1);
    bgContinuousTaskMgr_->continuousTaskSubText_.clear();
    std::fill_n(std::back_inserter(bgContinuousTaskMgr_->continuousTaskText_), PROMPT_NUMS, "bgmmode_test");
    EXPECT_EQ(bgContinuousTaskMgr_->CheckNotificationText(notificationText, record),
        ERR_BGTASK_NOTIFICATION_VERIFY_FAILED);

    std::fill_n(std::back_inserter(bgContinuousTaskMgr_->continuousTaskSubText_), PROMPT_NUMS, "bgmsubmode_test");
    EXPECT_EQ(bgContinuousTaskMgr_->CheckNotificationText(notificationText, record), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_050
 * @tc.desc: test SuspendContinuousTask.
 * @tc.type: FUNC
 * @tc.require: issueIC6B53
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_050, TestSize.Level1)
{
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    bgContinuousTaskMgr_->isSysReady_.store(false);
    bgContinuousTaskMgr_->SuspendContinuousTask(1, 1, 4, "");
    bgContinuousTaskMgr_->isSysReady_.store(true);
    bgContinuousTaskMgr_->SuspendContinuousTask(-1, 1, 4, "");
    SleepForFC();
    bgContinuousTaskMgr_->SuspendContinuousTask(1, 1, 4, "");
    EXPECT_TRUE(bgContinuousTaskMgr_->continuousTaskInfosMap_.empty());
}

/**
 * @tc.name: BgTaskManagerUnitTest_051
 * @tc.desc: test HandleSuspendContinuousTask.
 * @tc.type: FUNC
 * @tc.require: issueIC6B53
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_051, TestSize.Level1)
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
    // 查不到对应的key值
    bgContinuousTaskMgr_->HandleSuspendContinuousTask(TEST_NUM_ONE, TEST_NUM_ONE, 4, "");
    EXPECT_NE((int32_t)bgContinuousTaskMgr_->continuousTaskInfosMap_.size(), 0);
    // 查到对应的key值
    bgContinuousTaskMgr_->HandleSuspendContinuousTask(TEST_NUM_ONE, TEST_NUM_ONE, 4, "key1");
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->continuousTaskInfosMap_.size(), 2);
}

/**
 * @tc.name: BgTaskManagerUnitTest_052
 * @tc.desc: test ActiveContinuousTask.
 * @tc.type: FUNC
 * @tc.require: issueIC6B53
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_052, TestSize.Level1)
{
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    bgContinuousTaskMgr_->isSysReady_.store(false);
    bgContinuousTaskMgr_->ActiveContinuousTask(1, 1, "");
    bgContinuousTaskMgr_->isSysReady_.store(true);
    bgContinuousTaskMgr_->ActiveContinuousTask(1, 1, "");
    SleepForFC();
    bgContinuousTaskMgr_->ActiveContinuousTask(1, 1, "");
    EXPECT_TRUE(bgContinuousTaskMgr_->continuousTaskInfosMap_.empty());
}

/**
 * @tc.name: BgTaskManagerUnitTest_053
 * @tc.desc: test HandleActiveContinuousTask.
 * @tc.type: FUNC
 * @tc.require: issueIC6B53
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_053, TestSize.Level1)
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
    // 暂停长时任务
    bgContinuousTaskMgr_->HandleSuspendContinuousTask(TEST_NUM_ONE, TEST_NUM_ONE, 4, "key1");
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->continuousTaskInfosMap_.size(), 2);
    // 恢复长时任务
    bgContinuousTaskMgr_->HandleActiveContinuousTask(TEST_NUM_ONE, TEST_NUM_ONE, "");
    bgContinuousTaskMgr_->HandleActiveContinuousTask(TEST_NUM_ONE, TEST_NUM_ONE, "key1");
    EXPECT_EQ((int32_t)bgContinuousTaskMgr_->continuousTaskInfosMap_.size(), 2);
}

/**
 * @tc.name: GetAllContinuousTasks_001
 * @tc.desc: test GetAllContinuousTasks interface.
 * @tc.type: FUNC
 * @tc.require: issueIBY0DN
 */
HWTEST_F(BgContinuousTaskMgrTest, GetAllContinuousTasks_001, TestSize.Level1)
{
    bgContinuousTaskMgr_->isSysReady_.store(false);
    std::vector<std::shared_ptr<ContinuousTaskInfo>> list;
    EXPECT_EQ(bgContinuousTaskMgr_->GetAllContinuousTasks(list), ERR_BGTASK_SYS_NOT_READY);

    bgContinuousTaskMgr_->isSysReady_.store(true);
    EXPECT_EQ(bgContinuousTaskMgr_->GetAllContinuousTasks(list), ERR_OK);

    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord1 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->abilityName_ = "abilityName";
    continuousTaskRecord1->uid_ = 1;
    continuousTaskRecord1->bgModeId_ = TEST_NUM_TWO;
    continuousTaskRecord1->bgModeIds_.push_back(TEST_NUM_TWO);
    continuousTaskRecord1->bgSubModeIds_.push_back(TEST_NUM_TWO);
    continuousTaskRecord1->notificationId_ = 1;
    continuousTaskRecord1->continuousTaskId_ = 1;
    continuousTaskRecord1->abilityId_ = 1;
    std::shared_ptr<WantAgentInfo> info = std::make_shared<WantAgentInfo>();
    info->bundleName_ = "wantAgentBundleName";
    info->abilityName_ = "wantAgentAbilityName";
    continuousTaskRecord1->wantAgentInfo_ = info;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;

    std::vector<std::shared_ptr<ContinuousTaskInfo>> list2;
    EXPECT_EQ(bgContinuousTaskMgr_->GetAllContinuousTasks(list2), ERR_OK);
}

/**
 * @tc.name: RequestGetContinuousTasksByUidForInner_001
 * @tc.desc: test RequestGetContinuousTasksByUidForInner interface.
 * @tc.type: FUNC
 * @tc.require: issueIBY0DN
 */
HWTEST_F(BgContinuousTaskMgrTest, RequestGetContinuousTasksByUidForInner_001, TestSize.Level1)
{
    bgContinuousTaskMgr_->isSysReady_.store(false);
    std::vector<std::shared_ptr<ContinuousTaskInfo>> list;
    int32_t uid = 1;
    EXPECT_EQ(bgContinuousTaskMgr_->RequestGetContinuousTasksByUidForInner(uid, list), ERR_BGTASK_SYS_NOT_READY);

    bgContinuousTaskMgr_->isSysReady_.store(true);
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord1 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->abilityName_ = "abilityName";
    continuousTaskRecord1->uid_ = 1;
    continuousTaskRecord1->bgModeId_ = TEST_NUM_TWO;
    continuousTaskRecord1->bgModeIds_.push_back(TEST_NUM_TWO);
    continuousTaskRecord1->bgSubModeIds_.push_back(TEST_NUM_TWO);
    continuousTaskRecord1->notificationId_ = 1;
    continuousTaskRecord1->continuousTaskId_ = 1;
    continuousTaskRecord1->abilityId_ = 1;
    std::shared_ptr<WantAgentInfo> info = std::make_shared<WantAgentInfo>();
    info->bundleName_ = "wantAgentBundleName";
    info->abilityName_ = "wantAgentAbilityName";
    continuousTaskRecord1->wantAgentInfo_ = info;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    uid = 2;
    EXPECT_EQ(bgContinuousTaskMgr_->RequestGetContinuousTasksByUidForInner(uid, list), ERR_OK);

    uid = 1;
    std::vector<std::shared_ptr<ContinuousTaskInfo>> list2;
    EXPECT_EQ(bgContinuousTaskMgr_->RequestGetContinuousTasksByUidForInner(uid, list2), ERR_OK);
}

/**
 * @tc.name: AVSessionNotifyUpdateNotification_001
 * @tc.desc: test AVSessionNotifyUpdateNotification interface.
 * @tc.type: FUNC
 * @tc.require: issueIC9VN9
 */
HWTEST_F(BgContinuousTaskMgrTest, AVSessionNotifyUpdateNotification_001, TestSize.Level1)
{
    bgContinuousTaskMgr_->isSysReady_.store(false);
    int32_t uid = 0;
    int32_t pid = 1;
    EXPECT_EQ(bgContinuousTaskMgr_->AVSessionNotifyUpdateNotification(uid, pid, true), ERR_BGTASK_SYS_NOT_READY);

    bgContinuousTaskMgr_->isSysReady_.store(true);
    EXPECT_EQ(bgContinuousTaskMgr_->AVSessionNotifyUpdateNotification(uid, pid, true), ERR_BGTASK_CHECK_TASK_PARAM);
}

/**
 * @tc.name: AVSessionNotifyUpdateNotification_002
 * @tc.desc: test AVSessionNotifyUpdateNotification interface.
 * @tc.type: FUNC
 * @tc.require: issueIC9VN9
 */
HWTEST_F(BgContinuousTaskMgrTest, AVSessionNotifyUpdateNotification_002, TestSize.Level1)
{
    int32_t uid = 0;
    int32_t pid = 1;
    bgContinuousTaskMgr_->isSysReady_.store(true);
    EXPECT_EQ(bgContinuousTaskMgr_->AVSessionNotifyUpdateNotificationInner(uid, pid, true),
        ERR_BGTASK_OBJECT_NOT_EXIST);

    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    uid = 1;
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord1 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->uid_ = 1;
    continuousTaskRecord1->bgModeId_ = 2;
    continuousTaskRecord1->isNewApi_ = true;
    continuousTaskRecord1->notificationId_ = 1;
    continuousTaskRecord1->continuousTaskId_ = 1;
    continuousTaskRecord1->notificationLabel_ = "label1";

    CachedBundleInfo info = CachedBundleInfo();
    info.abilityBgMode_["abilityName"] = 2;
    info.appName_ = "appName";
    bgContinuousTaskMgr_->cachedBundleInfos_.emplace(1, info);
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    EXPECT_EQ(bgContinuousTaskMgr_->AVSessionNotifyUpdateNotificationInner(uid, pid, true), ERR_OK);
    EXPECT_EQ(bgContinuousTaskMgr_->AVSessionNotifyUpdateNotificationInner(uid, pid, false), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_054
 * @tc.desc: test SendContinuousTaskNotification.
 * @tc.type: FUNC
 * @tc.require: issueICC87K
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_054, TestSize.Level1)
{
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    CachedBundleInfo info = CachedBundleInfo();
    info.abilityBgMode_["abilityName"] = 1;
    info.appName_ = "appName";
    bgContinuousTaskMgr_->cachedBundleInfos_.emplace(1, info);
    continuousTaskRecord->uid_ = 1;
    continuousTaskRecord->bgModeId_ = 2;
    continuousTaskRecord->bgModeIds_.clear();
    continuousTaskRecord->bgModeIds_.push_back(continuousTaskRecord->bgModeId_);
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord;
    EXPECT_EQ(bgContinuousTaskMgr_->SendContinuousTaskNotification(continuousTaskRecord), ERR_OK);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS