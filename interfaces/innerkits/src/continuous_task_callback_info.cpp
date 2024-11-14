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
#include "ipc_util.h"

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

void ContinuousTaskCallbackInfo::SetContinuousTaskId(const int32_t id)
{
    continuousTaskId_ = id;
}
 
void ContinuousTaskCallbackInfo::SetCancelReason(const int32_t reason)
{
    cancelReason_ = reason;
}
 
int32_t ContinuousTaskCallbackInfo::GetContinuousTaskId() const
{
    return continuousTaskId_;
}
 
int32_t ContinuousTaskCallbackInfo::GetCancelReason() const
{
    return cancelReason_;
}

bool ContinuousTaskCallbackInfo::Marshalling(Parcel &parcel) const
{
    WRITE_PARCEL_WITH_RET(parcel, Uint32, typeId_, false);
    WRITE_PARCEL_WITH_RET(parcel, Int32, creatorUid_, false);
    WRITE_PARCEL_WITH_RET(parcel, Int32, creatorPid_, false);
    WRITE_PARCEL_WITH_RET(parcel, Bool, isFromWebview_, false);
    std::u16string u16AbilityName = Str8ToStr16(abilityName_);
    WRITE_PARCEL_WITH_RET(parcel, String16, u16AbilityName, false);
    WRITE_PARCEL_WITH_RET(parcel, Bool, isBatchApi_, false);
    WRITE_PARCEL_WITH_RET(parcel, UInt32Vector, typeIds_, false);
    WRITE_PARCEL_WITH_RET(parcel, Int32, abilityId_, false);
    WRITE_PARCEL_WITH_RET(parcel, Uint64, tokenId_, false);
    WRITE_PARCEL_WITH_RET(parcel, Int32, continuousTaskId_, false);
    WRITE_PARCEL_WITH_RET(parcel, Int32, cancelReason_, false);
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
    READ_PARCEL_WITH_RET(parcel, Uint32, typeId_, false);
    READ_PARCEL_WITH_RET(parcel, Int32, creatorUid_, false);
    READ_PARCEL_WITH_RET(parcel, Int32, creatorPid_, false);
    READ_PARCEL_WITH_RET(parcel, Bool, isFromWebview_, false);
    std::u16string u16AbilityName;
    READ_PARCEL_WITH_RET(parcel, String16, u16AbilityName, false);
    abilityName_ = Str16ToStr8(u16AbilityName);
    READ_PARCEL_WITH_RET(parcel, Bool, isBatchApi_, false);
    READ_PARCEL_WITH_RET(parcel, UInt32Vector, (&typeIds_), false);
    READ_PARCEL_WITH_RET(parcel, Int32, abilityId_, false);
    READ_PARCEL_WITH_RET(parcel, Uint64, tokenId_, false);
    READ_PARCEL_WITH_RET(parcel, Int32, continuousTaskId_, false);
    READ_PARCEL_WITH_RET(parcel, Int32, cancelReason_, false);	
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS