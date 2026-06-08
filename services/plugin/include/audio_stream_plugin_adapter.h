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

#ifndef BACKGROUND_TASK_MGR_SERVICES_PLUGIN_INCLUDE_AUDIO_STREAM_PLUGIN_ADAPTER_H
#define BACKGROUND_TASK_MGR_SERVICES_PLUGIN_INCLUDE_AUDIO_STREAM_PLUGIN_ADAPTER_H

#include <unordered_map>
#include <functional>
#include <mutex>
#include <list>
#include "singleton.h"
#include "nlohmann/json.hpp"
#include "bgtask_plugin.h"

namespace OHOS {
namespace BackgroundTaskMgr {
struct AudioInfo {
    int32_t uid_ {-1};
    int32_t sessionId_ {-1};
    AudioInfo() = default;
    AudioInfo(int32_t uid, int32_t sessionId)
        : uid_(uid), sessionId_(sessionId) {};
};

class AudioStreamPluginAdapter final : public BgtaskPlugin,
                                       public DelayedSingleton<AudioStreamPluginAdapter> {
public:
    static bool SelfRegister();
    void Init() override;
    void Uninit() override;
    std::string GetPluginName() const override;
    bool CheckAppIsPlaying(int32_t uid);

private:
    void InitAudioRendererStateChangeListener();
    void RendererStateChange(const nlohmann::json& payload);
    void InitCbMap(std::unordered_map<uint32_t,
        std::unordered_map<int32_t, std::function<void(const int32_t, const nlohmann::json&)>>>& cbMap) override;

    std::list<std::shared_ptr<AudioInfo>> audioPlayerInfos_ {};
    std::mutex pluginMutex_;
};
} // namespace BackgroundTaskMgr
} // namespace OHOS
#endif // BACKGROUND_TASK_MGR_SERVICES_PLUGIN_INCLUDE_AUDIO_STREAM_PLUGIN_ADAPTER_H