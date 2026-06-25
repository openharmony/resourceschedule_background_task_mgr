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
 * @tc.name: DumpGetTask_001
 * @tc.desc: test BgContinuousTaskDumper class.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgContinuousTaskMgrTest, DumpGetTask_001, TestSize.Level3)
{
    std::vector<std::string> dumpOption;
    std::vector<std::string> dumpInfo;
    BgContinuousTaskDumper::GetInstance()->DumpGetTask(dumpOption, dumpInfo);
    EXPECT_EQ(dumpInfo.size(), 1);

    dumpOption.clear();
    dumpInfo.clear();
    dumpOption.emplace_back("-a");
    dumpOption.emplace_back("-C");
    dumpOption.emplace_back("-1");
    BgContinuousTaskDumper::GetInstance()->DumpGetTask(dumpOption, dumpInfo);
    EXPECT_EQ(dumpInfo.size(), 1);
    
    dumpOption.clear();
    dumpInfo.clear();
    dumpOption.emplace_back("-a");
    dumpOption.emplace_back("-C");
    dumpOption.emplace_back("1");
    bgContinuousTaskMgr_->isSysReady_.store(false);
    BgContinuousTaskDumper::GetInstance()->DumpGetTask(dumpOption, dumpInfo);
    EXPECT_EQ(dumpInfo.size(), 1);

    dumpOption.clear();
    dumpInfo.clear();
    dumpOption.emplace_back("-a");
    dumpOption.emplace_back("-C");
    dumpOption.emplace_back("1");
    bgContinuousTaskMgr_->isSysReady_.store(true);
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    BgContinuousTaskDumper::GetInstance()->DumpGetTask(dumpOption, dumpInfo);
    EXPECT_EQ(dumpInfo.size(), 1);

    dumpOption.clear();
    dumpInfo.clear();
    dumpOption.emplace_back("-a");
    dumpOption.emplace_back("-C");
    dumpOption.emplace_back("1");
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord1 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord1->uid_ = TEST_NUM_ONE;
    continuousTaskRecord1->bgModeId_ = TEST_NUM_TWO;
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord2 = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord2->uid_ = TEST_NUM_ONE;
    continuousTaskRecord2->bgModeId_ = TEST_NUM_THREE;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key1"] = continuousTaskRecord1;
    bgContinuousTaskMgr_->continuousTaskInfosMap_["key2"] = continuousTaskRecord2;
    BgContinuousTaskDumper::GetInstance()->DumpGetTask(dumpOption, dumpInfo);
    EXPECT_EQ(dumpInfo.size(), 2);
}

/**
 * @tc.name: DebugContinuousTask_001
 * @tc.desc: test BgContinuousTaskDumper class.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgContinuousTaskMgrTest, DebugContinuousTask_001, TestSize.Level3)
{
    std::vector<std::string> dumpOption;
    std::vector<std::string> dumpInfo;
    BgContinuousTaskDumper::GetInstance()->DebugContinuousTask(dumpOption, dumpInfo);
    EXPECT_EQ(dumpInfo.size(), 1);

    dumpOption.clear();
    dumpInfo.clear();
    dumpOption.emplace_back("-a");
    dumpOption.emplace_back("-C");
    dumpOption.emplace_back("DEMO");
    dumpOption.emplace_back("apply");
    BgContinuousTaskDumper::GetInstance()->DebugContinuousTask(dumpOption, dumpInfo);
    EXPECT_EQ(dumpInfo.size(), 1);
    
    dumpOption.clear();
    dumpInfo.clear();
    dumpOption.emplace_back("-a");
    dumpOption.emplace_back("-C");
    dumpOption.emplace_back("WORKOUT");
    dumpOption.emplace_back("DEMO");
    BgContinuousTaskDumper::GetInstance()->DebugContinuousTask(dumpOption, dumpInfo);
    EXPECT_EQ(dumpInfo.size(), 1);

    dumpOption.clear();
    dumpInfo.clear();
    dumpOption.emplace_back("-a");
    dumpOption.emplace_back("-C");
    dumpOption.emplace_back("WORKOUT");
    dumpOption.emplace_back("apply");
    dumpOption.emplace_back("1");
    bgContinuousTaskMgr_->isSysReady_.store(true);
    bgContinuousTaskMgr_->continuousTaskInfosMap_.clear();
    BgContinuousTaskDumper::GetInstance()->DumpGetTask(dumpOption, dumpInfo);
    EXPECT_EQ(dumpInfo.size(), 1);
}