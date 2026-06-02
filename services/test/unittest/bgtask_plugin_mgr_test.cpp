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
#include <gmock/gmock.h>
#include <memory>
#include <vector>

#include "nlohmann/json.hpp"
#include "bgtask_plugin_mgr.h"
#include "bgtask_plugin.h"
#include "res_data.h"

using namespace testing::ext;
using namespace OHOS::ResourceSchedule;

namespace OHOS {
namespace BackgroundTaskMgr {

// 测试用插件类
class MockBgtaskPlugin final : public BgtaskPlugin {
public:
    static constexpr uint32_t TEST_RES_TYPE_ALL = 1;
    static constexpr uint32_t TEST_RES_TYPE_SPECIFIC = 2;
    static constexpr int32_t TEST_STATE_TYPE_1 = 1;
    static constexpr int32_t TEST_STATE_TYPE_2 = 2;
    static constexpr int32_t TEST_STATE_TYPE_3 = 3;
    static constexpr uint32_t TEST_RES_TYPE_UNKNOWN = 999;

    void Init() override
    {
        initCalled = true;
        InitCbMap(cbMap_);
    }
    void Uninit() override { uninitCalled = true; }
    std::string GetPluginName() const override { return "MockBgtaskPlugin"; }
    std::vector<int32_t> GetPluginValue(const uint32_t resType) const override
    {
        if (resType == TEST_RES_TYPE_ALL) {
            return { RES_VALUE_FOR_ALL };
        } else if (resType == TEST_RES_TYPE_SPECIFIC) {
            return { TEST_STATE_TYPE_1, TEST_STATE_TYPE_2, TEST_STATE_TYPE_3 };
        }
        return {};
    }
    void InitCbMap(CallBackMap& cbMap) override
    {
        cbMap[TEST_RES_TYPE_ALL] = {{RES_VALUE_FOR_ALL,
            [this](const int32_t, const nlohmann::json&) { callbackCalled = true; }}};
        cbMap[TEST_RES_TYPE_SPECIFIC] = {{TEST_STATE_TYPE_1,
            [this](const int32_t, const nlohmann::json&) { callbackCalled = true; }}};
    }
    bool initCalled = false;
    bool uninitCalled = false;
    bool callbackCalled = false;
};

class BgtaskPluginMgrTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp()
    {
        // 重置插件管理器状态
        BgtaskPluginMgr::GetInstance().pluginEnable = false;
        BgtaskPluginMgr::GetInstance().asyncCbMap_.clear();
    }
    void TearDown() {}
};

/**
 * @tc.name: BgtaskPluginMgrTest_001
 * @tc.desc: test RegisterAsyncPlugin method with valid plugin.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_001, TestSize.Level2)
{
    auto plugin = std::make_shared<MockBgtaskPlugin>();
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_ALL, plugin);
    EXPECT_EQ(BgtaskPluginMgr::GetInstance().asyncCbMap_.size(), 1);
    EXPECT_EQ(BgtaskPluginMgr::GetInstance().asyncCbMap_[MockBgtaskPlugin::TEST_RES_TYPE_ALL]->
        GetPluginName(), "MockBgtaskPlugin");
}

/**
 * @tc.name: BgtaskPluginMgrTest_002
 * @tc.desc: test RegisterAsyncPlugin method with duplicate resType.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_002, TestSize.Level2)
{
    auto plugin1 = std::make_shared<MockBgtaskPlugin>();
    auto plugin2 = std::make_shared<MockBgtaskPlugin>();
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_SPECIFIC, plugin1);
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_SPECIFIC, plugin2);
    EXPECT_EQ(BgtaskPluginMgr::GetInstance().asyncCbMap_.size(), 1);
    EXPECT_EQ(BgtaskPluginMgr::GetInstance().asyncCbMap_[MockBgtaskPlugin::TEST_RES_TYPE_SPECIFIC]->
        GetPluginName(), "MockBgtaskPlugin");
}

/**
 * @tc.name: BgtaskPluginMgrTest_003
 * @tc.desc: test RegisterAsyncPlugin method with nullptr plugin.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_003, TestSize.Level2)
{
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_UNKNOWN, nullptr);
    EXPECT_EQ(BgtaskPluginMgr::GetInstance().asyncCbMap_.size(), 0);
}

/**
 * @tc.name: BgtaskPluginMgrTest_004
 * @tc.desc: test Init method with registered plugins.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_004, TestSize.Level2)
{
    auto plugin1 = std::make_shared<MockBgtaskPlugin>();
    auto plugin2 = std::make_shared<MockBgtaskPlugin>();
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_ALL, plugin1);
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_SPECIFIC, plugin2);

    BgtaskPluginMgr::GetInstance().Init();

    EXPECT_TRUE(BgtaskPluginMgr::GetInstance().pluginEnable.load());
    EXPECT_TRUE(plugin1->initCalled);
    EXPECT_TRUE(plugin2->initCalled);
}

/**
 * @tc.name: BgtaskPluginMgrTest_005
 * @tc.desc: test Init method with no registered plugins.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_005, TestSize.Level2)
{
    BgtaskPluginMgr::GetInstance().Init();

    EXPECT_TRUE(BgtaskPluginMgr::GetInstance().pluginEnable.load());
    EXPECT_EQ(BgtaskPluginMgr::GetInstance().asyncCbMap_.size(), 0);
}

/**
 * @tc.name: BgtaskPluginMgrTest_006
 * @tc.desc: test Disable method with registered plugins.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_006, TestSize.Level2)
{
    auto plugin1 = std::make_shared<MockBgtaskPlugin>();
    auto plugin2 = std::make_shared<MockBgtaskPlugin>();
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_ALL, plugin1);
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_SPECIFIC, plugin2);
    BgtaskPluginMgr::GetInstance().Init();

    BgtaskPluginMgr::GetInstance().Disable();

    EXPECT_FALSE(BgtaskPluginMgr::GetInstance().pluginEnable.load());
    EXPECT_TRUE(plugin1->uninitCalled);
    EXPECT_TRUE(plugin2->uninitCalled);
}

/**
 * @tc.name: BgtaskPluginMgrTest_007
 * @tc.desc: test Disable method with no registered plugins.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_007, TestSize.Level2)
{
    BgtaskPluginMgr::GetInstance().Disable();

    EXPECT_FALSE(BgtaskPluginMgr::GetInstance().pluginEnable.load());
}

/**
 * @tc.name: BgtaskPluginMgrTest_008
 * @tc.desc: test DispatchResource method with valid data.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_008, TestSize.Level2)
{
    auto plugin = std::make_shared<MockBgtaskPlugin>();
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_ALL, plugin);
    BgtaskPluginMgr::GetInstance().Init();

    auto resData = std::make_shared<ResData>();
    resData->resType = MockBgtaskPlugin::TEST_RES_TYPE_ALL;
    resData->value = 1;
    resData->payload = nlohmann::json::object();
    BgtaskPluginMgr::GetInstance().DispatchResource(resData);

    EXPECT_TRUE(plugin->callbackCalled);
}

/**
 * @tc.name: BgtaskPluginMgrTest_009
 * @tc.desc: test DispatchResource method with unknown resType.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_009, TestSize.Level2)
{
    auto plugin = std::make_shared<MockBgtaskPlugin>();
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_ALL, plugin);
    BgtaskPluginMgr::GetInstance().Init();

    auto resData = std::make_shared<ResData>();
    resData->resType = 999;
    resData->value = 1;
    resData->payload = nlohmann::json::object();
    BgtaskPluginMgr::GetInstance().DispatchResource(resData);

    EXPECT_FALSE(plugin->callbackCalled);
}

/**
 * @tc.name: BgtaskPluginMgrTest_010
 * @tc.desc: test DispatchResource method with nullptr data.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_010, TestSize.Level2)
{
    auto plugin = std::make_shared<MockBgtaskPlugin>();
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_ALL, plugin);
    BgtaskPluginMgr::GetInstance().Init();

    BgtaskPluginMgr::GetInstance().DispatchResource(nullptr);

    EXPECT_FALSE(plugin->callbackCalled);
}

/**
 * @tc.name: BgtaskPluginMgrTest_011
 * @tc.desc: test DispatchResource method when plugin not enabled.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_011, TestSize.Level2)
{
    auto plugin = std::make_shared<MockBgtaskPlugin>();
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_ALL, plugin);

    auto resData = std::make_shared<ResData>();
    resData->resType = MockBgtaskPlugin::TEST_RES_TYPE_ALL;
    resData->value = 1;
    resData->payload = nlohmann::json::object();
    BgtaskPluginMgr::GetInstance().DispatchResource(resData);

    EXPECT_FALSE(plugin->callbackCalled);
}

/**
 * @tc.name: BgtaskPluginMgrTest_012
 * @tc.desc: test RegisterAsyncPlugin with RES_VALUE_FOR_ALL plugin.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_012, TestSize.Level2)
{
    auto plugin = std::make_shared<MockBgtaskPlugin>();
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_ALL, plugin);

    EXPECT_EQ(BgtaskPluginMgr::GetInstance().asyncCbMap_.size(), 1);
    EXPECT_EQ(BgtaskPluginMgr::GetInstance().asyncCbMap_[MockBgtaskPlugin::TEST_RES_TYPE_ALL]->
        GetPluginName(), "MockBgtaskPlugin");
}

/**
 * @tc.name: BgtaskPluginMgrTest_013
 * @tc.desc: test RegisterAsyncPlugin with specific values plugin.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_013, TestSize.Level2)
{
    auto plugin = std::make_shared<MockBgtaskPlugin>();
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_SPECIFIC, plugin);

    EXPECT_EQ(BgtaskPluginMgr::GetInstance().asyncCbMap_.size(), 1);
    EXPECT_EQ(BgtaskPluginMgr::GetInstance().asyncCbMap_[MockBgtaskPlugin::TEST_RES_TYPE_SPECIFIC]->
        GetPluginName(), "MockBgtaskPlugin");
}

/**
 * @tc.name: BgtaskPluginMgrTest_014
 * @tc.desc: test RegisterAsyncPlugin with nullptr plugin.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_014, TestSize.Level2)
{
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_ALL, nullptr);

    EXPECT_EQ(BgtaskPluginMgr::GetInstance().asyncCbMap_.size(), 0);
}

/**
 * @tc.name: BgtaskPluginMgrTest_015
 * @tc.desc: test multiple RegisterAsyncPlugin and DispatchResource calls.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_015, TestSize.Level2)
{
    auto plugin1 = std::make_shared<MockBgtaskPlugin>();
    auto plugin2 = std::make_shared<MockBgtaskPlugin>();
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_ALL, plugin1);
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_SPECIFIC, plugin2);
    BgtaskPluginMgr::GetInstance().Init();

    auto resData1 = std::make_shared<ResData>();
    resData1->resType = 1;
    resData1->value = 1;
    resData1->payload = nlohmann::json::object();
    BgtaskPluginMgr::GetInstance().DispatchResource(resData1);

    EXPECT_TRUE(plugin1->callbackCalled);
    EXPECT_FALSE(plugin2->callbackCalled);

    plugin1->callbackCalled = false;
    auto resData2 = std::make_shared<ResData>();
    resData2->resType = 2;
    resData2->value = 1;
    resData2->payload = nlohmann::json::object();
    BgtaskPluginMgr::GetInstance().DispatchResource(resData2);

    EXPECT_FALSE(plugin1->callbackCalled);
    EXPECT_TRUE(plugin2->callbackCalled);
}

/**
 * @tc.name: BgtaskPluginMgrTest_016
 * @tc.desc: test Init and Disable sequence.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_016, TestSize.Level2)
{
    auto plugin = std::make_shared<MockBgtaskPlugin>();
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_ALL, plugin);

    BgtaskPluginMgr::GetInstance().Init();
    EXPECT_TRUE(BgtaskPluginMgr::GetInstance().pluginEnable.load());
    EXPECT_TRUE(plugin->initCalled);

    BgtaskPluginMgr::GetInstance().Disable();
    EXPECT_FALSE(BgtaskPluginMgr::GetInstance().pluginEnable.load());
    EXPECT_TRUE(plugin->uninitCalled);
}

/**
 * @tc.name: BgtaskPluginMgrTest_017
 * @tc.desc: test RegisterAsyncPlugin after Init.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_017, TestSize.Level2)
{
    BgtaskPluginMgr::GetInstance().Init();

    auto plugin = std::make_shared<MockBgtaskPlugin>();
    BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(MockBgtaskPlugin::TEST_RES_TYPE_ALL, plugin);

    EXPECT_TRUE(BgtaskPluginMgr::GetInstance().pluginEnable.load());
    EXPECT_EQ(BgtaskPluginMgr::GetInstance().asyncCbMap_.size(), 1);
}

/**
 * @tc.name: BgtaskPluginMgrTest_018
 * @tc.desc: test GetPluginValue with empty result.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_018, TestSize.Level2)
{
    auto plugin = std::make_shared<MockBgtaskPlugin>();
    auto values = plugin->GetPluginValue(999);
    EXPECT_TRUE(values.empty());
}

/**
 * @tc.name: BgtaskPluginMgrTest_019
 * @tc.desc: test GetPluginValue with RES_VALUE_FOR_ALL.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_019, TestSize.Level2)
{
    auto plugin = std::make_shared<MockBgtaskPlugin>();
    auto values = plugin->GetPluginValue(1);
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values[0], -1);
}

/**
 * @tc.name: BgtaskPluginMgrTest_020
 * @tc.desc: test GetPluginValue with specific values.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskPluginMgrTest, BgtaskPluginMgrTest_020, TestSize.Level2)
{
    auto plugin = std::make_shared<MockBgtaskPlugin>();
    auto values = plugin->GetPluginValue(2);
    EXPECT_EQ(values.size(), 3);
    EXPECT_EQ(values[0], 1);
    EXPECT_EQ(values[1], 2);
    EXPECT_EQ(values[2], 3);
}

}  // namespace BackgroundTaskMgr
}  // namespace OHOS