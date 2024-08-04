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

#include "event_hub.h"

#include <map>

#include <common_event_subscribe_info.h>
#include <common_event_manager.h>
#include <common_event_support.h>

#include "transient_task_log.h"
#include "input_manager.h"
#include "want.h"

namespace OHOS {
namespace BackgroundTaskMgr {
static std::map<std::string, EventId> g_actionMap = {
    { EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON, EVENT_SCREEN_ON },
    { EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF, EVENT_SCREEN_OFF },
    { EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED, EVENT_SCREEN_UNLOCK },
    { EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_LOW, EVENT_BATTERY_LOW },
    { EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY, EVENT_BATTERY_OKAY },
};

std::shared_ptr<EventHub> EventHub::RegisterEvent(InputManager& inputManager)
{
    auto skill = std::make_shared<EventFwk::MatchingSkills>();
    for (auto &actionPair : g_actionMap) {
        skill->AddEvent(actionPair.first);
    }
    auto info = std::make_shared<EventFwk::CommonEventSubscribeInfo>(*skill);
    auto hub = std::make_shared<EventHub>(*info, inputManager);
    bool result = EventFwk::CommonEventManager::SubscribeCommonEvent(hub);
    if (!result) {
        BGTASK_LOGW("Failed to subscribe common event");
    }
    return hub;
}

void EventHub::OnReceiveEvent(const EventFwk::CommonEventData &event)
{
    std::vector<int32_t> intArgs;
    std::vector<std::string> stringArgs;
    intArgs.push_back(event.GetCode());
    stringArgs.push_back(event.GetData());

    const auto want = event.GetWant();
    const auto action = want.GetAction();
    BGTASK_LOGD("Receive action: %{public}s, eventId: %{public}d", action.c_str(), g_actionMap[action]);
    auto eventInfoEx = std::make_shared<EventInfo>();
    eventInfoEx->SetIntArgs(intArgs);
    eventInfoEx->SetStringArgs(stringArgs);
    eventInfoEx->SetEventId(g_actionMap[action]);
    inputManager_.SendEventInfo(eventInfoEx);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS