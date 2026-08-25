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

#include "bgtask_data_mgr.h"
#include "bgtaskmgr_log_wrapper.h"
#include "audio_stream_manager.h"

namespace OHOS {
namespace BackgroundTaskMgr {
bool BgtaskDataMgr::CheckAppIsPlaying(int32_t uid)
{
    return IsAudioPlay(uid) || IsMultiDeviceCast(uid);
}

bool BgtaskDataMgr::IsAudioPlay(int32_t uid)
{
    std::lock_guard<std::mutex> lock(dataMutex_);
    if (audioPlayerInfos_.empty()) {
        return false;
    }
    auto findRecord = [uid](const auto &target) {
        return target->uid_ == uid;
    };
    auto findRecordIter = find_if(audioPlayerInfos_.begin(), audioPlayerInfos_.end(), findRecord);
    return findRecordIter != audioPlayerInfos_.end();
}

bool BgtaskDataMgr::AddAudioPlayerInfo(const std::shared_ptr<AudioInfo> audioInfo)
{
    if (audioInfo == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lock(dataMutex_);
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

void BgtaskDataMgr::RemoveAudioPlayerInfo(int32_t uid, int32_t sessionId)
{
    std::lock_guard<std::mutex> lock(dataMutex_);
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

void BgtaskDataMgr::AfterAddSaListener()
{
    {
        std::lock_guard<std::mutex> lock(dataMutex_);
        audioPlayerInfos_.clear();
    }
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

void BgtaskDataMgr::HandleMultiDeviceCastStart(int32_t uid, int32_t said)
{
    std::lock_guard<std::mutex> lock(dataMutex_);
    auto findRecord = [uid, said](const auto &target) {
        return target->uid_ == uid && target->said_ == said;
    };
    auto findRecordIter = find_if(multiDeviceInfo_.begin(), multiDeviceInfo_.end(), findRecord);
    if (findRecordIter == multiDeviceInfo_.end()) {
        auto info = std::make_shared<MultiDeviceInfo>(uid, said, 1);
        multiDeviceInfo_.emplace_back(info);
    } else {
        (*findRecordIter)->count_++;
    }
}

void BgtaskDataMgr::HandleMultiDeviceCastStop(int32_t uid, int32_t said)
{
    std::lock_guard<std::mutex> lock(dataMutex_);
    auto findRecord = [uid, said](const auto &target) {
        return target->uid_ == uid && target->said_ == said;
    };
    auto findRecordIter = find_if(multiDeviceInfo_.begin(), multiDeviceInfo_.end(), findRecord);
    if (findRecordIter == multiDeviceInfo_.end()) {
        return;
    }
    (*findRecordIter)->count_--;
    if ((*findRecordIter)->count_ <= 0) {
        multiDeviceInfo_.erase(findRecordIter);
    }
}

void BgtaskDataMgr::RemoveMultiDeviceInfoByUid(int32_t uid)
{
    std::lock_guard<std::mutex> lock(dataMutex_);
    multiDeviceInfo_.remove_if([uid](const auto &target) {
        return target->uid_ == uid;
    });
}

void BgtaskDataMgr::RemoveMultiDeviceInfoBySaid(int32_t said)
{
    std::lock_guard<std::mutex> lock(dataMutex_);
    multiDeviceInfo_.remove_if([said](const auto &target) {
        return target->said_ == said;
    });
}

bool BgtaskDataMgr::IsMultiDeviceCast(int32_t uid)
{
    std::lock_guard<std::mutex> lock(dataMutex_);
    if (multiDeviceInfo_.empty()) {
        return false;
    }
    auto findRecord = [uid](const auto &target) {
        return target->uid_ == uid;
    };
    auto findRecordIter = find_if(multiDeviceInfo_.begin(), multiDeviceInfo_.end(), findRecord);
    return findRecordIter != multiDeviceInfo_.end();
}

void BgtaskDataMgr::OnAppStopped(int32_t uid)
{
    RemoveMultiDeviceInfoByUid(uid);
}

void BgtaskDataMgr::OnRemoveSystemAbility(int32_t said)
{
    RemoveMultiDeviceInfoBySaid(said);
}

void BgtaskDataMgr::ClearAll()
{
    std::lock_guard<std::mutex> lock(dataMutex_);
    audioPlayerInfos_.clear();
    multiDeviceInfo_.clear();
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS