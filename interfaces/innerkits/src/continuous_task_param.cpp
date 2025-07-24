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

#include "continuous_task_param.h"

#include "string_ex.h"

#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
bool ContinuousTaskParam::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadBool(isNewApi_)) {
        BGTASK_LOGE("Failed to read the flag which indicate whether is called from newApi");
        return false;
    }

    if (!parcel.ReadUint32(bgModeId_)) {
        BGTASK_LOGE("Failed to read request background mode info");
        return false;
    }
    bool valid = parcel.ReadBool();
    if (valid) {
        wantAgent_ = std::shared_ptr<AbilityRuntime::WantAgent::WantAgent>(
            parcel.ReadParcelable<AbilityRuntime::WantAgent::WantAgent>());
        if (!wantAgent_) {
            BGTASK_LOGE("Failed to read wantAgent");
            return false;
        }
    }

    std::u16string u16AbilityName;
    if (!parcel.ReadString16(u16AbilityName)) {
        BGTASK_LOGE("Failed to read ability name");
        return false;
    }
    abilityName_ = Str16ToStr8(u16AbilityName);

    std::u16string u16AppName;
    if (!parcel.ReadString16(u16AppName)) {
        BGTASK_LOGE("Failed to read app name");
        return false;
    }
    appName_ = Str16ToStr8(u16AppName);

    if (!parcel.ReadBool(isBatchApi_)) {
        BGTASK_LOGE("Failed to read the flag isBatchApi");
        return false;
    }
    if (isBatchApi_) {
        if (!parcel.ReadUInt32Vector(&bgModeIds_)) {
            BGTASK_LOGE("read parce bgmodes error");
            return false;
        }
        BGTASK_LOGD("read parce bgmodes_ size %{public}d", static_cast<uint32_t>(bgModeIds_.size()));
    }
    if (!parcel.ReadInt32(abilityId_)) {
        BGTASK_LOGE("Failed to read the abilityId");
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
        BGTASK_LOGE("Failed to write the flag which indicate whether is called from newApi");
        return false;
    }

    if (!parcel.WriteUint32(bgModeId_)) {
        BGTASK_LOGE("Failed to write request background mode info");
        return false;
    }
    bool valid = wantAgent_ != nullptr;
    if (!parcel.WriteBool(valid)) {
        BGTASK_LOGE("Failed to write the flag which indicate whether wantAgent is null");
        return false;
    }
    if (valid) {
        if (!parcel.WriteParcelable(wantAgent_.get())) {
            BGTASK_LOGE("Failed to write wantAgent");
            return false;
        }
    }

    std::u16string u16AbilityName = Str8ToStr16(abilityName_);
    if (!parcel.WriteString16(u16AbilityName)) {
        BGTASK_LOGE("Failed to write abilityName");
        return false;
    }
    std::u16string u16AppName = Str8ToStr16(appName_);
    if (!parcel.WriteString16(u16AppName)) {
        BGTASK_LOGE("Failed to write appName");
        return false;
    }
    if (!parcel.WriteBool(isBatchApi_)) {
        BGTASK_LOGE("Failed to write the isBatchApi");
        return false;
    }

    if (isBatchApi_) {
        BGTASK_LOGD("write modes %{public}u", static_cast<uint32_t>(bgModeIds_.size()));
        if (!parcel.WriteUInt32Vector(bgModeIds_)) {
            BGTASK_LOGE("Failed to write bgModeIds");
            return false;
        }
    }

    if (!parcel.WriteInt32(abilityId_)) {
        BGTASK_LOGE("Failed to write the abilityId");
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

void ContinuousTaskParamForInner::SetPid(int32_t pid)
{
    pid_ = pid;
}

int32_t ContinuousTaskParamForInner::GetPid()
{
    return pid_;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS