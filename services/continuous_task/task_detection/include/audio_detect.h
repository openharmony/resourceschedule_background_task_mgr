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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_AUDIO_DETECT_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_AUDIO_DETECT_H

#include <list>
#include <set>

#include "nlohmann/json.hpp"
#include "audio_stream_manager.h"
#ifdef AV_SESSION_PART_ENABLE
#include "avsession_info.h"
#include "avsession_manager.h"
#endif // AV_SESSION_PART_ENABLE

namespace OHOS {
namespace BackgroundTaskMgr {
struct AudioInfo {
    int32_t uid_ {-1};
    int32_t sessionId_ {-1};
    AudioInfo() = default;
    AudioInfo(int32_t uid, int32_t sessionId) : uid_(uid), sessionId_(sessionId) {};
};

struct AVSessionInfo {
    int32_t uid_ {-1};
    int32_t pid_ {-1};
    std::string sessionId_ {""};
    AVSessionInfo() = default;
    AVSessionInfo(int32_t uid, int32_t pid, const std::string sessionId)
        : uid_(uid), pid_(pid), sessionId_(sessionId) {};
};

class AudioDetect {
public:
    void HandleAudioStreamInfo(const std::list<std::tuple<int32_t, int32_t, int32_t>> &streamInfos,
        const std::string &type);
#ifdef AV_SESSION_PART_ENABLE
    void HandleAVSessionInfo(const AVSession::AVSessionDescriptor &descriptor, const std::string &action);
#endif // AV_SESSION_PART_ENABLE
    bool CheckAudioCondition(int32_t uid, uint32_t taskType);
    void ClearData();
    void ParseAudioRecordToStr(nlohmann::json &value);
    bool ParseAudioRecordFromJson(const nlohmann::json &value, std::set<int32_t> &uidSet);

private:
    void UpdateAudioRecord(const std::list<std::tuple<int32_t, int32_t, int32_t>> &streamInfos,
        std::list<std::shared_ptr<AudioInfo>> &records, std::set<int32_t> &uidRemoved);

private:
    std::list<std::shared_ptr<AudioInfo>> audioPlayerInfos_ {};
    std::list<std::shared_ptr<AudioInfo>> audioRecorderInfos_ {};
    std::list<std::shared_ptr<AVSessionInfo>> avSessionInfos_ {};

    friend class TaskDetectionManager;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_AUDIO_DETECT_H