/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "bgtaskSubscriber_fuzzer.h"
#include "ibackground_task_subscriber.h"
#include "securec.h"

#include "background_task_subscriber_stub.h"
#include "background_task_subscriber.h"

namespace OHOS {
namespace BackgroundTaskMgr {
    constexpr int32_t U32_AT_SIZE = 4;
    constexpr int32_t MAX_CODE = 15;
    constexpr uint8_t TWENTYFOUR = 24;
    constexpr uint8_t SIXTEEN = 16;
    constexpr uint8_t EIGHT = 8;
    constexpr int32_t THREE = 3;
    constexpr int32_t TWO = 2;
    const std::u16string BACKGROUND_TASK_MGR_SUBSCRIBER_TOKEN = u"ohos.resourceschedule.IBackgroundTaskSubscriber";
class TestBackgroundTaskSubscriber : public BackgroundTaskSubscriber {
public:
    TestBackgroundTaskSubscriber() : BackgroundTaskSubscriber() {}
};

    uint32_t GetU32Data(const char* ptr)
    {
        return (ptr[0] << TWENTYFOUR) | (ptr[1] << SIXTEEN) | (ptr[TWO] << EIGHT) | (ptr[THREE]);
    }

    bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
    {
        uint32_t code = GetU32Data(data);
        MessageParcel datas;
        datas.WriteInterfaceToken(BACKGROUND_TASK_MGR_SUBSCRIBER_TOKEN);
        datas.WriteBuffer(data, size);
        datas.RewindRead(0);
        MessageParcel reply;
        MessageOption option;
        auto subscriber = TestBackgroundTaskSubscriber();
        auto subscriberImpl = std::make_shared<BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl>(subscriber);
        subscriberImpl->OnRemoteRequest(code % MAX_CODE, datas, reply, option);
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

    (void)memset_s(ch, size + 1, 0x00, size+1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::BackgroundTaskMgr::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}
