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

#include "background_task_mgr_proxy.h"

#include <message_parcel.h>
#include <string_ex.h>

#include "bgtaskmgr_log_wrapper.h"
#include "datetime_ex.h"
#include "mock_background_task_mgr_proxy_helper.h"

namespace {
    int32_t g_innerTransactFlag = 0;
}

namespace OHOS {
namespace BackgroundTaskMgr {
void BgTaskMgrProxyHelper::BgTaskFwkAbnormalSetBgTaskMgrProxyInnerTransactFlag(int32_t flag)
{
    g_innerTransactFlag = flag;
}

ErrCode BackgroundTaskMgrProxy::InnerTransact(uint32_t code, MessageOption &flags,
    MessageParcel &data, MessageParcel &reply)
{
    if (g_innerTransactFlag == 1) {
        return ERR_OK;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("InnerTransact get Remote fail code %{public}d", code);
        return ERR_DEAD_OBJECT;
    }
    int32_t err = remote->SendRequest(code, data, reply, flags);
    switch (err) {
        case NO_ERROR: {
            return ERR_OK;
        }
        case DEAD_OBJECT: {
            BGTASK_LOGE("[InnerTransact] fail: ipcErr=%{public}d code %{public}u", err, code);
            return ERR_DEAD_OBJECT;
        }
        default: {
            BGTASK_LOGE("[InnerTransact] fail: ipcErr=%{public}d code %{public}u", err, code);
            return ERR_BGTASK_TRANSACT_FAILED;
        }
    }
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS