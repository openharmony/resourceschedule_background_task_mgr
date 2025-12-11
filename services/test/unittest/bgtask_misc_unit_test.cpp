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

#include "ability_state_data.h"
#include "app_state_data.h"
#include "app_state_observer.h"
#include "background_task_mgr_service.h"
#include "bg_continuous_task_mgr.h"
#include "bg_efficiency_resources_mgr.h"
#include "bundle_info.h"
#include "bundle_manager_helper.h"
#include "common_event_data.h"
#include "continuous_task_record.h"
#include "decision_maker.h"
#include "delay_suspend_info_ex.h"
#include "device_info_manager.h"
#include "event_info.h"
#include "event_handler.h"
#include "event_runner.h"
#include "input_manager.h"
#include "key_info.h"
#include "notification.h"
#include "notification_sorting_map.h"
#include "notification_tools.h"
#include "pkg_delay_suspend_info.h"
#include "process_data.h"
#include "singleton.h"
#include "string_wrapper.h"
#include "suspend_controller.h"
#include "task_notification_subscriber.h"
#include "time_provider.h"
#include "timer_manager.h"
#include "watchdog.h"
#include "notification_content.h"
#include "int_wrapper.h"

using namespace testing::ext;

namespace OHOS {
extern void SetPublishContinuousTaskNotificationFlag(int32_t flag);
extern void SetCancelContinuousTaskNotificationFlag(int32_t flag);
extern void SetGetAllActiveNotificationsFlag(int32_t flag);

namespace BackgroundTaskMgr {
namespace {
static constexpr int32_t SLEEP_TIME = 500;
static constexpr int32_t BGTASKMGR_UID = 1096;
static constexpr int32_t TEST_NUM_ONE = 1;
static constexpr int32_t TEST_NUM_TWO = 2;
static constexpr int32_t MIN_ALLOW_QUOTA_TIME = 10 * MSEC_PER_SEC; // 10s
}

class BgTaskMiscUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    inline void SleepForFC()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    }
};

void BgTaskMiscUnitTest::SetUpTestCase() {}

void BgTaskMiscUnitTest::TearDownTestCase() {}

void BgTaskMiscUnitTest::SetUp() {}

void BgTaskMiscUnitTest::TearDown() {}

/**
 * @tc.name: AppStateObserverTest_001
 * @tc.desc: test AppStateObserver class CheckParamValid method.
 * @tc.type: FUNC
 * @tc.require: issueI4QT3W issueI4QU0V issueIB08SV
 */
HWTEST_F(BgTaskMiscUnitTest, AppStateObserverTest_001, TestSize.Level2)
{
    sptr<AppStateObserver> appStateObserver = sptr<AppStateObserver>(new AppStateObserver());
    AppExecFwk::ProcessData processData = AppExecFwk::ProcessData();
    appStateObserver->OnProcessDied(processData);
    appStateObserver->OnProcessDiedEfficiencyRes(processData);
    AppExecFwk::AbilityStateData abilityStateData = AppExecFwk::AbilityStateData();
    appStateObserver->OnAbilityStateChanged(abilityStateData);
    AppExecFwk::AppStateData appStateDataCache = AppExecFwk::AppStateData();
    // state is invalid
    appStateDataCache.state = static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_SET_COLD_START);
    appStateObserver->OnAppCacheStateChanged(appStateDataCache);
    // handle is null
    appStateDataCache.state = static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_CACHED);
    appStateObserver->OnAppCacheStateChanged(appStateDataCache);
    auto handler = std::make_shared<OHOS::AppExecFwk::EventHandler>(nullptr);
    appStateObserver->SetEventHandler(handler);
    appStateDataCache.uid = 1;
    appStateDataCache.pid = 1;
    appStateDataCache.bundleName = "bundleName";
    appStateObserver->OnAppCacheStateChanged(appStateDataCache);
    abilityStateData.abilityState = static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_TERMINATED);
    appStateObserver->OnAbilityStateChanged(abilityStateData);
    abilityStateData.abilityState = static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_CREATE);
    appStateObserver->OnAbilityStateChanged(abilityStateData);
    abilityStateData.abilityState = static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_TERMINATED);
    appStateObserver->OnAbilityStateChanged(abilityStateData);
    AppExecFwk::AppStateData appStateData = AppExecFwk::AppStateData();
    appStateObserver->OnAppStopped(appStateData);
    appStateData.uid = 1;
    appStateData.bundleName = "bundleName";
    appStateObserver->OnAppStopped(appStateData);
    appStateData.state = static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_TERMINATED);
    appStateObserver->OnAppStopped(appStateData);
    appStateData.state = static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_END);
    appStateObserver->OnAppStopped(appStateData);
    EXPECT_TRUE(appStateObserver->ValidateAppStateData(appStateData));
}

/**
 * @tc.name: BundleManagerHelperTest_001
 * @tc.desc: test BundleManagerHelper class.
 * @tc.type: FUNC
 * @tc.require: issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskMiscUnitTest, BundleManagerHelperTest_001, TestSize.Level2)
{
    EXPECT_EQ(BundleManagerHelper::GetInstance()->GetClientBundleName(1), "");
    EXPECT_FALSE(BundleManagerHelper::GetInstance()->CheckPermission("permission"));
    EXPECT_FALSE(BundleManagerHelper::GetInstance()->IsSystemApp(1LLU));
    AppExecFwk::BundleInfo bundleInfo;
    EXPECT_FALSE(BundleManagerHelper::GetInstance()->GetBundleInfo("bundleName",
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo));
    AppExecFwk::ApplicationInfo applicationInfo;
    EXPECT_FALSE(BundleManagerHelper::GetInstance()->GetApplicationInfo("bundleName",
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, 100, applicationInfo));
    BundleManagerHelper::GetInstance()->OnRemoteDied(nullptr);
    BundleManagerHelper::GetInstance()->bundleMgr_ = nullptr;
    BundleManagerHelper::GetInstance()->OnRemoteDied(nullptr);
    AAFwk::Want want;
    want.SetAction("action.system.home");
    want.AddEntity("entity.system.home");
    want.SetElementName("", "bundleName", "", "");
    AppExecFwk::AbilityInfo abilityInfo;
    BundleManagerHelper::GetInstance()->QueryAbilityInfo(want,
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, 0, abilityInfo);
    BundleManagerHelper::GetInstance()->bundleMgr_ = nullptr;
    BundleManagerHelper::GetInstance()->QueryAbilityInfo(want,
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, 0, abilityInfo);
}

/**
 * @tc.name: ContinuousTaskRecordTest_001
 * @tc.desc: test ContinuousTaskRecord class.
 * @tc.type: FUNC
 * @tc.require: issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskMiscUnitTest, ContinuousTaskRecordTest_001, TestSize.Level2)
{
    ContinuousTaskRecord record = ContinuousTaskRecord();
    std::string str1 = record.ParseToJsonStr();
    EXPECT_NE(str1, "");
    record.wantAgentInfo_ = std::make_shared<WantAgentInfo>();
    std::string str2 = record.ParseToJsonStr();
    EXPECT_NE(str2, "");
    ContinuousTaskRecord record2 = ContinuousTaskRecord();
    nlohmann::json json2 = nlohmann::json::parse("", nullptr, false);
    EXPECT_FALSE(record2.ParseFromJson(json2));
    nlohmann::json json3;
    json3["bundleName"] = "bundleName";
    EXPECT_FALSE(record2.ParseFromJson(json3));
    nlohmann::json json4 = nlohmann::json::parse(str1, nullptr, false);
    ContinuousTaskRecord record3 = ContinuousTaskRecord();
    EXPECT_TRUE(record3.ParseFromJson(json4));
    nlohmann::json json5 = nlohmann::json::parse(str2, nullptr, false);
    ContinuousTaskRecord record4 = ContinuousTaskRecord();
    EXPECT_TRUE(record4.ParseFromJson(json5));
}

/**
 * @tc.name: NotificationToolsTest_001
 * @tc.desc: test NotificationTools class.
 * @tc.type: FUNC
 * @tc.require: issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskMiscUnitTest, NotificationToolsTest_001, TestSize.Level2)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    auto taskRecord = std::make_shared<ContinuousTaskRecord>();
    NotificationTools::GetInstance()->PublishNotification(taskRecord, "appName", "prompt", 1);
    SetPublishContinuousTaskNotificationFlag(1);
    EXPECT_EQ(NotificationTools::GetInstance()->PublishNotification(taskRecord, "appName", "prompt", 1),
        ERR_BGTASK_NOTIFICATION_ERR);
    NotificationTools::GetInstance()->CancelNotification("label", 0);
    SetCancelContinuousTaskNotificationFlag(1);
    EXPECT_EQ(NotificationTools::GetInstance()->CancelNotification("label", 0), ERR_BGTASK_NOTIFICATION_ERR);
    std::set<std::string> notificationLabels;
    NotificationTools::GetInstance()->GetAllActiveNotificationsLabels(notificationLabels);

    std::map<std::string, std::pair<std::string, std::string>> newPromptInfos;
    newPromptInfos.emplace("label", std::make_pair<std::string, std::string>("test1", "test2"));
    SetGetAllActiveNotificationsFlag(TEST_NUM_ONE);
    SetPublishContinuousTaskNotificationFlag(0);
    SetPublishContinuousTaskNotificationFlag(0);
    NotificationTools::GetInstance()->RefreshContinuousNotifications(newPromptInfos, 0);
    SetGetAllActiveNotificationsFlag(TEST_NUM_TWO);
    SetPublishContinuousTaskNotificationFlag(1);
    NotificationTools::GetInstance()->RefreshContinuousNotifications(newPromptInfos, 0);
#endif
}

/**
 * @tc.name: TaskNotificationSubscriber_001
 * @tc.desc: test TaskNotificationSubscriber class.
 * @tc.type: FUNC
 * @tc.require: issueI4QT3W issueI4QU0V issueIBSI0L
 */
HWTEST_F(BgTaskMiscUnitTest, TaskNotificationSubscriber_001, TestSize.Level2)
{
    auto subscriber = std::make_shared<TaskNotificationSubscriber>();
    subscriber->OnCanceled(nullptr, nullptr, 1);
    auto notificationMap = std::make_shared<Notification::NotificationSortingMap>();
    auto notificationRequest = sptr<Notification::NotificationRequest>(new Notification::NotificationRequest());
    auto notification = std::make_shared<Notification::Notification>(notificationRequest);
    subscriber->OnCanceled(notification, notificationMap, 1);
    BgContinuousTaskMgr::GetInstance()->bgTaskUid_ = BGTASKMGR_UID;

    notification->request_->creatorUid_ = BGTASKMGR_UID;
    subscriber->OnCanceled(notification, notificationMap, 1);
    notification->request_->label_ = "label";
    subscriber->OnCanceled(notification, notificationMap, 1);
    notification->request_->label_ = "bgmode_1";
    subscriber->OnCanceled(notification, notificationMap, 1);
    notification->request_->label_ = "bgmode_1_1";
    subscriber->OnCanceled(notification, notificationMap, 1);
    notification->request_->label_ = "bgmode_1_1_1";
    subscriber->OnCanceled(notification, notificationMap,
        Notification::NotificationConstant::APP_CANCEL_REASON_DELETE);

    subscriber->OnCanceled(notification, notificationMap,
        Notification::NotificationConstant::USER_STOPPED_REASON_DELETE);

    std::shared_ptr<AAFwk::WantParams> extraInfo = std::make_shared<AAFwk::WantParams>();
    extraInfo->SetParam("abilityName", AAFwk::String::Box("abilityName"));
    notification->request_->additionalParams_ = extraInfo;
    subscriber->OnCanceled(notification, notificationMap,
        Notification::NotificationConstant::USER_STOPPED_REASON_DELETE);

    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    BgContinuousTaskMgr::GetInstance()->continuousTaskInfosMap_["1_abilityName"] = continuousTaskRecord;
    subscriber->OnCanceled(notification, notificationMap,
        Notification::NotificationConstant::USER_STOPPED_REASON_DELETE);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: TaskNotificationSubscriber_002
 * @tc.desc: test TaskNotificationSubscriber class.
 * @tc.type: FUNC
 * @tc.require: issueIBSI0L
 */
HWTEST_F(BgTaskMiscUnitTest, TaskNotificationSubscriber_002, TestSize.Level2)
{
    auto subscriber = std::make_shared<TaskNotificationSubscriber>();
    subscriber->OnConsumed(nullptr, nullptr);
    subscriber->OnUpdate(nullptr);
    subscriber->OnDied();
    subscriber->OnDoNotDisturbDateChange(nullptr);
    subscriber->OnEnabledNotificationChanged(nullptr);
    subscriber->OnBadgeChanged(nullptr);
    subscriber->OnBadgeEnabledChanged(nullptr);
    const std::vector<std::shared_ptr<Notification::Notification>> requestList;
    auto notificationMap = std::make_shared<Notification::NotificationSortingMap>();
    subscriber->OnBatchCanceled(requestList, notificationMap, 1);
    subscriber->OnDisconnected();
    EXPECT_TRUE(true);
}

/**
 * @tc.name: DecisionMakerTest_001
 * @tc.desc: test DecisionMaker class decide method.
 * @tc.type: FUNC
 * @tc.require: issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskMiscUnitTest, DecisionMakerTest_001, TestSize.Level2)
{
    auto deviceInfoManeger = std::make_shared<DeviceInfoManager>();
    auto bgtaskService = sptr<BackgroundTaskMgrService>(new BackgroundTaskMgrService());
    auto timerManager = std::make_shared<TimerManager>(bgtaskService,
        AppExecFwk::EventRunner::Create("tdd_test_handler"));
    auto decisionMaker = std::make_shared<DecisionMaker>(timerManager, deviceInfoManeger);
    decisionMaker->lastRequestTime_ = TimeProvider::GetCurrentTime();
    EXPECT_EQ(decisionMaker->Decide(nullptr, nullptr), ERR_BGTASK_NO_MEMORY);

    auto keyInfo = std::make_shared<KeyInfo>("bundleName1", 1);
    auto delayInfo = std::make_shared<DelaySuspendInfoEx>(1);
    decisionMaker->pkgBgDurationMap_[keyInfo] = TimeProvider::GetCurrentTime() - ALLOW_REQUEST_TIME_BG - 1;
    EXPECT_EQ(decisionMaker->Decide(keyInfo, delayInfo), ERR_BGTASK_NOT_IN_PRESET_TIME);
    decisionMaker->pkgBgDurationMap_[keyInfo] = TimeProvider::GetCurrentTime();
    EXPECT_EQ(decisionMaker->Decide(keyInfo, nullptr), ERR_BGTASK_NO_MEMORY);

    auto keyInfo2 = std::make_shared<KeyInfo>("bundleName2", 2);
    auto pkgDelaySuspendInfo = std::make_shared<PkgDelaySuspendInfo>("bundleName2", 2, timerManager);
    auto delayInfo1 = std::make_shared<DelaySuspendInfoEx>(1);
    auto delayInfo2 = std::make_shared<DelaySuspendInfoEx>(2);
    auto delayInfo3 = std::make_shared<DelaySuspendInfoEx>(3);
    pkgDelaySuspendInfo->requestList_.push_back(delayInfo1);
    pkgDelaySuspendInfo->requestList_.push_back(delayInfo2);
    pkgDelaySuspendInfo->requestList_.push_back(delayInfo3);
    decisionMaker->pkgDelaySuspendInfoMap_[keyInfo2] = pkgDelaySuspendInfo;
    EXPECT_EQ(decisionMaker->Decide(keyInfo2, delayInfo1), ERR_BGTASK_EXCEEDS_THRESHOLD);
    decisionMaker->pkgDelaySuspendInfoMap_.clear();
    deviceInfoManeger->isScreenOn_ = true;
    EXPECT_EQ(decisionMaker->Decide(keyInfo, delayInfo1), ERR_OK);
    decisionMaker->pkgDelaySuspendInfoMap_.clear();
    deviceInfoManeger->isScreenOn_ = false;
    EXPECT_EQ(decisionMaker->Decide(keyInfo, delayInfo1), ERR_OK);
}

/**
 * @tc.name: DecisionMakerTest_002
 * @tc.desc: test DecisionMaker class misc method.
 * @tc.type: FUNC
 * @tc.require: issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskMiscUnitTest, DecisionMakerTest_002, TestSize.Level2)
{
    auto deviceInfoManeger = std::make_shared<DeviceInfoManager>();
    auto bgtaskService = sptr<BackgroundTaskMgrService>(new BackgroundTaskMgrService());
    auto timerManager = std::make_shared<TimerManager>(bgtaskService,
        AppExecFwk::EventRunner::Create("tdd_test_handler"));
    auto decisionMaker = std::make_shared<DecisionMaker>(timerManager, deviceInfoManeger);

    decisionMaker->RemoveRequest(nullptr, -1);
    auto keyInfo = std::make_shared<KeyInfo>("bundleName1", 1);
    decisionMaker->RemoveRequest(keyInfo, -1);

    auto pkgDelaySuspendInfo = std::make_shared<PkgDelaySuspendInfo>("bundleName1", 1, timerManager);
    auto delayInfo1 = std::make_shared<DelaySuspendInfoEx>(1);
    pkgDelaySuspendInfo->requestList_.push_back(delayInfo1);
    decisionMaker->pkgDelaySuspendInfoMap_[keyInfo] = pkgDelaySuspendInfo;
    decisionMaker->RemoveRequest(keyInfo, -1);
    decisionMaker->RemoveRequest(keyInfo, 1);

    decisionMaker->pkgDelaySuspendInfoMap_.clear();
    EXPECT_EQ(decisionMaker->GetRemainingDelayTime(nullptr, -1), -1);
    EXPECT_EQ(decisionMaker->GetRemainingDelayTime(nullptr, -1), -1);
    decisionMaker->pkgDelaySuspendInfoMap_[keyInfo] = pkgDelaySuspendInfo;
    EXPECT_EQ(decisionMaker->GetRemainingDelayTime(keyInfo, -1), 0);

    EXPECT_EQ(decisionMaker->GetQuota(nullptr), -1);
    decisionMaker->pkgDelaySuspendInfoMap_.clear();
    EXPECT_EQ(decisionMaker->GetQuota(keyInfo), INIT_QUOTA);
    pkgDelaySuspendInfo->quota_ = -1;
    decisionMaker->pkgDelaySuspendInfoMap_[keyInfo] = pkgDelaySuspendInfo;
    EXPECT_EQ(decisionMaker->GetQuota(keyInfo), 0);
    EXPECT_FALSE(decisionMaker->IsFrontApp("pkgName", 1));

    decisionMaker->requestId_ = INT_MAX;
    EXPECT_EQ(decisionMaker->NewDelaySuspendRequestId(), 1);
    EXPECT_EQ(decisionMaker->NewDelaySuspendRequestId(), TEST_NUM_TWO);

    decisionMaker->lastRequestTime_ = TimeProvider::GetCurrentTime() - 1;
    decisionMaker->ResetDayQuotaLocked();
    decisionMaker->lastRequestTime_ = TimeProvider::GetCurrentTime() - QUOTA_UPDATE - 1;
    decisionMaker->pkgDelaySuspendInfoMap_.clear();
    decisionMaker->pkgDelaySuspendInfoMap_[keyInfo] = pkgDelaySuspendInfo;
    auto keyInfo2 = std::make_shared<KeyInfo>("bundleName2", TEST_NUM_TWO);
    auto pkgDelaySuspendInfo2 = std::make_shared<PkgDelaySuspendInfo>("bundleName2", TEST_NUM_TWO, timerManager);
    decisionMaker->pkgDelaySuspendInfoMap_[keyInfo2] = pkgDelaySuspendInfo2;
    decisionMaker->ResetDayQuotaLocked();

    EventInfo eventInfo = EventInfo();
    eventInfo.eventId_ = 0;
    decisionMaker->OnInputEvent(eventInfo);
    eventInfo.eventId_ = EVENT_SCREEN_ON;
    decisionMaker->OnInputEvent(eventInfo);
    eventInfo.eventId_ = EVENT_SCREEN_UNLOCK;
    decisionMaker->OnInputEvent(eventInfo);
    decisionMaker->pkgDelaySuspendInfoMap_[keyInfo2] = pkgDelaySuspendInfo2;
    eventInfo.eventId_ = EVENT_SCREEN_OFF;
    decisionMaker->OnInputEvent(eventInfo);
}

/**
 * @tc.name: DeviceInfoManagerTest_001
 * @tc.desc: test DeviceInfoManager class.
 * @tc.type: FUNC
 * @tc.require: issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskMiscUnitTest, DeviceInfoManagerTest_001, TestSize.Level2)
{
    auto deviceInfoManeger = std::make_shared<DeviceInfoManager>();
    deviceInfoManeger->isDump_ = true;
    EventInfo eventInfo = EventInfo();
    std::vector<std::string> args;
    args.emplace_back("test");
    eventInfo.SetStringArgs(args);
    deviceInfoManeger->OnInputEvent(eventInfo);
    args.clear();
    args.emplace_back("dump");
    eventInfo.SetStringArgs(args);
    deviceInfoManeger->isDump_ = false;
    eventInfo.eventId_ = EVENT_SCREEN_ON;
    deviceInfoManeger->OnInputEvent(eventInfo);
    eventInfo.eventId_ = EVENT_SCREEN_OFF;
    deviceInfoManeger->OnInputEvent(eventInfo);
    eventInfo.eventId_ = EVENT_SCREEN_UNLOCK;
    deviceInfoManeger->OnInputEvent(eventInfo);
    eventInfo.eventId_ = EVENT_BATTERY_LOW;
    deviceInfoManeger->OnInputEvent(eventInfo);
    eventInfo.eventId_ = EVENT_BATTERY_OKAY;
    deviceInfoManeger->OnInputEvent(eventInfo);
    eventInfo.eventId_ = EVENT_MAX;
    deviceInfoManeger->OnInputEvent(eventInfo);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: PkgDelaySuspendInfoTest_001
 * @tc.desc: test PkgDelaySuspendInfo class.
 * @tc.type: FUNC
 * @tc.require: issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskMiscUnitTest, PkgDelaySuspendInfoTest_001, TestSize.Level2)
{
    auto bgtaskService = sptr<BackgroundTaskMgrService>(new BackgroundTaskMgrService());
    auto timerManager = std::make_shared<TimerManager>(bgtaskService,
        AppExecFwk::EventRunner::Create("tdd_test_handler"));
    auto pkgDelaySuspendInfo = std::make_shared<PkgDelaySuspendInfo>("bundleName1", 1, timerManager);
    pkgDelaySuspendInfo->isCounting_ = true;
    pkgDelaySuspendInfo->baseTime_ = (int32_t)TimeProvider::GetCurrentTime() + MIN_ALLOW_QUOTA_TIME;
    EXPECT_EQ(pkgDelaySuspendInfo->IsAllowRequest(), ERR_OK);
    pkgDelaySuspendInfo->quota_ = 0;
    pkgDelaySuspendInfo->baseTime_ = (int32_t)TimeProvider::GetCurrentTime();
    EXPECT_EQ(pkgDelaySuspendInfo->IsAllowRequest(), ERR_BGTASK_TIME_INSUFFICIENT);
    auto delayInfo1 = std::make_shared<DelaySuspendInfoEx>(1, 1);
    auto delayInfo2 = std::make_shared<DelaySuspendInfoEx>(1, 2);
    pkgDelaySuspendInfo->AddRequest(delayInfo1, 1);
    pkgDelaySuspendInfo->AddRequest(delayInfo1, 1);
    pkgDelaySuspendInfo->AddRequest(delayInfo2, 1);
    EXPECT_EQ(pkgDelaySuspendInfo->IsAllowRequest(), ERR_BGTASK_EXCEEDS_THRESHOLD);
    pkgDelaySuspendInfo->RemoveRequest(1);
    pkgDelaySuspendInfo->RemoveRequest(1);
    pkgDelaySuspendInfo->RemoveRequest(2);

    pkgDelaySuspendInfo->requestList_.clear();
    EXPECT_EQ(pkgDelaySuspendInfo->GetRemainDelayTime(-1), 0);
    delayInfo1->actualDelayTime_ = 1;
    auto delayInfo3 = std::make_shared<DelaySuspendInfoEx>(1, 1, 1);
    pkgDelaySuspendInfo->requestList_.emplace_back(delayInfo3);
    EXPECT_EQ(pkgDelaySuspendInfo->GetRemainDelayTime(-1), 0);
    EXPECT_EQ(pkgDelaySuspendInfo->GetRemainDelayTime(1), 1);
}

/**
 * @tc.name: PkgDelaySuspendInfoTest_002
 * @tc.desc: test PkgDelaySuspendInfo class.
 * @tc.type: FUNC
 * @tc.require: issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskMiscUnitTest, PkgDelaySuspendInfoTest_002, TestSize.Level2)
{
    auto bgtaskService = sptr<BackgroundTaskMgrService>(new BackgroundTaskMgrService());
    auto timerManager =
        std::make_shared<TimerManager>(bgtaskService, AppExecFwk::EventRunner::Create("tdd_test_handler"));
    auto pkgDelaySuspendInfo = std::make_shared<PkgDelaySuspendInfo>("bundleName1", 1, timerManager);
    auto delayInfo1 = std::make_shared<DelaySuspendInfoEx>(1, 1);

    pkgDelaySuspendInfo->requestList_.clear();
    pkgDelaySuspendInfo->StartAccounting(1);
    pkgDelaySuspendInfo->AddRequest(delayInfo1, 1);
    pkgDelaySuspendInfo->StartAccounting(2);
    pkgDelaySuspendInfo->StartAccounting(-1);
    pkgDelaySuspendInfo->isCounting_ = false;
    pkgDelaySuspendInfo->baseTime_ = 0;
    pkgDelaySuspendInfo->StartAccounting(1);
    pkgDelaySuspendInfo->isCounting_ = true;
    pkgDelaySuspendInfo->baseTime_ = 1;
    pkgDelaySuspendInfo->StartAccounting(1);

    pkgDelaySuspendInfo->requestList_.clear();
    pkgDelaySuspendInfo->StopAccounting(1);
    pkgDelaySuspendInfo->AddRequest(delayInfo1, 1);
    pkgDelaySuspendInfo->StopAccounting(-1);
    pkgDelaySuspendInfo->baseTime_ = 0;
    pkgDelaySuspendInfo->StopAccounting(1);
    pkgDelaySuspendInfo->baseTime_ = 1;
    pkgDelaySuspendInfo->StopAccounting(1);
    pkgDelaySuspendInfo->requestList_.clear();
    pkgDelaySuspendInfo->StopAccountingAll();
    pkgDelaySuspendInfo->AddRequest(delayInfo1, 1);
    pkgDelaySuspendInfo->baseTime_ = 0;
    pkgDelaySuspendInfo->StopAccounting(1);
    pkgDelaySuspendInfo->baseTime_ = 1;
    pkgDelaySuspendInfo->StopAccounting(1);

    pkgDelaySuspendInfo->isCounting_ = true;
    pkgDelaySuspendInfo->quota_ = 0;
    pkgDelaySuspendInfo->baseTime_ = (int32_t)TimeProvider::GetCurrentTime() - 1;
    pkgDelaySuspendInfo->UpdateQuota(false);
    pkgDelaySuspendInfo->quota_ = 0;
    pkgDelaySuspendInfo->baseTime_ = (int32_t)TimeProvider::GetCurrentTime() + 1;
    pkgDelaySuspendInfo->UpdateQuota(true);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: SuspendControllerTest_001
 * @tc.desc: test SuspendController.
 * @tc.type: FUNC
 * @tc.require: issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskMiscUnitTest, SuspendControllerTest_001, TestSize.Level2)
{
    SuspendController suspendController = SuspendController();
    auto keyInfo = std::make_shared<KeyInfo>("bundleName", 1);
    suspendController.RequestSuspendDelay(nullptr);
    suspendController.RequestSuspendDelay(keyInfo);
    suspendController.CancelSuspendDelay(nullptr);
    suspendController.CancelSuspendDelay(keyInfo);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: ConfigChangeObserver_001
 * @tc.desc: test ConfigChangeObserver class.
 * @tc.type: FUNC
 * @tc.require: issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskMiscUnitTest, ConfigChangeObserver_001, TestSize.Level2)
{
    // Given
    sptr<ConfigChangeObserver> configChangeObserver1 = sptr<ConfigChangeObserver>(
        new ConfigChangeObserver(nullptr, nullptr));
    
    // When & Then
    EXPECT_FALSE(configChangeObserver1->CheckExpired());
}

/**
 * @tc.name: ConfigChangeObserver_002
 * @tc.desc: test ConfigChangeObserver class.
 * @tc.type: FUNC
 * @tc.require: issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskMiscUnitTest, ConfigChangeObserver_002, TestSize.Level2)
{
    // Given
    auto handler = std::make_shared<OHOS::AppExecFwk::EventHandler>(nullptr);
    auto bgContinuousTaskMgr = std::make_shared<BgContinuousTaskMgr>();
    sptr<ConfigChangeObserver> configChangeObserver2 = sptr<ConfigChangeObserver>(
        new ConfigChangeObserver(handler, nullptr));

    // When & Then
    EXPECT_FALSE(configChangeObserver2->CheckExpired());
}

/**
 * @tc.name: ConfigChangeObserver_003
 * @tc.desc: test ConfigChangeObserver class.
 * @tc.type: FUNC
 * @tc.require: issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskMiscUnitTest, ConfigChangeObserver_003, TestSize.Level2)
{
    // Given
    auto handler = std::make_shared<OHOS::AppExecFwk::EventHandler>(nullptr);
    auto bgContinuousTaskMgr = std::make_shared<BgContinuousTaskMgr>();
    sptr<ConfigChangeObserver> configChangeObserver3 = sptr<ConfigChangeObserver>(
        new ConfigChangeObserver(handler, bgContinuousTaskMgr));

    // When & Then
    EXPECT_TRUE(configChangeObserver3->CheckExpired());

    AppExecFwk::Configuration configuration;
    configChangeObserver3->OnConfigurationUpdated(configuration);
    SUCCEED();
}

/**
 * @tc.name: DataStorageHelper_001
 * @tc.desc: test Watchdog class.
 * @tc.type: FUNC
 * @tc.require: issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskMiscUnitTest, DataStorageHelper_001, TestSize.Level2)
{
    std::unordered_map<std::string, std::shared_ptr<ContinuousTaskRecord>> continuousTaskInfosMap1;
    auto continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    continuousTaskInfosMap1.emplace("key", continuousTaskRecord);
    DelayedSingleton<DataStorageHelper>::GetInstance()->RefreshTaskRecord(continuousTaskInfosMap1);
    std::unordered_map<std::string, std::shared_ptr<ContinuousTaskRecord>> continuousTaskInfosMap2;
    EXPECT_EQ(DelayedSingleton<DataStorageHelper>::GetInstance()->RestoreTaskRecord(continuousTaskInfosMap2),
        ERR_OK);
    EXPECT_EQ(DelayedSingleton<DataStorageHelper>::GetInstance()->SaveJsonValueToFile("", ""),
        ERR_BGTASK_GET_ACTUAL_FILE_ERR);
    nlohmann::json json1;
    EXPECT_EQ(DelayedSingleton<DataStorageHelper>::GetInstance()->ParseJsonValueFromFile(json1, ""),
        ERR_BGTASK_DATA_STORAGE_ERR);
    std::string fullPath;
    EXPECT_FALSE(DelayedSingleton<DataStorageHelper>::GetInstance()->ConvertFullPath("", fullPath));
}

/**
 * @tc.name: DataStorageHelper_002
 * @tc.desc: test ParseFastSuspendDozeTime.
 * @tc.type: FUNC
 * @tc.require: issue#I99360
 */
HWTEST_F(BgTaskMiscUnitTest, DataStorageHelper_002, TestSize.Level2)
{
    int time = -1;
    // 文件路径错误
    std::string file("");
    EXPECT_FALSE(DelayedSingleton<DataStorageHelper>::GetInstance()->ParseFastSuspendDozeTime(file, time));
    // 文件路径正确
    file = "/etc/efficiency_manager/suspend_manager_config.json";
    EXPECT_FALSE(DelayedSingleton<DataStorageHelper>::GetInstance()->ParseFastSuspendDozeTime(file, time));
}

/**
 * @tc.name: DecisionMakerTest_004
 * @tc.desc: test PauseTransientTaskTimeForInner.
 * @tc.type: FUNC
 * @tc.require: issueI936BL
 */
HWTEST_F(BgTaskMiscUnitTest, DecisionMakerTest_004, TestSize.Level2)
{
    auto deviceInfoManeger = std::make_shared<DeviceInfoManager>();
    auto bgtaskService = sptr<BackgroundTaskMgrService>(new BackgroundTaskMgrService());
    auto timerManager = std::make_shared<TimerManager>(bgtaskService,
        AppExecFwk::EventRunner::Create("tdd_test_handler"));
    auto decisionMaker = std::make_shared<DecisionMaker>(timerManager, deviceInfoManeger);

    decisionMaker->pkgBgDurationMap_.clear();
    std::string name = "bundleName1";
    int32_t uid = 1;
    EXPECT_EQ(decisionMaker->PauseTransientTaskTimeForInner(uid, name), ERR_BGTASK_FOREGROUND);

    auto keyInfo = std::make_shared<KeyInfo>("bundleName1", 1);
    decisionMaker->pkgBgDurationMap_[keyInfo] = TimeProvider::GetCurrentTime() - ALLOW_REQUEST_TIME_BG - 1;
    decisionMaker->pkgDelaySuspendInfoMap_.clear();
    EXPECT_EQ(decisionMaker->PauseTransientTaskTimeForInner(uid, name), ERR_BGTASK_NOREQUEST_TASK);
    
    auto keyInfo1 = std::make_shared<KeyInfo>("bundleName1", 1);
    auto pkgDelaySuspendInfo = std::make_shared<PkgDelaySuspendInfo>("bundleName1", 1, timerManager);
    auto delayInfo = std::make_shared<DelaySuspendInfoEx>(1);
    pkgDelaySuspendInfo->requestList_.push_back(delayInfo);
    decisionMaker->pkgDelaySuspendInfoMap_[keyInfo1] = pkgDelaySuspendInfo;
    EXPECT_EQ(decisionMaker->PauseTransientTaskTimeForInner(uid, name), ERR_OK);
}

/**
 * @tc.name: DecisionMakerTest_005
 * @tc.desc: test StartTransientTaskTimeForInner.
 * @tc.type: FUNC
 * @tc.require: issueI936BL
 */
HWTEST_F(BgTaskMiscUnitTest, DecisionMakerTest_005, TestSize.Level2)
{
    auto deviceInfoManeger = std::make_shared<DeviceInfoManager>();
    auto bgtaskService = sptr<BackgroundTaskMgrService>(new BackgroundTaskMgrService());
    auto timerManager = std::make_shared<TimerManager>(bgtaskService,
        AppExecFwk::EventRunner::Create("tdd_test_handler"));
    auto decisionMaker = std::make_shared<DecisionMaker>(timerManager, deviceInfoManeger);

    decisionMaker->pkgBgDurationMap_.clear();
    std::string name = "bundleName1";
    int32_t uid = 1;
    EXPECT_EQ(decisionMaker->StartTransientTaskTimeForInner(uid, name), ERR_BGTASK_FOREGROUND);

    auto keyInfo = std::make_shared<KeyInfo>("bundleName1", 1);
    decisionMaker->pkgBgDurationMap_[keyInfo] = TimeProvider::GetCurrentTime() - ALLOW_REQUEST_TIME_BG - 1;
    decisionMaker->pkgDelaySuspendInfoMap_.clear();
    EXPECT_EQ(decisionMaker->StartTransientTaskTimeForInner(uid, name), ERR_BGTASK_NOREQUEST_TASK);
    
    auto keyInfo1 = std::make_shared<KeyInfo>("bundleName1", 1);
    auto pkgDelaySuspendInfo = std::make_shared<PkgDelaySuspendInfo>("bundleName1", 1, timerManager);
    auto delayInfo = std::make_shared<DelaySuspendInfoEx>(1);
    pkgDelaySuspendInfo->requestList_.push_back(delayInfo);
    decisionMaker->pkgDelaySuspendInfoMap_[keyInfo1] = pkgDelaySuspendInfo;
    decisionMaker->foregroundUidPidMap_.clear();
    EXPECT_EQ(decisionMaker->StartTransientTaskTimeForInner(uid, name), ERR_OK);
}

/**
 * @tc.name: DecisionMakerTest_006
 * @tc.desc: test GetRequestListByKey.
 * @tc.type: FUNC
 * @tc.require: issueIB08SV
 */
HWTEST_F(BgTaskMiscUnitTest, DecisionMakerTest_006, TestSize.Level2)
{
    auto deviceInfoManeger = std::make_shared<DeviceInfoManager>();
    auto bgtaskService = sptr<BackgroundTaskMgrService>(new BackgroundTaskMgrService());
    auto timerManager = std::make_shared<TimerManager>(bgtaskService,
        AppExecFwk::EventRunner::Create("tdd_test_handler"));
    auto decisionMaker = std::make_shared<DecisionMaker>(timerManager, deviceInfoManeger);

    vector<int32_t> requestIdList;
    requestIdList = decisionMaker->GetRequestIdListByKey(nullptr);
    EXPECT_TRUE(requestIdList.empty());

    auto keyInfo = std::make_shared<KeyInfo>("bundleName", 1, 1);
    decisionMaker->pkgDelaySuspendInfoMap_.clear();
    requestIdList = decisionMaker->GetRequestIdListByKey(keyInfo);
    EXPECT_TRUE(requestIdList.empty());

    decisionMaker->pkgDelaySuspendInfoMap_.clear();
    auto pkgDelaySuspendInfo = std::make_shared<PkgDelaySuspendInfo>("bundleName", 1, timerManager);
    auto delayInfo1 = std::make_shared<DelaySuspendInfoEx>(1);
    pkgDelaySuspendInfo->requestList_.push_back(delayInfo1);
    decisionMaker->pkgDelaySuspendInfoMap_[keyInfo] = pkgDelaySuspendInfo;
    requestIdList = decisionMaker->GetRequestIdListByKey(keyInfo);
    EXPECT_FALSE(requestIdList.empty());
    decisionMaker->ResetAppMgrProxy();
    EXPECT_EQ(decisionMaker->appMgrProxy_, nullptr);
}

/**
 * @tc.name: DelaySuspendInfoEx_001
 * @tc.desc: test DelaySuspendInfoEx.
 * @tc.type: FUNC
 * @tc.require: issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskMiscUnitTest, DelaySuspendInfoEx_001, TestSize.Level2)
{
    auto delayInfo = std::make_shared<DelaySuspendInfoEx>(1);
    delayInfo->baseTime_ = 1;
    delayInfo->StartAccounting();
    delayInfo->baseTime_ = 0;
    delayInfo->StopAccounting();
    EXPECT_EQ(delayInfo->spendTime_, 0);
}

/**
 * @tc.name: SystemEventObserverTest_001
 * @tc.desc: test SystemEventObserver class.
 * @tc.type: FUNC
 * @tc.require: issueI4QT3W issueI4QU0V
 */
HWTEST_F(BgTaskMiscUnitTest, SystemEventObserver_001, TestSize.Level2)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);
    EventFwk::CommonEventSubscribeInfo commonEventSubscribeInfo(matchingSkills);
    auto systemEventListener = std::make_shared<SystemEventObserver>(commonEventSubscribeInfo);

    EventFwk::CommonEventData eventData = EventFwk::CommonEventData();
    systemEventListener->OnReceiveEvent(eventData);

    auto handler = std::make_shared<OHOS::AppExecFwk::EventHandler>(nullptr);
    systemEventListener->SetEventHandler(handler);
    systemEventListener->OnReceiveEventContinuousTask(eventData);
    auto bgContinuousTaskMgr = std::make_shared<BgContinuousTaskMgr>();
    systemEventListener->SetBgContinuousTaskMgr(bgContinuousTaskMgr);
    systemEventListener->OnReceiveEventContinuousTask(eventData);
    AAFwk::Want want = AAFwk::Want();
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    eventData.SetWant(want);
    systemEventListener->OnReceiveEventContinuousTask(eventData);
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_ADDED);
    eventData.SetWant(want);
    systemEventListener->OnReceiveEventContinuousTask(eventData);
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED);
    eventData.SetWant(want);
    systemEventListener->OnReceiveEventContinuousTask(eventData);

    EventFwk::CommonEventData eventData2 = EventFwk::CommonEventData();
    AAFwk::Want want2 = AAFwk::Want();
    want2.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    eventData2.SetWant(want2);
    systemEventListener->OnReceiveEventEfficiencyRes(eventData2);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: OnProcessStateChanged_001
 * @tc.desc: test ApplicationStateObserver class.
 * @tc.type: FUNC
 * @tc.require: https://gitcode.com/openharmony/resourceschedule_background_task_mgr/issues/776
 */
HWTEST_F(BgTaskMiscUnitTest, OnProcessStateChanged_001, TestSize.Level2)
{
    auto deviceInfoManeger = std::make_shared<DeviceInfoManager>();
    auto bgtaskService = sptr<BackgroundTaskMgrService>(new BackgroundTaskMgrService());
    auto timerManager =
        std::make_shared<TimerManager>(bgtaskService, AppExecFwk::EventRunner::Create("tdd_test_handler"));
    auto decisionMaker = std::make_shared<DecisionMaker>(timerManager, deviceInfoManeger);
    auto applicationStateObserver = sptr<DecisionMaker::ApplicationStateObserver>(
        new (std::nothrow) DecisionMaker::ApplicationStateObserver(*decisionMaker));

    AppExecFwk::ProcessData processData;
    processData.uid = 1;
    processData.bundleName = "bundleName1";
    processData.state = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    applicationStateObserver->OnProcessStateChanged(processData);

    auto keyInfo1 = std::make_shared<KeyInfo>("bundleName1", 1);
    auto pkgDelaySuspendInfo = std::make_shared<PkgDelaySuspendInfo>("bundleName1", 1, timerManager);
    auto delayInfo = std::make_shared<DelaySuspendInfoEx>(1);
    pkgDelaySuspendInfo->requestList_.push_back(delayInfo);
    decisionMaker->pkgDelaySuspendInfoMap_[keyInfo1] = pkgDelaySuspendInfo;
    auto keyInfo = std::make_shared<KeyInfo>("bundleName1", 1);
    decisionMaker->pkgBgDurationMap_[keyInfo] = TimeProvider::GetCurrentTime() - ALLOW_REQUEST_TIME_BG - 1;
    processData.state = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    applicationStateObserver->OnProcessStateChanged(processData);

    decisionMaker->pkgDelaySuspendInfoMap_.clear();
    processData.state = AppExecFwk::AppProcessState::APP_STATE_BACKGROUND;
    applicationStateObserver->OnProcessStateChanged(processData);
    decisionMaker->pkgDelaySuspendInfoMap_[keyInfo1] = pkgDelaySuspendInfo;
    applicationStateObserver->OnProcessStateChanged(processData);
    EXPECT_EQ((int32_t)decisionMaker->pkgDelaySuspendInfoMap_.size(), 1);
}

/**
 * @tc.name: TaskNotificationSubscriber_003
 * @tc.desc: test TaskNotificationSubscriber class.
 * @tc.type: FUNC
 * @tc.require: 800
 */
HWTEST_F(BgTaskMiscUnitTest, TaskNotificationSubscriber_003, TestSize.Level2)
{
    auto subscriber = std::make_shared<TaskNotificationSubscriber>();
    subscriber->OnConsumed(nullptr, nullptr);
    auto notificationMap = std::make_shared<Notification::NotificationSortingMap>();
    auto notificationRequest = sptr<Notification::NotificationRequest>(new Notification::NotificationRequest());
    auto notification = std::make_shared<Notification::Notification>(notificationRequest);
    subscriber->OnConsumed(notification, notificationMap);
    BgContinuousTaskMgr::GetInstance()->bgTaskUid_ = BGTASKMGR_UID;

    notification->request_->creatorUid_ = BGTASKMGR_UID;
    subscriber->OnConsumed(notification, notificationMap);

    notification->request_->creatorUid_ = 1;
    notification->request_->notificationContentType_ = Notification::NotificationContent::Type::BASIC_TEXT;
    subscriber->OnConsumed(notification, notificationMap);

    notification->request_->notificationContentType_ = Notification::NotificationContent::Type::LIVE_VIEW;
    subscriber->OnConsumed(notification, notificationMap);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: TaskNotificationSubscriber_004
 * @tc.desc: test TaskNotificationSubscriber class.
 * @tc.type: FUNC
 * @tc.require: 800
 */
HWTEST_F(BgTaskMiscUnitTest, TaskNotificationSubscriber_004, TestSize.Level2)
{
    auto subscriber = std::make_shared<TaskNotificationSubscriber>();
    subscriber->OnCanceled(nullptr, nullptr, 1);
    auto notificationMap = std::make_shared<Notification::NotificationSortingMap>();
    auto notificationRequest = sptr<Notification::NotificationRequest>(new Notification::NotificationRequest());
    auto notification = std::make_shared<Notification::Notification>(notificationRequest);
    subscriber->OnCanceled(notification, notificationMap, 1);
    BgContinuousTaskMgr::GetInstance()->bgTaskUid_ = BGTASKMGR_UID;

    notification->request_->creatorUid_ = BGTASKMGR_UID;
    subscriber->OnCanceled(notification, notificationMap,
        Notification::NotificationConstant::TRIGGER_TEN_MINUTES_REASON_DELETE);

    notification->request_->creatorUid_ = 1;
    notification->request_->ownerUid_ = 1;
    notification->request_->notificationContentType_ = Notification::NotificationContent::Type::BASIC_TEXT;
    subscriber->OnCanceled(notification, notificationMap, 1);

    notification->request_->notificationContentType_ = Notification::NotificationContent::Type::LIVE_VIEW;
    subscriber->OnCanceled(notification, notificationMap, 1);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: TaskNotificationSubscriber_005
 * @tc.desc: test TaskNotificationSubscriber class.
 * @tc.type: FUNC
 * @tc.require: 800
 */
HWTEST_F(BgTaskMiscUnitTest, TaskNotificationSubscriber_005, TestSize.Level2)
{
    auto subscriber = std::make_shared<TaskNotificationSubscriber>();
    auto notificationMap = std::make_shared<Notification::NotificationSortingMap>();
    auto notificationRequest = sptr<Notification::NotificationRequest>(new Notification::NotificationRequest());
    auto notification = std::make_shared<Notification::Notification>(notificationRequest);

    notification->request_->creatorUid_ = 1;
    notification->request_->ownerUid_ = 1;
    notification->request_->notificationContentType_ = Notification::NotificationContent::Type::LIVE_VIEW;
    std::shared_ptr<AAFwk::WantParams> extraInfo = std::make_shared<AAFwk::WantParams>();
    extraInfo->SetParam("event", AAFwk::String::Box("NAVIGATION"));
    extraInfo->SetParam("CapsuleData.status", AAFwk::Integer::Box(1));
    auto liveViewContent = std::make_shared<Notification::NotificationLiveViewContent>();
    liveViewContent->SetExtraInfo(extraInfo);
    auto content = std::make_shared<Notification::NotificationContent>(liveViewContent);
    notification->request_->SetContent(content);
    subscriber->OnConsumed(notification, notificationMap);
    subscriber->OnCanceled(notification, notificationMap, 1);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: NotificationToolsTest_002
 * @tc.desc: test NotificationTools class.
 * @tc.type: FUNC
 * @tc.require: 809
 */
HWTEST_F(BgTaskMiscUnitTest, NotificationToolsTest_002, TestSize.Level2)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    auto taskRecord = std::make_shared<ContinuousTaskRecord>();
    NotificationTools::GetInstance()->PublishNotification(taskRecord, "appName", "prompt", 1);
    SetPublishContinuousTaskNotificationFlag(0);
    SetGetAllActiveNotificationsFlag(TEST_NUM_ONE);
    std::map<std::string, std::pair<std::string, std::string>> newPromptInfos;
    newPromptInfos.emplace("label", std::make_pair<std::string, std::string>("test1", "test2"));
    SetPublishContinuousTaskNotificationFlag(1);
    EXPECT_EQ(NotificationTools::GetInstance()->RefreshContinuousNotificationWantAndContext(
        0, newPromptInfos, taskRecord, false), ERR_BGTASK_CONTINUOUS_UPDATE_NOTIFICATION_FAIL);
    SetGetAllActiveNotificationsFlag(TEST_NUM_TWO);
    EXPECT_EQ(NotificationTools::GetInstance()->RefreshContinuousNotificationWantAndContext(
        0, newPromptInfos, taskRecord, true), ERR_BGTASK_CONTINUOUS_UPDATE_NOTIFICATION_FAIL);
#endif
}

/**
 * @tc.name: NotificationToolsTest_003
 * @tc.desc: test NotificationTools class.
 * @tc.type: FUNC
 * @tc.require: 809
 */
HWTEST_F(BgTaskMiscUnitTest, NotificationToolsTest_003, TestSize.Level2)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    auto subRecord = std::make_shared<ContinuousTaskRecord>();
    auto mainRecord = std::make_shared<ContinuousTaskRecord>();
    subRecord->abilityName_ = "abilityName";
    subRecord->uid_ = 1;
    subRecord->abilityId_ = 1;
    subRecord->isByRequestObject_ = false;
    subRecord->continuousTaskId_ = 0;
    subRecord->wantAgent_ = std::make_shared<AbilityRuntime::WantAgent::WantAgent>();
    mainRecord->notificationId_ = -1;
    EXPECT_EQ(NotificationTools::GetInstance()->PublishMainNotification(
        subRecord, "appName", "prompt", 1, mainRecord), ERR_BGTASK_NOTIFICATION_ERR);
    mainRecord->notificationId_ = 1;
    EXPECT_EQ(NotificationTools::GetInstance()->PublishMainNotification(
        subRecord, "appName", "prompt", 1, mainRecord), ERR_BGTASK_NOTIFICATION_ERR);
#endif
}

/**
 * @tc.name: NotificationToolsTest_004
 * @tc.desc: test NotificationTools class.
 * @tc.type: FUNC
 * @tc.require: 809
 */
HWTEST_F(BgTaskMiscUnitTest, NotificationToolsTest_004, TestSize.Level2)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    auto subRecord = std::make_shared<ContinuousTaskRecord>();
    auto mainRecord = std::make_shared<ContinuousTaskRecord>();
    subRecord->abilityName_ = "abilityName";
    subRecord->uid_ = 1;
    subRecord->abilityId_ = 1;
    subRecord->isByRequestObject_ = false;
    subRecord->continuousTaskId_ = 0;
    subRecord->wantAgent_ = std::make_shared<AbilityRuntime::WantAgent::WantAgent>();
    mainRecord->notificationId_ = -1;
    EXPECT_EQ(NotificationTools::GetInstance()->PublishSubNotification(
        subRecord, "appName", "prompt", 1, mainRecord), ERR_BGTASK_NOTIFICATION_ERR);
    mainRecord->notificationId_ = 1;
    EXPECT_EQ(NotificationTools::GetInstance()->PublishSubNotification(
        subRecord, "appName", "prompt", 1, mainRecord), ERR_BGTASK_NOTIFICATION_ERR);
#endif
}

/**
 * @tc.name: NotificationToolsTest_005
 * @tc.desc: test NotificationTools class.
 * @tc.type: FUNC
 * @tc.require: 809
 */
HWTEST_F(BgTaskMiscUnitTest, NotificationToolsTest_005, TestSize.Level2)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    auto bannerNotification = std::make_shared<BannerNotificationRecord>();
    bannerNotification->bundleName_ = "bundleName";
    bannerNotification->appName_ = "appName";
    bannerNotification->uid_ = 1;
    bannerNotification->notificationId_ = -1;
    bannerNotification->appIndex_ = 0;
    std::vector<std::string> bannerNotificaitonBtn;
    bannerNotificaitonBtn.push_back("banner1");
    bannerNotificaitonBtn.push_back("banner2");
    EXPECT_EQ(NotificationTools::GetInstance()->PublishBannerNotification(
        bannerNotification, "prompt", 0, bannerNotificaitonBtn), ERR_BGTASK_NOTIFICATION_ERR);
    std::map<std::string, std::pair<std::string, std::string>> newPromptInfos;
    newPromptInfos.emplace("label", std::make_pair<std::string, std::string>("test1", "test2"));
    NotificationTools::GetInstance()->RefreshBannerNotifications(
        bannerNotificaitonBtn, newPromptInfos, bannerNotification, 0);
    EXPECT_TRUE(true);
#endif
}

/**
 * @tc.name: SystemEventObserverTest_002
 * @tc.desc: test SystemEventObserver class.
 * @tc.type: FUNC
 * @tc.require: 822
 */
HWTEST_F(BgTaskMiscUnitTest, SystemEventObserverTest_002, TestSize.Level2)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);
    EventFwk::CommonEventSubscribeInfo commonEventSubscribeInfo(matchingSkills);
    auto systemEventListener = std::make_shared<SystemEventObserver>(commonEventSubscribeInfo);

    EventFwk::CommonEventData eventData = EventFwk::CommonEventData();
    auto handler = std::make_shared<OHOS::AppExecFwk::EventHandler>(nullptr);
    systemEventListener->SetEventHandler(handler);
    auto bgContinuousTaskMgr = std::make_shared<BgContinuousTaskMgr>();
    systemEventListener->SetBgContinuousTaskMgr(bgContinuousTaskMgr);

    AAFwk::Want want = AAFwk::Want();
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_RESOURCES_CHANGED);
    eventData.SetWant(want);
    systemEventListener->OnReceiveEventContinuousTask(eventData);
    EXPECT_TRUE(true);
}
}
}
