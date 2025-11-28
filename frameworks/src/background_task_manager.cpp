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

#include "background_task_manager.h"

#include "hitrace_meter.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "bgtaskmgr_inner_errors.h"
#include "bgtaskmgr_log_wrapper.h"
#include "delay_suspend_info.h"

namespace OHOS {
namespace BackgroundTaskMgr {
BackgroundTaskManager::BackgroundTaskManager() {}

BackgroundTaskManager::~BackgroundTaskManager() {}

#define GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN              \
    if (!GetBackgroundTaskManagerProxy()) {                    \
        BGTASK_LOGE("GetBackgroundTaskManager Proxy failed."); \
        return ERR_BGTASK_SERVICE_NOT_CONNECTED;               \
    }

ErrCode BackgroundTaskManager::CancelSuspendDelay(int32_t requestId)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::TransientTask::Mgr::CancelSuspendDelay");

    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->CancelSuspendDelay(requestId);
}

ErrCode BackgroundTaskManager::RequestSuspendDelay(const std::u16string &reasonU16,
    const ExpiredCallback &callback, std::shared_ptr<DelaySuspendInfo> &delayInfo)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::TransientTask::Mgr::RequestSuspendDelay");

    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    sptr<ExpiredCallback::ExpiredCallbackImpl> callbackSptr = callback.GetImpl();
    if (callbackSptr == nullptr) {
        BGTASK_LOGE("callbackSptr is nullptr");
        return ERR_CALLBACK_NULL_OR_TYPE_ERR;
    }
    delayInfo = std::make_shared<DelaySuspendInfo>();
    if (!delayInfo) {
        BGTASK_LOGE("delayInfo is nullptr");
        return ERR_CALLBACK_NULL_OR_TYPE_ERR;
    }
    std::string reason = Str16ToStr8(reasonU16);
    return proxy_->RequestSuspendDelay(reason, callbackSptr, *delayInfo.get());
}

ErrCode BackgroundTaskManager::GetRemainingDelayTime(int32_t requestId, int32_t &delayTime)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::TransientTask::Mgr::GetRemainingDelayTime");

    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->GetRemainingDelayTime(requestId, delayTime);
}

ErrCode BackgroundTaskManager::GetAllTransientTasks(int32_t &remainingQuota,
    std::vector<std::shared_ptr<DelaySuspendInfo>> &list)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Mgr::GetAllTransientTasks");

    std::lock_guard<std::mutex> lock(mutex_);
    if (!GetBackgroundTaskManagerProxy()) {
        BGTASK_LOGE("GetBackgroundTaskManager Proxy failed.");
        return ERR_BGTASK_TRANSIENT_SERVICE_NOT_CONNECTED;
    }

    return proxy_->GetAllTransientTasks(remainingQuota, list);
}

ErrCode BackgroundTaskManager::RequestStartBackgroundRunning(ContinuousTaskParam &taskParam)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Mgr::RequestStartBackgroundRunning");

    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    sptr<ContinuousTaskParam> taskParamPtr = new (std::nothrow) ContinuousTaskParam(taskParam);
    if (taskParamPtr == nullptr || taskParamPtr.GetRefPtr() == nullptr) {
        BGTASK_LOGE("Failed to create continuous task param");
        return ERR_BGTASK_NO_MEMORY;
    }
    BGTASK_LOGD("%{public}u = %{public}u, %{public}d = %{public}d, abilityId = %{public}d",
        static_cast<uint32_t>(taskParamPtr->bgModeIds_.size()),
        static_cast<uint32_t>(taskParam.bgModeIds_.size()),
        taskParamPtr->isBatchApi_, taskParam.isBatchApi_,
        taskParamPtr->abilityId_);
    int32_t notificationId = taskParamPtr->notificationId_;
    int32_t continuousTaskId = taskParamPtr->continuousTaskId_;
    ErrCode res = proxy_->StartBackgroundRunning(*taskParamPtr.GetRefPtr(), notificationId, continuousTaskId);
    taskParam.notificationId_ = notificationId;
    taskParam.continuousTaskId_ = continuousTaskId;
    return res;
}

ErrCode BackgroundTaskManager::RequestUpdateBackgroundRunning(ContinuousTaskParam &taskParam)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Mgr::RequestUpdateBackgroundRunning");

    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    sptr<ContinuousTaskParam> taskParamPtr = new (std::nothrow) ContinuousTaskParam(taskParam);
    if (taskParamPtr == nullptr || taskParamPtr.GetRefPtr() == nullptr) {
        BGTASK_LOGE("Failed to create continuous task param");
        return ERR_BGTASK_NO_MEMORY;
    }

    BGTASK_LOGD(" %{public}u = %{public}u, %{public}d = %{public}d, abilityId = %{public}d",
        static_cast<uint32_t>(taskParamPtr->bgModeIds_.size()),
        static_cast<uint32_t>(taskParam.bgModeIds_.size()), taskParamPtr->isBatchApi_, taskParam.isBatchApi_,
        taskParamPtr->abilityId_);
    int32_t notificationId = taskParamPtr->notificationId_;
    int32_t continuousTaskId = taskParamPtr->continuousTaskId_;
    ErrCode ret = proxy_->UpdateBackgroundRunning(*taskParamPtr.GetRefPtr(), notificationId, continuousTaskId);
    taskParam.notificationId_ = notificationId;
    taskParam.continuousTaskId_ = continuousTaskId;
    return ret;
}

ErrCode BackgroundTaskManager::RequestBackgroundRunningForInner(const ContinuousTaskParamForInner &taskParam)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Mgr::RequestBackgroundRunningForInner");

    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    sptr<ContinuousTaskParamForInner> taskParamPtr = new (std::nothrow) ContinuousTaskParamForInner(taskParam);
    if (taskParamPtr == nullptr || taskParamPtr.GetRefPtr() == nullptr) {
        BGTASK_LOGE("Failed to create continuous task param");
        return ERR_BGTASK_NO_MEMORY;
    }

    return proxy_->RequestBackgroundRunningForInner(*taskParamPtr.GetRefPtr());
}

ErrCode BackgroundTaskManager::RequestGetContinuousTasksByUidForInner(int32_t uid,
    std::vector<std::shared_ptr<ContinuousTaskInfo>> &list)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Mgr::RequestGetContinuousTasksByUidForInner");

    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    if (uid < 0) {
        BGTASK_LOGE("param uid: %{public}d is invaild.", uid);
        return ERR_BGTASK_INVALID_UID;
    }

    std::vector<ContinuousTaskInfo> tasksList;
    ErrCode result = proxy_->RequestGetContinuousTasksByUidForInner(uid, tasksList);
    if (result == ERR_OK) {
        list.clear();
        for (const auto& item : tasksList) {
            list.push_back(std::make_shared<ContinuousTaskInfo>(item));
        }
    }
    return result;
}

ErrCode BackgroundTaskManager::RequestStopBackgroundRunning(const std::string &abilityName,
    const sptr<IRemoteObject> &abilityToken, int32_t abilityId, int32_t continuousTaskId)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Mgr::RequestStopBackgroundRunning");

    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN
    BGTASK_LOGI("requestStopBackgroundRunning abilityName: %{public}s, abilityId: %{public}d, "\
        "continuousTaskId: %{public}d.", abilityName.c_str(), abilityId, continuousTaskId);
    return proxy_->StopBackgroundRunning(abilityName, abilityToken, abilityId, continuousTaskId);
}

ErrCode BackgroundTaskManager::RequestGetAllContinuousTasks(std::vector<std::shared_ptr<ContinuousTaskInfo>> &list)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Mgr::RequestGetAllContinuousTasks");

    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    std::vector<ContinuousTaskInfo> tasksList;
    ErrCode result = proxy_->GetAllContinuousTasks(tasksList);
    if (result == ERR_OK) {
        list.clear();
        for (const auto& item : tasksList) {
            list.push_back(std::make_shared<ContinuousTaskInfo>(item));
        }
    }
    return result;
}

ErrCode BackgroundTaskManager::RequestGetAllContinuousTasks(
    std::vector<std::shared_ptr<ContinuousTaskInfo>> &list, bool includeSuspended)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Mgr::GetAllContinuousTasksIncludeSuspended");

    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->GetAllContinuousTasks(list, includeSuspended);
}

__attribute__((no_sanitize("cfi"))) ErrCode BackgroundTaskManager::SubscribeBackgroundTask(
    const BackgroundTaskSubscriber &subscriber)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    sptr<BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl> subscriberSptr = subscriber.GetImpl();
    if (subscriberSptr == nullptr) {
        BGTASK_LOGE("subscriberSptr is nullptr");
        return ERR_BGTASK_INVALID_PARAM;
    }
    return proxy_->SubscribeBackgroundTask(subscriberSptr, subscriber.flag_);
}

ErrCode BackgroundTaskManager::UnsubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    sptr<BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl> subscriberSptr = subscriber.GetImpl();
    if (subscriberSptr == nullptr) {
        BGTASK_LOGE("subscriberSptr is nullptr");
        return ERR_BGTASK_INVALID_PARAM;
    }
    return proxy_->UnsubscribeBackgroundTask(subscriberSptr);
}

ErrCode BackgroundTaskManager::GetTransientTaskApps(std::vector<std::shared_ptr<TransientTaskAppInfo>> &list)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    std::vector<TransientTaskAppInfo> TaskAppsList;
    ErrCode result = proxy_->GetTransientTaskApps(TaskAppsList);
    if (result == ERR_OK) {
        list.clear();
        for (const auto& item : TaskAppsList) {
            list.push_back(std::make_shared<TransientTaskAppInfo>(item));
        }
    }

    return result;
}

ErrCode BackgroundTaskManager::PauseTransientTaskTimeForInner(int32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->PauseTransientTaskTimeForInner(uid);
}

ErrCode BackgroundTaskManager::StartTransientTaskTimeForInner(int32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->StartTransientTaskTimeForInner(uid);
}

ErrCode BackgroundTaskManager::ApplyEfficiencyResources(const EfficiencyResourceInfo &resourceInfo)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::EfficiencyResource::Mgr::ApplyEfficiencyResources");

    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    sptr<EfficiencyResourceInfo> resourceInfoPtr = new (std::nothrow) EfficiencyResourceInfo(resourceInfo);
    if (resourceInfoPtr == nullptr || resourceInfoPtr.GetRefPtr() == nullptr) {
        BGTASK_LOGE("Failed to create efficiency resource info");
        return ERR_BGTASK_NO_MEMORY;
    }
    return proxy_->ApplyEfficiencyResources(*resourceInfoPtr.GetRefPtr());
}

ErrCode BackgroundTaskManager::ResetAllEfficiencyResources()
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::EfficiencyResource::Mgr::ResetAllEfficiencyResources");

    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->ResetAllEfficiencyResources();
}

ErrCode BackgroundTaskManager::GetAllEfficiencyResources(
    std::vector<std::shared_ptr<EfficiencyResourceInfo>> &resourceInfoList)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::EfficiencyResource::Mgr::GetAllEfficiencyResources");

    std::lock_guard<std::mutex> lock(mutex_);
    if (!GetBackgroundTaskManagerProxy()) {
        BGTASK_LOGE("GetBackgroundTaskManager Proxy failed.");
        return ERR_BGTASK_RESOURCES_SERVICE_NOT_CONNECTED;
    }

    std::vector<EfficiencyResourceInfo> list;
    ErrCode result = proxy_->GetAllEfficiencyResources(list);
    if (result == ERR_OK) {
        resourceInfoList.clear();
        for (const auto& item : list) {
            resourceInfoList.push_back(std::make_shared<EfficiencyResourceInfo>(item));
        }
    }
    return result;
}

ErrCode BackgroundTaskManager::GetEfficiencyResourcesInfos(std::vector<std::shared_ptr<ResourceCallbackInfo>> &appList,
    std::vector<std::shared_ptr<ResourceCallbackInfo>> &procList)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    std::vector<ResourceCallbackInfo> resourceAppList;
    std::vector<ResourceCallbackInfo> resourceProcList;
    ErrCode result = proxy_->GetEfficiencyResourcesInfos(resourceAppList, resourceProcList);
    if (result == ERR_OK) {
        appList.clear();
        procList.clear();
        for (const auto& item : resourceAppList) {
            appList.push_back(std::make_shared<ResourceCallbackInfo>(item));
        }

        for (const auto& item : resourceProcList) {
            procList.push_back(std::make_shared<ResourceCallbackInfo>(item));
        }
    }
    return result;
}

bool BackgroundTaskManager::GetBackgroundTaskManagerProxy()
{
    if (proxy_ != nullptr) {
        return true;
    }
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        BGTASK_LOGE("GetBackgroundTaskManagerProxy GetSystemAbilityManager failed.");
        return false;
    }

    sptr<IRemoteObject> remoteObject =
        systemAbilityManager->GetSystemAbility(BACKGROUND_TASK_MANAGER_SERVICE_ID);
    if (remoteObject == nullptr) {
        BGTASK_LOGE("GetBackgroundTaskManagerProxy GetSystemAbility failed.");
        return false;
    }

    proxy_ = iface_cast<BackgroundTaskMgr::IBackgroundTaskMgr>(remoteObject);
    if ((proxy_ == nullptr) || (proxy_->AsObject() == nullptr)) {
        BGTASK_LOGE("GetBackgroundTaskManagerProxy iface_cast remoteObject failed.");
        return false;
    }

    recipient_ = new (std::nothrow) BgTaskMgrDeathRecipient(*this);
    if (recipient_ == nullptr) {
        return false;
    }
    proxy_->AsObject()->AddDeathRecipient(recipient_);
    return true;
}

ErrCode BackgroundTaskManager::GetContinuousTaskApps(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    std::vector<ContinuousTaskCallbackInfo> TaskAppsList;
    ErrCode result = proxy_->GetContinuousTaskApps(TaskAppsList);
    if (result == ERR_OK) {
        list.clear();
        for (const auto& item : TaskAppsList) {
            list.push_back(std::make_shared<ContinuousTaskCallbackInfo>(item));
        }
    }

    return result;
}

ErrCode BackgroundTaskManager::StopContinuousTask(int32_t uid, int32_t pid, uint32_t taskType, const std::string &key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->StopContinuousTask(uid, pid, taskType, key);
}

ErrCode BackgroundTaskManager::SuspendContinuousTask(int32_t uid, int32_t pid, int32_t reason, const std::string &key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->SuspendContinuousTask(uid, pid, reason, key);
}

ErrCode BackgroundTaskManager::ActiveContinuousTask(int32_t uid, int32_t pid, const std::string &key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->ActiveContinuousTask(uid, pid, key);
}

ErrCode BackgroundTaskManager::AVSessionNotifyUpdateNotification(int32_t uid, int32_t pid, bool isPublish)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->AVSessionNotifyUpdateNotification(uid, pid, isPublish);
}

void BackgroundTaskManager::ResetBackgroundTaskManagerProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if ((proxy_ != nullptr) && (proxy_->AsObject() != nullptr)) {
        proxy_->AsObject()->RemoveDeathRecipient(recipient_);
    }
    proxy_ = nullptr;
}

ErrCode BackgroundTaskManager::SetBgTaskConfig(const std::string &configData, int32_t sourceType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->SetBgTaskConfig(configData, sourceType);
}

ErrCode BackgroundTaskManager::SuspendContinuousAudioTask(int32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->SuspendContinuousAudioTask(uid);
}

ErrCode BackgroundTaskManager::IsModeSupported(ContinuousTaskParam &taskParam)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->IsModeSupported(taskParam);
}

ErrCode BackgroundTaskManager::SetSupportedTaskKeepingProcesses(const std::set<std::string> &processSet)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->SetSupportedTaskKeepingProcesses(processSet);
}

ErrCode BackgroundTaskManager::SetMaliciousAppConfig(const std::set<std::string> &maliciousAppSet)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->SetMaliciousAppConfig(maliciousAppSet);
}

ErrCode BackgroundTaskManager::RequestAuthFromUser(const ContinuousTaskParam &taskParam,
    const ExpiredCallback &callback, int32_t &notificationId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    sptr<ExpiredCallback::ExpiredCallbackImpl> callbackSptr = callback.GetImpl();
    if (callbackSptr == nullptr) {
        BGTASK_LOGE("callbackSptr is nullptr");
        return ERR_BGTASK_CONTINUOUS_CALLBACK_NULL_OR_TYPE_ERR;
    }

    return proxy_->RequestAuthFromUser(taskParam, callbackSptr, notificationId);
}

ErrCode BackgroundTaskManager::CheckSpecialScenarioAuth(uint32_t &authResult)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->CheckSpecialScenarioAuth(authResult);
}

ErrCode BackgroundTaskManager::CheckTaskAuthResult(const std::string &bundleName, int32_t userId, int32_t appIndex)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->CheckTaskAuthResult(bundleName, userId, appIndex);
}

ErrCode BackgroundTaskManager::EnableContinuousTaskRequest(int32_t uid, bool isEnable)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->EnableContinuousTaskRequest(uid, isEnable);
}

ErrCode BackgroundTaskManager::SetBackgroundTaskState(std::shared_ptr<BackgroundTaskStateInfo> taskParam)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    if (!taskParam) {
        return ERR_BGTASK_CHECK_TASK_PARAM;
    }
    return proxy_->SetBackgroundTaskState(*taskParam.get());
}

ErrCode BackgroundTaskManager::GetBackgroundTaskState(std::shared_ptr<BackgroundTaskStateInfo> taskParam)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    if (!taskParam) {
        return ERR_BGTASK_CHECK_TASK_PARAM;
    }
    uint32_t authResult = 0;
    ErrCode ret = proxy_->GetBackgroundTaskState(*taskParam.get(), authResult);
    if (ret != ERR_OK) {
        return ret;
    }
    taskParam->SetUserAuthResult(authResult);
    return ERR_OK;
}

ErrCode BackgroundTaskManager::GetAllContinuousTasksBySystem(std::vector<std::shared_ptr<ContinuousTaskInfo>> &list)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Mgr::GetAllContinuousTasksBySystem");

    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    ErrCode result = proxy_->GetAllContinuousTasksBySystem(list);
    return result;
}

BackgroundTaskManager::BgTaskMgrDeathRecipient::BgTaskMgrDeathRecipient(BackgroundTaskManager &backgroundTaskManager)
    : backgroundTaskManager_(backgroundTaskManager) {}

BackgroundTaskManager::BgTaskMgrDeathRecipient::~BgTaskMgrDeathRecipient() {}

void BackgroundTaskManager::BgTaskMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    backgroundTaskManager_.ResetBackgroundTaskManagerProxy();
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS