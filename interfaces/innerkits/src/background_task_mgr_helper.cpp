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

#include "background_task_mgr_helper.h"

#include "singleton.h"

#include "bgtaskmgr_log_wrapper.h"
#include "background_task_manager.h"
#include "ibackground_task_mgr.h"

namespace OHOS {
namespace BackgroundTaskMgr {
ErrCode BackgroundTaskMgrHelper::RequestStartBackgroundRunning(ContinuousTaskParam &taskParam)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestStartBackgroundRunning(taskParam);
}

ErrCode BackgroundTaskMgrHelper::RequestUpdateBackgroundRunning(ContinuousTaskParam &taskParam)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestUpdateBackgroundRunning(taskParam);
}

ErrCode BackgroundTaskMgrHelper::RequestStopBackgroundRunning(const std::string &abilityName,
    const sptr<IRemoteObject> &abilityToken, int32_t abilityId, int32_t continuousTaskId)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        RequestStopBackgroundRunning(abilityName, abilityToken, abilityId, continuousTaskId);
}

ErrCode BackgroundTaskMgrHelper::RequestGetAllContinuousTasks(std::vector<std::shared_ptr<ContinuousTaskInfo>> &list)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestGetAllContinuousTasks(list);
}

ErrCode BackgroundTaskMgrHelper::RequestGetAllContinuousTasks(
    std::vector<std::shared_ptr<ContinuousTaskInfo>> &list, bool includeSuspended)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestGetAllContinuousTasks(list, includeSuspended);
}

ErrCode BackgroundTaskMgrHelper::RequestBackgroundRunningForInner(const ContinuousTaskParamForInner &taskParam)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestBackgroundRunningForInner(taskParam);
}

ErrCode BackgroundTaskMgrHelper::RequestGetContinuousTasksByUidForInner(int32_t uid,
    std::vector<std::shared_ptr<ContinuousTaskInfo>> &list)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestGetContinuousTasksByUidForInner(uid, list);
}

__attribute__((no_sanitize("cfi"))) ErrCode BackgroundTaskMgrHelper::SubscribeBackgroundTask(
    const BackgroundTaskSubscriber &subscriber)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->SubscribeBackgroundTask(subscriber);
}

ErrCode BackgroundTaskMgrHelper::UnsubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->UnsubscribeBackgroundTask(subscriber);
}

ErrCode BackgroundTaskMgrHelper::GetTransientTaskApps(std::vector<std::shared_ptr<TransientTaskAppInfo>> &list)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetTransientTaskApps(list);
}

ErrCode BackgroundTaskMgrHelper::PauseTransientTaskTimeForInner(int32_t uid)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->PauseTransientTaskTimeForInner(uid);
}

ErrCode BackgroundTaskMgrHelper::StartTransientTaskTimeForInner(int32_t uid)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->StartTransientTaskTimeForInner(uid);
}

ErrCode BackgroundTaskMgrHelper::GetContinuousTaskApps(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetContinuousTaskApps(list);
}

ErrCode BackgroundTaskMgrHelper::GetEfficiencyResourcesInfos(std::vector<std::shared_ptr<
    ResourceCallbackInfo>> &appList, std::vector<std::shared_ptr<ResourceCallbackInfo>> &procList)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetEfficiencyResourcesInfos(appList, procList);
}

ErrCode BackgroundTaskMgrHelper::ApplyEfficiencyResources(const EfficiencyResourceInfo &resourceInfo)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->ApplyEfficiencyResources(resourceInfo);
}

ErrCode BackgroundTaskMgrHelper::ResetAllEfficiencyResources()
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->ResetAllEfficiencyResources();
}

ErrCode BackgroundTaskMgrHelper::StopContinuousTask(int32_t uid, int32_t pid, uint32_t taskType, const std::string &key)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->StopContinuousTask(uid, pid, taskType, key);
}
ErrCode BackgroundTaskMgrHelper::SetBgTaskConfig(const std::string &configData, int32_t sourceType)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->SetBgTaskConfig(configData, sourceType);
}

ErrCode BackgroundTaskMgrHelper::SuspendContinuousTask(
    int32_t uid, int32_t pid, int32_t reason, const std::string &key)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->SuspendContinuousTask(uid, pid, reason, key);
}

ErrCode BackgroundTaskMgrHelper::ActiveContinuousTask(int32_t uid, int32_t pid, const std::string &key)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->ActiveContinuousTask(uid, pid, key);
}

ErrCode BackgroundTaskMgrHelper::AVSessionNotifyUpdateNotification(int32_t uid, int32_t pid, bool isPublish)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        AVSessionNotifyUpdateNotification(uid, pid, isPublish);
}

ErrCode BackgroundTaskMgrHelper::SuspendContinuousAudioTask(int32_t uid)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->SuspendContinuousAudioTask(uid);
}

ErrCode BackgroundTaskMgrHelper::IsModeSupported(ContinuousTaskParam &taskParam)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->IsModeSupported(taskParam);
}

ErrCode BackgroundTaskMgrHelper::SetSupportedTaskKeepingProcesses(const std::set<std::string> &processSet)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->SetSupportedTaskKeepingProcesses(processSet);
}

ErrCode BackgroundTaskMgrHelper::SetMaliciousAppConfig(const std::set<std::string> &maliciousAppSet)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->SetMaliciousAppConfig(maliciousAppSet);
}

ErrCode BackgroundTaskMgrHelper::CheckSpecialScenarioAuth(uint32_t &authResult)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->CheckSpecialScenarioAuth(authResult);
}

ErrCode BackgroundTaskMgrHelper::CheckTaskAuthResult(const std::string &bundleName, int32_t userId, int32_t appIndex)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->CheckTaskAuthResult(bundleName, userId, appIndex);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS