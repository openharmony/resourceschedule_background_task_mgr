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

#include "resource_callback_info.h"

#include <string_ex.h>

#include "ipc_util.h"

namespace OHOS {
namespace BackgroundTaskMgr {
bool ResourceCallbackInfo::Marshalling(Parcel& out) const
{
    WRITE_PARCEL_WITH_RET(out, Int32, uid_, false);
    WRITE_PARCEL_WITH_RET(out, Int32, pid_, false);
    WRITE_PARCEL_WITH_RET(out, Uint32, resourceNumber_, false);
    WRITE_PARCEL_WITH_RET(out, String, bundleName_, false);
    return true;
}

bool ResourceCallbackInfo::ReadFromParcel(Parcel& in)
{
    READ_PARCEL_WITH_RET(in, Int32, uid_, false);
    READ_PARCEL_WITH_RET(in, Int32, pid_, false);
    READ_PARCEL_WITH_RET(in, Uint32, resourceNumber_, false);
    READ_PARCEL_WITH_RET(in, String, bundleName_, false);
    return true;
}

ResourceCallbackInfo* ResourceCallbackInfo::Unmarshalling(Parcel& in)
{
    ResourceCallbackInfo* resourceInfo = new (std::nothrow) ResourceCallbackInfo();
    if (resourceInfo && !resourceInfo->ReadFromParcel(in)) {
        BGTASK_LOGE("read ResourceCallbackInfo from parcel failed");
        delete resourceInfo;
        resourceInfo = nullptr;
    }
    return resourceInfo;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS