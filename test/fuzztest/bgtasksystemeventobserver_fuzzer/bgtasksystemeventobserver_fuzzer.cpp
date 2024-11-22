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

#include "bgtasksystemeventobserver_fuzzer.h"
#include "securec.h"

#include "background_task_mgr_service.h"
#include "common_event_data.h"
#include "event_info.h"
#include "event_handler.h"
#include "event_runner.h"
#include "system_event_observer.h"


namespace OHOS {
namespace BackgroundTaskMgr {
    constexpr int32_t U32_AT_SIZE = 4;
    constexpr uint8_t TWENTYFOUR = 24;
    constexpr uint8_t SIXTEEN = 16;
    constexpr uint8_t EIGHT = 8;
    constexpr int32_t THREE = 3;
    constexpr int32_t TWO = 2;

    uint32_t GetU32Data(const char* ptr)
    {
        return (ptr[0] << TWENTYFOUR) | (ptr[1] << SIXTEEN) | (ptr[TWO] << EIGHT) | (ptr[THREE]);
    }

    bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
    {
        GetU32Data(data);
        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);
        EventFwk::CommonEventSubscribeInfo commonEventSubscribeInfo(matchingSkills);
        auto systemEventListener = std::make_shared<SystemEventObserver>(commonEventSubscribeInfo);
        systemEventListener->Subscribe();
        systemEventListener->Unsubscribe();

        EventFwk::CommonEventData eventData = EventFwk::CommonEventData();
        systemEventListener->OnReceiveEvent(eventData);

        auto handler = std::make_shared<OHOS::AppExecFwk::EventHandler>(nullptr);
        systemEventListener->SetEventHandler(handler);
        systemEventListener->OnReceiveEventContinuousTask(eventData);
        auto bgContinuousTaskMgr = std::make_shared<BgContinuousTaskMgr>();
        systemEventListener->SetBgContinuousTaskMgr(bgContinuousTaskMgr);
        systemEventListener->OnReceiveEventContinuousTask(eventData);
        AAFwk::Want want = AAFwk::Want();
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
        eventData.SetWant(want);
        systemEventListener->OnReceiveEventContinuousTask(eventData);
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_ADDED);
        eventData.SetWant(want);
        systemEventListener->OnReceiveEventContinuousTask(eventData);
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED);
        eventData.SetWant(want);
        systemEventListener->OnReceiveEventContinuousTask(eventData);

        EventFwk::CommonEventData eventData2 = EventFwk::CommonEventData();
        AAFwk::Want want2 = AAFwk::Want();
        want2.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
        eventData2.SetWant(want2);
        systemEventListener->OnReceiveEventEfficiencyRes(eventData2);
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
