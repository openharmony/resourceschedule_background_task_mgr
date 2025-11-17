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

#include "background_task_state_info.h"
#include "ipc_util.h"
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
bool BackgroundTaskStateInfo::Marshalling(Parcel& out) const
{
    WRITE_PARCEL_WITH_RET(out, Int32, userId_, false);
    WRITE_PARCEL_WITH_RET(out, String, bundleName_, false);
    WRITE_PARCEL_WITH_RET(out, Int32, appIndex_, false);
    WRITE_PARCEL_WITH_RET(out, Int32, authResult_, false);
    return true;
}

BackgroundTaskStateInfo* BackgroundTaskStateInfo::Unmarshalling(Parcel &in)
{
    BackgroundTaskStateInfo* info = new (std::nothrow) BackgroundTaskStateInfo();
    if (info && !info->ReadFromParcel(in)) {
        BGTASK_LOGE("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool BackgroundTaskStateInfo::ReadFromParcel(Parcel& in)
{
    READ_PARCEL_WITH_RET(in, Int32, userId_, false);
    READ_PARCEL_WITH_RET(in, String, bundleName_, false);
    READ_PARCEL_WITH_RET(in, Int32, appIndex_, false);
    READ_PARCEL_WITH_RET(in, Int32, authResult_, false);
    return true;
}

void BackgroundTaskStateInfo::SetUserId(int32_t userId)
{
    userId_ = userId;
}

void BackgroundTaskStateInfo::SetBundleName(const std::string &bundleName)
{
    bundleName_ = bundleName;
}

void BackgroundTaskStateInfo::SetAppIndex(int32_t appIndex)
{
    appIndex_ = appIndex;
}

void BackgroundTaskStateInfo::SetUserAuthResult(int32_t authResult)
{
    authResult_ = authResult;
}

int32_t BackgroundTaskStateInfo::GetUserId() const
{
    return userId_;
}
    
std::string BackgroundTaskStateInfo::GetBundleName() const
{
    return bundleName_;
}

int32_t BackgroundTaskStateInfo::GetAppIndex() const
{
    return appIndex_;
}

int32_t BackgroundTaskStateInfo::GetUserAuthResult() const
{
    return authResult_;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS