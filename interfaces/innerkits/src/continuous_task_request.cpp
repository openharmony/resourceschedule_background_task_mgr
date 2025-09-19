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

#include "continuous_task_request.h"
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {

ContinuousTaskRequest* ContinuousTaskRequest::Unmarshalling(Parcel& in)
{
    ContinuousTaskRequest* info = new (std::nothrow) ContinuousTaskRequest();
    if (info && !info->ReadFromParcel(in)) {
        BGTASK_LOGE("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool ContinuousTaskRequest::Marshalling(Parcel& out) const
{
    if (!out.WriteUInt32Vector(backgroundTaskModes_)) {
        BGTASK_LOGE("Failed to write backgroundTaskModes");
        return false;
    }
    if (!out.WriteUInt32Vector(backgroundTaskSubmodes_)) {
        BGTASK_LOGE("Failed to write backgroundTaskSubmodes");
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
    if (!out.WriteBool(combinedTaskNotification_)) {
        BGTASK_LOGE("Failed to write combinedTaskNotification");
        return false;
    }
    if (!out.WriteInt32(continuousTaskId_)) {
        BGTASK_LOGE("Failed to write continuousTaskId");
        return false;
    }
    if (!out.WriteBool(isBuildByRequest_)) {
        BGTASK_LOGE("Failed to write isBuildByRequest");
        return false;
    }
    return true;
}

bool ContinuousTaskRequest::ReadFromParcel(Parcel& in)
{
    if (!in.ReadUInt32Vector(&backgroundTaskModes_)) {
        BGTASK_LOGE("read parcel backgroundTaskModes error");
        return false;
    }
    if (!in.ReadUInt32Vector(&backgroundTaskSubmodes_)) {
        BGTASK_LOGE("read parcel backgroundTaskSubmodes error");
        return false;
    }
    bool valid = in.ReadBool();
    if (valid) {
        wantAgent_ = std::shared_ptr<AbilityRuntime::WantAgent::WantAgent>(
            in.ReadParcelable<AbilityRuntime::WantAgent::WantAgent>());
        if (!wantAgent_) {
            BGTASK_LOGE("read parcel wantAgent error");
            return false;
        }
    }
    if (!in.ReadBool(combinedTaskNotification_)) {
        BGTASK_LOGE("read parcel combinedTaskNotification error");
        return false;
    }
    if (!in.ReadInt32(continuousTaskId_)) {
        BGTASK_LOGE("read parcel continuousTaskId error");
        return false;
    }
    if (!in.ReadBool(isBuildByRequest_)) {
        BGTASK_LOGE("read parcel isBuildByRequest error");
        return false;
    }
    return true;
}

std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> ContinuousTaskRequest::GetWantAgent() const
{
    return wantAgent_;
}

std::vector<uint32_t> ContinuousTaskRequest::GetBackgroundTaskModes() const
{
    return backgroundTaskModes_;
}

std::vector<uint32_t> ContinuousTaskRequest::GetBackgroundTaskSubmodes() const
{
    return backgroundTaskSubmodes_;
}

bool ContinuousTaskRequest::IsBuildByRequest() const
{
    return isBuildByRequest_;
}

void ContinuousTaskRequest::SetIsBuildByRequest(bool isBuildByRequest)
{
    isBuildByRequest_ = isBuildByRequest;
}

int32_t ContinuousTaskRequest::GetContinuousTaskId() const
{
    return continuousTaskId_;
}

void ContinuousTaskRequest::SetContinuousTaskId(const int32_t continuousTaskId)
{
    continuousTaskId_ = continuousTaskId;
}

void ContinuousTaskRequest::SetCombinedTaskNotification(const bool combinedTaskNotification)
{
    combinedTaskNotification_ = combinedTaskNotification;
}

bool ContinuousTaskRequest::IsCombinedTaskNotification() const
{
    return combinedTaskNotification_;
}

void ContinuousTaskRequest::SetWantAgent(
    const std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent)
{
    wantAgent_ = wantAgent;
}

void ContinuousTaskRequest::SetBackgroundTaskMode(const std::vector<uint32_t> &backgroundTaskMode)
{
    backgroundTaskModes_ = backgroundTaskMode;
}

void ContinuousTaskRequest::SetBackgroundTaskSubMode(const std::vector<uint32_t> &backgroundTaskSubMode)
{
    backgroundTaskSubmodes_ = backgroundTaskSubMode;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS