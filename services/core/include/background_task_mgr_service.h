/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "bg_efficiency_resources_mgr.h"

namespace OHOS {
namespace BackgroundTaskMgr {
enum class ServiceRunningState {
    STATE_NOT_START,
    STATE_RUNNING
};

enum ServiceReadyState {
    TRANSIENT_SERVICE_READY = 1 << 0,
    CONTINUOUS_SERVICE_READY = 1 << 1,
    EFFICIENCY_RESOURCES_SERVICE_READY = 1 << 2,
    ALL_READY = TRANSIENT_SERVICE_READY | CONTINUOUS_SERVICE_READY | EFFICIENCY_RESOURCES_SERVICE_READY
};

class BackgroundTaskMgrService final : public SystemAbility, public BackgroundTaskMgrStub,
    public std::enable_shared_from_this<BackgroundTaskMgrService> {
    DISALLOW_COPY_AND_MOVE(BackgroundTaskMgrService);
    DECLARE_SYSTEM_ABILITY(BackgroundTaskMgrService);
    DECLARE_DELAYED_SINGLETON(BackgroundTaskMgrService);
public:
    BackgroundTaskMgrService(const int32_t systemAbilityId, bool runOnCreate);
    void OnStart() final;
    void OnStop() final;
    void SetReady(uint32_t flag);
    int32_t OnExtension(const std::string& extension, MessageParcel& data, MessageParcel& reply) override;

    ErrCode RequestSuspendDelay(const std::string& reason,
        const sptr<IExpiredCallback>& callback, DelaySuspendInfo &delayInfo) override;
    ErrCode CancelSuspendDelay(int32_t requestId) override;
    ErrCode GetRemainingDelayTime(int32_t requestId, int32_t &delayTime) override;
    ErrCode GetAllTransientTasks(int32_t &remainingQuota,
        std::vector<std::shared_ptr<DelaySuspendInfo>> &list) override;
    // notificationId和continuousTaskId为idl化整改工具能力暂不具备而添加，在proxy中使用
    ErrCode StartBackgroundRunning(const ContinuousTaskParam &taskParam, int32_t& notificationId,
        int32_t& continuousTaskId) override;
    ErrCode UpdateBackgroundRunning(const ContinuousTaskParam &taskParam, int32_t& notificationId,
        int32_t& continuousTaskId) override;
    ErrCode RequestBackgroundRunningForInner(const ContinuousTaskParamForInner &taskParam) override;
    ErrCode RequestGetContinuousTasksByUidForInner(int32_t uid, std::vector<ContinuousTaskInfo> &list) override;
    ErrCode StopBackgroundRunning(const std::string &abilityName, const sptr<IRemoteObject> &abilityToken,
        int32_t abilityId, int32_t continuousTaskId) override;
    ErrCode GetAllContinuousTasks(std::vector<ContinuousTaskInfo> &list) override;
    ErrCode GetAllContinuousTasks(
        std::vector<std::shared_ptr<ContinuousTaskInfo>> &list, bool includeSuspended) override;
    ErrCode SubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber, uint32_t flag) override;
    ErrCode UnsubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber) override;
    ErrCode GetTransientTaskApps(std::vector<TransientTaskAppInfo> &list) override;
    ErrCode PauseTransientTaskTimeForInner(int32_t uid) override;
    ErrCode StartTransientTaskTimeForInner(int32_t uid) override;
    ErrCode GetContinuousTaskApps(std::vector<ContinuousTaskCallbackInfo> &list) override;
    ErrCode ApplyEfficiencyResources(const EfficiencyResourceInfo &resourceInfo) override;
    ErrCode ResetAllEfficiencyResources() override;
    ErrCode GetEfficiencyResourcesInfos(std::vector<ResourceCallbackInfo> &appList,
        std::vector<ResourceCallbackInfo> &procList) override;
    ErrCode GetAllEfficiencyResources(std::vector<EfficiencyResourceInfo> &resourceInfoList) override;
    ErrCode StopContinuousTask(int32_t uid, int32_t pid, uint32_t taskType, const std::string &key) override;
    ErrCode SuspendContinuousTask(int32_t uid, int32_t pid, int32_t reason, const std::string &key) override;
    ErrCode ActiveContinuousTask(int32_t uid, int32_t pid, const std::string &key) override;
    ErrCode AVSessionNotifyUpdateNotification(int32_t uid, int32_t pid, bool isPublish = false) override;
    ErrCode SetBgTaskConfig(const std::string &configData, int32_t sourceType) override;
    ErrCode SuspendContinuousAudioTask(int32_t uid) override;
    ErrCode IsModeSupported(const ContinuousTaskParam &taskParam) override;
    ErrCode SetSupportedTaskKeepingProcesses(const std::set<std::string> &processSet) override;
    ErrCode SetMaliciousAppConfig(const std::set<std::string> &maliciousAppSet) override;
    ErrCode RequestAuthFromUser(const ContinuousTaskParam &taskParam, const sptr<IExpiredCallback> &callback,
        int32_t &notificationId) override;
    ErrCode CheckSpecialScenarioAuth(uint32_t &authResult) override;
    ErrCode CheckTaskAuthResult(const std::string &bundleName, int32_t userId, int32_t appIndex) override;
    ErrCode EnableContinuousTaskRequest(int32_t uid, bool isEnable) override;
    ErrCode SetBackgroundTaskState(const BackgroundTaskStateInfo &taskParam) override;
    ErrCode GetBackgroundTaskState(const BackgroundTaskStateInfo &taskParam, uint32_t &authResult) override;
    ErrCode GetAllContinuousTasksBySystem(std::vector<std::shared_ptr<ContinuousTaskInfo>> &list) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;

    void ForceCancelSuspendDelay(int32_t requestId);
    void HandleRequestExpired(const int32_t requestId);
    void HandleExpiredCallbackDeath(const wptr<IRemoteObject>& remote);
    void HandleSubscriberDeath(const wptr<IRemoteObject>& remote);

private:
    void Init();
    void DumpUsage(std::string &result);
    bool AllowDump();
    bool CheckCallingToken();
    bool CheckHapCalling(bool &isHap);
    bool CheckCallingProcess();
    bool CheckAtomicService();
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

private:
    ServiceRunningState state_ {ServiceRunningState::STATE_NOT_START};
    std::shared_ptr<AppExecFwk::EventRunner> runner_ {nullptr};
    std::mutex readyMutex_;
    uint32_t dependsReady_ {0};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CORE_INCLUDE_BACKGROUND_TASK_MGR_SERVICE_H