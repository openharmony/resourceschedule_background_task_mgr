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

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {
static constexpr int32_t SLEEP_TIME = 2000;
static constexpr int32_t REMAIN_TIME = 1000;
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
    bool isSuccess = false;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        nullptr, isSuccess), (int32_t)ERR_BGTASK_INVALID_PARAM);
    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo();
    resourceInfo->isApply_ = true;
    EXPECT_NE(resourceInfo, nullptr);
    resourceInfo->resourceNumber_ = 1 << MAX_RESOURCES_TYPE_NUM;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_BGTASK_INVALID_PARAM);
    resourceInfo->resourceNumber_ = 1;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_BGTASK_INVALID_PARAM);
    resourceInfo->isPersist_ = true;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    EXPECT_EQ(isSuccess, true);

    isSuccess = false;
    resourceInfo->isPersist_ = false;
    resourceInfo->timeOut_ = -10;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_BGTASK_INVALID_PARAM);
    resourceInfo->timeOut_ = 0;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_BGTASK_INVALID_PARAM);
    resourceInfo->timeOut_ = 10;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    EXPECT_EQ(isSuccess, true);
    isSuccess = false;
    resourceInfo->isPersist_ = true;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    EXPECT_EQ(isSuccess, true);
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
    bool isSuccess = false;
    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo(1, true, 0, "apply",
        true, false);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    EXPECT_EQ(isSuccess, true);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->appResourceApplyMap_.size(), 1);
    resourceInfo->isApply_ = false;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->appResourceApplyMap_.size(), 0);

    resourceInfo->isProcess_ = true;
    resourceInfo->isApply_ = true;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    EXPECT_EQ(isSuccess, true);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 1);
    resourceInfo->isApply_ = false;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
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
    bool isSuccess = false;
    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo(1, true, 0, "apply",
        true, true);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    EXPECT_EQ((int32_t)(bgEfficiencyResourcesMgr_->procResourceApplyMap_.size()), 1);
    resourceInfo->isPersist_ = false;
    resourceInfo->timeOut_ = SLEEP_TIME;
    resourceInfo->isProcess_ = false;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
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
    bool isSuccess = false;
    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo(1, true, 0, "apply",
        true, false);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    EXPECT_EQ(isSuccess, true);
    EXPECT_EQ((int32_t)(bgEfficiencyResourcesMgr_->appResourceApplyMap_.size()), 1);
    resourceInfo->isProcess_ = true;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    EXPECT_EQ(isSuccess, true);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 1);

    resourceInfo->isProcess_ = false;
    resourceInfo->isApply_ = false;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->appResourceApplyMap_.size(), 0);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 0);
    bgEfficiencyResourcesMgr_->ResetAllEfficiencyResources();
}

/**
 * @tc.name: ResetAllEfficiencyResources_001
 * @tc.desc: reset all efficiency resources using ResetAllEfficiencyResources function.
 * @tc.type: FUNC
 * @tc.require: issuesI5OD7X
 */
HWTEST_F(BgEfficiencyResourcesMgrTest, ResetAllEfficiencyResources_001, TestSize.Level1)
{
    bool isSuccess = false;
    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo(1, true, 0, "apply",
        true, false);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->appResourceApplyMap_.size(), 0);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 0);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    EXPECT_EQ(isSuccess, true);
    resourceInfo->resourceNumber_ = 1 << 1;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->appResourceApplyMap_.size(), 1);
    resourceInfo->isProcess_ = true;
    resourceInfo->resourceNumber_ = 1;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    resourceInfo->resourceNumber_ = 1 << 1;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 1);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ResetAllEfficiencyResources(), (int32_t)ERR_OK);
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
    bool isSuccess = false;
    sptr<EfficiencyResourceInfo> resourceInfo = new (std::nothrow) EfficiencyResourceInfo();
    resourceInfo->isApply_ = true;
    resourceInfo->resourceNumber_ = 1;
    resourceInfo->isPersist_ = true;
    resourceInfo->reason_ = "apply";
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->appResourceApplyMap_.size(), 0);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 0);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    resourceInfo->resourceNumber_ = 1 << 1;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->appResourceApplyMap_.size(), 1);
    resourceInfo->isProcess_ = true;
    resourceInfo->resourceNumber_ = 1;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    resourceInfo->resourceNumber_ = 1 << 1;
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ApplyEfficiencyResources(
        resourceInfo, isSuccess), (int32_t)ERR_OK);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->procResourceApplyMap_.size(), 1);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->ResetAllEfficiencyResources(), (int32_t)ERR_OK);
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
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->AddSubscriber(
        subscriber->GetImpl()), (int32_t)ERR_OK);
    SleepFor(SLEEP_TIME);
    EXPECT_EQ((int32_t)bgEfficiencyResourcesMgr_->AddSubscriber(
        subscriber->GetImpl()), (int32_t)ERR_BGTASK_OBJECT_EXISTS);
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
}  // namespace BackgroundTaskMgr
}  // namespace OHOS