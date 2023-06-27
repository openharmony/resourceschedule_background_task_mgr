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
#include <gtest/gtest.h>

#include "bgtask_common.h"
#include "background_task_subscriber.h"
#include "background_task_subscriber_proxy.h"
#include "bg_continuous_task_mgr.h"
#include "bg_transient_task_mgr.h"
#include "bgtaskmgr_inner_errors.h"
#include "bundle_mgr_interface.h"
#include "common_event_support.h"
#include "expired_callback_proxy.h"
#include "expired_callback_stub.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "resources_subscriber_mgr.h"
#include "system_ability_definition.h"
#include "want_agent.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr int32_t SLEEP_TIME = 500;
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
static constexpr uint32_t TEST_NUM_UONE = 1;
static constexpr int32_t TEST_NUM_TWO = 2;
static constexpr uint32_t TEST_NUM_UTWO = 2;
static constexpr int32_t TEST_NUM_THREE = 3;
static constexpr char LAUNCHER_BUNDLE_NAME[] = "com.ohos.launcher";
}
class BgTaskManagerUnitTest : public testing::Test {
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
    static std::shared_ptr<BgTransientTaskMgr> bgTransientTaskMgr_;

    int32_t GetUidByBundleName(const std::string &bundleName, const int32_t userId)
    {
        sptr<ISystemAbilityManager> systemMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemMgr == nullptr) {
            return -1;
        }

        sptr<IRemoteObject> remoteObject = systemMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (remoteObject == nullptr) {
            return -1;
        }

        sptr<OHOS::AppExecFwk::IBundleMgr> bundleMgrProxy = iface_cast<OHOS::AppExecFwk::IBundleMgr>(remoteObject);
        if (bundleMgrProxy == nullptr) {
            return -1;
        }

        return bundleMgrProxy->GetUidByBundleName(bundleName, userId);
    }
};

std::shared_ptr<BgContinuousTaskMgr> BgTaskManagerUnitTest::bgContinuousTaskMgr_ = nullptr;
std::shared_ptr<BgTransientTaskMgr> BgTaskManagerUnitTest::bgTransientTaskMgr_ = nullptr;

void BgTaskManagerUnitTest::SetUpTestCase()
{
    bgContinuousTaskMgr_ = BgContinuousTaskMgr::GetInstance();
    bgTransientTaskMgr_ = DelayedSingleton<BgTransientTaskMgr>::GetInstance();
}

void BgTaskManagerUnitTest::TearDownTestCase() {}

void BgTaskManagerUnitTest::SetUp() {}

void BgTaskManagerUnitTest::TearDown() {}

class TestBackgroundTaskSubscriber : public BackgroundTaskSubscriber {};

class TestExpiredCallbackStub : public ExpiredCallbackStub {
public:
    void OnExpired() override {}
};

/**
 * @tc.name: BgTaskManagerUnitTest_002
 * @tc.desc: test SetCachedBundleInfo.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_002, TestSize.Level1)
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_003, TestSize.Level1)
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_004, TestSize.Level1)
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_005, TestSize.Level1)
{
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord->bgModeId_ = 1;
    continuousTaskRecord->isNewApi_ = false;
    EXPECT_EQ(bgContinuousTaskMgr_->SendContinuousTaskNotification(continuousTaskRecord),
        ERR_BGTASK_NOTIFICATION_VERIFY_FAILED);
    continuousTaskRecord->isNewApi_ = true;
    EXPECT_EQ(bgContinuousTaskMgr_->SendContinuousTaskNotification(continuousTaskRecord),
        ERR_BGTASK_NOTIFICATION_VERIFY_FAILED);
    continuousTaskRecord->bgModeId_ = INVALID_BGMODE_ID;
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
    EXPECT_EQ(bgContinuousTaskMgr_->SendContinuousTaskNotification(continuousTaskRecord), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_006
 * @tc.desc: test StopContinuousTask.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_006, TestSize.Level1)
{
    bgContinuousTaskMgr_->isSysReady_.store(false);
    bgContinuousTaskMgr_->StopContinuousTask(1, 1, 1);
    bgContinuousTaskMgr_->isSysReady_.store(true);
    bgContinuousTaskMgr_->StopContinuousTask(-1, 1, 1);
    SleepForFC();
    bgContinuousTaskMgr_->StopContinuousTask(1, 1, 1);
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
    bgContinuousTaskMgr_->StopContinuousTask(-1, 1, 1);
    SleepForFC();
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();

    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key2"] = continuousTaskRecord2;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key3"] = continuousTaskRecord3;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key4"] = continuousTaskRecord4;
    bgContinuousTaskMgr_->StopContinuousTask(1, 1, 1);
    SleepForFC();
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_007
 * @tc.desc: test GetContinuousTaskApps.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_007, TestSize.Level1)
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_008, TestSize.Level1)
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_009, TestSize.Level1)
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_010, TestSize.Level1)
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_011, TestSize.Level1)
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
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_012, TestSize.Level1)
{
    bgContinuousTaskMgr_->isSysReady_.store(false);
    bgContinuousTaskMgr_->OnAbilityStateChanged(-1, "test");
    bgContinuousTaskMgr_->isSysReady_.store(true);
    bgContinuousTaskMgr_->OnAbilityStateChanged(-1, "test");

    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    bgContinuousTaskMgr_->bgTaskSubscribers_.clear();
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord->uid_ = 1;
    continuousTaskRecord->abilityName_ = "test";
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key"] = continuousTaskRecord;
    bgContinuousTaskMgr_->OnAbilityStateChanged(-1, "test");
    bgContinuousTaskMgr_->OnAbilityStateChanged(1, "test1");
    bgContinuousTaskMgr_->OnAbilityStateChanged(-1, "test1");
    bgContinuousTaskMgr_->OnAbilityStateChanged(1, "test");
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_013
 * @tc.desc: test OnProcessDied.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_013, TestSize.Level1)
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_014, TestSize.Level1)
{
    bgContinuousTaskMgr_->OnContinuousTaskChanged(nullptr, ContinuousTaskEventTriggerType::TASK_START);
    bgContinuousTaskMgr_->bgTaskSubscribers_.clear();
    auto continuousTaskInfo =  std::make_shared<ContinuousTaskRecord>();
    bgContinuousTaskMgr_->OnContinuousTaskChanged(continuousTaskInfo, ContinuousTaskEventTriggerType::TASK_START);
    TestBackgroundTaskSubscriber subscriber = TestBackgroundTaskSubscriber();
    bgContinuousTaskMgr_->bgTaskSubscribers_.emplace_back(subscriber.GetImpl());
    bgContinuousTaskMgr_->OnContinuousTaskChanged(continuousTaskInfo, ContinuousTaskEventTriggerType::TASK_START);
    bgContinuousTaskMgr_->OnContinuousTaskChanged(continuousTaskInfo, ContinuousTaskEventTriggerType::TASK_CANCEL);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_015
 * @tc.desc: test OnBundleInfoChanged.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_015, TestSize.Level1)
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_016, TestSize.Level1)
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_017, TestSize.Level1)
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
 * @tc.name: BgTaskManagerUnitTest_018
 * @tc.desc: test BgTransientTaskMgr init.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_018, TestSize.Level1)
{
    EXPECT_EQ(bgTransientTaskMgr_->isReady_.load(), false);
    bgTransientTaskMgr_->Init(AppExecFwk::EventRunner::Create("tdd_test_handler"));
    EXPECT_EQ(bgTransientTaskMgr_->isReady_.load(), true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_019
 * @tc.desc: test IsCallingInfoLegal.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_019, TestSize.Level1)
{
    std::string bundleName;
    EXPECT_EQ(bgTransientTaskMgr_->IsCallingInfoLegal(-1, -1, bundleName, nullptr), ERR_BGTASK_INVALID_PID_OR_UID);
    EXPECT_EQ(bgTransientTaskMgr_->IsCallingInfoLegal(1, 1, bundleName, nullptr), ERR_BGTASK_INVALID_BUNDLE_NAME);
    int32_t uid = GetUidByBundleName(LAUNCHER_BUNDLE_NAME, DEFAULT_USERID);
    EXPECT_EQ(bgTransientTaskMgr_->IsCallingInfoLegal(uid, 1, bundleName, nullptr), ERR_BGTASK_INVALID_CALLBACK);
    sptr<ExpiredCallbackProxy> proxy1 = sptr<ExpiredCallbackProxy>(new ExpiredCallbackProxy(nullptr));
    EXPECT_EQ(bgTransientTaskMgr_->IsCallingInfoLegal(uid, 1, bundleName, proxy1), ERR_BGTASK_INVALID_CALLBACK);
    sptr<TestExpiredCallbackStub> expiredCallbackStub = sptr<TestExpiredCallbackStub>(new TestExpiredCallbackStub());
    sptr<ExpiredCallbackProxy> proxy2 = sptr<ExpiredCallbackProxy>(
        new ExpiredCallbackProxy(expiredCallbackStub->AsObject()));
    EXPECT_EQ(bgTransientTaskMgr_->IsCallingInfoLegal(uid, 1, bundleName, proxy2), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_020
 * @tc.desc: test RequestSuspendDelay.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_020, TestSize.Level1)
{
    std::shared_ptr<DelaySuspendInfo> delayInfo = std::make_shared<DelaySuspendInfo>();
    bgTransientTaskMgr_->isReady_.store(false);
    EXPECT_EQ(bgTransientTaskMgr_->RequestSuspendDelay(u"test", nullptr, delayInfo), ERR_BGTASK_SYS_NOT_READY);
    bgTransientTaskMgr_->isReady_.store(true);
    EXPECT_EQ(bgTransientTaskMgr_->RequestSuspendDelay(u"test", nullptr, delayInfo), ERR_BGTASK_INVALID_CALLBACK);

    sptr<TestExpiredCallbackStub> expiredCallbackStub = sptr<TestExpiredCallbackStub>(new TestExpiredCallbackStub());
    sptr<ExpiredCallbackProxy> proxy = sptr<ExpiredCallbackProxy>(
        new ExpiredCallbackProxy(expiredCallbackStub->AsObject()));

    bgTransientTaskMgr_->expiredCallbackMap_[1] = proxy;
    EXPECT_EQ(bgTransientTaskMgr_->RequestSuspendDelay(u"test", proxy, delayInfo), ERR_BGTASK_CALLBACK_EXISTS);
    bgTransientTaskMgr_->expiredCallbackMap_.clear();
    EXPECT_EQ(bgTransientTaskMgr_->RequestSuspendDelay(u"test", proxy, delayInfo), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_021
 * @tc.desc: test HandleTransientTaskSuscriberTask.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_021, TestSize.Level1)
{
    bgTransientTaskMgr_->handler_ = nullptr;
    bgTransientTaskMgr_->HandleTransientTaskSuscriberTask(nullptr, TransientTaskEventType::TASK_START);
    bgTransientTaskMgr_->handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create("tdd_test_handler"));
    bgTransientTaskMgr_->HandleTransientTaskSuscriberTask(nullptr, TransientTaskEventType::TASK_START);

    shared_ptr<TransientTaskAppInfo> appInfo = std::make_shared<TransientTaskAppInfo>();
    bgTransientTaskMgr_->HandleTransientTaskSuscriberTask(appInfo, TransientTaskEventType::TASK_START);

    TestBackgroundTaskSubscriber subscriber = TestBackgroundTaskSubscriber();
    bgTransientTaskMgr_->subscriberList_.emplace_back(subscriber.GetImpl());
    bgTransientTaskMgr_->HandleTransientTaskSuscriberTask(appInfo, TransientTaskEventType::TASK_START);
    bgTransientTaskMgr_->HandleTransientTaskSuscriberTask(appInfo, TransientTaskEventType::TASK_END);
    bgTransientTaskMgr_->HandleTransientTaskSuscriberTask(appInfo, TransientTaskEventType::APP_TASK_START);
    bgTransientTaskMgr_->HandleTransientTaskSuscriberTask(appInfo, TransientTaskEventType::APP_TASK_END);
    bgTransientTaskMgr_->subscriberList_.clear();
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_022
 * @tc.desc: test CancelSuspendDelay.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_022, TestSize.Level1)
{
    bgTransientTaskMgr_->keyInfoMap_.clear();
    bgTransientTaskMgr_->isReady_.store(false);
    EXPECT_EQ(bgTransientTaskMgr_->CancelSuspendDelay(-1), ERR_BGTASK_SYS_NOT_READY);
    bgTransientTaskMgr_->isReady_.store(true);
    EXPECT_EQ(bgTransientTaskMgr_->CancelSuspendDelay(1), ERR_BGTASK_INVALID_REQUEST_ID);
    int32_t uid = GetUidByBundleName(LAUNCHER_BUNDLE_NAME, 100);
    auto keyInfo = std::make_shared<KeyInfo>(LAUNCHER_BUNDLE_NAME, uid);
    bgTransientTaskMgr_->keyInfoMap_[1] = keyInfo;
    bgTransientTaskMgr_->CancelSuspendDelay(1);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_023
 * @tc.desc: test CancelSuspendDelayLocked.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_023, TestSize.Level1)
{
    EXPECT_EQ(bgTransientTaskMgr_->CancelSuspendDelayLocked(-1), ERR_BGTASK_CALLBACK_NOT_EXIST);

    sptr<ExpiredCallbackProxy> proxy1 = sptr<ExpiredCallbackProxy>(new ExpiredCallbackProxy(nullptr));
    bgTransientTaskMgr_->expiredCallbackMap_[1] = proxy1;
    bgTransientTaskMgr_->CancelSuspendDelayLocked(-1);

    sptr<TestExpiredCallbackStub> expiredCallbackStub = sptr<TestExpiredCallbackStub>(new TestExpiredCallbackStub());
    sptr<ExpiredCallbackProxy> proxy2 = sptr<ExpiredCallbackProxy>(
        new ExpiredCallbackProxy(expiredCallbackStub->AsObject()));
    bgTransientTaskMgr_->CancelSuspendDelayLocked(-1);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_024
 * @tc.desc: test ForceCancelSuspendDelay.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_024, TestSize.Level1)
{
    bgTransientTaskMgr_->keyInfoMap_.clear();
    bgTransientTaskMgr_->ForceCancelSuspendDelay(1);

    auto keyInfo = std::make_shared<KeyInfo>("bundleName", 1);
    bgTransientTaskMgr_->keyInfoMap_[1] = keyInfo;
    bgTransientTaskMgr_->ForceCancelSuspendDelay(1);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_025
 * @tc.desc: test GetRemainingDelayTime.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_025, TestSize.Level1)
{
    int32_t delayTime;
    bgTransientTaskMgr_->isReady_.store(false);
    EXPECT_EQ(bgTransientTaskMgr_->GetRemainingDelayTime(-1, delayTime), ERR_BGTASK_SYS_NOT_READY);
    bgTransientTaskMgr_->isReady_.store(true);
    EXPECT_EQ(bgTransientTaskMgr_->GetRemainingDelayTime(1, delayTime), ERR_BGTASK_INVALID_REQUEST_ID);
    int32_t uid = GetUidByBundleName(LAUNCHER_BUNDLE_NAME, DEFAULT_USERID);
    auto keyInfo = std::make_shared<KeyInfo>(LAUNCHER_BUNDLE_NAME, uid);
    bgTransientTaskMgr_->keyInfoMap_[1] = keyInfo;
    bgTransientTaskMgr_->GetRemainingDelayTime(1, delayTime);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_026
 * @tc.desc: test HandleExpiredCallbackDeath.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_026, TestSize.Level1)
{
    bgTransientTaskMgr_->HandleExpiredCallbackDeath(nullptr);
    sptr<TestExpiredCallbackStub> expiredCallbackStub = sptr<TestExpiredCallbackStub>(new TestExpiredCallbackStub());
    sptr<ExpiredCallbackProxy> proxy = sptr<ExpiredCallbackProxy>(
        new ExpiredCallbackProxy(expiredCallbackStub->AsObject()));
    bgTransientTaskMgr_->expiredCallbackMap_.clear();
    bgTransientTaskMgr_->HandleExpiredCallbackDeath(proxy);

    bgTransientTaskMgr_->expiredCallbackMap_[1] = proxy;
    bgTransientTaskMgr_->HandleExpiredCallbackDeath(proxy);

    auto keyInfo = std::make_shared<KeyInfo>("bundleName", 1);
    bgTransientTaskMgr_->keyInfoMap_[1] = keyInfo;
    bgTransientTaskMgr_->expiredCallbackMap_[1] = proxy;
    bgTransientTaskMgr_->HandleExpiredCallbackDeath(proxy);
    bgTransientTaskMgr_->keyInfoMap_.clear();
    bgTransientTaskMgr_->expiredCallbackMap_.clear();
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_027
 * @tc.desc: test HandleSubscriberDeath.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_027, TestSize.Level1)
{
    bgTransientTaskMgr_->subscriberList_.clear();
    bgTransientTaskMgr_->HandleSubscriberDeath(nullptr);
    TestBackgroundTaskSubscriber subscriber = TestBackgroundTaskSubscriber();
    bgTransientTaskMgr_->HandleSubscriberDeath(subscriber.GetImpl());
    bgTransientTaskMgr_->subscriberList_.emplace_back(subscriber.GetImpl());
    bgTransientTaskMgr_->HandleSubscriberDeath(subscriber.GetImpl());
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_028
 * @tc.desc: test HandleRequestExpired.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_028, TestSize.Level1)
{
    bgTransientTaskMgr_->keyInfoMap_.clear();
    bgTransientTaskMgr_->expiredCallbackMap_.clear();
    bgTransientTaskMgr_->HandleRequestExpired(1);
    sptr<TestExpiredCallbackStub> expiredCallbackStub = sptr<TestExpiredCallbackStub>(new TestExpiredCallbackStub());
    sptr<ExpiredCallbackProxy> proxy = sptr<ExpiredCallbackProxy>(
        new ExpiredCallbackProxy(expiredCallbackStub->AsObject()));
    bgTransientTaskMgr_->expiredCallbackMap_[1] = proxy;
    bgTransientTaskMgr_->HandleRequestExpired(1);
    auto keyInfo = std::make_shared<KeyInfo>("bundleName", 1);
    bgTransientTaskMgr_->keyInfoMap_[1] = keyInfo;
    bgTransientTaskMgr_->HandleRequestExpired(1);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_029
 * @tc.desc: test SubscribeBackgroundTask.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_029, TestSize.Level1)
{
    EXPECT_EQ(bgTransientTaskMgr_->SubscribeBackgroundTask(nullptr), ERR_BGTASK_INVALID_PARAM);
    TestBackgroundTaskSubscriber subscriber = TestBackgroundTaskSubscriber();
    sptr<BackgroundTaskSubscriberProxy> subscirberProxy1
        = sptr<BackgroundTaskSubscriberProxy>(new BackgroundTaskSubscriberProxy(nullptr));
    EXPECT_EQ(bgTransientTaskMgr_->SubscribeBackgroundTask(subscirberProxy1), ERR_BGTASK_INVALID_PARAM);
    sptr<BackgroundTaskSubscriberProxy> subscirberProxy2
        = sptr<BackgroundTaskSubscriberProxy>(new BackgroundTaskSubscriberProxy(subscriber.GetImpl()));
    EXPECT_EQ(bgTransientTaskMgr_->SubscribeBackgroundTask(subscirberProxy2), ERR_OK);

    bgTransientTaskMgr_->subscriberList_.emplace_back(subscirberProxy2);
    EXPECT_EQ(bgTransientTaskMgr_->SubscribeBackgroundTask(subscirberProxy2), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_030
 * @tc.desc: test UnsubscribeBackgroundTask.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_030, TestSize.Level1)
{
    EXPECT_EQ(bgTransientTaskMgr_->UnsubscribeBackgroundTask(nullptr), ERR_BGTASK_INVALID_PARAM);
    TestBackgroundTaskSubscriber subscriber = TestBackgroundTaskSubscriber();
    sptr<BackgroundTaskSubscriberProxy> subscirberProxy1
        = sptr<BackgroundTaskSubscriberProxy>(new BackgroundTaskSubscriberProxy(nullptr));
    EXPECT_EQ(bgTransientTaskMgr_->UnsubscribeBackgroundTask(subscirberProxy1), ERR_BGTASK_INVALID_PARAM);
    sptr<BackgroundTaskSubscriberProxy> subscirberProxy2
        = sptr<BackgroundTaskSubscriberProxy>(new BackgroundTaskSubscriberProxy(subscriber.GetImpl()));
    EXPECT_EQ(bgTransientTaskMgr_->UnsubscribeBackgroundTask(subscirberProxy2), ERR_OK);

    bgTransientTaskMgr_->subscriberList_.emplace_back(subscirberProxy2);
    EXPECT_EQ(bgTransientTaskMgr_->UnsubscribeBackgroundTask(subscirberProxy2), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_031
 * @tc.desc: test GetTransientTaskApps.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_031, TestSize.Level1)
{
    std::vector<std::shared_ptr<TransientTaskAppInfo>> list;
    bgTransientTaskMgr_->keyInfoMap_.clear();
    EXPECT_EQ(bgTransientTaskMgr_->GetTransientTaskApps(list), ERR_OK);

    auto keyInfo = std::make_shared<KeyInfo>("bundleName", 1);
    bgTransientTaskMgr_->keyInfoMap_[1] = keyInfo;
    EXPECT_EQ(bgTransientTaskMgr_->GetTransientTaskApps(list), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_032
 * @tc.desc: test ShellDump.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_032, TestSize.Level1)
{
    bgTransientTaskMgr_->keyInfoMap_.clear();
    std::vector<std::string> dumpOption;
    std::vector<std::string> dumpInfo;
    bgTransientTaskMgr_->isReady_.store(false);
    EXPECT_EQ(bgTransientTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_BGTASK_SYS_NOT_READY);
    bgTransientTaskMgr_->isReady_.store(true);
    dumpOption.emplace_back("-C");
    dumpOption.emplace_back("All");
    EXPECT_EQ(bgTransientTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_OK);
    auto keyInfo = std::make_shared<KeyInfo>("bundleName", 1);
    bgTransientTaskMgr_->keyInfoMap_[1] = keyInfo;
    EXPECT_EQ(bgTransientTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_OK);
    dumpOption.pop_back();
    dumpOption.emplace_back("BATTARY_LOW");
    EXPECT_EQ(bgTransientTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_OK);
    dumpOption.pop_back();
    dumpOption.emplace_back("BATTARY_OKAY");
    EXPECT_EQ(bgTransientTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_OK);
    dumpOption.pop_back();
    dumpOption.emplace_back("DUMP_CANCEL");
    EXPECT_EQ(bgTransientTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_OK);
    dumpOption.pop_back();
    dumpOption.emplace_back("invalid");
    EXPECT_EQ(bgTransientTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_BGTASK_PERMISSION_DENIED);
}

/**
 * @tc.name: BgTaskManagerUnitTest_033
 * @tc.desc: test CheckPersistenceData.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_033, TestSize.Level1)
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_034, TestSize.Level1)
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
    bgContinuousTaskMgr_->HandleStopContinuousTask(TEST_NUM_ONE, TEST_NUM_ONE, TEST_NUM_ONE);
}

/**
 * @tc.name: BgTaskManagerUnitTest_035
 * @tc.desc: test RemoveSpecifiedBgTask.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_035, TestSize.Level1)
{
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord1 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->uid_ = TEST_NUM_ONE;
    continuousTaskRecord1->bgModeId_ = TEST_NUM_UONE;
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord2 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->uid_ = TEST_NUM_ONE;
    continuousTaskRecord1->bgModeId_ = TEST_NUM_UTWO;

    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key2"] = continuousTaskRecord2;
    bgContinuousTaskMgr_->RemoveSpecifiedBgTask(TEST_NUM_UONE);

    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    bgContinuousTaskMgr_->RemoveSpecifiedBgTask(TEST_NUM_UONE);
}

/**
 * @tc.name: BgTaskManagerUnitTest_036
 * @tc.desc: test SubscriberChange.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_036, TestSize.Level1)
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_037, TestSize.Level1)
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_038, TestSize.Level1)
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
}

/**
 * @tc.name: BgTaskManagerUnitTest_039
 * @tc.desc: test OnConfigurationChanged.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_039, TestSize.Level1)
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
    bgContinuousTaskMgr_->OnConfigurationChanged(configuration);
}

/**
 * @tc.name: BgTaskManagerUnitTest_040
 * @tc.desc: test ResourcesSubscriberMgr AddSubscriber.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_040, TestSize.Level1)
{
    DelayedSingleton<ResourcesSubscriberMgr>::GetInstance()->subscriberList_.clear();
    sptr<BackgroundTaskSubscriberProxy> subscirberProxy1
        = sptr<BackgroundTaskSubscriberProxy>(new BackgroundTaskSubscriberProxy(nullptr));
    EXPECT_EQ(DelayedSingleton<ResourcesSubscriberMgr>::GetInstance()->AddSubscriber(subscirberProxy1),
        ERR_BGTASK_INVALID_PARAM);
    EXPECT_EQ(DelayedSingleton<ResourcesSubscriberMgr>::GetInstance()->RemoveSubscriber(subscirberProxy1),
        ERR_BGTASK_INVALID_PARAM);
    TestBackgroundTaskSubscriber subscriber1 = TestBackgroundTaskSubscriber();
    DelayedSingleton<ResourcesSubscriberMgr>::GetInstance()->deathRecipient_ = nullptr;
    EXPECT_EQ(DelayedSingleton<ResourcesSubscriberMgr>::GetInstance()->AddSubscriber(subscriber1.GetImpl()),
        ERR_BGTASK_INVALID_PARAM);
    EXPECT_EQ(DelayedSingleton<ResourcesSubscriberMgr>::GetInstance()->RemoveSubscriber(subscriber1.GetImpl()),
        ERR_BGTASK_OBJECT_EXISTS);

    DelayedSingleton<ResourcesSubscriberMgr>::GetInstance()->deathRecipient_
        = new (std::nothrow) ObserverDeathRecipient();
    EXPECT_EQ(DelayedSingleton<ResourcesSubscriberMgr>::GetInstance()->RemoveSubscriber(subscriber1.GetImpl()),
        ERR_BGTASK_OBJECT_EXISTS);
}

/**
 * @tc.name: BgTaskManagerUnitTest_041
 * @tc.desc: test ResourcesSubscriberMgr OnResourceChanged.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_041, TestSize.Level1)
{
    DelayedSingleton<ResourcesSubscriberMgr>::GetInstance()->OnResourceChanged(nullptr,
        EfficiencyResourcesEventType::APP_RESOURCE_APPLY);
    auto callbackInfo = std::make_shared<ResourceCallbackInfo>();
    TestBackgroundTaskSubscriber subscriber1 = TestBackgroundTaskSubscriber();
    DelayedSingleton<ResourcesSubscriberMgr>::GetInstance()->subscriberList_.emplace_back(subscriber1.GetImpl());

    DelayedSingleton<ResourcesSubscriberMgr>::GetInstance()->OnResourceChanged(callbackInfo,
        EfficiencyResourcesEventType::APP_RESOURCE_APPLY);
    DelayedSingleton<ResourcesSubscriberMgr>::GetInstance()->OnResourceChanged(callbackInfo,
        EfficiencyResourcesEventType::RESOURCE_APPLY);
    DelayedSingleton<ResourcesSubscriberMgr>::GetInstance()->OnResourceChanged(callbackInfo,
        EfficiencyResourcesEventType::APP_RESOURCE_RESET);
    DelayedSingleton<ResourcesSubscriberMgr>::GetInstance()->OnResourceChanged(callbackInfo,
        EfficiencyResourcesEventType::RESOURCE_RESET);
    EXPECT_EQ((int32_t)DelayedSingleton<ResourcesSubscriberMgr>::GetInstance()->subscriberList_.size(), 1);
}

/**
 * @tc.name: BgTaskManagerUnitTest_042
 * @tc.desc: test BgTransientTaskMgr NotifyTransientTaskSuscriber.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_042, TestSize.Level1)
{
    auto taskInfo = std::make_shared<TransientTaskAppInfo>();
    TestBackgroundTaskSubscriber subscriber1 = TestBackgroundTaskSubscriber();
    bgTransientTaskMgr_->subscriberList_.emplace_back(subscriber1.GetImpl());

    bgTransientTaskMgr_->NotifyTransientTaskSuscriber(taskInfo,
        TransientTaskEventType::TASK_START);
    bgTransientTaskMgr_->NotifyTransientTaskSuscriber(taskInfo,
        TransientTaskEventType::TASK_END);
    bgTransientTaskMgr_->NotifyTransientTaskSuscriber(taskInfo,
        TransientTaskEventType::APP_TASK_START);
    bgTransientTaskMgr_->NotifyTransientTaskSuscriber(taskInfo,
        TransientTaskEventType::APP_TASK_END);
    EXPECT_NE((int32_t)bgTransientTaskMgr_->subscriberList_.size(), 0);
}
}
}
