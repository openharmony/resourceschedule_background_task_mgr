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

#include "transient_task_app_info.h"

#include <string_ex.h>

#include "ipc_util.h"

namespace OHOS {
namespace BackgroundTaskMgr {
bool TransientTaskAppInfo::Marshalling(Parcel& out) const
{
    WRITE_PARCEL_WITH_RET(out, String16, Str8ToStr16(packageName_), false);
    WRITE_PARCEL_WITH_RET(out, Int32, uid_, false);
    WRITE_PARCEL_WITH_RET(out, Int32, pid_, false);
    return true;
}

bool TransientTaskAppInfo::ReadFromParcel(Parcel& in)
{
    std::u16string package = in.ReadString16();
    packageName_ = Str16ToStr8(package);
    READ_PARCEL_WITH_RET(in, Int32, uid_, false);
    READ_PARCEL_WITH_RET(in, Int32, pid_, false);
    return true;
}

TransientTaskAppInfo* TransientTaskAppInfo::Unmarshalling(Parcel& in)
{
    TransientTaskAppInfo* info = new (std::nothrow) TransientTaskAppInfo();
    if (info && !info->ReadFromParcel(in)) {
        BGTASK_LOGE("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS