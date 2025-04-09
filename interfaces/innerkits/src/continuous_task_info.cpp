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

#include "continuous_task_info.h"
#include "ipc_util.h"
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
std::string ContinuousTaskInfo::GetAbilityName() const
{
    return abilityName_;
}

std::string ContinuousTaskInfo::GetWantAgentBundleName() const
{
    return wantAgentBundleName_;
}

std::string ContinuousTaskInfo::GetWantAgentAbilityName() const
{
    return wantAgentAbilityName_;
}

int32_t ContinuousTaskInfo::GetUid() const
{
    return uid_;
}

int32_t ContinuousTaskInfo::GetPid() const
{
    return pid_;
}

int32_t ContinuousTaskInfo::GetNotificationId() const
{
    return notificationId_;
}

int32_t ContinuousTaskInfo::GetContinuousTaskId() const
{
    return continuousTaskId_;
}

int32_t ContinuousTaskInfo::GetAbilityId() const
{
    return abilityId_;
}

std::vector<uint32_t> ContinuousTaskInfo::GetBackgroundModes() const
{
    return backgroundModes_;
}

std::vector<uint32_t> ContinuousTaskInfo::GetBackgroundSubModes() const
{
    return backgroundSubModes_;
}

bool ContinuousTaskInfo::IsFromWebView() const
{
    return isFromWebView_;
}

std::string ContinuousTaskInfo::ToString(const std::vector<uint32_t> &modes) const
{
    std::string result;
    for (auto it : modes) {
        result += std::to_string(it);
        result += ",";
    }
    return result;
}

bool ContinuousTaskInfo::Marshalling(Parcel& out) const
{
    WRITE_PARCEL_WITH_RET(out, String, abilityName_, false);
    WRITE_PARCEL_WITH_RET(out, Int32, uid_, false);
    WRITE_PARCEL_WITH_RET(out, Int32, pid_, false);
    WRITE_PARCEL_WITH_RET(out, Bool, isFromWebView_, false);
    if (!out.WriteUInt32Vector(backgroundModes_)) {
        BGTASK_LOGE("Failed to write backgroundModes");
        return false;
    }
    if (!out.WriteUInt32Vector(backgroundSubModes_)) {
        BGTASK_LOGE("Failed to write backgroundSubModes");
        return false;
    }
    WRITE_PARCEL_WITH_RET(out, Int32, notificationId_, false);
    WRITE_PARCEL_WITH_RET(out, Int32, continuousTaskId_, false);
    WRITE_PARCEL_WITH_RET(out, Int32, abilityId_, false);
    WRITE_PARCEL_WITH_RET(out, String, wantAgentBundleName_, false);
    WRITE_PARCEL_WITH_RET(out, String, wantAgentAbilityName_, false);
    return true;
}

ContinuousTaskInfo* ContinuousTaskInfo::Unmarshalling(Parcel &in)
{
    ContinuousTaskInfo* info = new (std::nothrow) ContinuousTaskInfo();
    if (info && !info->ReadFromParcel(in)) {
        BGTASK_LOGE("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool ContinuousTaskInfo::ReadFromParcel(Parcel& in)
{
    READ_PARCEL_WITH_RET(in, String, abilityName_, false);
    READ_PARCEL_WITH_RET(in, Int32, uid_, false);
    READ_PARCEL_WITH_RET(in, Int32, pid_, false);
    READ_PARCEL_WITH_RET(in, Bool, isFromWebView_, false);
    if (!in.ReadUInt32Vector(&backgroundModes_)) {
        BGTASK_LOGE("read parce backgroundModes error");
        return false;
    }
    if (!in.ReadUInt32Vector(&backgroundSubModes_)) {
        BGTASK_LOGE("read parce backgroundSubModes error");
        return false;
    }
    READ_PARCEL_WITH_RET(in, Int32, notificationId_, false);
    READ_PARCEL_WITH_RET(in, Int32, continuousTaskId_, false);
    READ_PARCEL_WITH_RET(in, Int32, abilityId_, false);
    READ_PARCEL_WITH_RET(in, String, wantAgentBundleName_, false);
    READ_PARCEL_WITH_RET(in, String, wantAgentAbilityName_, false);
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS