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

#include "continuous_task_callback_info.h"

#include <new>

#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
ContinuousTaskCallbackInfo::ContinuousTaskCallbackInfo() {}

ContinuousTaskCallbackInfo::ContinuousTaskCallbackInfo(uint32_t typeId, int32_t creatorUid,
    pid_t creatorPid, std::string abilityName)
    : typeId_(typeId), creatorUid_(creatorUid), creatorPid_(creatorPid), abilityName_(abilityName) {}

uint32_t ContinuousTaskCallbackInfo::GetTypeId() const
{
    return typeId_;
}

int32_t ContinuousTaskCallbackInfo::GetCreatorUid() const
{
    return creatorUid_;
}

pid_t ContinuousTaskCallbackInfo::GetCreatorPid() const
{
    return creatorPid_;
}

std::string ContinuousTaskCallbackInfo::GetAbilityName() const
{
    return abilityName_;
}

bool ContinuousTaskCallbackInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(typeId_)) {
        BGTASK_LOGE("Failed to write typeId");
        return false;
    }

    if (!parcel.WriteInt32(creatorUid_)) {
        BGTASK_LOGE("Failed to write creator uid");
        return false;
    }

    if (!parcel.WriteInt32(creatorPid_)) {
        BGTASK_LOGE("Failed to write creator pid");
        return false;
    }

    if (!parcel.WriteString(abilityName_)) {
        BGTASK_LOGE("Failed to write ability name");
        return false;
    }
    return true;
}

ContinuousTaskCallbackInfo *ContinuousTaskCallbackInfo::Unmarshalling(Parcel &parcel)
{
    auto object = new (std::nothrow) ContinuousTaskCallbackInfo();
    if ((object != nullptr) && !object->ReadFromParcel(parcel)) {
        delete object;
        object = nullptr;
    }

    return object;
}

bool ContinuousTaskCallbackInfo::ReadFromParcel(Parcel &parcel)
{
    typeId_ = parcel.ReadUint32();

    creatorUid_ = static_cast<int32_t>(parcel.ReadInt32());
    creatorPid_ = static_cast<pid_t>(parcel.ReadInt32());

    if (!parcel.ReadString(abilityName_)) {
        BGTASK_LOGE("Failed to read creator ability name");
        return false;
    }
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS