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

#include <new>

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

    if (!parcel.ReadString(abilityName_)) {
        BGTASK_LOGE("Failed to read ability name");
        return false;
    }

    if (!parcel.ReadString(appName_)) {
        BGTASK_LOGE("Failed to read app name");
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

    if (!parcel.WriteString(abilityName_)) {
        BGTASK_LOGE("Failed to write abilityName");
        return false;
    }
    if (!parcel.WriteString(appName_)) {
        BGTASK_LOGE("Failed to write appName");
        return false;
    }
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS