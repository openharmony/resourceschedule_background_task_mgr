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

#include "watchdog.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "background_task_mgr_service.h"
#include "hisysevent.h"
#include "transient_task_log.h"

using namespace std;

namespace OHOS {
namespace BackgroundTaskMgr {

Watchdog::Watchdog(const wptr<BackgroundTaskMgrService>& service, const std::shared_ptr<DecisionMaker>& decision,
    const std::shared_ptr<AppExecFwk::EventRunner>& runner) : service_(service), decision_(decision)
{
    if (runner != nullptr) {
        SetEventRunner(runner);
    }
}

bool Watchdog::AddWatchdog(int32_t requestId, const std::shared_ptr<KeyInfo>& info, int32_t interval)
{
    BGTASK_LOGD("AddWatchdog %{public}d", requestId);
    return SendEvent(requestId, info, interval);
}

void Watchdog::RemoveWatchdog(int32_t requestId)
{
    BGTASK_LOGD("RemoveWatchdog %{public}d", requestId);
    RemoveEvent(requestId);
}

void Watchdog::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    if (event == nullptr) {
        return;
    }
    int32_t requestId = event->GetInnerEventId();
    const shared_ptr<KeyInfo>& info = event->GetSharedObject<KeyInfo>();
    if (info == nullptr) {
        return;
    }
    auto bgTask = service_.promote();
    if (bgTask == nullptr) {
        BGTASK_LOGE("fail to get bgTask service.");
        return;
    }
    BGTASK_LOGI("handle watchdog, force cancel requestId: %{public}d", requestId);
    bgTask->ForceCancelSuspendDelay(requestId);
}

bool Watchdog::KillApplicationByUid(const std::string &bundleName, const int32_t uid, const int32_t pid)
{
    BGTASK_LOGW("kill running application, app name is %{public}s, uid is %{public}d, pid is %{public}d",
        bundleName.c_str(), uid, pid);
    if (appMgrClient_ == nullptr) {
        appMgrClient_ = std::make_unique<AppExecFwk::AppMgrClient>();
        if (appMgrClient_ == nullptr) {
            BGTASK_LOGE("failed to get appMgrClient");
            return false;
        }
    }
    int32_t ret = (int32_t)appMgrClient_->KillApplicationByUid(bundleName, uid);
    if (ret != ERR_OK) {
        BGTASK_LOGE("Fail to kill application by uid.");
        return false;
    }
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::FRAMEWORK,
        "PROCESS_KILL", HiviewDFX::HiSysEvent::EventType::FAULT,
        "PID", pid,
        "PROCESS_NAME", bundleName,
        "MSG", "TRANSIENT_TASK_TIMEOUT",
        "FOREGROUND", false);
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS