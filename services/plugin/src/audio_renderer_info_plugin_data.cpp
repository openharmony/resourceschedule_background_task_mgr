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

#include "audio_renderer_info_plugin_data.h"
#include "bg_continuous_task_mgr.h"
#include "bgtaskmgr_log_wrapper.h"
#include "audio_stream_manager.h"

namespace OHOS {
namespace BackgroundTaskMgr {
bool AudioRendererInfoPluginData::CheckAppIsPlaying(int32_t uid)
{
    std::lock_guard<std::mutex> lock(pluginAudioDataMutex_);
    if (audioPlayerInfos_.empty()) {
        return false;
    }
    auto findRecord = [uid](const auto &target) {
        return target->uid_ == uid;
    };
    auto findRecordIter = find_if(audioPlayerInfos_.begin(), audioPlayerInfos_.end(), findRecord);
    return findRecordIter != audioPlayerInfos_.end();
}

bool AudioRendererInfoPluginData::AddAudioPlayerInfo(const std::shared_ptr<AudioInfo> audioInfo)
{
    if (audioInfo == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lock(pluginAudioDataMutex_);
    int32_t uid = audioInfo->uid_;
    int32_t sessionId = audioInfo->sessionId_;
    auto findRecord = [uid, sessionId](const auto &target) {
        return target->uid_ == uid && target->sessionId_ == sessionId;
    };
    auto findRecordIter = find_if(audioPlayerInfos_.begin(), audioPlayerInfos_.end(), findRecord);
    if (findRecordIter == audioPlayerInfos_.end()) {
        audioPlayerInfos_.emplace_back(audioInfo);
        return true;
    }
    return false;
}

void AudioRendererInfoPluginData::RemoveAudioPlayerInfo(int32_t uid, int32_t sessionId)
{
    std::lock_guard<std::mutex> lock(pluginAudioDataMutex_);
    if (audioPlayerInfos_.empty()) {
        return;
    }
    auto findRecord = [uid, sessionId](const auto &target) {
        return target->uid_ == uid && target->sessionId_ == sessionId;
    };
    auto findRecordIter = find_if(audioPlayerInfos_.begin(), audioPlayerInfos_.end(), findRecord);
    if (findRecordIter != audioPlayerInfos_.end()) {
        audioPlayerInfos_.erase(findRecordIter);
    }
}

void AudioRendererInfoPluginData::ClearAudioPlayerInfo()
{
    std::lock_guard<std::mutex> lock(pluginAudioDataMutex_);
    audioPlayerInfos_.clear();
}

void AudioRendererInfoPluginData::AfterAddSaListener()
{
    ClearAudioPlayerInfo();
    std::vector<std::shared_ptr<AudioStandard::AudioRendererChangeInfo>> audioInfos;
    auto ret = AudioStandard::AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioInfos);
    if (ret != 0) {
        BGTASK_LOGE("GetCurrentRendererChangeInfos failed, errcode: %{public}d.", ret);
    }
    for (const auto &info : audioInfos) {
        if (info == nullptr) {
            continue;
        }
        BGTASK_LOGI("uid:%{public}d, sessionId:%{public}d, state:%{public}d",
            info->clientUID, info->sessionId, info->rendererState);
        if (info->rendererState != AudioStandard::RendererState::RENDERER_RUNNING) {
            continue;
        }
        auto recorderInfo = std::make_shared<AudioInfo>(info->clientUID, info->sessionId);
        AddAudioPlayerInfo(recorderInfo);
    }
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS