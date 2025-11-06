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
#include <gtest/gtest.h>

#include "bgtask_common.h"
#include "background_task_subscriber.h"
#include "background_task_subscriber_proxy.h"
#include "bg_transient_task_mgr.h"
#include "bgtaskmgr_inner_errors.h"
#include "bundle_mgr_interface.h"
#include "config_data_source_type.h"
#include "expired_callback_proxy.h"
#include "expired_callback_stub.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "resources_subscriber_mgr.h"
#include "system_ability_definition.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr int32_t SLEEP_TIME = 500;
static constexpr int32_t DEFAULT_USERID = 100;
static constexpr int32_t JSON_FORMAT_DUMP = 4;
static constexpr int32_t TRANSIENT_EXEMPTED_QUOTA_TIME = 10000;
static constexpr char TRANSIENT_ERR_DELAYED_FROZEN_LIST[] = "transient_err_delayed_frozen_list";
static constexpr char CONTINUOUS_TASK_KEEPING_EXEMPTED_LIST[] = "continuous_task_keeping_exemption_list";
static constexpr char TRANSIENT_ERR_DELAYED_FROZEN_TIME[] = "transient_err_delayed_frozen_time";
static constexpr char TRANSIENT_EXEMPTED_QUOTA[] = "transient_exempted_quota";
static constexpr char CONFIG_JSON_INDEX_TOP[] = "params";
static constexpr char CONFIG_JSON_INDEX_SUSPEND_SECOND[] = "param";
static constexpr char LAUNCHER_BUNDLE_NAME[] = "com.ohos.launcher";
static constexpr char SCB_BUNDLE_NAME[] = "com.ohos.sceneboard";
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

std::shared_ptr<BgTransientTaskMgr> BgTaskManagerUnitTest::bgTransientTaskMgr_ = nullptr;

void BgTaskManagerUnitTest::SetUpTestCase()
{
    bgTransientTaskMgr_ = DelayedSingleton<BgTransientTaskMgr>::GetInstance();
}

void BgTaskManagerUnitTest::TearDownTestCase() {}

void BgTaskManagerUnitTest::SetUp() {}

void BgTaskManagerUnitTest::TearDown() {}

class TestBackgroundTaskSubscriber : public BackgroundTaskSubscriber {};

class TestExpiredCallbackStub : public ExpiredCallbackStub {
public:
    ErrCode OnExpired() override {return ERR_OK;}
    ErrCode OnExpiredAuth(int32_t authResult) override {return ERR_OK;}
};

/**
 * @tc.name: BgTaskManagerUnitTest_018
 * @tc.desc: test BgTransientTaskMgr init.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_018, TestSize.Level0)
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_019, TestSize.Level0)
{
    std::string bundleName;
    EXPECT_EQ(bgTransientTaskMgr_->IsCallingInfoLegal(-1, -1, bundleName, nullptr), ERR_BGTASK_INVALID_PID_OR_UID);
    EXPECT_EQ(bgTransientTaskMgr_->IsCallingInfoLegal(1, 1, bundleName, nullptr), ERR_BGTASK_INVALID_BUNDLE_NAME);
    int32_t uid = GetUidByBundleName(LAUNCHER_BUNDLE_NAME, DEFAULT_USERID);
    if (uid == -1) {
        uid = GetUidByBundleName(SCB_BUNDLE_NAME, DEFAULT_USERID);
    }
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_020, TestSize.Level0)
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_021, TestSize.Level0)
{
    bgTransientTaskMgr_->handler_ = nullptr;
    bgTransientTaskMgr_->HandleTransientTaskSuscriberTask(nullptr, TransientTaskEventType::TASK_START);
    bgTransientTaskMgr_->handler_ =
        std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create("tdd_test_handler"));
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_022, TestSize.Level0)
{
    bgTransientTaskMgr_->keyInfoMap_.clear();
    bgTransientTaskMgr_->isReady_.store(false);
    EXPECT_EQ(bgTransientTaskMgr_->CancelSuspendDelay(-1), ERR_BGTASK_SYS_NOT_READY);
    bgTransientTaskMgr_->isReady_.store(true);
    EXPECT_EQ(bgTransientTaskMgr_->CancelSuspendDelay(1), ERR_BGTASK_INVALID_REQUEST_ID);
    std::string bundleName = LAUNCHER_BUNDLE_NAME;
    int32_t uid = GetUidByBundleName(bundleName, DEFAULT_USERID);
    if (uid == -1) {
        bundleName = SCB_BUNDLE_NAME;
        uid = GetUidByBundleName(bundleName, DEFAULT_USERID);
    }
    auto keyInfo = std::make_shared<KeyInfo>(bundleName, uid);
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_023, TestSize.Level0)
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_024, TestSize.Level0)
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_025, TestSize.Level0)
{
    int32_t delayTime;
    bgTransientTaskMgr_->isReady_.store(false);
    EXPECT_EQ(bgTransientTaskMgr_->GetRemainingDelayTime(-1, delayTime), ERR_BGTASK_SYS_NOT_READY);
    bgTransientTaskMgr_->isReady_.store(true);
    EXPECT_EQ(bgTransientTaskMgr_->GetRemainingDelayTime(1, delayTime), ERR_BGTASK_INVALID_REQUEST_ID);
    std::string bundleName = LAUNCHER_BUNDLE_NAME;
    int32_t uid = GetUidByBundleName(bundleName, DEFAULT_USERID);
    if (uid == -1) {
        bundleName = SCB_BUNDLE_NAME;
        uid = GetUidByBundleName(bundleName, DEFAULT_USERID);
    }
    auto keyInfo = std::make_shared<KeyInfo>(bundleName, uid);
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
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_026, TestSize.Level0)
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
    dumpOption.emplace_back("PAUSE");
    dumpOption.emplace_back("1");
    EXPECT_EQ(bgTransientTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_OK);
    dumpOption.pop_back();
    dumpOption.pop_back();
    dumpOption.emplace_back("START");
    dumpOption.emplace_back("1");
    EXPECT_EQ(bgTransientTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_OK);
    dumpOption.pop_back();
    dumpOption.pop_back();
    dumpOption.emplace_back("invalid");
    EXPECT_EQ(bgTransientTaskMgr_->ShellDump(dumpOption, dumpInfo), ERR_BGTASK_PERMISSION_DENIED);
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
    bgTransientTaskMgr_->subscriberList_.clear();
    auto taskInfo = std::make_shared<TransientTaskAppInfo>();
    TestBackgroundTaskSubscriber subscriber1 = TestBackgroundTaskSubscriber();
    bgTransientTaskMgr_->subscriberList_.emplace_back(subscriber1.GetImpl());

    bgTransientTaskMgr_->NotifyTransientTaskSuscriber(taskInfo, TransientTaskEventType::TASK_START);
    bgTransientTaskMgr_->NotifyTransientTaskSuscriber(taskInfo, TransientTaskEventType::TASK_END);
    bgTransientTaskMgr_->NotifyTransientTaskSuscriber(taskInfo, TransientTaskEventType::TASK_ERR);
    bgTransientTaskMgr_->NotifyTransientTaskSuscriber(taskInfo, TransientTaskEventType::APP_TASK_START);
    bgTransientTaskMgr_->NotifyTransientTaskSuscriber(taskInfo, TransientTaskEventType::APP_TASK_END);
    EXPECT_NE((int32_t)bgTransientTaskMgr_->subscriberList_.size(), 0);
}

/**
 * @tc.name: BgTaskManagerUnitTest_043
 * @tc.desc: test BgTransientTaskMgr PauseTransientTaskTimeForInner.
 * @tc.type: FUNC
 * @tc.require: issueI936BL
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_043, TestSize.Level1)
{
    int32_t uid = -1;
    bgTransientTaskMgr_->isReady_.store(false);
    EXPECT_EQ(bgTransientTaskMgr_->PauseTransientTaskTimeForInner(uid), ERR_BGTASK_SYS_NOT_READY);

    bgTransientTaskMgr_->isReady_.store(true);
    EXPECT_EQ(bgTransientTaskMgr_->PauseTransientTaskTimeForInner(uid), ERR_BGTASK_INVALID_PID_OR_UID);

    uid = 1;
    EXPECT_EQ(bgTransientTaskMgr_->PauseTransientTaskTimeForInner(uid), ERR_BGTASK_SERVICE_INNER_ERROR);

    std::string bundleName = LAUNCHER_BUNDLE_NAME;
    uid = GetUidByBundleName(bundleName, DEFAULT_USERID);
    if (uid == -1) {
        bundleName = SCB_BUNDLE_NAME;
        uid = GetUidByBundleName(bundleName, DEFAULT_USERID);
    }
    bgTransientTaskMgr_->decisionMaker_->pkgBgDurationMap_.clear();
    EXPECT_NE(bgTransientTaskMgr_->PauseTransientTaskTimeForInner(uid), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_044
 * @tc.desc: test BgTransientTaskMgr StartTransientTaskTimeForInner.
 * @tc.type: FUNC
 * @tc.require: issueI936BL
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_044, TestSize.Level1)
{
    int32_t uid = -1;
    bgTransientTaskMgr_->isReady_.store(false);
    EXPECT_EQ(bgTransientTaskMgr_->StartTransientTaskTimeForInner(uid), ERR_BGTASK_SYS_NOT_READY);

    bgTransientTaskMgr_->isReady_.store(true);
    EXPECT_EQ(bgTransientTaskMgr_->StartTransientTaskTimeForInner(uid), ERR_BGTASK_INVALID_PID_OR_UID);

    uid = 1;
    EXPECT_EQ(bgTransientTaskMgr_->StartTransientTaskTimeForInner(uid), ERR_BGTASK_SERVICE_INNER_ERROR);

    std::string bundleName = LAUNCHER_BUNDLE_NAME;
    uid = GetUidByBundleName(bundleName, DEFAULT_USERID);
    if (uid == -1) {
        bundleName = SCB_BUNDLE_NAME;
        uid = GetUidByBundleName(bundleName, DEFAULT_USERID);
    }
    bgTransientTaskMgr_->decisionMaker_->pkgBgDurationMap_.clear();
    EXPECT_NE(bgTransientTaskMgr_->StartTransientTaskTimeForInner(uid), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_045
 * @tc.desc: test BgTransientTaskMgr SendCloudConfig.
 * @tc.type: FUNC
 * @tc.require: issueI936BL
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_045, TestSize.Level1)
{
    bgTransientTaskMgr_->isReady_.store(false);
    EXPECT_EQ(bgTransientTaskMgr_->SetBgTaskConfig("", ConfigDataSourceType::CONFIG_CLOUD), ERR_BGTASK_SYS_NOT_READY);

    bgTransientTaskMgr_->isReady_.store(true);
    EXPECT_EQ(bgTransientTaskMgr_->SetBgTaskConfig("", ConfigDataSourceType::CONFIG_CLOUD), ERR_PARAM_NUMBER_ERR);

    auto appInfo = nlohmann::json::array();
    appInfo.push_back("com.myapplication.demo1");
    appInfo.push_back("com.myapplication.demo2");
    nlohmann::json appParam;
    appParam[TRANSIENT_ERR_DELAYED_FROZEN_LIST] = appInfo;
    appParam[TRANSIENT_ERR_DELAYED_FROZEN_TIME] = TRANSIENT_EXEMPTED_QUOTA_TIME;
    const std::string strParam = appParam.dump(JSON_FORMAT_DUMP);
    EXPECT_EQ(bgTransientTaskMgr_->SetBgTaskConfig(strParam, ConfigDataSourceType::CONFIG_SUSPEND_MANAGER), ERR_OK);

    nlohmann::json param;
    param[TRANSIENT_EXEMPTED_QUOTA] = TRANSIENT_EXEMPTED_QUOTA_TIME;
    nlohmann::json params;
    params[TRANSIENT_ERR_DELAYED_FROZEN_LIST] = appInfo;

    auto appInfo2 = nlohmann::json::array();
    appInfo2.push_back("com.myapplication.demo1");
    appInfo2.push_back("com.myapplication.demo2");
    params[CONTINUOUS_TASK_KEEPING_EXEMPTED_LIST] = appInfo2;
    params[CONFIG_JSON_INDEX_SUSPEND_SECOND] = param;
    nlohmann::json cloudConfig;
    cloudConfig[CONFIG_JSON_INDEX_TOP] = params;
    const std::string cloudConfigStr = cloudConfig.dump(JSON_FORMAT_DUMP);
    EXPECT_EQ(bgTransientTaskMgr_->SetBgTaskConfig(cloudConfigStr, ConfigDataSourceType::CONFIG_CLOUD), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_046
 * @tc.desc: test BgTransientTaskMgr OnAppCacheStateChanged.
 * @tc.type: FUNC
 * @tc.require: issueIB08SV
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_046, TestSize.Level1)
{
    int32_t uid = 1;
    int32_t pid = 1;
    std::string bundleName = "bundleName";
    bgTransientTaskMgr_->isReady_.store(false);
    bgTransientTaskMgr_->OnAppCacheStateChanged(uid, pid, bundleName);

    bgTransientTaskMgr_->isReady_.store(true);
    uid = -1;
    bgTransientTaskMgr_->OnAppCacheStateChanged(uid, pid, bundleName);

    uid = 1;
    bgTransientTaskMgr_->OnAppCacheStateChanged(uid, pid, bundleName);
    EXPECT_NE(bgTransientTaskMgr_->CancelSuspendDelay(-1), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_047
 * @tc.desc: test BgTransientTaskMgr HandleSuspendManagerDie.
 * @tc.type: FUNC
 * @tc.require: issueIB08SV
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_047, TestSize.Level1)
{
    bgTransientTaskMgr_->HandleSuspendManagerDie();

    bgTransientTaskMgr_->transientPauseUid_.insert(1);
    bgTransientTaskMgr_->HandleSuspendManagerDie();
    int32_t uid = GetUidByBundleName(LAUNCHER_BUNDLE_NAME, DEFAULT_USERID);
    if (uid == -1) {
        uid = GetUidByBundleName(SCB_BUNDLE_NAME, DEFAULT_USERID);
    }
    bgTransientTaskMgr_->transientPauseUid_.insert(uid);
    bgTransientTaskMgr_->HandleSuspendManagerDie();
    EXPECT_EQ(bgTransientTaskMgr_->transientPauseUid_.size(), 0);
}

/**
 * @tc.name: BgTaskManagerUnitTest_048
 * @tc.desc: test BgTransientTaskMgr DumpTaskTime.
 * @tc.type: FUNC
 * @tc.require: issueIB08SV
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_048, TestSize.Level1)
{
    std::vector<std::string> dumpOption;
    std::vector<std::string> dumpInfo;
    int32_t uid = GetUidByBundleName(LAUNCHER_BUNDLE_NAME, DEFAULT_USERID);
    if (uid == -1) {
        uid = GetUidByBundleName(SCB_BUNDLE_NAME, DEFAULT_USERID);
    }
    std::string sUid = std::to_string(uid);
    dumpOption.emplace_back("-C");
    dumpOption.emplace_back("START");
    dumpOption.emplace_back(sUid);
    bgTransientTaskMgr_->DumpTaskTime(dumpOption, true, dumpInfo);
    EXPECT_EQ(dumpInfo.size(), 1);
    bgTransientTaskMgr_->DumpTaskTime(dumpOption, false, dumpInfo);
    EXPECT_EQ(dumpInfo.size(), 2);
}

/**
 * @tc.name: BgTaskManagerUnitTest_049
 * @tc.desc: test BgTransientTaskMgr OnRemoveSystemAbility.
 * @tc.type: FUNC
 * @tc.require: issueIB08SV
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_049, TestSize.Level1)
{
    bgTransientTaskMgr_->isReady_.store(true);
    bgTransientTaskMgr_->OnRemoveSystemAbility(-1, "");
    bgTransientTaskMgr_->OnRemoveSystemAbility(SUSPEND_MANAGER_SYSTEM_ABILITY_ID, "");
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BgTaskManagerUnitTest_050
 * @tc.desc: test BgtaskConfig Init.
 * @tc.type: FUNC
 * @tc.require: DTS2025031836567
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_050, TestSize.Level1)
{
    DelayedSingleton<BgtaskConfig>::GetInstance()->transientTaskExemptedQuatoList_.clear();
    DelayedSingleton<BgtaskConfig>::GetInstance()->isInit_ = true;
    DelayedSingleton<BgtaskConfig>::GetInstance()->Init();
    
    EXPECT_EQ(DelayedSingleton<BgtaskConfig>::GetInstance()->transientTaskExemptedQuatoList_.size(), 0);
}

/**
 * @tc.name: BgTaskManagerUnitTest_051
 * @tc.desc: test BgtaskConfig Init.
 * @tc.type: FUNC
 * @tc.require: DTS2025031836567
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_051, TestSize.Level1)
{
    DelayedSingleton<BgtaskConfig>::GetInstance()->transientTaskExemptedQuatoList_.clear();
    DelayedSingleton<BgtaskConfig>::GetInstance()->isInit_ = false;
    DelayedSingleton<BgtaskConfig>::GetInstance()->Init();
    DelayedSingleton<BgtaskConfig>::GetInstance()->transientTaskExemptedQuatoList_.emplace("com.myapplication.demo");
    EXPECT_NE(DelayedSingleton<BgtaskConfig>::GetInstance()->transientTaskExemptedQuatoList_.size(), 0);
}

/**
 * @tc.name: BgTaskManagerUnitTest_052
 * @tc.desc: test BgtaskConfig ParseTransientTaskExemptedQuatoList.
 * @tc.type: FUNC
 * @tc.require: DTS2025031836567
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_052, TestSize.Level1)
{
    DelayedSingleton<BgtaskConfig>::GetInstance()->transientTaskExemptedQuatoList_.clear();
    nlohmann::json root = nullptr;
    DelayedSingleton<BgtaskConfig>::GetInstance()->ParseTransientTaskExemptedQuatoList(root);
    EXPECT_EQ(DelayedSingleton<BgtaskConfig>::GetInstance()->transientTaskExemptedQuatoList_.size(), 0);

    nlohmann::json root2;
    DelayedSingleton<BgtaskConfig>::GetInstance()->ParseTransientTaskExemptedQuatoList(root2);
    EXPECT_EQ(DelayedSingleton<BgtaskConfig>::GetInstance()->transientTaskExemptedQuatoList_.size(), 0);

    root2[TRANSIENT_ERR_DELAYED_FROZEN_LIST] = 1;
    DelayedSingleton<BgtaskConfig>::GetInstance()->ParseTransientTaskExemptedQuatoList(root2);
    EXPECT_EQ(DelayedSingleton<BgtaskConfig>::GetInstance()->transientTaskExemptedQuatoList_.size(), 0);

    nlohmann::json root3;
    auto appInfo = nlohmann::json::array();
    appInfo.push_back("com.myapplication.demo1");
    appInfo.push_back("com.myapplication.demo2");
    root3[TRANSIENT_ERR_DELAYED_FROZEN_LIST] = appInfo;
    DelayedSingleton<BgtaskConfig>::GetInstance()->ParseTransientTaskExemptedQuatoList(root3);
    EXPECT_NE(DelayedSingleton<BgtaskConfig>::GetInstance()->transientTaskExemptedQuatoList_.size(), 0);
}

/**
 * @tc.name: BgTaskManagerUnitTest_053
 * @tc.desc: test BgtaskConfig GetTransientTaskExemptedQuato.
 * @tc.type: FUNC
 * @tc.require: DTS2025031836567
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_053, TestSize.Level1)
{
    int32_t timeValue = DelayedSingleton<BgtaskConfig>::GetInstance()->GetTransientTaskExemptedQuato();
    EXPECT_NE(timeValue, 0);
}

/**
 * @tc.name: BgTaskManagerUnitTest_054
 * @tc.desc: test BgtaskConfig ParseTransientTaskExemptedQuato.
 * @tc.type: FUNC
 * @tc.require: DTS2025031836567
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_054, TestSize.Level1)
{
    nlohmann::json root = nullptr;
    DelayedSingleton<BgtaskConfig>::GetInstance()->ParseTransientTaskExemptedQuato(root);
    int32_t timeValue = DelayedSingleton<BgtaskConfig>::GetInstance()->GetTransientTaskExemptedQuato();
    EXPECT_NE(timeValue, 0);

    nlohmann::json root2;
    DelayedSingleton<BgtaskConfig>::GetInstance()->ParseTransientTaskExemptedQuato(root2);
    int32_t timeValue2 = DelayedSingleton<BgtaskConfig>::GetInstance()->GetTransientTaskExemptedQuato();
    EXPECT_NE(timeValue2, 0);

    root2[TRANSIENT_EXEMPTED_QUOTA] = "1";
    DelayedSingleton<BgtaskConfig>::GetInstance()->ParseTransientTaskExemptedQuato(root2);
    int32_t timeValue3 = DelayedSingleton<BgtaskConfig>::GetInstance()->GetTransientTaskExemptedQuato();
    EXPECT_NE(timeValue3, 0);

    nlohmann::json root3;
    root3[TRANSIENT_EXEMPTED_QUOTA] = 0;
    DelayedSingleton<BgtaskConfig>::GetInstance()->ParseTransientTaskExemptedQuato(root3);
    int32_t timeValue4 = DelayedSingleton<BgtaskConfig>::GetInstance()->GetTransientTaskExemptedQuato();
    EXPECT_EQ(timeValue4, 0);
}

/**
 * @tc.name: BgTaskManagerUnitTest_055
 * @tc.desc: test GetAllTransientTasks.
 * @tc.type: FUNC
 * @tc.require: issueIC1HDY
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_055, TestSize.Level0)
{
    int32_t remainingQuota = -1;
    std::vector<std::shared_ptr<DelaySuspendInfo>> list;
    bgTransientTaskMgr_->isReady_.store(false);
    EXPECT_EQ(bgTransientTaskMgr_->GetAllTransientTasks(remainingQuota, list), ERR_BGTASK_TRANSIENT_SYS_NOT_READY);


    bgTransientTaskMgr_->isReady_.store(true);
    EXPECT_EQ(bgTransientTaskMgr_->GetAllTransientTasks(remainingQuota, list), ERR_OK);

    std::string bundleName = LAUNCHER_BUNDLE_NAME;
    int32_t uid = GetUidByBundleName(bundleName, DEFAULT_USERID);
    if (uid == -1) {
        bundleName = SCB_BUNDLE_NAME;
        uid = GetUidByBundleName(bundleName, DEFAULT_USERID);
    }
    auto keyInfo = std::make_shared<KeyInfo>(bundleName, uid);
    bgTransientTaskMgr_->keyInfoMap_.clear();
    bgTransientTaskMgr_->keyInfoMap_[1] = keyInfo;
    EXPECT_EQ(bgTransientTaskMgr_->GetAllTransientTasks(remainingQuota, list), ERR_OK);
}

/**
 * @tc.name: BgTaskManagerUnitTest_056
 * @tc.desc: test SetSupportedTaskKeepingProcesses.
 * @tc.type: FUNC
 * @tc.require: issueICVQZF
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_056, TestSize.Level1)
{
    std::set<std::string> processSet;
    processSet.insert("com.test.app");
    DelayedSingleton<BgtaskConfig>::GetInstance()->SetSupportedTaskKeepingProcesses(processSet);
    EXPECT_EQ(DelayedSingleton<BgtaskConfig>::GetInstance()->taskKeepingExemptedQuatoList_.empty(), false);
}

/**
 * @tc.name: BgTaskManagerUnitTest_057
 * @tc.desc: test SetMaliciousAppConfig.
 * @tc.type: FUNC
 * @tc.require: issueICVQZF
 */
HWTEST_F(BgTaskManagerUnitTest, BgTaskManagerUnitTest_057, TestSize.Level1)
{
    std::set<std::string> maliciousAppSet;
    maliciousAppSet.insert("com.test.app");
    DelayedSingleton<BgtaskConfig>::GetInstance()->SetMaliciousAppConfig(maliciousAppSet);
    EXPECT_EQ(DelayedSingleton<BgtaskConfig>::GetInstance()->maliciousAppBlocklist_.empty(), false);
}
}
}
