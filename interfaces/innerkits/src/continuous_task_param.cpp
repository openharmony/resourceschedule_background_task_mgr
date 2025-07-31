/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "continuous_task_param.h"

#include "string_ex.h"

#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
bool ContinuousTaskParam::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadBool(isNewApi_)) {
        return false;
    }
    if (!parcel.ReadUint32(bgModeId_)) {
        return false;
    }
    bool valid = parcel.ReadBool();
    if (valid) {
        wantAgent_ = std::shared_ptr<AbilityRuntime::WantAgent::WantAgent>(
            parcel.ReadParcelable<AbilityRuntime::WantAgent::WantAgent>());
        if (!wantAgent_) {
            return false;
        }
    }
    std::u16string u16AbilityName;
    if (!parcel.ReadString16(u16AbilityName)) {
        return false;
    }
    abilityName_ = Str16ToStr8(u16AbilityName);

    std::u16string u16AppName;
    if (!parcel.ReadString16(u16AppName)) {
        return false;
    }
    appName_ = Str16ToStr8(u16AppName);

    if (!parcel.ReadBool(isBatchApi_)) {
        return false;
    }
    if (isBatchApi_) {
        if (!parcel.ReadUInt32Vector(&bgModeIds_)) {
            return false;
        }
    }
    if (!parcel.ReadInt32(abilityId_)) {
        return false;
    }
    if (!parcel.ReadBool(isACLTaskkeeping_)) {
        return false;
    }
    return true;
}

bool ContinuousTaskParamForInner::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadBool(isStart_)) {
        BGTASK_LOGE("Failed to read the flag which indicate keep or stop running background");
        return false;
    }

    if (!parcel.ReadUint32(bgModeId_)) {
        BGTASK_LOGE("Failed to read request background mode info");
        return false;
    }

    if (!parcel.ReadInt32(uid_)) {
        BGTASK_LOGE("Failed to read uid info");
        return false;
    }

    if (!parcel.ReadInt32(abilityId_)) {
        BGTASK_LOGE("Failed to read the abilityId");
        return false;
    }
    
    if (!parcel.ReadUint64(tokenId_)) {
        BGTASK_LOGE("Failed to read the tokenId");
        return false;
    }

    if (!parcel.ReadInt32(pid_)) {
        BGTASK_LOGE("Failed to read the pid");
        return false;
    }
    return true;
}

ContinuousTaskParam *ContinuousTaskParam::Unmarshalling(Parcel &parcel)
{
    ContinuousTaskParam *param = new (std::nothrow) ContinuousTaskParam();
    if (param && !param->ReadFromParcel(parcel)) {
        BGTASK_LOGE("read from parcel failed");
        delete param;
        param = nullptr;
    }
    return param;
}

ContinuousTaskParamForInner *ContinuousTaskParamForInner::Unmarshalling(Parcel &parcel)
{
    ContinuousTaskParamForInner *param = new (std::nothrow) ContinuousTaskParamForInner();
    if (param && !param->ReadFromParcel(parcel)) {
        BGTASK_LOGE("read from parcel failed");
        delete param;
        param = nullptr;
    }
    return param;
}

bool ContinuousTaskParam::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(isNewApi_)) {
        return false;
    }
    if (!parcel.WriteUint32(bgModeId_)) {
        return false;
    }
    bool valid = wantAgent_ != nullptr;
    if (!parcel.WriteBool(valid)) {
        return false;
    }
    if (valid) {
        if (!parcel.WriteParcelable(wantAgent_.get())) {
            return false;
        }
    }
    std::u16string u16AbilityName = Str8ToStr16(abilityName_);
    if (!parcel.WriteString16(u16AbilityName)) {
        return false;
    }
    std::u16string u16AppName = Str8ToStr16(appName_);
    if (!parcel.WriteString16(u16AppName)) {
        return false;
    }
    if (!parcel.WriteBool(isBatchApi_)) {
        return false;
    }
    if (isBatchApi_) {
        if (!parcel.WriteUInt32Vector(bgModeIds_)) {
            return false;
        }
    }
    if (!parcel.WriteInt32(abilityId_)) {
        return false;
    }
    if (!parcel.WriteBool(isACLTaskkeeping_)) {
        return false;
    }
    return true;
}

bool ContinuousTaskParamForInner::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(isStart_)) {
        BGTASK_LOGE("Failed to write the flag which indicate keep or stop running background");
        return false;
    }

    if (!parcel.WriteUint32(bgModeId_)) {
        BGTASK_LOGE("Failed to write request background mode info");
        return false;
    }

    if (!parcel.WriteInt32(uid_)) {
        BGTASK_LOGE("Failed to write uid info");
        return false;
    }

    if (!parcel.WriteInt32(abilityId_)) {
        BGTASK_LOGE("Failed to write the abilityId");
        return false;
    }
    if (!parcel.WriteUint64(tokenId_)) {
        BGTASK_LOGE("Failed to write tokenId_");
        return false;
    }
    if (!parcel.WriteInt32(pid_)) {
        BGTASK_LOGE("Failed to write pid_");
        return false;
    }
    return true;
}

void ContinuousTaskParam::SetACLTaskkeeping(bool isACLTaskkeeping)
{
    isACLTaskkeeping_ = isACLTaskkeeping;
}

bool ContinuousTaskParam::IsACLTaskkeeping() const
{
    return isACLTaskkeeping_;
}

void ContinuousTaskParamForInner::SetPid(const int32_t pid)
{
    pid_ = pid;
}

int32_t ContinuousTaskParamForInner::GetPid() const
{
    return pid_;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS