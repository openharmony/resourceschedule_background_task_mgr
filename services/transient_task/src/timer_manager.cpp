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

#include "timer_manager.h"

#include "background_task_mgr_service.h"
#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
const std::string BACKGROUND_TASK_TIMER = "BgTaskTimer";

TimerManager::TimerManager(const wptr<BackgroundTaskMgrService>& service) : service_(service)
{
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create(BACKGROUND_TASK_TIMER);
    if (eventRunner.get() != nullptr) {
        SetEventRunner(eventRunner);
    }
}

bool TimerManager::AddTimer(int32_t requestId, int32_t interval)
{
    BGTASK_LOGI("Add request id: %{public}d", requestId);
    return SendEvent(requestId, 0, interval);
}

void TimerManager::RemoveTimer(int32_t requestId)
{
    BGTASK_LOGI("Remove request id: %{public}d", requestId);
    return RemoveEvent(requestId);
}

void TimerManager::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    if (event == nullptr) {
        return;
    }
    int32_t requestId = event->GetInnerEventId();
    auto bgTask = service_.promote();
    if (bgTask == nullptr) {
        return;
    }
    bgTask->HandleRequestExpired(requestId);
}
} // namespace BackgroundTaskMgr
} // namespace OHOS