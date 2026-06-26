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

#include "audio_stream_manager.h"
#include "audio_info.h"
#include "bgtaskmgr_log_wrapper.h"

namespace OHOS {
namespace {
    int32_t g_mockAudioRenderState = 0;
}

void BgMockGetCurrentRendererChangeInfos(int32_t mockRet)
{
    g_mockAudioRenderState = mockRet;
}

namespace AudioStandard {
using namespace std;
int32_t AudioStreamManager::GetCurrentRendererChangeInfos(
    vector<shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    if (g_mockAudioRenderState == -1) {
        audioRendererChangeInfos.push_back(nullptr);
        return 0;
    }
    auto renderInfo = std::make_shared<AudioStandard::AudioRendererChangeInfo>();
    if (g_mockAudioRenderState == 0) {
        renderInfo->rendererState = AudioStandard::RendererState::RENDERER_PAUSED;
    } else if (g_mockAudioRenderState == 1) {
        renderInfo->rendererState = AudioStandard::RendererState::RENDERER_RUNNING;
    }
    renderInfo->clientUID = 1;
    renderInfo->clientPid = 1;
    renderInfo->sessionId = 1;
    audioRendererChangeInfos.push_back(renderInfo);
    return 0;
}
}
}