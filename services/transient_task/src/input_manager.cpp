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

#include "input_manager.h"

#include <event_runner.h>

#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {

InputManager::InputManager(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
{
    if (runner != nullptr) {
        SetEventRunner(runner);
    }
}

InputManager::~InputManager() {}

void InputManager::RegisterEventHub()
{
    eventHub_ = EventHub::RegisterEvent(*this);
    if (eventHub_ == nullptr) {
        BGTASK_LOGE("Failed to register event");
    }
}

void InputManager::RegisterEventListener(const std::shared_ptr<IEventListener>& listener)
{
    if (listener == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(listenerLock_);
    listenerList_.push_back(listener);
}

void InputManager::SendEventInfo(const std::shared_ptr<EventInfo>& eventInfo)
{
    if (eventInfo == nullptr) {
        return;
    }

    BGTASK_LOGD("Send event info: %{public}s", eventInfo->ToString().c_str());
    SendImmediateEvent(eventInfo->GetEventId(), eventInfo);
}

void InputManager::UnRegisterEventListener(const std::shared_ptr<IEventListener>& listener)
{
    if (listener == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(listenerLock_);
    auto findIt = find(listenerList_.begin(), listenerList_.end(), listener);
    if (findIt == listenerList_.end()) {
        BGTASK_LOGE("Listener not found");
        return;
    }
    listenerList_.erase(findIt);
}

void InputManager::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    if (event == nullptr) {
        return;
    }
    int32_t eventId = event->GetInnerEventId();
    BGTASK_LOGD("Process event, eventId: %{public}d", eventId);
    const std::shared_ptr<EventInfo>& eventInfo = event->GetSharedObject<EventInfo>();
    if (eventInfo == nullptr) {
        BGTASK_LOGE("Invailed event info, eventId: %{public}d", eventId);
        return;
    }
    std::lock_guard<std::mutex> lock(listenerLock_);
    for (const auto& listener : listenerList_) {
        listener->OnInputEvent(*eventInfo);
    }
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS