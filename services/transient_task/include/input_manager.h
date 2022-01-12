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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_INPUT_MANAGER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_INPUT_MANAGER_H

#include <event_handler.h>
#include <refbase.h>

#include "event_hub.h"
#include "event_info.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class IEventListener {
public:
    IEventListener() = default;
    virtual ~IEventListener() = default;
    virtual void OnInputEvent(const EventInfo& eventInfo) = 0;
};

class InputManager : public AppExecFwk::EventHandler {
public:
    InputManager();
    ~InputManager() override;
    void SendEventInfo(const std::shared_ptr<EventInfo>& eventInfo);
    void RegisterEventListener(const std::shared_ptr<IEventListener>& listener);
    void UnRegisterEventListener(const std::shared_ptr<IEventListener>& listener);
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;

private:
    std::mutex listenerLock_;
    std::vector<std::shared_ptr<IEventListener>> listenerList_;
    std::shared_ptr<EventHub> eventHub_ {nullptr};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_INPUT_MANAGER_H