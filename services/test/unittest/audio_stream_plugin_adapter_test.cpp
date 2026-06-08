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
#include <unordered_map>
#include <functional>
#include "audio_stream_plugin_adapter.h"
#include "audio_info.h"
#include "bg_continuous_task_mgr.h"
#include "bgtask_plugin_mgr.h"
#include "common_utils.h"
#include "res_type.h"
#include "nlohmann/json.hpp"

using namespace testing::ext;
using namespace OHOS::ResourceSchedule;
using namespace nlohmann;

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
constexpr int32_t TEST_UID_1 = 1001;
constexpr int32_t TEST_UID_2 = 1002;
constexpr int32_t TEST_SESSION_ID_1 = 2001;
constexpr int32_t TEST_SESSION_ID_2 = 2002;
constexpr int32_t INVALID_UID = -1;
constexpr int32_t INVALID_SESSION_ID = -1;
} // namespace

class AudioStreamPluginAdapterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<AudioStreamPluginAdapter> adapter_;
};

void AudioStreamPluginAdapterTest::SetUpTestCase()
{
    // 测试用例类初始化
}

void AudioStreamPluginAdapterTest::TearDownTestCase()
{
    // 测试用例类清理
}

void AudioStreamPluginAdapterTest::SetUp()
{
    adapter_ = AudioStreamPluginAdapter::GetInstance();
    EXPECT_NE(adapter_, nullptr);
}

void AudioStreamPluginAdapterTest::TearDown()
{
    // 每个测试用例后的清理
}

/**
 * @tc.name: SelfRegister_001
 * @tc.desc: 测试 SelfRegister 正常注册流程
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, SelfRegister_001, TestSize.Level1)
{
    // 获取实例
    auto instance = AudioStreamPluginAdapter::GetInstance();
    EXPECT_NE(instance, nullptr);
    
    // 验证实例与当前适配器一致
    EXPECT_EQ(instance, adapter_);
}

/**
 * @tc.name: InitCbMap_001
 * @tc.desc: 测试 InitCbMap 初始化回调映射
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, InitCbMap_001, TestSize.Level1)
{
    std::unordered_map<uint32_t, std::unordered_map<int32_t,
        std::function<void(const int32_t, const json&)>>> testCbMap;
    
    adapter_->InitCbMap(testCbMap);
    
    // 验证映射是否包含音频渲染状态变化的键
    EXPECT_TRUE(testCbMap.find(ResType::RES_TYPE_AUDIO_RENDER_STATE_CHANGE) != testCbMap.end());
    
    // 验证内部映射结构
    auto& innerMap = testCbMap[ResType::RES_TYPE_AUDIO_RENDER_STATE_CHANGE];
    EXPECT_TRUE(innerMap.find(-1) != innerMap.end());
}

/**
 * @tc.name: RendererStateChange_001
 * @tc.desc: 测试 payload 为 null 的情况
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, RendererStateChange_001, TestSize.Level1)
{
    json nullPayload = nullptr;
    
    // 应该正常返回，不抛出异常
    adapter_->RendererStateChange(nullPayload);
    
    // 验证音频播放信息列表为空
    bool isPlaying = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_FALSE(isPlaying);
}

/**
 * @tc.name: RendererStateChange_002
 * @tc.desc: 测试 payload 缺少必要字段的情况
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, RendererStateChange_002, TestSize.Level1)
{
    // 缺少 uid 字段
    json payload1 = {
        {"rendererState", 1},
        {"sessionId", "2001"}
    };
    
    adapter_->RendererStateChange(payload1);
    bool isPlaying1 = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_FALSE(isPlaying1);
    
    // 缺少 rendererState 字段
    json payload2 = {
        {"uid", "1001"},
        {"sessionId", "2001"}
    };
    
    adapter_->RendererStateChange(payload2);
    bool isPlaying2 = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_FALSE(isPlaying2);
    
    // 缺少 sessionId 字段
    json payload3 = {
        {"uid", "1001"},
        {"rendererState", 1}
    };
    
    adapter_->RendererStateChange(payload3);
    bool isPlaying3 = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_FALSE(isPlaying3);
}

/**
 * @tc.name: RendererStateChange_003
 * @tc.desc: 测试 payload 字段类型不正确的情况
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, RendererStateChange_003, TestSize.Level1)
{
    // uid 不是字符串类型
    json payload1 = {
        {"uid", 1001},
        {"rendererState", 1},
        {"sessionId", "2001"}
    };
    
    adapter_->RendererStateChange(payload1);
    bool isPlaying1 = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_FALSE(isPlaying1);
    
    // rendererState 不是整数类型
    json payload2 = {
        {"uid", "1001"},
        {"rendererState", "1"},
        {"sessionId", "2001"}
    };
    
    adapter_->RendererStateChange(payload2);
    bool isPlaying2 = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_FALSE(isPlaying2);
    
    // sessionId 不是字符串类型
    json payload3 = {
        {"uid", "1001"},
        {"rendererState", 1},
        {"sessionId", 2001}
    };
    
    adapter_->RendererStateChange(payload3);
    bool isPlaying3 = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_FALSE(isPlaying3);
}

/**
 * @tc.name: RendererStateChange_004
 * @tc.desc: 测试新音频播放状态通知（RENDERER_RUNNING）
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, RendererStateChange_004, TestSize.Level1)
{
    // 准备有效的 payload
    json payload = {
        {"uid", std::to_string(TEST_UID_1)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_RUNNING)},
        {"sessionId", std::to_string(TEST_SESSION_ID_1)}
    };
    
    // 初始状态应该不是播放中
    bool isPlayingBefore = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_FALSE(isPlayingBefore);
    
    // 调用状态变化
    adapter_->RendererStateChange(payload);
    
    // 验证应用现在正在播放
    bool isPlayingAfter = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_TRUE(isPlayingAfter);
}

/**
 * @tc.name: RendererStateChange_005
 * @tc.desc: 测试音频播放状态停止（非RENDERER_RUNNING）
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, RendererStateChange_005, TestSize.Level1)
{
    // 首先添加播放状态
    json startPayload = {
        {"uid", std::to_string(TEST_UID_1)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_RUNNING)},
        {"sessionId", std::to_string(TEST_SESSION_ID_1)}
    };
    
    adapter_->RendererStateChange(startPayload);
    bool isPlayingStart = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_TRUE(isPlayingStart);
    
    // 然后发送停止状态
    json stopPayload = {
        {"uid", std::to_string(TEST_UID_1)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_STOPPED)},
        {"sessionId", std::to_string(TEST_SESSION_ID_1)}
    };
    
    adapter_->RendererStateChange(stopPayload);
    
    // 验证应用不再播放
    bool isPlayingStop = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_FALSE(isPlayingStop);
}

/**
 * @tc.name: RendererStateChange_006
 * @tc.desc: 测试重复播放状态的去重逻辑
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, RendererStateChange_006, TestSize.Level1)
{
    // 第一次播放状态
    json payload1 = {
        {"uid", std::to_string(TEST_UID_1)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_RUNNING)},
        {"sessionId", std::to_string(TEST_SESSION_ID_1)}
    };
    
    adapter_->RendererStateChange(payload1);
    bool isPlaying1 = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_TRUE(isPlaying1);
    
    // 第二次相同的播放状态（应该被去重）
    json payload2 = {
        {"uid", std::to_string(TEST_UID_1)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_RUNNING)},
        {"sessionId", std::to_string(TEST_SESSION_ID_1)}
    };
    
    adapter_->RendererStateChange(payload2);
    bool isPlaying2 = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_TRUE(isPlaying2); // 仍然是播放状态
}

/**
 * @tc.name: RendererStateChange_007
 * @tc.desc: 测试不同 uid 和 sessionId 的组合
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, RendererStateChange_007, TestSize.Level1)
{
    // 添加第一个应用的播放状态
    json payload1 = {
        {"uid", std::to_string(TEST_UID_1)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_RUNNING)},
        {"sessionId", std::to_string(TEST_SESSION_ID_1)}
    };
    
    adapter_->RendererStateChange(payload1);
    bool isPlaying1 = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_TRUE(isPlaying1);
    
    // 添加第二个应用的播放状态
    json payload2 = {
        {"uid", std::to_string(TEST_UID_2)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_RUNNING)},
        {"sessionId", std::to_string(TEST_SESSION_ID_2)}
    };
    
    adapter_->RendererStateChange(payload2);
    bool isPlaying2 = adapter_->CheckAppIsPlaying(TEST_UID_2);
    EXPECT_TRUE(isPlaying2);
    
    // 验证第一个应用仍在播放
    bool isPlaying1Again = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_TRUE(isPlaying1Again);
    
    // 停止第一个应用
    json stopPayload1 = {
        {"uid", std::to_string(TEST_UID_1)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_STOPPED)},
        {"sessionId", std::to_string(TEST_SESSION_ID_1)}
    };
    
    adapter_->RendererStateChange(stopPayload1);
    bool isPlaying1Stopped = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_FALSE(isPlaying1Stopped);
    
    // 验证第二个应用仍在播放
    bool isPlaying2Still = adapter_->CheckAppIsPlaying(TEST_UID_2);
    EXPECT_TRUE(isPlaying2Still);
}

/**
 * @tc.name: RendererStateChange_008
 * @tc.desc: 测试同一个应用不同会话的播放状态
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, RendererStateChange_008, TestSize.Level1)
{
    // 同一个应用，第一个会话开始播放
    json payload1 = {
        {"uid", std::to_string(TEST_UID_1)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_RUNNING)},
        {"sessionId", std::to_string(TEST_SESSION_ID_1)}
    };
    
    adapter_->RendererStateChange(payload1);
    bool isPlaying1 = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_TRUE(isPlaying1);
    
    // 同一个应用，第二个会话开始播放
    json payload2 = {
        {"uid", std::to_string(TEST_UID_1)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_RUNNING)},
        {"sessionId", std::to_string(TEST_SESSION_ID_2)}
    };
    
    adapter_->RendererStateChange(payload2);
    bool isPlaying2 = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_TRUE(isPlaying2);
    
    // 停止第一个会话
    json stopPayload1 = {
        {"uid", std::to_string(TEST_UID_1)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_STOPPED)},
        {"sessionId", std::to_string(TEST_SESSION_ID_1)}
    };
    
    adapter_->RendererStateChange(stopPayload1);
    bool isPlayingAfterStop1 = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_TRUE(isPlayingAfterStop1); // 第二个会话仍在播放
    
    // 停止第二个会话
    json stopPayload2 = {
        {"uid", std::to_string(TEST_UID_1)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_STOPPED)},
        {"sessionId", std::to_string(TEST_SESSION_ID_2)}
    };
    
    adapter_->RendererStateChange(stopPayload2);
    bool isPlayingAfterStop2 = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_FALSE(isPlayingAfterStop2);
}

/**
 * @tc.name: Init_001
 * @tc.desc: 测试 Init 方法
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, Init_001, TestSize.Level1)
{
    // 调用 Init 方法，应该正常执行
    adapter_->Init();
    
    // 验证插件名称仍然正确
    std::string pluginName = adapter_->GetPluginName();
    EXPECT_EQ(pluginName, "AudioStreamPluginAdapter");
}

/**
 * @tc.name: Uninit_001
 * @tc.desc: 测试 Uninit 方法
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, Uninit_001, TestSize.Level1)
{
    // 先添加一个播放状态
    json payload = {
        {"uid", std::to_string(TEST_UID_1)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_RUNNING)},
        {"sessionId", std::to_string(TEST_SESSION_ID_1)}
    };
    
    adapter_->RendererStateChange(payload);
    bool isPlayingBefore = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_TRUE(isPlayingBefore);
    
    // 调用 Uninit 方法
    adapter_->Uninit();
    
    // 验证插件名称仍然正确
    std::string pluginName = adapter_->GetPluginName();
    EXPECT_EQ(pluginName, "AudioStreamPluginAdapter");
}

/**
 * @tc.name: GetPluginName_001
 * @tc.desc: 测试 GetPluginName 方法返回正确的插件名称
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, GetPluginName_001, TestSize.Level1)
{
    std::string pluginName = adapter_->GetPluginName();
    EXPECT_EQ(pluginName, "AudioStreamPluginAdapter");
}

/**
 * @tc.name: GetPluginName_002
 * @tc.desc: 测试多次调用 GetPluginName 返回一致的结果
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, GetPluginName_002, TestSize.Level1)
{
    std::string pluginName1 = adapter_->GetPluginName();
    std::string pluginName2 = adapter_->GetPluginName();
    std::string pluginName3 = adapter_->GetPluginName();
    
    EXPECT_EQ(pluginName1, "AudioStreamPluginAdapter");
    EXPECT_EQ(pluginName2, "AudioStreamPluginAdapter");
    EXPECT_EQ(pluginName3, "AudioStreamPluginAdapter");
    EXPECT_EQ(pluginName1, pluginName2);
    EXPECT_EQ(pluginName2, pluginName3);
}

/**
 * @tc.name: CheckAppIsPlaying_003
 * @tc.desc: 测试多个应用播放音频的查询
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, CheckAppIsPlaying_003, TestSize.Level1)
{
    // 添加多个应用的播放状态
    json payload1 = {
        {"uid", std::to_string(TEST_UID_1)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_RUNNING)},
        {"sessionId", std::to_string(TEST_SESSION_ID_1)}
    };
    
    json payload2 = {
        {"uid", std::to_string(TEST_UID_2)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_RUNNING)},
        {"sessionId", std::to_string(TEST_SESSION_ID_2)}
    };
    
    adapter_->RendererStateChange(payload1);
    adapter_->RendererStateChange(payload2);
    
    // 验证两个应用都在播放
    bool isPlaying1 = adapter_->CheckAppIsPlaying(TEST_UID_1);
    bool isPlaying2 = adapter_->CheckAppIsPlaying(TEST_UID_2);
    
    EXPECT_TRUE(isPlaying1);
    EXPECT_TRUE(isPlaying2);
    
    // 验证不存在的应用不在播放
    bool isPlaying3 = adapter_->CheckAppIsPlaying(9999);
    EXPECT_FALSE(isPlaying3);
}

/**
 * @tc.name: CheckAppIsPlaying_005
 * @tc.desc: 测试应用停止播放后的状态查询
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, CheckAppIsPlaying_005, TestSize.Level1)
{
    // 添加播放状态
    json startPayload = {
        {"uid", std::to_string(TEST_UID_1)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_RUNNING)},
        {"sessionId", std::to_string(TEST_SESSION_ID_1)}
    };
    
    adapter_->RendererStateChange(startPayload);
    bool isPlayingStart = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_TRUE(isPlayingStart);
    
    // 停止播放
    json stopPayload = {
        {"uid", std::to_string(TEST_UID_1)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_STOPPED)},
        {"sessionId", std::to_string(TEST_SESSION_ID_1)}
    };
    
    adapter_->RendererStateChange(stopPayload);
    bool isPlayingStop = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_FALSE(isPlayingStop);
}

/**
 * @tc.name: RendererStateChange_009
 * @tc.desc: 测试边界情况：uid 为 0
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, RendererStateChange_009, TestSize.Level1)
{
    json payload = {
        {"uid", "0"},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_RUNNING)},
        {"sessionId", std::to_string(TEST_SESSION_ID_1)}
    };
    
    adapter_->RendererStateChange(payload);
    bool isPlaying = adapter_->CheckAppIsPlaying(0);
    EXPECT_TRUE(isPlaying);
}

/**
 * @tc.name: RendererStateChange_010
 * @tc.desc: 测试边界情况：sessionId 为 0
 * @tc.type: FUNC
 */
HWTEST_F(AudioStreamPluginAdapterTest, RendererStateChange_010, TestSize.Level1)
{
    json payload = {
        {"uid", std::to_string(TEST_UID_1)},
        {"rendererState", static_cast<int32_t>(AudioStandard::RendererState::RENDERER_RUNNING)},
        {"sessionId", "0"}
    };
    
    adapter_->RendererStateChange(payload);
    bool isPlaying = adapter_->CheckAppIsPlaying(TEST_UID_1);
    EXPECT_TRUE(isPlaying);
}
} // namespace BackgroundTaskMgr
} // namespace OHOS