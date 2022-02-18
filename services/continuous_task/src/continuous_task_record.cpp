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

#include "continuous_task_record.h"

#include "iremote_object.h"

namespace OHOS {
namespace BackgroundTaskMgr {
const char *ContinuousTaskModeName[10] = {
    "dataTransfer",
    "audioPlayback",
    "audioRecording",
    "location",
    "bluetoothInteraction",
    "multiDeviceConnection",
    "wifiInteraction",
    "voip",
    "taskKeeping",
    "default",
};

ContinuousTaskRecord::ContinuousTaskRecord(const std::string &bundleName, const std::string &abilityName,
    const std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> &wantAgent, const sptr<IRemoteObject> &abilityToken,
    int32_t userId, uid_t uid, pid_t pid, uint32_t bgModeId, bool isNewApi)
    : bundleName_(bundleName), abilityName_(abilityName), wantAgent_(wantAgent), abilityToken_(abilityToken),
    userId_(userId), uid_(uid), pid_(pid), bgModeId_(bgModeId), isNewApi_(isNewApi) {}

std::string ContinuousTaskRecord::GetBundleName() const
{
    return bundleName_;
}

std::string ContinuousTaskRecord::GetAbilityName() const
{
    return abilityName_;
}

bool ContinuousTaskRecord::IsNewApi() const
{
    return isNewApi_;
}

uint32_t ContinuousTaskRecord::GetBgModeId() const
{
    return bgModeId_;
}

int32_t ContinuousTaskRecord::GetUserId() const
{
    return userId_;
}

uid_t ContinuousTaskRecord::GetUid() const
{
    return uid_;
}

pid_t ContinuousTaskRecord::GetPid() const
{
    return pid_;
}

std::string ContinuousTaskRecord::GetNotificationLabel() const
{
    return notificationLabel_;
}

std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> ContinuousTaskRecord::GetWantAgent() const
{
    return wantAgent_;
}

sptr<IRemoteObject> ContinuousTaskRecord::GetAbilityToken() const
{
    return abilityToken_;
}
}
}