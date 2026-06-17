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

#ifndef BACKGROUND_TASK_MGR_SERVICES_PLUGIN_INCLUDE_AUDIO_RENDERER_INFO_PLUGIN_DATA_H
#define BACKGROUND_TASK_MGR_SERVICES_PLUGIN_INCLUDE_AUDIO_RENDERER_INFO_PLUGIN_DATA_H

#include <cstdint>
#include <mutex>
#include <list>
#include "singleton.h"

namespace OHOS {
namespace BackgroundTaskMgr {

struct AudioInfo {
    int32_t uid_ {-1};
    int32_t sessionId_ {-1};
    AudioInfo() = default;
    AudioInfo(int32_t uid, int32_t sessionId)
        : uid_(uid), sessionId_(sessionId) {};
};

class AudioRendererInfoPluginData : public DelayedSingleton<AudioRendererInfoPluginData> {
public:
    bool CheckAppIsPlaying(int32_t uid);
    bool AddAudioPlayerInfo(const std::shared_ptr<AudioInfo> audioInfo);
    void RemoveAudioPlayerInfo(int32_t uid, int32_t sessionId);
    void ClearAudioPlayerInfo();
    void AfterAddSaListener();
private:
    std::list<std::shared_ptr<AudioInfo>> audioPlayerInfos_ {};
    std::mutex pluginAudioDataMutex_;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // BACKGROUND_TASK_MGR_SERVICES_PLUGIN_INCLUDE_AUDIO_RENDERER_INFO_PLUGIN_DATA_H