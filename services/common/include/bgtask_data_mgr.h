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

#ifndef BACKGROUND_TASK_MGR_SERVICES_COMMON_INCLUDE_BGTASK_DATA_MGR_H
#define BACKGROUND_TASK_MGR_SERVICES_COMMON_INCLUDE_BGTASK_DATA_MGR_H

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

struct MultiDeviceInfo {
    int32_t uid_ {-1};
    int32_t said_ {-1};
    int32_t count_ {0};
    MultiDeviceInfo() = default;
    MultiDeviceInfo(int32_t uid, int32_t said, int32_t num)
        : uid_(uid), said_(said), count_(num) {};
};

class BgtaskDataMgr : public DelayedSingleton<BgtaskDataMgr> {
public:
    bool CheckAppIsPlaying(int32_t uid);
    bool IsAudioPlay(int32_t uid);
    bool IsMultiDeviceCast(int32_t uid);
    bool AddAudioPlayerInfo(const std::shared_ptr<AudioInfo> audioInfo);
    void RemoveAudioPlayerInfo(int32_t uid, int32_t sessionId);
    void AfterAddSaListener();
    void HandleMultiDeviceCastStart(int32_t uid, int32_t said);
    void HandleMultiDeviceCastStop(int32_t uid, int32_t said);
    void OnAppStopped(int32_t uid);
    void OnRemoveSystemAbility(int32_t said);
    void ClearAll();

private:
    void RemoveMultiDeviceInfoByUid(int32_t uid);
    void RemoveMultiDeviceInfoBySaid(int32_t said);

private:
    std::mutex dataMutex_;
    std::list<std::shared_ptr<AudioInfo>> audioPlayerInfos_ {};
    std::list<std::shared_ptr<MultiDeviceInfo>> multiDeviceInfo_ {};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // BACKGROUND_TASK_MGR_SERVICES_COMMON_INCLUDE_BGTASK_DATA_MGR_H