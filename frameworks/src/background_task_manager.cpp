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

ErrCode BackgroundTaskManager::CancelSuspendDelay(int32_t requestId)
{
    if (!GetBackgroundTaskManagerProxy()) {
        BGTASK_LOGE("GetBackgroundTaskManagerProxy failed.");
        return ERR_BGTASK_SERVICE_NOT_CONNECTED;
    }
    return backgroundTaskMgrProxy_->CancelSuspendDelay(requestId);
}

ErrCode BackgroundTaskManager::RequestSuspendDelay(const std::u16string &reason,
    const ExpiredCallback &callback, std::shared_ptr<DelaySuspendInfo> &delayInfo)
{
    if (!GetBackgroundTaskManagerProxy()) {
        BGTASK_LOGE("GetBackgroundTaskManagerProxy failed.");
        return ERR_BGTASK_SERVICE_NOT_CONNECTED;
    }

    sptr<ExpiredCallback::ExpiredCallbackImpl> callbackSptr = callback.GetImpl();
    if (callbackSptr == nullptr) {
        BGTASK_LOGE("callbackSptr is nullptr");
        return ERR_BGTASK_INVALID_PARAM;
    }
    return backgroundTaskMgrProxy_->RequestSuspendDelay(reason, callbackSptr, delayInfo);
}

ErrCode BackgroundTaskManager::GetRemainingDelayTime(int32_t requestId, int32_t &delayTime)
{
    if (!GetBackgroundTaskManagerProxy()) {
        BGTASK_LOGE("GetBackgroundTaskManagerProxy failed.");
        return ERR_BGTASK_SERVICE_NOT_CONNECTED;
    }
    return backgroundTaskMgrProxy_->GetRemainingDelayTime(requestId, delayTime);
}

ErrCode BackgroundTaskManager::RequestStartBackgroundRunning(const ContinuousTaskParam &taskParam)
{
    StartTrace(HITRACE_TAG_OHOS, "BackgroundTaskManager::RequestStartBackgroundRunning");
    if (!GetBackgroundTaskManagerProxy()) {
        BGTASK_LOGE("GetBackgroundTaskManagerProxy failed.");
        FinishTrace(HITRACE_TAG_OHOS);
        return ERR_BGTASK_SERVICE_NOT_CONNECTED;
    }

    sptr<ContinuousTaskParam> taskParamPtr = new (std::nothrow) ContinuousTaskParam(taskParam);
    if (taskParamPtr == nullptr) {
        BGTASK_LOGE("Failed to create continuous task param");
        return ERR_BGTASK_NO_MEMORY;
    }
    ErrCode ret = backgroundTaskMgrProxy_->StartBackgroundRunning(taskParamPtr);
    FinishTrace(HITRACE_TAG_OHOS);
    return ret;
}

ErrCode BackgroundTaskManager::RequestStopBackgroundRunning(const std::string &abilityName,
    const sptr<IRemoteObject> &abilityToken)
{
    StartTrace(HITRACE_TAG_OHOS, "BackgroundTaskManager::RequestStopBackgroundRunning");
    if (!GetBackgroundTaskManagerProxy()) {
        BGTASK_LOGE("GetBackgroundTaskManagerProxy failed.");
        FinishTrace(HITRACE_TAG_OHOS);
        return ERR_BGTASK_SERVICE_NOT_CONNECTED;
    }

    ErrCode ret = backgroundTaskMgrProxy_->StopBackgroundRunning(abilityName, abilityToken);
    FinishTrace(HITRACE_TAG_OHOS);
    return ret;
}

ErrCode BackgroundTaskManager::SubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber)
{
    if (!GetBackgroundTaskManagerProxy()) {
        BGTASK_LOGE("GetBackgroundTaskManagerProxy failed.");
        return ERR_BGTASK_SERVICE_NOT_CONNECTED;
    }
    sptr<BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl> subscriberSptr = subscriber.GetImpl();
    if (subscriberSptr == nullptr) {
        BGTASK_LOGE("subscriberSptr is nullptr");
        return ERR_BGTASK_INVALID_PARAM;
    }
    return backgroundTaskMgrProxy_->SubscribeBackgroundTask(subscriberSptr);
}

ErrCode BackgroundTaskManager::UnsubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber)
{
    if (!GetBackgroundTaskManagerProxy()) {
        BGTASK_LOGE("GetBackgroundTaskManagerProxy failed.");
        return ERR_BGTASK_SERVICE_NOT_CONNECTED;
    }
    sptr<BackgroundTaskSubscriber::BackgroundTaskSubscriberImpl> subscriberSptr = subscriber.GetImpl();
    if (subscriberSptr == nullptr) {
        BGTASK_LOGE("subscriberSptr is nullptr");
        return ERR_BGTASK_INVALID_PARAM;
    }
    return backgroundTaskMgrProxy_->UnsubscribeBackgroundTask(subscriberSptr);
}

ErrCode BackgroundTaskManager::GetTransientTaskApps(std::vector<std::shared_ptr<TransientTaskAppInfo>> &list)
{
    if (!GetBackgroundTaskManagerProxy()) {
        BGTASK_LOGE("GetBackgroundTaskManagerProxy failed.");
        return ERR_BGTASK_SERVICE_NOT_CONNECTED;
    }
    return backgroundTaskMgrProxy_->GetTransientTaskApps(list);
}

ErrCode BackgroundTaskManager::ApplyEfficiencyResources(const EfficiencyResourceInfo &resourceInfo, bool &isSuccess)
{
    if (!GetBackgroundTaskManagerProxy()) {
        BGTASK_LOGE("GetBackgroundTaskManagerProxy failed.");
        return ERR_BGTASK_SERVICE_NOT_CONNECTED;
    }

    sptr<EfficiencyResourceInfo> resourceInfoPtr = new (std::nothrow) EfficiencyResourceInfo(resourceInfo);
    if (resourceInfoPtr == nullptr) {
        BGTASK_LOGE("Failed to create efficiency resource info");
        return ERR_BGTASK_NO_MEMORY;
    }
    ErrCode ret = backgroundTaskMgrProxy_->ApplyEfficiencyResources(resourceInfoPtr, isSuccess);
    return ret;
}

ErrCode BackgroundTaskManager::ResetAllEfficiencyResources()
{
    if (!GetBackgroundTaskManagerProxy()) {
        BGTASK_LOGE("GetBackgroundTaskManagerProxy failed.");
        return ERR_BGTASK_SERVICE_NOT_CONNECTED;
    }

    ErrCode ret = backgroundTaskMgrProxy_->ResetAllEfficiencyResources();
    return ret;
}

ErrCode BackgroundTaskManager::ReportStateChangeEvent(const EventType type, const std::string &infos)
{
    if (!GetBackgroundTaskManagerProxy()) {
        BGTASK_LOGE("GetBackgroundTaskManagerProxy failed.");
        return ERR_BGTASK_SERVICE_NOT_CONNECTED;
    }
    return backgroundTaskMgrProxy_->ReportStateChangeEvent(type, infos);
}

bool BackgroundTaskManager::GetBackgroundTaskManagerProxy()
{
    if (backgroundTaskMgrProxy_ != nullptr) {
        return true;
    }
    std::lock_guard<std::mutex> lock(mutex_);
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

    backgroundTaskMgrProxy_ = iface_cast<BackgroundTaskMgr::IBackgroundTaskMgr>(remoteObject);
    if ((backgroundTaskMgrProxy_ == nullptr) || (backgroundTaskMgrProxy_->AsObject() == nullptr)) {
        BGTASK_LOGE("GetBackgroundTaskManagerProxy iface_cast remoteObject failed.");
        return false;
    }

    recipient_ = new (std::nothrow) BgTaskMgrDeathRecipient(*this);
    if (recipient_ == nullptr) {
        return false;
    }
    backgroundTaskMgrProxy_->AsObject()->AddDeathRecipient(recipient_);
    return true;
}

ErrCode BackgroundTaskManager::GetContinuousTaskApps(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list)
{
    if (!GetBackgroundTaskManagerProxy()) {
        BGTASK_LOGE("GetBackgroundTaskManagerProxy failed.");
        return ERR_BGTASK_SERVICE_NOT_CONNECTED;
    }
    return backgroundTaskMgrProxy_->GetContinuousTaskApps(list);
}

void BackgroundTaskManager::ResetBackgroundTaskManagerProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if ((backgroundTaskMgrProxy_ != nullptr) && (backgroundTaskMgrProxy_->AsObject() != nullptr)) {
        backgroundTaskMgrProxy_->AsObject()->RemoveDeathRecipient(recipient_);
    }
    backgroundTaskMgrProxy_ = nullptr;
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