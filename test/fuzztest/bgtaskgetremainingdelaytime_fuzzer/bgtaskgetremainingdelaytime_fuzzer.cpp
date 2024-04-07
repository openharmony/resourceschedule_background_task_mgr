/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "bgtaskgetremainingdelaytime_fuzzer.h"
#include "securec.h"

#include "background_task_mgr_service.h"
#include "ibackground_task_mgr_ipc_interface_code.h"

namespace OHOS {
namespace BackgroundTaskMgr {
    const std::string BGTASK_SERVICE_NAME = "BgtaskMgrService";
    constexpr int32_t U32_AT_SIZE = 4;
    bool g_isOnstarted = false;
    const std::u16string BACKGROUND_TASK_MGR_STUB_TOKEN = u"ohos.resourceschedule.IBackgroundTaskMgr";

    bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
    {
        MessageParcel datas;
        datas.WriteInterfaceToken(BACKGROUND_TASK_MGR_STUB_TOKEN);
        datas.WriteBuffer(data, size);
        datas.RewindRead(0);
        MessageParcel reply;
        MessageOption option;
        if (!g_isOnstarted) {
            std::shared_ptr<AppExecFwk::EventRunner> runner_ {nullptr};
            runner_ = AppExecFwk::EventRunner::Create(BGTASK_SERVICE_NAME);
            DelayedSingleton<BgTransientTaskMgr>::GetInstance()->Init(runner_);
            g_isOnstarted = true;
        }
        uint32_t code = static_cast<uint32_t>(BackgroundTaskMgrStubInterfaceCode::GET_REMAINING_DELAY_TIME);
        DelayedSingleton<BackgroundTaskMgrService>::GetInstance()->OnRemoteRequest(
            code, datas, reply, option);
        return true;
    }
} // BackgroundTaskMgr
} // OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    if (size < OHOS::BackgroundTaskMgr::U32_AT_SIZE) {
        return 0;
    }

    char* ch = static_cast<char *>(malloc(size + 1));
    if (ch == nullptr) {
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::BackgroundTaskMgr::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}
