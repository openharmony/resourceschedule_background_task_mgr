/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "audio_detect.h"

#include "bg_continuous_task_mgr.h"
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr int32_t UNSET_PID = -1;
static constexpr int32_t AUDIO_RUNNING_STATE = 2;
static constexpr uint32_t AUDIO_PLAYBACK_BGMODE_ID = 2;
static constexpr uint32_t AUDIO_RECORDING_BGMODE_ID = 3;
}

void AudioDetect::HandleAudioStreamInfo(
    const std::list<std::tuple<int32_t, int32_t, int32_t>> &streamInfos, const std::string &type)
{
    std::set<int32_t> uidRemoved;
    for (const auto &var : streamInfos) {
        int32_t uid;
        int32_t sessionId;
        int32_t state;
        std::tie(uid, sessionId, state) = var;
        BGTASK_LOGI("Get %{public}s info uid: %{public}d, sessionId: %{public}d, State: %{public}d",
            type.c_str(), uid, sessionId, state);
        if (type == "player") {
            HandlePlayerInfos(uid, sessionId, state, uidRemoved);
        } else {
            HandleRecorderInfos(uid, sessionId, state, uidRemoved);
        }
    }
    for (int32_t uidToCheck : uidRemoved) {
        if (!CheckAudioCondition(uidToCheck, AUDIO_RECORDING_BGMODE_ID)) {
            BgContinuousTaskMgr::GetInstance()->ReportTaskRunningStateUnmet(uidToCheck,
                UNSET_PID, AUDIO_RECORDING_BGMODE_ID);
        }
    }
}

void AudioDetect::HandlePlayerInfos(int32_t uid, int32_t sessionId, int32_t state, std::set<int32_t> &uidRemoved)
{
    auto findRecord = [uid, sessionId](const auto &target) {
        return target->uid_ == uid && target->sessionId_ == sessionId;
    };
    auto findRecordIter = find_if(audioPlayerInfos_.begin(), audioPlayerInfos_.end(), findRecord);
    if (findRecordIter == audioPlayerInfos_.end() && state == AUDIO_RUNNING_STATE) {
        auto recorderInfo = std::make_shared<AudioInfo>(uid, sessionId);
        audioPlayerInfos_.emplace_back(recorderInfo);
    } else if (findRecordIter != audioPlayerInfos_.end() && state != AUDIO_RUNNING_STATE) {
        audioPlayerInfos_.erase(findRecordIter);
        uidRemoved.emplace(uid);
    }
}

void AudioDetect::HandleRecorderInfos(int32_t uid, int32_t sessionId, int32_t state, std::set<int32_t> &uidRemoved)
{
    auto findRecord = [uid, sessionId](const auto &target) {
        return target->uid_ == uid && target->sessionId_ == sessionId;
    };
    auto findRecordIter = find_if(audioRecorderInfos_.begin(), audioRecorderInfos_.end(), findRecord);
    if (findRecordIter == audioRecorderInfos_.end() && state == AUDIO_RUNNING_STATE) {
        auto recorderInfo = std::make_shared<AudioInfo>(uid, sessionId);
        audioRecorderInfos_.emplace_back(recorderInfo);
    } else if (findRecordIter != audioRecorderInfos_.end() && state != AUDIO_RUNNING_STATE) {
        audioRecorderInfos_.erase(findRecordIter);
        uidRemoved.emplace(uid);
    }
}

#ifdef AV_SESSION_PART_ENABLE
void AudioDetect::HandleAVSessionInfo(const AVSession::AVSessionDescriptor &descriptor,
    const std::string &action)
{
    std::string sessionId = descriptor.sessionId_;
    int32_t uid = descriptor.uid_;
    int32_t pid = descriptor.pid_;
    auto findRecord = [sessionId, pid, uid](const auto &target) {
        return target->sessionId_ == sessionId && target->pid_ == pid && target->uid_ == uid;
    };
    auto findRecordIter = find_if(avSessionInfos_.begin(), avSessionInfos_.end(), findRecord);
    if (action == "create" && findRecordIter == avSessionInfos_.end()) {
        avSessionInfos_.emplace_back(std::make_shared<AVSessionInfo>(uid, pid, sessionId));
    } else if (action == "release" && findRecordIter != avSessionInfos_.end()) {
        avSessionInfos_.erase(findRecordIter);
        if (!CheckAudioCondition(uid, AUDIO_PLAYBACK_BGMODE_ID)) {
            BgContinuousTaskMgr::GetInstance()->ReportTaskRunningStateUnmet(uid,
                UNSET_PID, AUDIO_PLAYBACK_BGMODE_ID);
        }
    }
}
#endif // AV_SESSION_PART_ENABLE

void AudioDetect::ParseAudioRecordToStr(Json::Value &value)
{
    Json::Value playerInfo;
    Json::Value audioRenderInfos;
    for (auto var : audioPlayerInfos_) {
        Json::Value info;
        info["uid"] = var->uid_;
        info["sessionId"] = var->sessionId_;
        audioRenderInfos.append(info);
    }
    playerInfo["audioRenderInfos"] = audioRenderInfos;

    Json::Value avSessionInfos;
    for (auto var : avSessionInfos_) {
        Json::Value info;
        info["uid"] = var->uid_;
        info["pid"] = var->pid_;
        info["sessionId"] = var->sessionId_;
        avSessionInfos.append(info);
    }
    playerInfo["AVSessionInfos"] = avSessionInfos;
    
    value["audioPlayer"] = playerInfo;

    Json::Value RecorderInfo;
    for (auto var : audioRecorderInfos_) {
        Json::Value info;
        info["uid"] = var->uid_;
        info["sessionId"] = var->sessionId_;
        RecorderInfo.append(info);
    }
    value["audioRecorder"] = RecorderInfo;
}

bool AudioDetect::ParseAudioRecordFromJson(const Json::Value &value, std::set<int32_t> &uidSet)
{
    if (!value.isMember("audioPlayer") || !value.isMember("audioRecorder")) {
        return false;
    }
    Json::Value playerInfo = value["audioPlayer"];
    Json::Value arrayObj = playerInfo["audioRenderInfos"];
    int32_t uid;
    for (uint32_t i = 0; i < arrayObj.size(); i++) {
        uid = arrayObj[i]["uid"].asInt();
        uidSet.emplace(uid);
        auto record = std::make_shared<AudioInfo>(uid, arrayObj[i]["sessionId"].asInt());
        audioPlayerInfos_.emplace_back(record);
    }

    arrayObj = playerInfo["AVSessionInfos"];
    for (uint32_t i = 0; i < arrayObj.size(); i++) {
        uid = arrayObj[i]["uid"].asInt();
        uidSet.emplace(uid);
        auto record = std::make_shared<AVSessionInfo>(uid, arrayObj[i]["pid"].asInt(),
            arrayObj[i]["sessionId"].asString());
        avSessionInfos_.emplace_back(record);
    }

    arrayObj = value["audioRecorder"];
    for (uint32_t i = 0; i < arrayObj.size(); i++) {
        uid = arrayObj[i]["uid"].asInt();
        uidSet.emplace(uid);
        auto record = std::make_shared<AudioInfo>(uid, arrayObj[i]["sessionId"].asInt());
        audioRecorderInfos_.emplace_back(record);
    }
    return true;
}

bool AudioDetect::CheckAudioCondition(int32_t uid, uint32_t taskType)
{
    bool isAudioConditionMet = false;
    bool isAVSessionConditionMet = false;
    if (taskType == AUDIO_PLAYBACK_BGMODE_ID) {
        for (auto var : audioPlayerInfos_) {
            if (var->uid_ == uid) {
                isAudioConditionMet = true;
                break;
            }
        }
        for (auto var : avSessionInfos_) {
            if (var->uid_ == uid) {
                isAVSessionConditionMet = true;
                break;
            }
        }
#ifdef AV_SESSION_PART_ENABLE
        return isAudioConditionMet && isAVSessionConditionMet;
#else
        return isAudioConditionMet;
#endif
    } else {
        for (auto var : audioRecorderInfos_) {
            if (var->uid_ == uid) {
                isAudioConditionMet = true;
                break;
            }
        }
        return isAudioConditionMet;
    }
}

void AudioDetect::ClearData()
{
    audioPlayerInfos_.clear();
    audioRecorderInfos_.clear();
}
}
}