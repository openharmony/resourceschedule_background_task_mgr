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
#include "audio_info.h"
#include "bg_continuous_task_mgr.h"
#include "bgtaskmgr_log_wrapper.h"
#include "bgtask_plugin_mgr.h"
#include "common_utils.h"
#include "res_type.h"
#include "res_data.h"

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

void AudioStreamPluginAdapter::InitCbMap(std::unordered_map<uint32_t,
    std::unordered_map<int32_t, std::function<void(const int32_t, const nlohmann::json&)>>>& cbMap)
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
    BGTASK_LOGD("RendererStateChange info: %{public}s", payload.dump().c_str());
    if (!CommonUtils::CheckJsonValue(payload, {"uid", "rendererState", "sessionId"}) ||
        !payload.at("uid").is_string() || !payload.at("rendererState").is_number_integer() ||
        !payload.at("sessionId").is_string()) {
        BGTASK_LOGE("RendererStateChange parse payload error");
        return;
    }
    int32_t rendererState = payload.at("rendererState").get<int32_t>();
    int32_t sessionId = std::atoi(payload.at("sessionId").get<std::string>().c_str());
    int32_t uid = std::atoi(payload.at("uid").get<std::string>().c_str());
    // 去重，避免同一个音频流连续两次以上都是播放状态回调
    auto findRecord = [uid, sessionId](const auto &target) {
        return target->uid_ == uid && target->sessionId_ == sessionId;
    };
    auto findRecordIter = find_if(audioPlayerInfos_.begin(), audioPlayerInfos_.end(), findRecord);
    std::lock_guard<std::mutex> lock(pluginMutex_);
    if (findRecordIter == audioPlayerInfos_.end() && rendererState == AudioStandard::RendererState::RENDERER_RUNNING) {
        BgContinuousTaskMgr::GetInstance()->NotifyAudioStart(uid);
        auto recorderInfo = std::make_shared<AudioInfo>(uid, sessionId);
        audioPlayerInfos_.emplace_back(recorderInfo);
        BGTASK_LOGI("uid: %{public}d, sessionId: %{public}d is play audio.", uid, sessionId);
    } else if (findRecordIter != audioPlayerInfos_.end() &&
        rendererState != AudioStandard::RendererState::RENDERER_RUNNING) {
        audioPlayerInfos_.erase(findRecordIter);
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
}

std::string AudioStreamPluginAdapter::GetPluginName() const
{
    return PLUGIN_NAME;
}

bool AudioStreamPluginAdapter::CheckAppIsPlaying(int32_t uid)
{
    std::lock_guard<std::mutex> lock(pluginMutex_);
    auto findRecord = [uid](const auto &target) {
        return target->uid_ == uid;
    };
    auto findRecordIter = find_if(audioPlayerInfos_.begin(), audioPlayerInfos_.end(), findRecord);
    return findRecordIter != audioPlayerInfos_.end();
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS