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

#include "task_detection_manager.h"

#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
bool TaskDetectionManager::Init(const std::shared_ptr<DataStorage> &dataStorage,
    const std::shared_ptr<AppExecFwk::EventHandler> &handler)
{
    return true;
}

void TaskDetectionManager::HandlePersistenceData(const std::vector<AppExecFwk::RunningProcessInfo> &allProcesses) {}

void TaskDetectionManager::Dump(std::vector<std::string> &dumpInfo) {}

bool TaskDetectionManager::CheckTaskRunningState(int32_t uid, uint32_t taskType)
{
    if (taskType == CommonUtils::AUDIO_PLAYBACK_BGMODE_ID) {
        return audioDetect_->CheckAudioCondition(uid, CommonUtils::AUDIO_PLAYBACK_BGMODE_ID);
    } else if (taskType == CommonUtils::AUDIO_RECORDING_BGMODE_ID) {
        return audioDetect_->CheckAudioCondition(uid, CommonUtils::AUDIO_RECORDING_BGMODE_ID);
    } else if (taskType == CommonUtils::LOCATION_BGMODE_ID) {
        return locationDetect_->CheckLocationCondition(uid);
    } else if (taskType == CommonUtils::BLUETOOTH_INTERACTION_BGMODE_ID) {
        return bluetoothDetect_->CheckBluetoothUsingScene(uid);
    } else if (taskType == CommonUtils::MULTIDEVICE_CONNECTION_BGMODE_ID) {
        return multiDeviceDetect_->CheckIsDisSchedScene(uid);
    } else {
        BGTASK_LOGE("Invalid taskType");
    }
    return false;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS