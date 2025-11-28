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

#include "efficiency_resource_info.h"
#include "ipc_util.h"
#include "efficiency_resource_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
bool EfficiencyResourceInfo::Marshalling(Parcel& out) const
{
    WRITE_PARCEL_WITH_RET(out, Uint32, resourceNumber_, false);
    WRITE_PARCEL_WITH_RET(out, Bool, isApply_, false);
    WRITE_PARCEL_WITH_RET(out, Uint32, timeOut_, false);
    WRITE_PARCEL_WITH_RET(out, String, reason_, false);
    WRITE_PARCEL_WITH_RET(out, Bool, isPersist_, false);
    WRITE_PARCEL_WITH_RET(out, Bool, isProcess_, false);
    WRITE_PARCEL_WITH_RET(out, Int32, uid_, false);
    WRITE_PARCEL_WITH_RET(out, Int32, pid_, false);
    WRITE_PARCEL_WITH_RET(out, Int32, static_cast<int32_t>(cpuLevel_), false);
    return true;
}

EfficiencyResourceInfo* EfficiencyResourceInfo::Unmarshalling(Parcel &in)
{
    EfficiencyResourceInfo* info = new (std::nothrow) EfficiencyResourceInfo();
    if (info && !info->ReadFromParcel(in)) {
        BGTASK_LOGE("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool EfficiencyResourceInfo::ReadFromParcel(Parcel& in)
{
    READ_PARCEL_WITH_RET(in, Uint32, resourceNumber_, false);
    READ_PARCEL_WITH_RET(in, Bool, isApply_, false);
    READ_PARCEL_WITH_RET(in, Uint32, timeOut_, false);
    READ_PARCEL_WITH_RET(in, String, reason_, false);
    READ_PARCEL_WITH_RET(in, Bool, isPersist_, false);
    READ_PARCEL_WITH_RET(in, Bool, isProcess_, false);
    READ_PARCEL_WITH_RET(in, Int32, uid_, false);
    READ_PARCEL_WITH_RET(in, Int32, pid_, false);
    int32_t cpuLevel = static_cast<int32_t>(EfficiencyResourcesCpuLevel::DEFAULT);
    READ_PARCEL_WITH_RET(in, Int32, cpuLevel, false);
    cpuLevel_ = static_cast<EfficiencyResourcesCpuLevel::Type>(cpuLevel);
    return true;
}

void EfficiencyResourceInfo::SetCpuLevel(int32_t cpuLevel)
{
    cpuLevel_ = EfficiencyResourcesCpuLevel::IsCpuLevelValid(cpuLevel) ?
        static_cast<EfficiencyResourcesCpuLevel::Type>(cpuLevel) : EfficiencyResourcesCpuLevel::DEFAULT;
}
}
}