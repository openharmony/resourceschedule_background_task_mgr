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
#include "app_state_observer_plugin_adapter.h"
#include "app_state_observer.h"
#include "decision_maker.h"
#include "process_data.h"
#include "app_state_data.h"
#include "ability_state_data.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {

class AppStateObserverPluginAdapterTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: AppStateObserverPluginAdapterTest_001
 * @tc.desc: test GetPluginName method.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverPluginAdapterTest, AppStateObserverPluginAdapterTest_001, TestSize.Level2)
{
    AppStateObserverPluginAdapter::SelfRegister();
    auto adapter = AppStateObserverPluginAdapter::GetInstance();
    EXPECT_EQ(adapter->GetPluginName(), "AppStateObserverPluginAdapter");
}

/**
 * @tc.name: AppStateObserverPluginAdapterTest_002
 * @tc.desc: test UnmarshallingProcessData method with valid data.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverPluginAdapterTest, AppStateObserverPluginAdapterTest_002, TestSize.Level2)
{
    auto adapter = AppStateObserverPluginAdapter::GetInstance();
    adapter->Init();
    nlohmann::json payload = {
        {"bundleName", "testBundle"},
        {"pid", "123"},
        {"uid", "456"},
        {"renderUid", "789"},
        {"processType", "0"},
        {"state", "1"},
        {"extensionType", "2"},
        {"isKeepAlive", "0"},
        {"isTestMode", "0"},
        {"hostPid", "0"},
        {"imageProcessType", "0"},
        {"preloadMode", "0"}
    };
    AppExecFwk::ProcessData processData;
    EXPECT_TRUE(adapter->UnmarshallingProcessData(payload, processData));
    EXPECT_EQ(processData.bundleName, "testBundle");
    EXPECT_EQ(processData.pid, 123);
    EXPECT_EQ(processData.uid, 456);
}

/**
 * @tc.name: AppStateObserverPluginAdapterTest_003
 * @tc.desc: test UnmarshallingProcessData method with invalid data.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverPluginAdapterTest, AppStateObserverPluginAdapterTest_003, TestSize.Level2)
{
    auto adapter = AppStateObserverPluginAdapter::GetInstance();
    nlohmann::json payload = {
        {"bundleName", "testBundle"}
    };
    AppExecFwk::ProcessData processData;
    EXPECT_FALSE(adapter->UnmarshallingProcessData(payload, processData));
}

/**
 * @tc.name: AppStateObserverPluginAdapterTest_004
 * @tc.desc: test UnmarshallingAppStateData method with valid data.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverPluginAdapterTest, AppStateObserverPluginAdapterTest_004, TestSize.Level2)
{
    auto adapter = AppStateObserverPluginAdapter::GetInstance();
    nlohmann::json payload = {
        {"bundleName", "testBundle"},
        {"pid", 123},
        {"uid", 456},
        {"state", 1u},
        {"extensionType", 2u},
        {"preloadMode", "0"}
    };
    AppExecFwk::AppStateData appStateData;
    EXPECT_TRUE(adapter->UnmarshallingAppStateData(payload, appStateData));
    EXPECT_EQ(appStateData.bundleName, "testBundle");
    EXPECT_EQ(appStateData.pid, 123);
    EXPECT_EQ(appStateData.uid, 456);
}

/**
 * @tc.name: AppStateObserverPluginAdapterTest_005
 * @tc.desc: test UnmarshallingAppStateData method with invalid data.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverPluginAdapterTest, AppStateObserverPluginAdapterTest_005, TestSize.Level2)
{
    auto adapter = AppStateObserverPluginAdapter::GetInstance();
    nlohmann::json payload = {
        {"bundleName", "testBundle"}
    };
    AppExecFwk::AppStateData appStateData;
    EXPECT_FALSE(adapter->UnmarshallingAppStateData(payload, appStateData));
}

/**
 * @tc.name: AppStateObserverPluginAdapterTest_006
 * @tc.desc: test UnmarshallingAbilityStateData method with valid data.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverPluginAdapterTest, AppStateObserverPluginAdapterTest_006, TestSize.Level2)
{
    auto adapter = AppStateObserverPluginAdapter::GetInstance();
    nlohmann::json payload = {
        {"pid", 123},
        {"uid", 456},
        {"recordId", 789},
        {"abilityType", 1},
        {"abilityName", "testAbility"},
        {"abilityState", 2},
        {"extType", 3}
    };
    AppExecFwk::AbilityStateData abilityStateData;
    EXPECT_TRUE(adapter->UnmarshallingAbilityStateData(payload, abilityStateData));
    EXPECT_EQ(abilityStateData.pid, 123);
    EXPECT_EQ(abilityStateData.uid, 456);
    EXPECT_EQ(abilityStateData.abilityName, "testAbility");
}

/**
 * @tc.name: AppStateObserverPluginAdapterTest_007
 * @tc.desc: test UnmarshallingAbilityStateData method with invalid data.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverPluginAdapterTest, AppStateObserverPluginAdapterTest_007, TestSize.Level2)
{
    auto adapter = AppStateObserverPluginAdapter::GetInstance();
    nlohmann::json payload = {
        {"pid", 123},
        {"uid", 456}
    };
    AppExecFwk::AbilityStateData abilityStateData;
    EXPECT_FALSE(adapter->UnmarshallingAbilityStateData(payload, abilityStateData));
}

/**
 * @tc.name: AppStateObserverPluginAdapterTest_008
 * @tc.desc: test OnProcessDied method with valid payload.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverPluginAdapterTest, AppStateObserverPluginAdapterTest_008, TestSize.Level2)
{
    auto adapter = AppStateObserverPluginAdapter::GetInstance();
    nlohmann::json payload = {
        {"bundleName", "testBundle"},
        {"pid", "123"},
        {"uid", "456"},
        {"renderUid", "789"},
        {"processType", "0"},
        {"state", "1"},
        {"extensionType", "2"},
        {"isKeepAlive", "0"},
        {"isTestMode", "0"},
        {"hostPid", "0"},
        {"imageProcessType", "0"},
        {"preloadMode", "0"}
    };
    AppExecFwk::ProcessData processData;
    EXPECT_TRUE(adapter->UnmarshallingProcessData(payload, processData));
    EXPECT_EQ(processData.bundleName, "testBundle");
    EXPECT_EQ(processData.pid, 123);
    EXPECT_EQ(processData.uid, 456);
}

/**
 * @tc.name: AppStateObserverPluginAdapterTest_009
 * @tc.desc: test OnProcessStateChanged method with valid payload.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverPluginAdapterTest, AppStateObserverPluginAdapterTest_009, TestSize.Level2)
{
    auto adapter = AppStateObserverPluginAdapter::GetInstance();
    nlohmann::json payload = {
        {"bundleName", "testBundle"},
        {"pid", "123"},
        {"uid", "456"},
        {"renderUid", "789"},
        {"processType", "0"},
        {"state", "1"},
        {"extensionType", "2"},
        {"isKeepAlive", "0"},
        {"isTestMode", "0"},
        {"hostPid", "0"},
        {"imageProcessType", "0"},
        {"preloadMode", "0"}
    };
    AppExecFwk::ProcessData processData;
    EXPECT_TRUE(adapter->UnmarshallingProcessData(payload, processData));
    EXPECT_EQ(processData.bundleName, "testBundle");
    EXPECT_EQ(processData.pid, 123);
    EXPECT_EQ(processData.uid, 456);
}

/**
 * @tc.name: AppStateObserverPluginAdapterTest_010
 * @tc.desc: test OnAppStateChanged method with valid payload.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverPluginAdapterTest, AppStateObserverPluginAdapterTest_010, TestSize.Level2)
{
    auto adapter = AppStateObserverPluginAdapter::GetInstance();
    nlohmann::json payload = {
        {"bundleName", "testBundle"},
        {"pid", 123},
        {"uid", 456},
        {"state", 1u},
        {"extensionType", 2u},
        {"preloadMode", "0"}
    };
    AppExecFwk::AppStateData appStateData;
    EXPECT_TRUE(adapter->UnmarshallingAppStateData(payload, appStateData));
    EXPECT_EQ(appStateData.bundleName, "testBundle");
    EXPECT_EQ(appStateData.pid, 123);
    EXPECT_EQ(appStateData.uid, 456);
}

/**
 * @tc.name: AppStateObserverPluginAdapterTest_011
 * @tc.desc: test OnAppStopped method with valid payload.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverPluginAdapterTest, AppStateObserverPluginAdapterTest_011, TestSize.Level2)
{
    auto adapter = AppStateObserverPluginAdapter::GetInstance();
    nlohmann::json payload = {
        {"bundleName", "testBundle"},
        {"pid", 123},
        {"uid", 456},
        {"state", 1u},
        {"extensionType", 2u},
        {"preloadMode", "0"}
    };
    AppExecFwk::AppStateData appStateData;
    EXPECT_TRUE(adapter->UnmarshallingAppStateData(payload, appStateData));
    EXPECT_EQ(appStateData.bundleName, "testBundle");
    EXPECT_EQ(appStateData.pid, 123);
    EXPECT_EQ(appStateData.uid, 456);
}

/**
 * @tc.name: AppStateObserverPluginAdapterTest_012
 * @tc.desc: test OnAppCacheStateChanged method with valid payload.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverPluginAdapterTest, AppStateObserverPluginAdapterTest_012, TestSize.Level2)
{
    auto adapter = AppStateObserverPluginAdapter::GetInstance();
    nlohmann::json payload = {
        {"bundleName", "testBundle"},
        {"pid", 123},
        {"uid", 456},
        {"state", 1u},
        {"extensionType", 2u},
        {"preloadMode", "0"}
    };
    AppExecFwk::AppStateData appStateData;
    EXPECT_TRUE(adapter->UnmarshallingAppStateData(payload, appStateData));
    EXPECT_EQ(appStateData.bundleName, "testBundle");
    EXPECT_EQ(appStateData.pid, 123);
    EXPECT_EQ(appStateData.uid, 456);
}

/**
 * @tc.name: AppStateObserverPluginAdapterTest_013
 * @tc.desc: test OnAbilityStateChanged method with valid payload.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverPluginAdapterTest, AppStateObserverPluginAdapterTest_013, TestSize.Level2)
{
    auto adapter = AppStateObserverPluginAdapter::GetInstance();
    nlohmann::json payload = {
        {"pid", 123},
        {"uid", 456},
        {"recordId", 789},
        {"abilityType", 1},
        {"abilityName", "testAbility"},
        {"abilityState", 2},
        {"extType", 3}
    };
    AppExecFwk::AbilityStateData abilityStateData;
    EXPECT_TRUE(adapter->UnmarshallingAbilityStateData(payload, abilityStateData));
    EXPECT_EQ(abilityStateData.pid, 123);
    EXPECT_EQ(abilityStateData.uid, 456);
    EXPECT_EQ(abilityStateData.abilityName, "testAbility");
}

/**
 * @tc.name: AppStateObserverPluginAdapterTest_014
 * @tc.desc: test OnAbilityStateChanged method with invalid payload.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverPluginAdapterTest, AppStateObserverPluginAdapterTest_014, TestSize.Level2)
{
    auto adapter = AppStateObserverPluginAdapter::GetInstance();
    nlohmann::json payload = {
        {"pid", 123},
        {"uid", 456}
    };
    AppExecFwk::AbilityStateData abilityStateData;
    EXPECT_FALSE(adapter->UnmarshallingAbilityStateData(payload, abilityStateData));
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS