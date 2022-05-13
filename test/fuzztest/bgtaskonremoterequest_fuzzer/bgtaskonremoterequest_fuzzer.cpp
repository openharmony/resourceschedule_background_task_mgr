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

#include "bgtaskonremoterequest_fuzzer.h"

#define private public
#include "background_task_mgr_service.h"
#include "background_task_mgr_stub.h"
#include "bg_continuous_task_mgr.h"

namespace OHOS {
namespace BackgroundTaskMgr {
    constexpr int32_t MIN_LEN = 4;
    constexpr int32_t MAX_CODE_TEST = 15; // current max code is 8
    static bool isInited = false;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data)
    {
        MessageParcel reply;
        MessageOption option;
        if (!isInited) {
            DelayedSingleton<BackgroundTaskMgrService>::GetInstance()->Init();
            isInited = true;
        }
        int32_t ret =
            DelayedSingleton<BackgroundTaskMgrService>::GetInstance()->OnRemoteRequest(code, data, reply, option);
        return ret;
    }

    void DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        if (size <= MIN_LEN) {
            return;
        }

        MessageParcel dataMessageParcel;
        if (!dataMessageParcel.WriteInterfaceToken(BackgroundTaskMgrStub::GetDescriptor())) {
            return;
        }

        uint32_t code = *(reinterpret_cast<const uint32_t*>(data));
        if (code > MAX_CODE_TEST) {
            return;
        }
        size -= sizeof(uint32_t);

        dataMessageParcel.WriteBuffer(data + sizeof(uint32_t), size);
        dataMessageParcel.RewindRead(0);

        OnRemoteRequest(code, dataMessageParcel);
    }
} // BackgroundTaskMgr
} // OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::BackgroundTaskMgr::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

