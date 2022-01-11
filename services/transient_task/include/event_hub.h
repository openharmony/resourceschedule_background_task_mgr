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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_SERVICES_TRANSIENT_TASK_INCLUDE_EVENT_HUB_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_SERVICES_TRANSIENT_TASK_INCLUDE_EVENT_HUB_H

#include <refbase.h>

#include "event_info.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class InputManager;

class EventHub : public OHOS::EventFwk::CommonEventSubscriber {
public:
    EventHub(const OHOS::EventFwk::CommonEventSubscribeInfo &info, InputManager& inputManager)
        : CommonEventSubscriber(info), inputManager_(inputManager) {}
    ~EventHub() override = default;

    static std::shared_ptr<EventHub> RegisterEvent(InputManager& inputManager);

    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &event) override;

private:
    InputManager& inputManager_;
};
} // namespace BackgroundTaskMgr
} // namespace OHOS
#endif