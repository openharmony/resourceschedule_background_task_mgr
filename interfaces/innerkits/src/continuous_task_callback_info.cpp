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

#include "string_ex.h"

#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
ContinuousTaskCallbackInfo::ContinuousTaskCallbackInfo() {}

uint32_t ContinuousTaskCallbackInfo::GetTypeId() const
{
    return typeId_;
}

const std::vector<uint32_t>& ContinuousTaskCallbackInfo::GetTypeIds() const
{
    return typeIds_;
}

bool ContinuousTaskCallbackInfo::IsBatchApi() const
{
    return isBatchApi_;
}

int32_t ContinuousTaskCallbackInfo::GetAbilityId() const
{
    return abilityId_;
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

bool ContinuousTaskCallbackInfo::IsFromWebview() const
{
    return isFromWebview_;
}

uint64_t ContinuousTaskCallbackInfo::GetTokenId() const
{
    return tokenId_;
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

    if (!parcel.WriteBool(isFromWebview_)) {
        BGTASK_LOGE("Failed to write the flag which indicates from webview");
        return false;
    }

    std::u16string u16AbilityName = Str8ToStr16(abilityName_);
    if (!parcel.WriteString16(u16AbilityName)) {
        BGTASK_LOGE("Failed to write ability name");
        return false;
    }
    if (!parcel.WriteBool(isBatchApi_)) {
        BGTASK_LOGE("Failed to write isBatchApi_");
        return false;
    }
    BGTASK_LOGD("write typeIds_ size %{public}u", static_cast<uint32_t>(typeIds_.size()));
    if (!parcel.WriteUInt32Vector(typeIds_)) {
        BGTASK_LOGE("Failed to write typeIds_");
        return false;
    }
    if (!parcel.WriteInt32(abilityId_)) {
        BGTASK_LOGE("Failed to write abilityId_");
        return false;
    }
    if (!parcel.WriteUint64(tokenId_)) {
        BGTASK_LOGE("Failed to write tokenId_");
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
    if (parcel.ReadUint32(typeId_)) {
        BGTASK_LOGE("read parce typeId error");
        return false;
    }
    if (parcel.ReadInt32(creatorUid_)) {
        BGTASK_LOGE("read parce creatorUid error");
        return false;
    }
    if (parcel.ReadInt32(creatorPid_)) {
        BGTASK_LOGE("read parce creatorPid error");
        return false;
    }

    if (!parcel.ReadBool(isFromWebview_)) {
        BGTASK_LOGE("Failed to read the flag which indicates from webview");
        return false;
    }

    std::u16string u16AbilityName;
    if (!parcel.ReadString16(u16AbilityName)) {
        BGTASK_LOGE("Failed to read creator ability name");
        return false;
    }
    abilityName_ = Str16ToStr8(u16AbilityName);
    if (!parcel.ReadBool(isBatchApi_)) {
        BGTASK_LOGE("Failed to read the flag isBatchApi_");
        return false;
    }
    if (!parcel.ReadUInt32Vector(&typeIds_)) {
        BGTASK_LOGE("read parce typeIds_ error");
        return false;
    }
    BGTASK_LOGD("read parce typeIds_ size %{public}u", static_cast<uint32_t>(typeIds_.size()));
    if (parcel.ReadInt32(abilityId_)) {
        BGTASK_LOGE("read parce abilityId error");
        return false;
    }
    if (parcel.ReadUint64(tokenId_)) {
        BGTASK_LOGE("read parce tokenId error");
        return false;
    }
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS