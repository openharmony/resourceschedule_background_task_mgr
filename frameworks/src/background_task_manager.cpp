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
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->CancelSuspendDelay(requestId);
}

ErrCode BackgroundTaskManager::RequestSuspendDelay(const std::u16string &reason,
    const ExpiredCallback &callback, std::shared_ptr<DelaySuspendInfo> &delayInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    sptr<ExpiredCallback::ExpiredCallbackImpl> callbackSptr = callback.GetImpl();
    if (callbackSptr == nullptr) {
        BGTASK_LOGE("callbackSptr is nullptr");
        return ERR_CALLBACK_NULL_OR_TYPE_ERR;
    }
    return proxy_->RequestSuspendDelay(reason, callbackSptr, delayInfo);
}

ErrCode BackgroundTaskManager::GetRemainingDelayTime(int32_t requestId, int32_t &delayTime)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->GetRemainingDelayTime(requestId, delayTime);
}

ErrCode BackgroundTaskManager::RequestStartBackgroundRunning(ContinuousTaskParam &taskParam)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "BackgroundTaskManager::RequestStartBackgroundRunning");

    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    sptr<ContinuousTaskParam> taskParamPtr = new (std::nothrow) ContinuousTaskParam(taskParam);
    if (taskParamPtr == nullptr) {
        BGTASK_LOGE("Failed to create continuous task param");
        return ERR_BGTASK_NO_MEMORY;
    }
    BGTASK_LOGD("%{public}u = %{public}u, %{public}d = %{public}d, abilityId = %{public}d",
        static_cast<uint32_t>(taskParamPtr->bgModeIds_.size()),
        static_cast<uint32_t>(taskParam.bgModeIds_.size()),
        taskParamPtr->isBatchApi_, taskParam.isBatchApi_,
        taskParamPtr->abilityId_);
    ErrCode res = proxy_->StartBackgroundRunning(taskParamPtr);
    taskParam.notificationId_ = taskParamPtr->notificationId_;
    return res;
}

ErrCode BackgroundTaskManager::RequestUpdateBackgroundRunning(const ContinuousTaskParam &taskParam)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "BackgroundTaskManager::RequestUpdateBackgroundRunning");
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    sptr<ContinuousTaskParam> taskParamPtr = new (std::nothrow) ContinuousTaskParam(taskParam);
    if (taskParamPtr == nullptr) {
        BGTASK_LOGE("Failed to create continuous task param");
        return ERR_BGTASK_NO_MEMORY;
    }

    BGTASK_LOGD(" %{public}u = %{public}u, %{public}d = %{public}d, abilityId = %{public}d",
        static_cast<uint32_t>(taskParamPtr->bgModeIds_.size()),
        static_cast<uint32_t>(taskParam.bgModeIds_.size()), taskParamPtr->isBatchApi_, taskParam.isBatchApi_,
        taskParamPtr->abilityId_);
    return proxy_->UpdateBackgroundRunning(taskParamPtr);
}

ErrCode BackgroundTaskManager::RequestBackgroundRunningForInner(const ContinuousTaskParamForInner &taskParam)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    sptr<ContinuousTaskParamForInner> taskParamPtr = new (std::nothrow) ContinuousTaskParamForInner(taskParam);
    if (taskParamPtr == nullptr) {
        BGTASK_LOGE("Failed to create continuous task param");
        return ERR_BGTASK_NO_MEMORY;
    }

    return proxy_->RequestBackgroundRunningForInner(taskParamPtr);
}

ErrCode BackgroundTaskManager::RequestStopBackgroundRunning(const std::string &abilityName,
    const sptr<IRemoteObject> &abilityToken, int32_t abilityId)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "BackgroundTaskManager::RequestStopBackgroundRunning");
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->StopBackgroundRunning(abilityName, abilityToken, abilityId);
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
    return proxy_->SubscribeBackgroundTask(subscriberSptr);
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

    return proxy_->GetTransientTaskApps(list);
}

ErrCode BackgroundTaskManager::ApplyEfficiencyResources(const EfficiencyResourceInfo &resourceInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    sptr<EfficiencyResourceInfo> resourceInfoPtr = new (std::nothrow) EfficiencyResourceInfo(resourceInfo);
    if (resourceInfoPtr == nullptr) {
        BGTASK_LOGE("Failed to create efficiency resource info");
        return ERR_BGTASK_NO_MEMORY;
    }
    return proxy_->ApplyEfficiencyResources(resourceInfoPtr);
}

ErrCode BackgroundTaskManager::ResetAllEfficiencyResources()
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->ResetAllEfficiencyResources();
}

ErrCode BackgroundTaskManager::GetEfficiencyResourcesInfos(std::vector<std::shared_ptr<ResourceCallbackInfo>> &appList,
    std::vector<std::shared_ptr<ResourceCallbackInfo>> &procList)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->GetEfficiencyResourcesInfos(appList, procList);
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

    return proxy_->GetContinuousTaskApps(list);
}

ErrCode BackgroundTaskManager::StopContinuousTask(int32_t uid, int32_t pid, uint32_t taskType, const std::string &key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    GET_BACK_GROUND_TASK_MANAGER_PROXY_RETURN

    return proxy_->StopContinuousTask(uid, pid, taskType, key);
}

void BackgroundTaskManager::ResetBackgroundTaskManagerProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if ((proxy_ != nullptr) && (proxy_->AsObject() != nullptr)) {
        proxy_->AsObject()->RemoveDeathRecipient(recipient_);
    }
    proxy_ = nullptr;
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