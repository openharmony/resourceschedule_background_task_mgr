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
#include "audio_renderer_info_plugin_data.h"
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
        AudioRendererInfoPluginData::GetInstance()->ClearAudioPlayerInfo();
    }

    static void TearDownTestCase()
    {
        AudioRendererInfoPluginData::GetInstance()->ClearAudioPlayerInfo();
    }

    void SetUp() override
    {
        AudioRendererInfoPluginData::GetInstance()->ClearAudioPlayerInfo();
    }

    void TearDown() override
    {
        AudioRendererInfoPluginData::GetInstance()->ClearAudioPlayerInfo();
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
    AudioRendererInfoPluginData::GetInstance()->AddAudioPlayerInfo(audioInfo);
    nlohmann::json payload;
    payload["saId"] = 3009; // AUDIO_POLICY_SERVICE_ID
    adapter->AfterAddSaListener(payload);
    EXPECT_FALSE(AudioRendererInfoPluginData::GetInstance()->CheckAppIsPlaying(1001));
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
    AudioRendererInfoPluginData::GetInstance()->AddAudioPlayerInfo(audioInfo);
    nlohmann::json payload;
    payload["saId"] = 1001; // Other service
    adapter->AfterAddSaListener(payload);
    EXPECT_TRUE(AudioRendererInfoPluginData::GetInstance()->CheckAppIsPlaying(1001));
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
    nlohmann::json payload = {
        "params" : {
            "demo" : {}
        }
    }
    adapter->HandleCloudConfigUpdateEvent(SUSPEND_MANAGER_SYSTEM_ABILITY_ID, payload);
    std::string bundleName = "com.ohos.demo";
    auto ret = DelayedSingleton<BgtaskConfig>::GetInstance()->IsSpecialExemptedQuatoApp(bundleName);
    EXPECT_FALSE(ret);

    nlohmann::json payload2 = {
        "params" : {
            "special_exempted_list" : {}
        }
    }
    adapter->HandleCloudConfigUpdateEvent(SUSPEND_MANAGER_SYSTEM_ABILITY_ID, payload2);
    auto ret2 = DelayedSingleton<BgtaskConfig>::GetInstance()->IsSpecialExemptedQuatoApp(bundleName);
    EXPECT_FALSE(ret2);

    nlohmann::json payload3 = {
        "params" : {
            "special_exempted_list" : [
                "com.ohos.demo"
            ]
        }
    }
    adapter->HandleCloudConfigUpdateEvent(SUSPEND_MANAGER_SYSTEM_ABILITY_ID, payload3);
    auto ret3 = DelayedSingleton<BgtaskConfig>::GetInstance()->IsSpecialExemptedQuatoApp(bundleName);
    EXPECT_TRUE(ret3);

    adapter->HandleCloudConfigUpdateEvent(BACKGROUND_TASK_MANAGER_SERVICE_ID, payload3);
    auto ret4 = DelayedSingleton<BgtaskConfig>::GetInstance()->CheckRequestCpuLevelBundleNameConfigured(bundleName);
    EXPECT_FALSE(ret4);
}
} // namespace BackgroundTaskMgr
} // namespace OHOS