/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "bgtask_plugin.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {

class TestBgtaskPlugin : public BgtaskPlugin {
public:
    void Init() override {}
    void Uninit() override {}
    std::string GetPluginName() const override { return "TestBgtaskPlugin"; }
    void InitCbMap(CallBackMap& cbMap) override
    {
        cbMap[1] = {{-1, [](const int32_t, const nlohmann::json&) {}}};
        cbMap[2] = {{1, [](const int32_t, const nlohmann::json&) {}}};
        cbMap[3] = {{-1, [this](const int32_t stateType, const nlohmann::json& payload) {
            callbackCalled = true;
            callbackStateType = stateType;
            callbackPayload = payload;
        }}};
        cbMap[4] = {{1, [this](const int32_t stateType, const nlohmann::json& payload) {
            callbackCalled = true;
            callbackStateType = stateType;
            callbackPayload = payload;
        }}, {2, [this](const int32_t stateType, const nlohmann::json& payload) {
            callbackCalled = true;
            callbackStateType = stateType;
            callbackPayload = payload;
        }}};
    }
    bool callbackCalled = false;
    int32_t callbackStateType = 0;
    nlohmann::json callbackPayload;
};

class BgtaskPluginTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: BgtaskPluginTest_001
 * @tc.desc: test GetPluginName method.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginTest, BgtaskPluginTest_001, TestSize.Level2)
{
    TestBgtaskPlugin plugin;
    EXPECT_EQ(plugin.GetPluginName(), "TestBgtaskPlugin");
}

/**
 * @tc.name: BgtaskPluginTest_002
 * @tc.desc: test GetPluginValue method.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginTest, BgtaskPluginTest_002, TestSize.Level2)
{
    TestBgtaskPlugin plugin;
    plugin.InitCbMap(plugin.cbMap_);

    auto values = plugin.GetPluginValue(1);
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values[0], -1);

    values = plugin.GetPluginValue(2);
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values[0], 1);

    values = plugin.GetPluginValue(999);
    EXPECT_EQ(values.size(), 0);
}

/**
 * @tc.name: BgtaskPluginTest_003
 * @tc.desc: test DispatchResource with RES_VALUE_FOR_ALL callback.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginTest, BgtaskPluginTest_003, TestSize.Level2)
{
    TestBgtaskPlugin plugin;
    plugin.InitCbMap(plugin.cbMap_);
    plugin.callbackCalled = false;

    nlohmann::json payload = {{"key", "value"}};
    plugin.DispatchResource(3, 5, payload);

    EXPECT_TRUE(plugin.callbackCalled);
    EXPECT_EQ(plugin.callbackStateType, 5);
    EXPECT_EQ(plugin.callbackPayload["key"], "value");
}

/**
 * @tc.name: BgtaskPluginTest_004
 * @tc.desc: test DispatchResource with specific stateType callback.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginTest, BgtaskPluginTest_004, TestSize.Level2)
{
    TestBgtaskPlugin plugin;
    plugin.InitCbMap(plugin.cbMap_);
    plugin.callbackCalled = false;

    nlohmann::json payload = {{"uid", 12345}};
    plugin.DispatchResource(4, 1, payload);

    EXPECT_TRUE(plugin.callbackCalled);
    EXPECT_EQ(plugin.callbackStateType, 1);
    EXPECT_EQ(plugin.callbackPayload["uid"], 12345);
}

/**
 * @tc.name: BgtaskPluginTest_005
 * @tc.desc: test DispatchResource with unknown resType.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginTest, BgtaskPluginTest_005, TestSize.Level2)
{
    TestBgtaskPlugin plugin;
    plugin.InitCbMap(plugin.cbMap_);
    plugin.callbackCalled = false;

    nlohmann::json payload = {{"key", "value"}};
    plugin.DispatchResource(999, 1, payload);

    EXPECT_FALSE(plugin.callbackCalled);
}

/**
 * @tc.name: BgtaskPluginTest_006
 * @tc.desc: test DispatchResource with unknown stateType.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginTest, BgtaskPluginTest_006, TestSize.Level2)
{
    TestBgtaskPlugin plugin;
    plugin.InitCbMap(plugin.cbMap_);
    plugin.callbackCalled = false;

    nlohmann::json payload = {{"key", "value"}};
    plugin.DispatchResource(4, 999, payload);

    EXPECT_FALSE(plugin.callbackCalled);
}

/**
 * @tc.name: BgtaskPluginTest_007
 * @tc.desc: test DispatchResource with multiple stateType callbacks.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginTest, BgtaskPluginTest_007, TestSize.Level2)
{
    TestBgtaskPlugin plugin;
    plugin.InitCbMap(plugin.cbMap_);
    plugin.callbackCalled = false;

    nlohmann::json payload = {{"pid", 100}};
    plugin.DispatchResource(4, 2, payload);

    EXPECT_TRUE(plugin.callbackCalled);
    EXPECT_EQ(plugin.callbackStateType, 2);
    EXPECT_EQ(plugin.callbackPayload["pid"], 100);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS


/**
 * @tc.name: BgTaskManagerUnitTest_072
 * @tc.desc: test BannerNotificationEventObserver::OnBannerNotificationActionButtonClick.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgContinuousTaskMgrTest, BgTaskManagerUnitTest_072, TestSize.Level1)
{
    bgContinuousTaskMgr_->RegisterBannerNotificationClickListener();
    EXPECT_NE(bgContinuousTaskMgr_->bannerNotificationClickListener_, nullptr);

    bgContinuousTaskMgr_->bannerNotificationRecord_.clear();
    std::shared_ptr<BannerNotificationRecord> bannerNotification1 = std::make_shared<BannerNotificationRecord>();
    bannerNotification1->SetBundleName("bundleName");
    bannerNotification1->SetUserId(0);
    bannerNotification1->SetAuthResult(UserAuthResult::GRANTED_ONCE);
    bannerNotification1->SetAppIndex(0);
    bannerNotification1->SetUid(0);
    std::string label = "default";
    bgContinuousTaskMgr_->bannerNotificationRecord_.emplace(label, bannerNotification1);

    EventFwk::CommonEventData eventData;
    bgContinuousTaskMgr_->bannerNotificationClickListener_->OnBannerNotificationActionButtonClick(
        bgContinuousTaskMgr_->handler_, bgContinuousTaskMgr_, eventData);
    auto authRecordIter = bgContinuousTaskMgr_->bannerNotificationRecord_.find(label);
    EXPECT_NE(authRecordIter->second->GetAuthResult(), 4);

    AAFwk::Want want;
    want.SetParam(BGTASK_BANNER_NOTIFICATION_ACTION_PARAM_BTN, BGTASK_BANNER_NOTIFICATION_BTN_ALLOW_ALLOWED);
    eventData.SetWant(want);
    bgContinuousTaskMgr_->bannerNotificationClickListener_->OnBannerNotificationActionButtonClick(
        bgContinuousTaskMgr_->handler_, bgContinuousTaskMgr_, eventData);
    auto authRecordIter1 = bgContinuousTaskMgr_->bannerNotificationRecord_.find(label);
    EXPECT_NE(authRecordIter1->second->GetAuthResult(), 4);

    want.SetParam(BGTASK_BANNER_NOTIFICATION_ACTION_PARAM_UID, 0);
    eventData.SetWant(want);
    bgContinuousTaskMgr_->bannerNotificationClickListener_->OnBannerNotificationActionButtonClick(
        bgContinuousTaskMgr_->handler_, bgContinuousTaskMgr_, eventData);
    auto authRecordIter2 = bgContinuousTaskMgr_->bannerNotificationRecord_.find(label);
    EXPECT_NE(authRecordIter2->second->GetAuthResult(), 4);

    want.SetParam(BGTASK_BANNER_NOTIFICATION_ACTION_LABEL, label);
    eventData.SetWant(want);
    bgContinuousTaskMgr_->bannerNotificationClickListener_->OnBannerNotificationActionButtonClick(
        bgContinuousTaskMgr_->handler_, bgContinuousTaskMgr_, eventData);
    auto authRecordIter3 = bgContinuousTaskMgr_->bannerNotificationRecord_.find(label);
    EXPECT_EQ(authRecordIter3->second->GetAuthResult(), 4);
}

/**
 * @tc.name: BannerNotificationEventObserverTest_001
 * @tc.desc: test BannerNotificationEventObserver class.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgTaskMiscUnitTest, BannerNotificationEventObserverTest_001, TestSize.Level2)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(BGTASK_BANNER_NOTIFICATION_ACTION_NAME);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto bannerNotificationClickListener = std::make_shared<SystemEventObserver>(subscribeInfo);
    EXPECT_NE(bannerNotificationClickListener, nullptr);
    bool result = bannerNotificationClickListener->Subscribe();
    EXPECT_TRUE(result);
    result = bannerNotificationClickListener->Unsubscribe();
    EXPECT_TRUE(result);

    auto handler = std::make_shared<OHOS::AppExecFwk::EventHandler>(nullptr);
    bannerNotificationClickListener->SetEventHandler(handler);
    auto bgContinuousTaskMgr = std::make_shared<BgContinuousTaskMgr>();
    bannerNotificationClickListener->SetBgContinuousTaskMgr(bgContinuousTaskMgr);
    bgContinuousTaskMgr->bannerNotificationRecord_.clear();
    std::shared_ptr<BannerNotificationRecord> bannerNotification1 = std::make_shared<BannerNotificationRecord>();
    bannerNotification1->SetBundleName("bundleName");
    bannerNotification1->SetUserId(0);
    bannerNotification1->SetAuthResult(UserAuthResult::GRANTED_ONCE);
    bannerNotification1->SetAppIndex(0);
    bannerNotification1->SetUid(0);
    std::string label = "default";
    bgContinuousTaskMgr->bannerNotificationRecord_.emplace(label, bannerNotification1);

    EventFwk::CommonEventData eventData = EventFwk::CommonEventData();
    AAFwk::Want want = AAFwk::Want();
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_RESOURCES_CHANGED);
    eventData.SetWant(want);
    bannerNotificationClickListener->OnReceiveEvent(eventData);
    auto authRecordIter = bgContinuousTaskMgr->bannerNotificationRecord_.find(label);
    EXPECT_EQ(authRecordIter->second->GetAuthResult(), 3);
}