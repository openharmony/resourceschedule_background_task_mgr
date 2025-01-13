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

#include "bgtasksubscriberstubonremote_fuzzer.h"
#include "ibackground_task_subscriber.h"
#include "securec.h"

#include "background_task_subscriber_stub.h"
#include "background_task_subscriber.h"

namespace OHOS {
namespace BackgroundTaskMgr {
    constexpr int32_t U32_AT_SIZE = 4;

    const std::u16string BACKGROUND_TASK_MGR_SUBSCRIBER_TOKEN = u"ohos.resourceschedule.IBackgroundTaskSubscriber";
class TestBackgroundTaskSubscriber : public BackgroundTaskSubscriber {
public:
    TestBackgroundTaskSubscriber() : BackgroundTaskSubscriber() {}
};

    bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
    {
        MessageParcel datas;
        datas.WriteInterfaceToken(BACKGROUND_TASK_MGR_SUBSCRIBER_TOKEN);
        datas.WriteBuffer(data, size);
        datas.RewindRead(0);
        MessageParcel reply;
        MessageOption option;
        auto subscriber = TestBackgroundTaskSubscriber();
        auto subscriberImpl = std::make_shared<BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl>(subscriber);
        
        uint32_t code1 = static_cast<uint32_t>(IBackgroundTaskSubscriberIpcCode::COMMAND_ON_CONNECTED);
        subscriberImpl->OnRemoteRequest(code1, datas, reply, option);

        uint32_t code2 = static_cast<uint32_t>(IBackgroundTaskSubscriberIpcCode::COMMAND_ON_DISCONNECTED);
        subscriberImpl->OnRemoteRequest(code2, datas, reply, option);

        uint32_t code3 = static_cast<uint32_t>(IBackgroundTaskSubscriberIpcCode::COMMAND_ON_TRANSIENT_TASK_START);
        subscriberImpl->OnRemoteRequest(code3, datas, reply, option);

        uint32_t code4 = static_cast<uint32_t>(IBackgroundTaskSubscriberIpcCode::COMMAND_ON_TRANSIENT_TASK_END);
        subscriberImpl->OnRemoteRequest(code4, datas, reply, option);

        uint32_t code15 = static_cast<uint32_t>(IBackgroundTaskSubscriberIpcCode::COMMAND_ON_TRANSIENT_TASK_ERR);
        subscriberImpl->OnRemoteRequest(code15, datas, reply, option);

        uint32_t code5 = static_cast<uint32_t>(IBackgroundTaskSubscriberIpcCode::COMMAND_ON_APP_TRANSIENT_TASK_START);
        subscriberImpl->OnRemoteRequest(code5, datas, reply, option);

        uint32_t code6 = static_cast<uint32_t>(IBackgroundTaskSubscriberIpcCode::COMMAND_ON_APP_TRANSIENT_TASK_END);
        subscriberImpl->OnRemoteRequest(code6, datas, reply, option);

        uint32_t code7 = static_cast<uint32_t>(IBackgroundTaskSubscriberIpcCode::COMMAND_ON_CONTINUOUS_TASK_START);
        subscriberImpl->OnRemoteRequest(code7, datas, reply, option);

        uint32_t code8 = static_cast<uint32_t>(IBackgroundTaskSubscriberIpcCode::COMMAND_ON_CONTINUOUS_TASK_UPDATE);
        subscriberImpl->OnRemoteRequest(code8, datas, reply, option);

        uint32_t code9 = static_cast<uint32_t>(IBackgroundTaskSubscriberIpcCode::COMMAND_ON_CONTINUOUS_TASK_STOP);
        subscriberImpl->OnRemoteRequest(code9, datas, reply, option);

        uint32_t code10 = static_cast<uint32_t>(IBackgroundTaskSubscriberIpcCode::COMMAND_ON_APP_CONTINUOUS_TASK_STOP);
        subscriberImpl->OnRemoteRequest(code10, datas, reply, option);

        uint32_t code11 =
            static_cast<uint32_t>(IBackgroundTaskSubscriberIpcCode::COMMAND_ON_APP_EFFICIENCY_RESOURCES_APPLY);
        subscriberImpl->OnRemoteRequest(code11, datas, reply, option);

        uint32_t code12 =
            static_cast<uint32_t>(IBackgroundTaskSubscriberIpcCode::COMMAND_ON_APP_EFFICIENCY_RESOURCES_RESET);
        subscriberImpl->OnRemoteRequest(code12, datas, reply, option);

        uint32_t code13 =
            static_cast<uint32_t>(IBackgroundTaskSubscriberIpcCode::COMMAND_ON_PROC_EFFICIENCY_RESOURCES_APPLY);
        subscriberImpl->OnRemoteRequest(code13, datas, reply, option);

        uint32_t code14 =
            static_cast<uint32_t>(IBackgroundTaskSubscriberIpcCode::COMMAND_ON_PROC_EFFICIENCY_RESOURCES_RESET);
        subscriberImpl->OnRemoteRequest(code14, datas, reply, option);
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
