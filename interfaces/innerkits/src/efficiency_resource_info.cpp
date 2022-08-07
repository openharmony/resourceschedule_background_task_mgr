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

#include "efficiency_resource_info.h"
#include "ipc_util.h"
#include "efficiency_resource_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
bool EfficiencyResourceInfo::Marshalling(Parcel& out) const
{
    WRITE_PARCEL_WITH_RET(out, Uint32, resourceNumber_, false);
    WRITE_PARCEL_WITH_RET(out, Bool, isApply_, false);
    WRITE_PARCEL_WITH_RET(out, Int32, timeOut_, false);
    WRITE_PARCEL_WITH_RET(out, String, reason_, false);
    WRITE_PARCEL_WITH_RET(out, Bool, isPersist_, false);
    WRITE_PARCEL_WITH_RET(out, Bool, isProcess_, false);
    return true;
}

std::shared_ptr<EfficiencyResourceInfo> EfficiencyResourceInfo::Unmarshalling(Parcel &in)
{
    auto info = std::make_shared<EfficiencyResourceInfo>();
    if (info == nullptr || !info->ReadFromParcel(in)) {
        BGTASK_LOGE("read from parcel failed");
        info.reset();
        return nullptr;
    }
    return info;
}

bool EfficiencyResourceInfo::ReadFromParcel(Parcel& in)
{
    READ_PARCEL_WITH_RET(in, Uint32, resourceNumber_, false);
    READ_PARCEL_WITH_RET(in, Bool, isApply_, false);
    READ_PARCEL_WITH_RET(in, Int32, timeOut_, false);
    READ_PARCEL_WITH_RET(in, String, reason_, false);
    READ_PARCEL_WITH_RET(in, Bool, isPersist_, false);
    READ_PARCEL_WITH_RET(in, Bool, isProcess_, false);
    return true;
}
}
}