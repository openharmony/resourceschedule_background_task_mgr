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

#include "audio_stream_plugin_adapter.h"
#include "audio_renderer_info_plugin_data.h"
#include "audio_info.h"
#include "bgtaskmgr_log_wrapper.h"
#include "bgtask_plugin_mgr.h"
#include "res_type.h"
#include "res_data.h"
#include "res_sched_json_util.h"
#include "system_ability_definition.h"
#include "bg_continuous_task_mgr.h"

namespace OHOS {
namespace BackgroundTaskMgr {
using namespace OHOS::ResourceSchedule;
namespace {
    const bool SELF_REGISTER = AudioStreamPluginAdapter::SelfRegister();
    const char PLUGIN_NAME[] = "AudioStreamPluginAdapter";
}

bool AudioStreamPluginAdapter::SelfRegister()
{
    auto obj = AudioStreamPluginAdapter::GetInstance();
    if (obj == nullptr) {
        BGTASK_LOGE("AudioStreamPluginAdapter null.");
        return false;
    }
 
    obj->InitCbMap(obj->cbMap_);
    for (const auto &info : obj->cbMap_) {
        BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(info.first, obj);
    }
    return true;
}

void AudioStreamPluginAdapter::InitCbMap(CallBackMap &cbMap)
{
    cbMap[ResType::RES_TYPE_AUDIO_RENDER_STATE_CHANGE] = {
        { -1, [this](const int32_t stateType, const nlohmann::json& payload) {
                this->RendererStateChange(payload);
            } }
    };
}

void AudioStreamPluginAdapter::RendererStateChange(const nlohmann::json& payload)
{
    if (payload.is_null()) {
        BGTASK_LOGE("RendererStateChange fail. json parse fail");
        return;
    }

    int32_t uid = 0;
    int32_t rendererState = 0;
    int32_t sessionId = 0;
    if (!ResCommonUtil::ParseIntParameterFromJson("uid", uid, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("rendererState", rendererState, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("sessionId", sessionId, payload)) {
        BGTASK_LOGE("RendererStateChange parse payload error");
        return;
    }

    if (rendererState == AudioStandard::RendererState::RENDERER_RUNNING) {
        auto recorderInfo = std::make_shared<AudioInfo>(uid, sessionId);
        if (AudioRendererInfoPluginData::GetInstance()->AddAudioPlayerInfo(recorderInfo)) {
            BGTASK_LOGI("uid: %{public}d, sessionId: %{public}d is play audio.", uid, sessionId);
            BgContinuousTaskMgr::GetInstance()->NotifyAudioStart(uid);
        }
    } else {
        AudioRendererInfoPluginData::GetInstance()->RemoveAudioPlayerInfo(uid, sessionId);
        BGTASK_LOGI("uid: %{public}d, sessionId: %{public}d is not play audio.", uid, sessionId);
    }
}

void AudioStreamPluginAdapter::Init()
{
    BGTASK_LOGI("AudioStreamPluginAdapter init");
}

void AudioStreamPluginAdapter::Uninit()
{
    BGTASK_LOGI("AudioStreamPluginAdapter uninit");
    AudioRendererInfoPluginData::GetInstance()->ClearAudioPlayerInfo();
}

std::string AudioStreamPluginAdapter::GetPluginName() const
{
    return PLUGIN_NAME;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS