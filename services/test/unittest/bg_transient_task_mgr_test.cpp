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
#include <gtest/gtest.h>
#include <ctime>

#include "background_task_mgr_service.h"
#include "bg_transient_task_mgr.h"
#include "background_task_subscriber.h"
#include "transient_task_guard.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {
class TestBackgroundTaskSubscriber : public BackgroundTaskSubscriber {
public:
    void OnTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info) override {}
    void OnTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) override {}
};

class BgTransientTaskMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
    static std::shared_ptr<BgTransientTaskMgr> bgTransientTaskMgr_;
};

std::shared_ptr<BgTransientTaskMgr> BgTransientTaskMgrTest::bgTransientTaskMgr_ = nullptr;

void BgTransientTaskMgrTest::SetUpTestCase()
{
    bgTransientTaskMgr_ = std::make_shared<BgTransientTaskMgr>();
    bgTransientTaskMgr_->isReady_.store(true);
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("tdd_test_handler");
    bgTransientTaskMgr_->handler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
}

/**
 * @tc.name: SubscribeTransientTask_001
 * @tc.desc: subscribe transient task event.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET AR000GH86P AR000GH86O
 */
HWTEST_F(BgTransientTaskMgrTest, SubscribeTransientTask_001, TestSize.Level1)
{
    auto subscriber = std::make_shared<TestBackgroundTaskSubscriber>();
    auto ret = bgTransientTaskMgr_->SubscribeBackgroundTask(subscriber->GetImpl());
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.name: SubscribeTransientTask_002
 * @tc.desc: subscribe transient task event.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET AR000GH86P AR000GH86O
 */
HWTEST_F(BgTransientTaskMgrTest, SubscribeTransientTask_002, TestSize.Level1)
{
    auto ret = bgTransientTaskMgr_->SubscribeBackgroundTask(nullptr);
    EXPECT_FALSE(ret == ERR_OK);
}

/**
 * @tc.name: SubscribeTransientTask_003
 * @tc.desc: subscribe transient task event.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET AR000GH86P AR000GH86O
 */
HWTEST_F(BgTransientTaskMgrTest, SubscribeTransientTask_003, TestSize.Level1)
{
    auto subscriber = std::make_shared<TestBackgroundTaskSubscriber>();
    auto ret = bgTransientTaskMgr_->SubscribeBackgroundTask(subscriber->GetImpl());
    EXPECT_TRUE(ret == ERR_OK);

    ret = bgTransientTaskMgr_->SubscribeBackgroundTask(subscriber->GetImpl());
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.name: UnsubscribeTransientTask_001
 * @tc.desc: unsubscribe transient task event.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET AR000GH86P AR000GH86O
 */
HWTEST_F(BgTransientTaskMgrTest, UnsubscribeTransientTask_001, TestSize.Level1)
{
    auto subscriber = std::make_shared<TestBackgroundTaskSubscriber>();
    auto ret = bgTransientTaskMgr_->SubscribeBackgroundTask(subscriber->GetImpl());
    EXPECT_TRUE(ret == ERR_OK);

    usleep(1000);

    ret = bgTransientTaskMgr_->UnsubscribeBackgroundTask(subscriber->GetImpl());
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.name: UnsubscribeTransientTask_002
 * @tc.desc: unsubscribe transient task event.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET AR000GH86P AR000GH86O
 */
HWTEST_F(BgTransientTaskMgrTest, UnsubscribeTransientTask_002, TestSize.Level1)
{
    auto subscriber = std::make_shared<TestBackgroundTaskSubscriber>();
    auto ret = bgTransientTaskMgr_->UnsubscribeBackgroundTask(subscriber->GetImpl());
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.name: UnsubscribeTransientTask_003
 * @tc.desc: unsubscribe transient task event.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET AR000GH86P AR000GH86O
 */
HWTEST_F(BgTransientTaskMgrTest, UnsubscribeTransientTask_003, TestSize.Level1)
{
    auto ret = bgTransientTaskMgr_->UnsubscribeBackgroundTask(nullptr);
    EXPECT_FALSE(ret == ERR_OK);
}

/**
 * @tc.name: Marshalling_001
 * @tc.desc: marshalling transient task app info.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET AR000GH86Q
 */
HWTEST_F(BgTransientTaskMgrTest, Marshalling_001, TestSize.Level1)
{
    auto appInfo = std::make_shared<TransientTaskAppInfo>();
    MessageParcel data;
    EXPECT_TRUE(appInfo->Marshalling(data));

    std::shared_ptr<TransientTaskAppInfo> transientTaskAppInfo (TransientTaskAppInfo::Unmarshalling(data));
    EXPECT_TRUE(transientTaskAppInfo != nullptr);
}

/**
 * @tc.name: Unmarshalling_001
 * @tc.desc: unmarshalling transient task app info.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET AR000GH86Q
 */
HWTEST_F(BgTransientTaskMgrTest, Unmarshalling_001, TestSize.Level1)
{
    MessageParcel data;
    std::shared_ptr<TransientTaskAppInfo> transientTaskAppInfo (TransientTaskAppInfo::Unmarshalling(data));
    EXPECT_TRUE(transientTaskAppInfo == nullptr);
}

/**
 * @tc.name: TransientTaskGuard_001
 * @tc.desc: test TransientTaskGuard Start and Stop.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTransientTaskMgrTest, TransientTaskGuard_001, TestSize.Level1)
{
    auto mgr = std::make_shared<BgTransientTaskMgr>();
    auto guard = std::make_shared<TransientTaskGuard>(mgr);
    guard->Start();
    EXPECT_TRUE(guard->running_.load());
    guard->Stop();
    EXPECT_FALSE(guard->running_.load());
}

/**
 * @tc.name: TransientTaskGuard_002
 * @tc.desc: test TransientTaskGuard double Start is no-op.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTransientTaskMgrTest, TransientTaskGuard_002, TestSize.Level1)
{
    auto mgr = std::make_shared<BgTransientTaskMgr>();
    auto guard = std::make_shared<TransientTaskGuard>(mgr);
    guard->Start();
    guard->Start();
    EXPECT_TRUE(guard->running_.load());
    guard->Stop();
    EXPECT_FALSE(guard->running_.load());
}

/**
 * @tc.name: CheckAndCancelOvertimeTasks_001
 * @tc.desc: test CheckAndCancelOvertimeTasks with empty keyInfoMap.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTransientTaskMgrTest, CheckAndCancelOvertimeTasks_001, TestSize.Level1)
{
    bgTransientTaskMgr_->keyInfoMap_.clear();
    bgTransientTaskMgr_->CheckAndCancelOvertimeTasks();
    EXPECT_TRUE(bgTransientTaskMgr_->keyInfoMap_.empty());
}

/**
 * @tc.name: CheckAndCancelOvertimeTasks_002
 * @tc.desc: test CheckAndCancelOvertimeTasks does not cancel non-overtime task.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTransientTaskMgrTest, CheckAndCancelOvertimeTasks_002, TestSize.Level1)
{
    auto deviceInfoManeger = std::make_shared<DeviceInfoManager>();
    auto bgtaskService = sptr<BackgroundTaskMgrService>(new BackgroundTaskMgrService());
    auto runner = AppExecFwk::EventRunner::Create("tdd_test_handler");
    auto timerManager = std::make_shared<TimerManager>(bgtaskService, runner);
    auto decisionMaker = std::make_shared<DecisionMaker>(timerManager, deviceInfoManeger);
    auto watchdog = std::make_shared<Watchdog>(bgtaskService, decisionMaker, runner);

    auto mgr = std::make_shared<BgTransientTaskMgr>();
    mgr->isReady_.store(true);
    mgr->decisionMaker_ = decisionMaker;
    mgr->watchdog_ = watchdog;

    auto keyInfo = std::make_shared<KeyInfo>("bundleName", 1, 1);
    auto pkgInfo = std::make_shared<PkgDelaySuspendInfo>("bundleName", 1, timerManager);
    auto delayInfo = std::make_shared<DelaySuspendInfoEx>(1, 1, MSEC_PER_MIN);
    pkgInfo->requestList_.push_back(delayInfo);
    decisionMaker->pkgDelaySuspendInfoMap_[keyInfo] = pkgInfo;
    mgr->keyInfoMap_[1] = keyInfo;

    mgr->CheckAndCancelOvertimeTasks();
    EXPECT_EQ(mgr->keyInfoMap_.size(), 1);
}

/**
 * @tc.name: CheckAndCancelOvertimeTasks_003
 * @tc.desc: test CheckAndCancelOvertimeTasks cancels overtime task.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTransientTaskMgrTest, CheckAndCancelOvertimeTasks_003, TestSize.Level1)
{
    auto deviceInfoManeger = std::make_shared<DeviceInfoManager>();
    auto bgtaskService = sptr<BackgroundTaskMgrService>(new BackgroundTaskMgrService());
    auto runner = AppExecFwk::EventRunner::Create("tdd_test_handler");
    auto timerManager = std::make_shared<TimerManager>(bgtaskService, runner);
    auto decisionMaker = std::make_shared<DecisionMaker>(timerManager, deviceInfoManeger);
    auto watchdog = std::make_shared<Watchdog>(bgtaskService, decisionMaker, runner);

    auto mgr = std::make_shared<BgTransientTaskMgr>();
    mgr->isReady_.store(true);
    mgr->decisionMaker_ = decisionMaker;
    mgr->watchdog_ = watchdog;

    auto keyInfo = std::make_shared<KeyInfo>("bundleName", 1, 1);
    auto pkgInfo = std::make_shared<PkgDelaySuspendInfo>("bundleName", 1, timerManager);
    auto delayInfo = std::make_shared<DelaySuspendInfoEx>(1, 1, 0);
    pkgInfo->requestList_.push_back(delayInfo);
    decisionMaker->pkgDelaySuspendInfoMap_[keyInfo] = pkgInfo;
    mgr->keyInfoMap_[1] = keyInfo;

    mgr->CheckAndCancelOvertimeTasks();
    EXPECT_EQ(mgr->keyInfoMap_.size(), 0);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
