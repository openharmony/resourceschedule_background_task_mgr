/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "continuous_task_request_info.h"
#include "ipc_util.h"
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {

ContinuousTaskRequestInfo* ContinuousTaskRequestInfo::Unmarshalling(Parcel& in)
{
    ContinuousTaskRequestInfo* info = new (std::nothrow) ContinuousTaskRequestInfo();
    if (info && !info->ReadFromParcel(in)) {
        BGTASK_LOGE("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool ContinuousTaskRequestInfo::Marshalling(Parcel& out) const
{
    if (!out.WriteUInt32Vector(continuousTaskModes_)) {
        BGTASK_LOGE("Failed to write continuousTaskModes");
        return false;
    }
    if (!out.WriteUInt32Vector(continuousTaskSubmodes_)) {
        BGTASK_LOGE("Failed to write continuousTaskSubmodes");
        return false;
    }
    bool valid = wantAgent_ != nullptr;
    if (!out.WriteBool(valid)) {
        BGTASK_LOGE("Failed to write the flag which indicate whether wantAgent is null");
        return false;
    }
    if (valid) {
        if (!out.WriteParcelable(wantAgent_.get())) {
            BGTASK_LOGE("Failed to write wantAgent");
            return false;
        }
    }
    WRITE_PARCEL_WITH_RET(out, Bool, combinedTaskNotification_, false);
    WRITE_PARCEL_WITH_RET(out, Int32, continuousTaskId_, false);
    WRITE_PARCEL_WITH_RET(out, Bool, isBuildByRequest_, false);
    return true;
}

bool ContinuousTaskRequestInfo::ReadFromParcel(Parcel& in)
{
    if (!in.ReadUInt32Vector(&continuousTaskModes_)) {
        BGTASK_LOGE("read parcel continuousTaskModes error");
        return false;
    }
    if (!in.ReadUInt32Vector(&continuousTaskSubmodes_)) {
        BGTASK_LOGE("read parcel continuousTaskSubmodes error");
        return false;
    }
    bool valid = in.ReadBool();
    if (valid) {
        wantAgent_ = std::shared_ptr<AbilityRuntime::WantAgent::WantAgent>(
            in.ReadParcelable<AbilityRuntime::WantAgent::WantAgent>());
        if (!wantAgent_) {
            BGTASK_LOGE("Failed to read wantAgent");
            return false;
        }
    }

    READ_PARCEL_WITH_RET(in, Bool, combinedTaskNotification_, false);
    READ_PARCEL_WITH_RET(in, Int32, continuousTaskId_, false);
    READ_PARCEL_WITH_RET(in, Bool, isBuildByRequest_, false);
    return true;
}

std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> ContinuousTaskRequestInfo::GetWantAgent() const
{
    return wantAgent_;
}

std::vector<uint32_t> ContinuousTaskRequestInfo::GetContinuousTaskModes() const
{
    return continuousTaskModes_;
}

std::vector<uint32_t> ContinuousTaskRequestInfo::GetContinuousTaskSubmodes() const
{
    return continuousTaskSubmodes_;
}

bool ContinuousTaskRequestInfo::GetIsBuildByRequest() const
{
    return isBuildByRequest_;
}

void ContinuousTaskRequestInfo::SetIsBuildByRequest(bool isBuildByRequest)
{
    isBuildByRequest_ = isBuildByRequest;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS