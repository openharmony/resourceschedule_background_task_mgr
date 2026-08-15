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
#include "event_msg_handler_plugin_adapter.h"
#include "bgtask_data_mgr.h"
#include "audio_info.h"
#include "nlohmann/json.hpp"
#include "system_ability_definition.h"
#include "bgtask_config.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {

class EventMsgHandlerPluginAdapterTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        BgtaskDataMgr::GetInstance()->ClearAll();
    }

    static void TearDownTestCase()
    {
        BgtaskDataMgr::GetInstance()->ClearAll();
    }

    void SetUp() override
    {
        BgtaskDataMgr::GetInstance()->ClearAll();
    }

    void TearDown() override
    {
        BgtaskDataMgr::GetInstance()->ClearAll();
    }
};

/**
 * @tc.name: EventMsgHandlerPluginAdapterTest_001
 * @tc.desc: test GetPluginName method.
 * @tc.type: FUNC
 */
HWTEST_F(EventMsgHandlerPluginAdapterTest, EventMsgHandlerPluginAdapterTest_001, TestSize.Level2)
{
    auto adapter = EventMsgHandlerPluginAdapter::GetInstance();
    adapter->Init();
    EXPECT_EQ(adapter->GetPluginName(), "EventMsgHandlerPluginAdapter");
    adapter->Uninit();
}

/**
 * @tc.name: EventMsgHandlerPluginAdapterTest_002
 * @tc.desc: test AfterAddSaListener method with Audio Policy Service.
 * @tc.type: FUNC
 */
HWTEST_F(EventMsgHandlerPluginAdapterTest, EventMsgHandlerPluginAdapterTest_002, TestSize.Level2)
{
    auto adapter = EventMsgHandlerPluginAdapter::GetInstance();
    auto audioInfo = std::make_shared<AudioInfo>(1001, 1);
    BgtaskDataMgr::GetInstance()->AddAudioPlayerInfo(audioInfo);
    nlohmann::json payload;
    payload["saId"] = 3009; // AUDIO_POLICY_SERVICE_ID
    adapter->AfterAddSaListener(payload);
    EXPECT_FALSE(BgtaskDataMgr::GetInstance()->CheckAppIsPlaying(1001));
}

/**
 * @tc.name: EventMsgHandlerPluginAdapterTest_003
 * @tc.desc: test AfterAddSaListener method with other service.
 * @tc.type: FUNC
 */
HWTEST_F(EventMsgHandlerPluginAdapterTest, EventMsgHandlerPluginAdapterTest_003, TestSize.Level2)
{
    auto adapter = EventMsgHandlerPluginAdapter::GetInstance();
    auto audioInfo = std::make_shared<AudioInfo>(1001, 1);
    BgtaskDataMgr::GetInstance()->AddAudioPlayerInfo(audioInfo);
    nlohmann::json payload;
    payload["saId"] = 1001; // Other service
    adapter->AfterAddSaListener(payload);
    EXPECT_TRUE(BgtaskDataMgr::GetInstance()->CheckAppIsPlaying(1001));
}

/**
 * @tc.name: HandleCloudConfigUpdateEvent_001
 * @tc.desc: test HandleCloudConfigUpdateEvent method
 * @tc.type: FUNC
 */
HWTEST_F(EventMsgHandlerPluginAdapterTest, HandleCloudConfigUpdateEvent_001, TestSize.Level2)
{
    auto adapter = EventMsgHandlerPluginAdapter::GetInstance();
    nlohmann::json payload;
    adapter->HandleCloudConfigUpdateEvent(SUSPEND_MANAGER_SYSTEM_ABILITY_ID, payload);
    std::string bundleName = "com.ohos.demo";
    auto ret = DelayedSingleton<BgtaskConfig>::GetInstance()->IsSpecialExemptedQuatoApp(bundleName);
    EXPECT_FALSE(ret);

    adapter->HandleCloudConfigUpdateEvent(BACKGROUND_TASK_MANAGER_SERVICE_ID, payload);
    auto ret2 = DelayedSingleton<BgtaskConfig>::GetInstance()->CheckRequestCpuLevelBundleNameConfigured(bundleName);
    EXPECT_FALSE(ret2);
}

/**
 * @tc.name: HandleCloudConfigUpdateEvent_002
 * @tc.desc: test HandleCloudConfigUpdateEvent SUSPEND_MANAGER_SYSTEM_ABILITY_ID method
 * @tc.type: FUNC
 */
HWTEST_F(EventMsgHandlerPluginAdapterTest, HandleCloudConfigUpdateEvent_002, TestSize.Level2)
{
    auto adapter = EventMsgHandlerPluginAdapter::GetInstance();
    auto specialExemptedList = nlohmann::json::array();
    std::string bundleName = "com.ohos.demo";
    nlohmann::json params;
    nlohmann::json payload;
    payload["demo"] = specialExemptedList;
    params["params"] = payload;
    adapter->HandleCloudConfigUpdateEvent(SUSPEND_MANAGER_SYSTEM_ABILITY_ID, params);
    auto ret = DelayedSingleton<BgtaskConfig>::GetInstance()->IsSpecialExemptedQuatoApp(bundleName);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: HandleCloudConfigUpdateEvent_003
 * @tc.desc: test HandleCloudConfigUpdateEvent SUSPEND_MANAGER_SYSTEM_ABILITY_ID method
 * @tc.type: FUNC
 */
HWTEST_F(EventMsgHandlerPluginAdapterTest, HandleCloudConfigUpdateEvent_003, TestSize.Level2)
{
    auto adapter = EventMsgHandlerPluginAdapter::GetInstance();
    auto specialExemptedList = nlohmann::json::object();
    std::string bundleName = "com.ohos.demo";
    nlohmann::json payload;
    nlohmann::json params;
    payload["special_exempted_list"] = specialExemptedList;
    params["params"] = payload; // Other service
    adapter->HandleCloudConfigUpdateEvent(SUSPEND_MANAGER_SYSTEM_ABILITY_ID, params);
    auto ret = DelayedSingleton<BgtaskConfig>::GetInstance()->IsSpecialExemptedQuatoApp(bundleName);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: HandleCloudConfigUpdateEvent_004
 * @tc.desc: test HandleCloudConfigUpdateEvent SUSPEND_MANAGER_SYSTEM_ABILITY_ID method
 * @tc.type: FUNC
 */
HWTEST_F(EventMsgHandlerPluginAdapterTest, HandleCloudConfigUpdateEvent_004, TestSize.Level2)
{
    auto adapter = EventMsgHandlerPluginAdapter::GetInstance();
    auto specialExemptedList = nlohmann::json::array();
    std::string bundleName = "com.ohos.demo";
    nlohmann::json payload;
    nlohmann::json params;
    specialExemptedList.push_back(bundleName);
    payload["special_exempted_list"] = specialExemptedList;
    params["params"] = payload;
    adapter->HandleCloudConfigUpdateEvent(SUSPEND_MANAGER_SYSTEM_ABILITY_ID, params);
    auto ret = DelayedSingleton<BgtaskConfig>::GetInstance()->IsSpecialExemptedQuatoApp(bundleName);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: HandleMultiDeviceCastEvent_001
 * @tc.desc: test HandleMultiDeviceCastEvent method with start and stop state.
 * @tc.type: FUNC
 */
HWTEST_F(EventMsgHandlerPluginAdapterTest, HandleMultiDeviceCastEvent_001, TestSize.Level2)
{
    auto adapter = EventMsgHandlerPluginAdapter::GetInstance();
    nlohmann::json payload;
    payload["uid"] = 1001;
    payload["said"] = 3009;
    payload["state"] = 1;
    adapter->HandleMultiDeviceCastEvent(payload);
    EXPECT_TRUE(BgtaskDataMgr::GetInstance()->IsMultiDeviceCast(1001));
    payload["state"] = 2;
    adapter->HandleMultiDeviceCastEvent(payload);
    EXPECT_FALSE(BgtaskDataMgr::GetInstance()->IsMultiDeviceCast(1001));
}
} // namespace BackgroundTaskMgr
} // namespace OHOS