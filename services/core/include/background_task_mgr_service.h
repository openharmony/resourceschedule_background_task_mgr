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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CORE_INCLUDE_BACKGROUND_TASK_MGR_SERVICE_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CORE_INCLUDE_BACKGROUND_TASK_MGR_SERVICE_H

#include <ctime>
#include <list>
#include <memory>
#include <mutex>

#include "event_handler.h"
#include "event_runner.h"
#include "refbase.h"
#include "singleton.h"
#include "system_ability.h"
#include "system_ability_definition.h"

#include "background_task_mgr_stub.h"
#include "bg_continuous_task_mgr.h"
#include "bg_transient_task_mgr.h"
#include "bgtaskmgr_inner_errors.h"

namespace OHOS {
namespace BackgroundTaskMgr {
enum class ServiceRunningState {
    STATE_NOT_START,
    STATE_RUNNING
};

class BackgroundTaskMgrService final : public SystemAbility, public BackgroundTaskMgrStub,
    public std::enable_shared_from_this<BackgroundTaskMgrService> {
    DISALLOW_COPY_AND_MOVE(BackgroundTaskMgrService);
    DECLARE_SYSTEM_ABILITY(BackgroundTaskMgrService);
    DECLARE_DELAYED_SINGLETON(BackgroundTaskMgrService);
public:
    BackgroundTaskMgrService(const int32_t systemAbilityId, bool runOnCreate);
    void OnStart() override final;
    void OnStop() override final;

    ErrCode RequestSuspendDelay(const std::u16string& reason,
        const sptr<IExpiredCallback>& callback, std::shared_ptr<DelaySuspendInfo> &delayInfo) override;
    ErrCode CancelSuspendDelay(int32_t requestId) override;
    ErrCode GetRemainingDelayTime(int32_t requestId, int32_t &delayTime) override;
    ErrCode StartBackgroundRunning(const sptr<ContinuousTaskParam> &taskParam) override;
    ErrCode StopBackgroundRunning(const std::string &abilityName, const sptr<IRemoteObject> &abilityToken) override;
    ErrCode SubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber) override;
    ErrCode UnsubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber) override;
    ErrCode ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo) override;

    void ForceCancelSuspendDelay(int32_t requestId);
    void HandleRequestExpired(const int32_t requestId);
    void HandleExpiredCallbackDeath(const wptr<IRemoteObject>& remote);
    void HandleSubscriberDeath(const wptr<IRemoteObject>& remote);

private:
    void Init();

private:
    ServiceRunningState state_ {ServiceRunningState::STATE_NOT_START};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CORE_INCLUDE_BACKGROUND_TASK_MGR_SERVICE_H