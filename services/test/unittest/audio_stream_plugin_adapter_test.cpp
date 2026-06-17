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
#include "audio_stream_plugin_adapter.h"
#include "audio_renderer_info_plugin_data.h"
#include "audio_info.h"
#include "nlohmann/json.hpp"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {

class AudioStreamPluginAdapterTest : public testing::Test {
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
 * @tc.name: AudioStreamPluginAdapterTest_001
 * @tc.desc: test GetPluginName method.
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, AudioStreamPluginAdapterTest_001, TestSize.Level2)
{
    auto adapter = AudioStreamPluginAdapter::GetInstance();
    EXPECT_EQ(adapter->GetPluginName(), "AudioStreamPluginAdapter");
}

/**
 * @tc.name: AudioStreamPluginAdapterTest_002
 * @tc.desc: test RendererStateChange method with running state.
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, AudioStreamPluginAdapterTest_002, TestSize.Level2)
{
    auto adapter = AudioStreamPluginAdapter::GetInstance();
    nlohmann::json payload;
    payload["uid"] = 1001;
    payload["rendererState"] = 2; // RENDERER_RUNNING
    payload["sessionId"] = 1;
    adapter->RendererStateChange(payload);
    EXPECT_TRUE(AudioRendererInfoPluginData::GetInstance()->CheckAppIsPlaying(1001));
}

/**
 * @tc.name: AudioStreamPluginAdapterTest_003
 * @tc.desc: test RendererStateChange method with stopped state.
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, AudioStreamPluginAdapterTest_003, TestSize.Level2)
{
    auto adapter = AudioStreamPluginAdapter::GetInstance();
    auto audioInfo = std::make_shared<AudioInfo>(1001, 1);
    AudioRendererInfoPluginData::GetInstance()->AddAudioPlayerInfo(audioInfo);
    nlohmann::json payload;
    payload["uid"] = 1001;
    payload["rendererState"] = 0; // RENDERER_STOPPED
    payload["sessionId"] = 1;
    adapter->RendererStateChange(payload);
    EXPECT_FALSE(AudioRendererInfoPluginData::GetInstance()->CheckAppIsPlaying(1001));
}

/**
 * @tc.name: AudioStreamPluginAdapterTest_004
 * @tc.desc: test Uninit method clears audio data.
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, AudioStreamPluginAdapterTest_004, TestSize.Level2)
{
    auto adapter = AudioStreamPluginAdapter::GetInstance();
    auto audioInfo = std::make_shared<AudioInfo>(1001, 1);
    AudioRendererInfoPluginData::GetInstance()->AddAudioPlayerInfo(audioInfo);
    adapter->Uninit();
    EXPECT_FALSE(AudioRendererInfoPluginData::GetInstance()->CheckAppIsPlaying(1001));
}
} // namespace BackgroundTaskMgr
} // namespace OHOS