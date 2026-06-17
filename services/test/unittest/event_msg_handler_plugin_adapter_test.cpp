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
    EXPECT_EQ(adapter->GetPluginName(), "EventMsgHandlerPluginAdapter");
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
} // namespace BackgroundTaskMgr
} // namespace OHOS