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
#include "bg_efficiency_resources_mgr.h"
#include "resource_type.h"
#include "system_ability_definition.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {
static constexpr int32_t SLEEP_TIME = 2000;
static constexpr int32_t REMAIN_TIME = 1000;
static constexpr int32_t WAIT_TIME = 1000;
static constexpr uint32_t MAX_RESOURCES_TYPE_NUM = 7;
static constexpr char MOCK_EFFICIENCY_RESOURCES_MGR_NAME[] = "MockEfficiencyResourcesMgr";

class BgEfficiencyResourcesMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    inline void SleepFor(int32_t sleepTime)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
    }

    static std::shared_ptr<BgEfficiencyResourcesMgr> bgEfficiencyResourcesMgr_;
};

std::shared_ptr<BgEfficiencyResourcesMgr> BgEfficiencyResourcesMgrTest::bgEfficiencyResourcesMgr_ = nullptr;

void BgEfficiencyResourcesMgrTest::SetUpTestCase()
{
    bgEfficiencyResourcesMgr_ = DelayedSingleton<BgEfficiencyResourcesMgr>::GetInstance();
    bgEfficiencyResourcesMgr_->recordStorage_ = std::make_unique<ResourceRecordStorage>();
    bgEfficiencyResourcesMgr_->subscriberMgr_ = DelayedSingleton<ResourcesSubscriberMgr>::GetInstance();
    bgEfficiencyResourcesMgr_->runner_ = AppExecFwk::EventRunner::Create(MOCK_EFFICIENCY_RESOURCES_MGR_NAME);
    bgEfficiencyResourcesMgr_->handler_ =
        std::make_shared<AppExecFwk::EventHandler>(bgEfficiencyResourcesMgr_->runner_);
    bgEfficiencyResourcesMgr_->isSysReady_.store(true);
}

void BgEfficiencyResourcesMgrTest::TearDownTestCase() {}

void BgEfficiencyResourcesMgrTest::SetUp() {}

void BgEfficiencyResourcesMgrTest::TearDown()
{
    std::vector<std::string> dumpOption;
    dumpOption.emplace_back("-E");
    dumpOption.emplace_back("--reset_all");
    std::vector<std::string> dumpInfo;
    bgEfficiencyResourcesMgr_->ShellDump(dumpOption, dumpInfo);
    SleepFor(WAIT_TIME);
}

class TestBackgroundTaskSubscriber : public BackgroundTaskMgr::BackgroundTaskSubscriber {
public:
    void OnAppEfficiencyResourcesApply(const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) override {}

    void OnAppEfficiencyResourcesReset(const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) override {}
    
    void OnProcEfficiencyResourcesApply(const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) override {}

    void OnProcEfficiencyResourcesReset(const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) override {}
};

/**
 * @tc.name: AppEfficiencyResources_001
 * @tc.desc: apply efficiency resources using ApplyEfficiencyResources function.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, AppEfficiencyResources_001, TestSize.Level1)
{
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        nullptr), (int32_t)ERR_BGTASK_INVALID_PARAM);
    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo();
    resourceInfo->isApply_ = true;
    EXPECT_NE(resourceInfo, nullptr);
    resourceInfo->resourceNumber_ = 1 << MAX_RESOURCES_TYPE_NUM;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_BGTASK_INVALID_PARAM);
    resourceInfo->resourceNumber_ = 1;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_BGTASK_INVALID_PARAM);
    resourceInfo->isPersist_ = true;
    resourceInfo->reason_ = "apply";
    resourceInfo->timeOut_ = 0;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);

    resourceInfo->isPersist_ = false;
    resourceInfo->timeOut_ = 0;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_BGTASK_INVALID_PARAM);
    resourceInfo->timeOut_ = 10;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    resourceInfo->isPersist_ = true;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    bgEfficiencyResourcesMgr_->ResetAllEfficiencyResources();
}

/**
 * @tc.name: AppEfficiencyResources_002
 * @tc.desc: apply and reset resources for process and app respectively using ApplyEfficiencyResources function.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, AppEfficiencyResources_002, TestSize.Level1)
{
    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo(1, true, 0, "apply",
        true, false);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->appResourceApplyMap_.size(), 1);
    resourceInfo->isApply_ = false;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->appResourceApplyMap_.size(), 0);

    resourceInfo->isProcess_ = true;
    resourceInfo->isApply_ = true;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 1);
    resourceInfo->isApply_ = false;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 0);
    bgEfficiencyResourcesMgr_->ResetAllEfficiencyResources();
}

/**
 * @tc.name: AppEfficiencyResources_003
 * @tc.desc: apply transient efficiency and reset ahead.
 * @tc.require: issuesI5OD7X
 * @tc.type: FUNC
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, AppEfficiencyResources_003, TestSize.Level1)
{
    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo(1, true, 0, "apply",
        true, true);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)(bgEfficiencyResourcesMgr_->procResourceApplyMap_.size()), 1);
    resourceInfo->isPersist_ = false;
    resourceInfo->timeOut_ = SLEEP_TIME;
    resourceInfo->isProcess_ = false;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)(bgEfficiencyResourcesMgr_->appResourceApplyMap_.size()), 1);
    SleepFor(SLEEP_TIME + REMAIN_TIME);
    EXPECT_EQ((int32_t)(bgEfficiencyResourcesMgr_->procResourceApplyMap_.size()), 1);
    EXPECT_EQ((int32_t)(bgEfficiencyResourcesMgr_->appResourceApplyMap_.size()), 0);
    bgEfficiencyResourcesMgr_->ResetAllEfficiencyResources();
}

/**
 * @tc.name: AppEfficiencyResources_004
 * @tc.desc: reset resources record of process using app ApplyEfficiencyResources function.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, AppEfficiencyResources_004, TestSize.Level1)
{
    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo(1, true, 0, "apply",
        true, false);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)(bgEfficiencyResourcesMgr_->appResourceApplyMap_.size()), 1);
    resourceInfo->isProcess_ = true;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 1);

    resourceInfo->isProcess_ = false;
    resourceInfo->isApply_ = false;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->appResourceApplyMap_.size(), 0);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 0);
    bgEfficiencyResourcesMgr_->ResetAllEfficiencyResources();
}

/**
 * @tc.name: AppEfficiencyResources_005
 * @tc.desc: repeatedly apply unpersisted resources.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, AppEfficiencyResources_005, TestSize.Level1)
{
    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo(1, true, 2000, "apply",
        false, true);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)(bgEfficiencyResourcesMgr_->procResourceApplyMap_.size()), 1);
    bgEfficiencyResourcesMgr_->ResetAllEfficiencyResources();
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)(bgEfficiencyResourcesMgr_->procResourceApplyMap_.size()), 0);

    resourceInfo->resourceNumber_ = ResourceType::CPU
        | ResourceType::TIMER | ResourceType::COMMON_EVENT;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    resourceInfo->resourceNumber_ = ResourceType::CPU;
    resourceInfo->timeOut_ = 0;
    resourceInfo->isPersist_ = true;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
}

/**
 * @tc.name: ResetAllEfficiencyResources_001
 * @tc.desc: reset all efficiency resources using ResetAllEfficiencyResources function.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, ResetAllEfficiencyResources_001, TestSize.Level1)
{
    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo(1, true, 0, "apply",
        true, false);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->appResourceApplyMap_.size(), 0);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 0);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    resourceInfo->resourceNumber_ = 1 << 1;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->appResourceApplyMap_.size(), 1);
    resourceInfo->isProcess_ = true;
    resourceInfo->resourceNumber_ = 1;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    resourceInfo->resourceNumber_ = 1 << 1;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 1);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ResetAllEfficiencyResources(), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->appResourceApplyMap_.size(), 0);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 0);
}

/**
 * @tc.name: AppEfficiencyResourcesApply_005
 * @tc.desc: reset all efficiency resources using ResetAllEfficiencyResources function.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, ResetAllEfficiencyResources_002, TestSize.Level1)
{
    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo();
    resourceInfo->isApply_ = true;
    resourceInfo->resourceNumber_ = 1;
    resourceInfo->isPersist_ = true;
    resourceInfo->reason_ = "apply";
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->appResourceApplyMap_.size(), 0);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 0);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    resourceInfo->resourceNumber_ = 1 << 1;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->appResourceApplyMap_.size(), 1);
    resourceInfo->isProcess_ = true;
    resourceInfo->resourceNumber_ = 1;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    resourceInfo->resourceNumber_ = 1 << 1;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 1);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ResetAllEfficiencyResources(), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->appResourceApplyMap_.size(), 0);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 0);
}

/**
 * @tc.name: SubscribeEfficiencyResources_001
 * @tc.desc: subscribe efficiency resources callback test.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, SubscribeEfficiencyResources_001, TestSize.Level1)
{
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->AddSubscriber(
        nullptr), (int32_t)ERR_BGTASK_INVALID_PARAM);
    SleepFor(SLEEP_TIME);
    auto subscriber =  std::make_shared<TestBackgroundTaskSubscriber>();
    EXPECT_NE(subscriber, nullptr);
    auto resourceInfo = std::make_shared<ResourceCallbackInfo>(0, 0, 1, "");
    subscriber->OnAppEfficiencyResourcesApply(resourceInfo);
    subscriber->OnAppEfficiencyResourcesReset(resourceInfo);
    subscriber->OnProcEfficiencyResourcesApply(resourceInfo);
    subscriber->OnProcEfficiencyResourcesReset(resourceInfo);

    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->AddSubscriber(
        subscriber->GetImpl()), (int32_t)ERR_OK);
    SleepFor(SLEEP_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->AddSubscriber(
        subscriber->GetImpl()), (int32_t)ERR_BGTASK_OBJECT_EXISTS);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->RemoveSubscriber(subscriber->GetImpl()), (int32_t)ERR_OK);
}

/**
 * @tc.name: SubscribeEfficiencyResources_002
 * @tc.desc: unsubscribe efficiency resources callback test.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, SubscribeEfficiencyResources_002, TestSize.Level1)
{
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->RemoveSubscriber(nullptr),
        (int32_t)ERR_BGTASK_INVALID_PARAM);
    SleepFor(SLEEP_TIME);
    auto subscriber =  std::make_shared<TestBackgroundTaskSubscriber>();
    EXPECT_NE(subscriber, nullptr);
    bgEfficiencyResourcesMgr_->AddSubscriber(subscriber->GetImpl());
    SleepFor(SLEEP_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->RemoveSubscriber(subscriber->GetImpl()), (int32_t)ERR_OK);
}

/**
 * @tc.name: Marshalling_001
 * @tc.desc: marshalling resource callback info.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, Marshalling_001, TestSize.Level1)
{
    auto resourceInfo = std::make_shared<ResourceCallbackInfo>();
    MessageParcel data;
    EXPECT_TRUE(resourceInfo->Marshalling(data));
    EXPECT_TRUE(ResourceCallbackInfo::Unmarshalling(data) != nullptr);
}

/**
 * @tc.name: Init_001
 * @tc.desc: cover init function.
 * @tc.type: FUNC
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, Init_001, TestSize.Level1)
{
    bgEfficiencyResourcesMgr_->runner_ = nullptr;
    bgEfficiencyResourcesMgr_->Init();
    bgEfficiencyResourcesMgr_->runner_ = AppExecFwk::EventRunner::Create(MOCK_EFFICIENCY_RESOURCES_MGR_NAME);
    bgEfficiencyResourcesMgr_->Init();
    EXPECT_TRUE(true);
}

/**
 * @tc.name: SystemAbility_001
 * @tc.desc: cover system ability callback function.
 * @tc.type: FUNC
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, SystemAbility_001, TestSize.Level1)
{
    bgEfficiencyResourcesMgr_->OnRemoveSystemAbility(APP_MGR_SERVICE_ID, "");
    bgEfficiencyResourcesMgr_->OnRemoveSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, "");
    bgEfficiencyResourcesMgr_->OnAddSystemAbility(APP_MGR_SERVICE_ID, "");
    bgEfficiencyResourcesMgr_->OnAddSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, "");

    bgEfficiencyResourcesMgr_->isSysReady_.store(true);
    bgEfficiencyResourcesMgr_->CheckAlivedApp(0);
    bgEfficiencyResourcesMgr_->Clear();
    bgEfficiencyResourcesMgr_->appStateObserver_ = nullptr;
    bgEfficiencyResourcesMgr_->CheckAlivedApp(0);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: EraseRecord_001
 * @tc.desc: cover EraseRecordIf function.
 * @tc.type: FUNC
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, EraseRecord_001, TestSize.Level1)
{
    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo(ResourceType::CPU, true,
        0, "apply", true, false);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)(bgEfficiencyResourcesMgr_->appResourceApplyMap_.size()), 1);
    
    resourceInfo->isProcess_ = true;
    resourceInfo->isPersist_ = false;
    resourceInfo->timeOut_ = 2000;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)(bgEfficiencyResourcesMgr_->procResourceApplyMap_.size()), 1);
    bgEfficiencyResourcesMgr_->HandlePersistenceData();
    EXPECT_TRUE(true);
}

/**
 * @tc.name: DeathCallback_001
 * @tc.desc: cover the condition of application death.
 * @tc.type: FUNC
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, DeathCallback_001, TestSize.Level1)
{
    bgEfficiencyResourcesMgr_->isSysReady_.store(false);
    bgEfficiencyResourcesMgr_->RemoveAppRecord(0, "", false);
    bgEfficiencyResourcesMgr_->RemoveProcessRecord(0, 0, "");
    bgEfficiencyResourcesMgr_->isSysReady_.store(true);
    bgEfficiencyResourcesMgr_->RemoveAppRecord(0, "", false);
    bgEfficiencyResourcesMgr_->RemoveAppRecord(0, "", true);
    bgEfficiencyResourcesMgr_->RemoveProcessRecord(-1, 0, "");
    bgEfficiencyResourcesMgr_->RemoveProcessRecord(0, 0, "");
    EXPECT_TRUE(true);
}

/**
 * @tc.name: SysUnload_001
 * @tc.desc: cover the condition when sysload_ is false.
 * @tc.type: FUNC
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, SysUnload_001, TestSize.Level1)
{
    bgEfficiencyResourcesMgr_->isSysReady_.store(false);
    bgEfficiencyResourcesMgr_->RemoveAppRecord(0, "", false);
    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo(1, true, 0, "apply",
        true, false);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(resourceInfo),
        (int32_t)ERR_BGTASK_SERVICE_NOT_READY);
    bgEfficiencyResourcesMgr_->ResetAllEfficiencyResources();
    std::vector<std::string> dumpInfo {};
    bgEfficiencyResourcesMgr_->ShellDump({"-E", "--reset_all"}, dumpInfo);

    bgEfficiencyResourcesMgr_->isSysReady_.store(true);
    bgEfficiencyResourcesMgr_->RemoveAppRecord(0, "", false);
    bgEfficiencyResourcesMgr_->RemoveAppRecord(0, "", true);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: Dump_001
 * @tc.desc: cover the condition when ShellDump function is called.
 * @tc.type: FUNC
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, Dump_001, TestSize.Level1)
{
    std::vector<std::string> dumpInfo {};
    bgEfficiencyResourcesMgr_->ShellDump({"-E", "--all"}, dumpInfo);
    bgEfficiencyResourcesMgr_->ShellDump({"-E", "--resetapp", "0"}, dumpInfo);
    std::vector<std::shared_ptr<ResourceCallbackInfo>> appList {};
    std::vector<std::shared_ptr<ResourceCallbackInfo>> procList {};
    bgEfficiencyResourcesMgr_->GetEfficiencyResourcesInfos(appList, procList);
    EXPECT_EQ((int32_t)appList.size(), 0);
    EXPECT_EQ((int32_t)procList.size(), 0);

    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo(
        ResourceType::CPU | ResourceType::WORK_SCHEDULER, true, 0, "apply", true, true);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    resourceInfo->isProcess_ = false;
    resourceInfo->isPersist_ = false;
    resourceInfo->timeOut_ = 2000;
    resourceInfo->resourceNumber_ = ResourceType::CPU;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    bgEfficiencyResourcesMgr_->ShellDump({"-E", "--all"}, dumpInfo);
    bgEfficiencyResourcesMgr_->GetEfficiencyResourcesInfos(appList, procList);
    EXPECT_EQ((int32_t)appList.size(), 1);
    EXPECT_EQ((int32_t)procList.size(), 1);
    bgEfficiencyResourcesMgr_->ShellDump({"-E", "--resetapp", "0", "1"}, dumpInfo);
}

/**
 * @tc.name: BoundaryCondition
 * @tc.desc: cover the boundary condition.
 * @tc.type: FUNC
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, BoundaryCondition_001, TestSize.Level1)
{
    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo(0, true, 0, "apply",
        true, false);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_BGTASK_INVALID_PARAM);
    std::string bundleName {""};
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->IsCallingInfoLegal(-1, 0, bundleName), 0);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->IsCallingInfoLegal(0, -1, bundleName), 0);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->CheckRunningResourcesApply(0, "bundleName"), 1);
    std::list<PersistTime> resourceUnitList {};
    bgEfficiencyResourcesMgr_->RemoveListRecord(resourceUnitList, 0);

    resourceInfo->resourceNumber_ = ResourceType::CPU | ResourceType::WORK_SCHEDULER | ResourceType::COMMON_EVENT;
    resourceInfo->isProcess_ = true;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    bgEfficiencyResourcesMgr_->RemoveRelativeProcessRecord(0, 0);
    resourceInfo->isApply_ = false;
    resourceInfo->resourceNumber_ = ResourceType::WORK_SCHEDULER;
    resourceInfo->isProcess_ = false;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    resourceInfo->resourceNumber_ = ResourceType::TIMER;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    resourceInfo->isProcess_ = true;
    resourceInfo->resourceNumber_ = ResourceType::CPU;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    resourceInfo->resourceNumber_ = ResourceType::COMMON_EVENT;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo), (int32_t)ERR_OK);
    SleepFor(WAIT_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->appResourceApplyMap_.size(), 1);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 0);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS