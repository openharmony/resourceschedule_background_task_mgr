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

#include <cstdint>
#include <gtest/gtest.h>
#include "audio_renderer_info_plugin_data.h"
#include "audio_info.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {

class AudioRendererInfoPluginDataTest : public testing::Test {
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
 * @tc.name: AudioRendererInfoPluginDataTest_001
 * @tc.desc: test CheckAppIsPlaying method with empty list.
 * @tc.type: FUNC
 */
HWTEST_F(AudioRendererInfoPluginDataTest, AudioRendererInfoPluginDataTest_001, TestSize.Level2)
{
    bool result = AudioRendererInfoPluginData::GetInstance()->CheckAppIsPlaying(1001);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AudioRendererInfoPluginDataTest_002
 * @tc.desc: test CheckAppIsPlaying method with playing app.
 * @tc.type: FUNC
 */
HWTEST_F(AudioRendererInfoPluginDataTest, AudioRendererInfoPluginDataTest_002, TestSize.Level2)
{
    auto audioInfo = std::make_shared<AudioInfo>(1001, 1);
    AudioRendererInfoPluginData::GetInstance()->AddAudioPlayerInfo(audioInfo);
    bool result = AudioRendererInfoPluginData::GetInstance()->CheckAppIsPlaying(1001);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: AudioRendererInfoPluginDataTest_003
 * @tc.desc: test CheckAppIsPlaying method with non-playing app.
 * @tc.type: FUNC
 */
HWTEST_F(AudioRendererInfoPluginDataTest, AudioRendererInfoPluginDataTest_003, TestSize.Level2)
{
    auto audioInfo = std::make_shared<AudioInfo>(1001, 1);
    AudioRendererInfoPluginData::GetInstance()->AddAudioPlayerInfo(audioInfo);
    bool result = AudioRendererInfoPluginData::GetInstance()->CheckAppIsPlaying(1002);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AudioRendererInfoPluginDataTest_004
 * @tc.desc: test AddAudioPlayerInfo method with valid info.
 * @tc.type: FUNC
 */
HWTEST_F(AudioRendererInfoPluginDataTest, AudioRendererInfoPluginDataTest_004, TestSize.Level2)
{
    auto audioInfo = std::make_shared<AudioInfo>(1001, 1);
    bool result = AudioRendererInfoPluginData::GetInstance()->AddAudioPlayerInfo(audioInfo);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: AudioRendererInfoPluginDataTest_005
 * @tc.desc: test AddAudioPlayerInfo method with nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AudioRendererInfoPluginDataTest, AudioRendererInfoPluginDataTest_005, TestSize.Level2)
{
    bool result = AudioRendererInfoPluginData::GetInstance()->AddAudioPlayerInfo(nullptr);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AudioRendererInfoPluginDataTest_006
 * @tc.desc: test AddAudioPlayerInfo method with duplicate info.
 * @tc.type: FUNC
 */
HWTEST_F(AudioRendererInfoPluginDataTest, AudioRendererInfoPluginDataTest_006, TestSize.Level2)
{
    auto audioInfo1 = std::make_shared<AudioInfo>(1001, 1);
    auto audioInfo2 = std::make_shared<AudioInfo>(1001, 1);
    AudioRendererInfoPluginData::GetInstance()->AddAudioPlayerInfo(audioInfo1);
    bool result = AudioRendererInfoPluginData::GetInstance()->AddAudioPlayerInfo(audioInfo2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AudioRendererInfoPluginDataTest_007
 * @tc.desc: test AddAudioPlayerInfo method with different session id.
 * @tc.type: FUNC
 */
HWTEST_F(AudioRendererInfoPluginDataTest, AudioRendererInfoPluginDataTest_007, TestSize.Level2)
{
    auto audioInfo1 = std::make_shared<AudioInfo>(1001, 1);
    auto audioInfo2 = std::make_shared<AudioInfo>(1001, 2);
    AudioRendererInfoPluginData::GetInstance()->AddAudioPlayerInfo(audioInfo1);
    bool result = AudioRendererInfoPluginData::GetInstance()->AddAudioPlayerInfo(audioInfo2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: AudioRendererInfoPluginDataTest_008
 * @tc.desc: test RemoveAudioPlayerInfo method with valid info.
 * @tc.type: FUNC
 */
HWTEST_F(AudioRendererInfoPluginDataTest, AudioRendererInfoPluginDataTest_008, TestSize.Level2)
{
    int32_t uid = 1001;
    int32_t sessionId = 1;
    auto audioInfo = std::make_shared<AudioInfo>(uid, sessionId);
    AudioRendererInfoPluginData::GetInstance()->AddAudioPlayerInfo(audioInfo);
    AudioRendererInfoPluginData::GetInstance()->RemoveAudioPlayerInfo(uid, sessionId);
    bool result = AudioRendererInfoPluginData::GetInstance()->CheckAppIsPlaying(uid);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AudioRendererInfoPluginDataTest_009
 * @tc.desc: test ClearAudioPlayerInfo method clears all data.
 * @tc.type: FUNC
 */
HWTEST_F(AudioRendererInfoPluginDataTest, AudioRendererInfoPluginDataTest_009, TestSize.Level2)
{
    auto audioInfo1 = std::make_shared<AudioInfo>(1001, 1);
    auto audioInfo2 = std::make_shared<AudioInfo>(1002, 2);
    AudioRendererInfoPluginData::GetInstance()->AddAudioPlayerInfo(audioInfo1);
    AudioRendererInfoPluginData::GetInstance()->AddAudioPlayerInfo(audioInfo2);
    AudioRendererInfoPluginData::GetInstance()->ClearAudioPlayerInfo();
    EXPECT_FALSE(AudioRendererInfoPluginData::GetInstance()->CheckAppIsPlaying(1001));
    EXPECT_FALSE(AudioRendererInfoPluginData::GetInstance()->CheckAppIsPlaying(1002));
}
} // namespace BackgroundTaskMgr
} // namespace OHOS