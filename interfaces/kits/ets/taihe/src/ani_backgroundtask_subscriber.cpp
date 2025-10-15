/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#include "ani_backgroundtask_subscriber.h"
#include "bgtaskmgr_log_wrapper.h"
#include "js_runtime_utils.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "ani_task.h"
 
namespace OHOS {
namespace BackgroundTaskMgr {
using namespace taihe;
using namespace OHOS::AbilityRuntime;
using namespace ohos::resourceschedule::backgroundTaskManager;

AniBackgroundTaskSubscriber::JsBackgroudTaskSystemAbilityStatusChange::JsBackgroudTaskSystemAbilityStatusChange(
    std::shared_ptr<AniBackgroundTaskSubscriber> subscriber) : subscriber_(subscriber)
{
    BGTASK_LOGD("JsBackgroudTaskSystemAbilityStatusChange constructor");
}
 
AniBackgroundTaskSubscriber::JsBackgroudTaskSystemAbilityStatusChange::~JsBackgroudTaskSystemAbilityStatusChange()
{
    BGTASK_LOGD("~JsBackgroudTaskSystemAbilityStatusChange");
}
 
void AniBackgroundTaskSubscriber::JsBackgroudTaskSystemAbilityStatusChange::OnAddSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    BGTASK_LOGI("JsBackgroudTaskSystemAbilityStatusChange::OnAddSystemAbility");
    if (systemAbilityId != BACKGROUND_TASK_MANAGER_SERVICE_ID) {
        return;
    }
    auto subscriber = subscriber_.lock();
    if (subscriber == nullptr) {
        return;
    }
    if (!subscriber->needRestoreSubscribeStatus_) {
        return;
    }
    ErrCode errCode = BackgroundTaskMgrHelper::SubscribeBackgroundTask(*subscriber);
    if (errCode) {
        BGTASK_LOGE("restore SubscribeBackgroundTask error");
    } else {
        BGTASK_LOGI("restore SubscribeBackgroundTask ok");
    }
    subscriber->needRestoreSubscribeStatus_ = false;
}
 
void AniBackgroundTaskSubscriber::JsBackgroudTaskSystemAbilityStatusChange::OnRemoveSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    if (systemAbilityId != BACKGROUND_TASK_MANAGER_SERVICE_ID) {
        return;
    }
    BGTASK_LOGI("JsBackgroudTaskSystemAbilityStatusChange::OnRemoveSystemAbility");
    auto subscriber = subscriber_.lock();
    if (subscriber == nullptr) {
        return;
    }
    if (!subscriber->IsEmpty()) {
        subscriber->needRestoreSubscribeStatus_ = true;
        BGTASK_LOGI("subscriber is not empty, need to restore");
    } else {
        BGTASK_LOGI("subscriber is empty, no need to restore");
    }
}
 
AniBackgroundTaskSubscriber::AniBackgroundTaskSubscriber()
{
    BGTASK_LOGD("AniBackgroundTaskSubscriber constructor");
}
 
AniBackgroundTaskSubscriber::~AniBackgroundTaskSubscriber()
{
    BGTASK_LOGD("~AniBackgroundTaskSubscriber");
}
 
void AniBackgroundTaskSubscriber::SubscriberBgtaskSaStatusChange()
{
    if (jsSaListner_ != nullptr) {
        return;
    }
    sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemManager) {
        BGTASK_LOGI("ResSchedClient::Fail to get sa mgr client.");
        return;
    }
    jsSaListner_ = new (std::nothrow) JsBackgroudTaskSystemAbilityStatusChange(shared_from_this());
    if (jsSaListner_ == nullptr) {
        BGTASK_LOGI("jsSaListner_ null");
        return;
    }
    int32_t ret = systemManager->SubscribeSystemAbility(BACKGROUND_TASK_MANAGER_SERVICE_ID, jsSaListner_);
    if (ret != ERR_OK) {
        BGTASK_LOGI("sBackgroundTaskSubscriber::InitSaStatusListener error");
        jsSaListner_ = nullptr;
    }
    BGTASK_LOGI("AniBackgroundTaskSubscriber::InitSaStatusListener ok");
}
 
void AniBackgroundTaskSubscriber::UnSubscriberBgtaskSaStatusChange()
{
    if (jsSaListner_ == nullptr) {
        return;
    }
    sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemManager) {
        BGTASK_LOGE("GetSystemAbilityManager error");
        return;
    }
    int32_t ret = systemManager->UnSubscribeSystemAbility(BACKGROUND_TASK_MANAGER_SERVICE_ID, jsSaListner_);
    if (ret != ERR_OK) {
        BGTASK_LOGE("UnSubscribeSystemAbility error");
    } else {
        BGTASK_LOGI("UnSubscribeSystemAbility ok");
    }
    jsSaListner_ = nullptr;
}
 
void AniBackgroundTaskSubscriber::OnContinuousTaskStop(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("OnContinuousTaskStop abilityname %{public}s continuousTaskId %{public}d cancelReason %{public}d",
        continuousTaskCallbackInfo->GetAbilityName().c_str(),
        continuousTaskCallbackInfo->GetContinuousTaskId(),
        continuousTaskCallbackInfo->GetCancelReason());
    
    auto task = [self = weak_from_this(), continuousTaskCallbackInfo]() {
            auto jsObserver = self.lock();
            if (jsObserver == nullptr) {
                BGTASK_LOGE("null observer");
                return;
            }
            BGTASK_LOGI("OnContinuousTaskStop js thread %{public}s",
                continuousTaskCallbackInfo->GetAbilityName().c_str());
            jsObserver->HandleOnContinuousTaskStop(continuousTaskCallbackInfo);
        };
    if (AniTask::AniSendEvent(task) != ANI_OK) {
        BGTASK_LOGE("Failed to aniSendEvent");
    }
}
 
void AniBackgroundTaskSubscriber::HandleOnContinuousTaskStop(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("HandleOnContinuousTaskStop called");
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    auto iter = cancelCallbacks_.find("continuousTaskCancel");
    if (iter == cancelCallbacks_.end()) {
        BGTASK_LOGW("null callback Type");
        return;
    }

    auto& jsObserverObjectSet_ = iter->second;
    for (auto &item : jsObserverObjectSet_) {
        ContinuousTaskCancelInfo aniCallBack{
            .reason = static_cast<ContinuousTaskCancelReason::key_t>(continuousTaskCallbackInfo->GetCancelReason()),
            .id = continuousTaskCallbackInfo->GetContinuousTaskId(),
        };
        (*item)(aniCallBack);
    }
}

void AniBackgroundTaskSubscriber::OnContinuousTaskSuspend(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("OnContinuousTaskSuspend abilityname: %{public}s, continuousTaskId: %{public}d,"
        "suspendReason: %{public}d, suspendState: %{public}d", continuousTaskCallbackInfo->GetAbilityName().c_str(),
        continuousTaskCallbackInfo->GetContinuousTaskId(), continuousTaskCallbackInfo->GetSuspendReason(),
        continuousTaskCallbackInfo->GetSuspendState());
    auto task = [self = weak_from_this(), continuousTaskCallbackInfo]() {
            auto jsObserver = self.lock();
            if (jsObserver == nullptr) {
                BGTASK_LOGE("null observer");
                return;
            }
            BGTASK_LOGI("OnContinuousTaskSuspend js thread %{public}s",
                continuousTaskCallbackInfo->GetAbilityName().c_str());
            jsObserver->HandleOnContinuousTaskSuspend(continuousTaskCallbackInfo);
        };
    if (AniTask::AniSendEvent(task) != ANI_OK) {
        BGTASK_LOGE("Failed to aniSendEvent");
    }
}

void AniBackgroundTaskSubscriber::HandleOnContinuousTaskSuspend(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("HandleOnContinuousTaskSuspend called");
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    auto iter = suspendCallbacks_.find("continuousTaskSuspend");
    if (iter == suspendCallbacks_.end()) {
        BGTASK_LOGW("null callback Type: continuousTaskSuspend");
        return;
    }
    auto& jsObserverObjectSet_ = iter->second;
    for (auto &item : jsObserverObjectSet_) {
        ContinuousTaskSuspendInfo aniCallBack{
            .continuousTaskId = continuousTaskCallbackInfo->GetContinuousTaskId(),
            .suspendState = continuousTaskCallbackInfo->GetSuspendState(),
            .suspendReason = static_cast<ContinuousTaskSuspendReason::key_t>(
                continuousTaskCallbackInfo->GetSuspendReason()),
        };
        (*item)(aniCallBack);
    }
}

void AniBackgroundTaskSubscriber::OnContinuousTaskActive(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("OnContinuousTaskActive abilityname: %{public}s, continuousTaskId: %{public}d",
        continuousTaskCallbackInfo->GetAbilityName().c_str(), continuousTaskCallbackInfo->GetContinuousTaskId());
    auto task = [self = weak_from_this(), continuousTaskCallbackInfo]() {
            auto jsObserver = self.lock();
            if (jsObserver == nullptr) {
                BGTASK_LOGE("null observer");
                return;
            }
            BGTASK_LOGI("OnContinuousTaskActive js thread %{public}s",
                continuousTaskCallbackInfo->GetAbilityName().c_str());
            jsObserver->HandleOnContinuousTaskActive(continuousTaskCallbackInfo);
        };
    if (AniTask::AniSendEvent(task) != ANI_OK) {
        BGTASK_LOGE("Failed to aniSendEvent");
    }
}

void AniBackgroundTaskSubscriber::HandleOnContinuousTaskActive(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("HandleOnContinuousTaskActive called");
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    auto iter = activeCallbacks_.find("continuousTaskActive");
    if (iter == activeCallbacks_.end()) {
        BGTASK_LOGW("null callback Type: continuousTaskActive");
        return;
    }

    auto& jsObserverObjectSet_ = iter->second;
    for (auto &item : jsObserverObjectSet_) {
        ContinuousTaskActiveInfo aniCallBack{
            .id = continuousTaskCallbackInfo->GetContinuousTaskId(),
        };
        (*item)(aniCallBack);
    }
}

void AniBackgroundTaskSubscriber::AddCancelObserverObject(const std::string& cbType,
    std::shared_ptr<taihe::callback<void(const ContinuousTaskCancelInfo&)>> taiheCallback)
{
    if (taiheCallback == nullptr) {
        BGTASK_LOGI("null observer");
        return;
    }
    if (GetCancelObserverObject(cbType, taiheCallback) == nullptr) {
        std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
        cancelCallbacks_[cbType].emplace(taiheCallback);
        BGTASK_LOGI("add observer, type: %{public}s, size: %{public}d", cbType.c_str(),
            static_cast<int32_t>(cancelCallbacks_[cbType].size()));
    } else {
        BGTASK_LOGI("observer exist");
    }
}

std::shared_ptr<callback<void(const ContinuousTaskCancelInfo&)>> AniBackgroundTaskSubscriber::GetCancelObserverObject(
    const std::string& cbType, std::shared_ptr<callback<void(const ContinuousTaskCancelInfo&)>> taiheCallback)
{
    if (taiheCallback == nullptr) {
        BGTASK_LOGI("null observer");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    auto iter = cancelCallbacks_.find(cbType);
    if (iter == cancelCallbacks_.end()) {
        BGTASK_LOGW("null callback Type: %{public}s", cbType.c_str());
        return nullptr;
    }

    for (const auto& observer : iter->second) {
        if (*observer == *taiheCallback) {
            return observer;
        }
    }
    return nullptr;
}

void AniBackgroundTaskSubscriber::RemoveCancelObserverObject(
    const std::string& cbType, std::shared_ptr<callback<void(const ContinuousTaskCancelInfo&)>> taiheCallback)
{
    if (taiheCallback == nullptr) {
        BGTASK_LOGI("null observer");
        return;
    }
 
    auto observer = GetCancelObserverObject(cbType, taiheCallback);
    if (observer != nullptr) {
        std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
        cancelCallbacks_[cbType].erase(observer);
        if (cancelCallbacks_[cbType].empty()) {
            cancelCallbacks_.erase(cbType);
        }
    }
}
    
void AniBackgroundTaskSubscriber::AddSuspendObserverObject(const std::string& cbType,
    std::shared_ptr<callback<void(const ContinuousTaskSuspendInfo&)>> taiheCallback)
{
    if (taiheCallback == nullptr) {
        BGTASK_LOGI("null observer");
        return;
    }
    if (GetSuspendObserverObject(cbType, taiheCallback) == nullptr) {
        std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
        suspendCallbacks_[cbType].emplace(taiheCallback);
        BGTASK_LOGI("add observer, type: %{public}s, size: %{public}d", cbType.c_str(),
            static_cast<int32_t>(suspendCallbacks_[cbType].size()));
    } else {
        BGTASK_LOGI("observer exist");
    }
}

std::shared_ptr<callback<void(const ContinuousTaskSuspendInfo&)>> AniBackgroundTaskSubscriber::GetSuspendObserverObject(
    const std::string& cbType, std::shared_ptr<callback<void(const ContinuousTaskSuspendInfo&)>> taiheCallback)
{
    if (taiheCallback == nullptr) {
        BGTASK_LOGI("null observer");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    auto iter = suspendCallbacks_.find(cbType);
    if (iter == suspendCallbacks_.end()) {
        BGTASK_LOGW("null callback Type: %{public}s", cbType.c_str());
        return nullptr;
    }

    for (const auto& observer : iter->second) {
        if (*observer == *taiheCallback) {
            return observer;
        }
    }
    return nullptr;
}

void AniBackgroundTaskSubscriber::RemoveSuspendObserverObject(const std::string& cbType,
    std::shared_ptr<callback<void(const ContinuousTaskSuspendInfo&)>> taiheCallback)
{
    if (taiheCallback == nullptr) {
        BGTASK_LOGI("null observer");
        return;
    }
 
    auto observer = GetSuspendObserverObject(cbType, taiheCallback);
    if (observer != nullptr) {
        std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
        suspendCallbacks_[cbType].erase(observer);
        if (suspendCallbacks_[cbType].empty()) {
            suspendCallbacks_.erase(cbType);
        }
    }
}
    
void AniBackgroundTaskSubscriber::AddActiveObserverObject(const std::string& cbType,
    std::shared_ptr<callback<void(const ContinuousTaskActiveInfo&)>> taiheCallback)
{
    if (taiheCallback == nullptr) {
        BGTASK_LOGI("null observer");
        return;
    }
    if (GetActiveObserverObject(cbType, taiheCallback) == nullptr) {
        std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
        activeCallbacks_[cbType].emplace(taiheCallback);
        BGTASK_LOGI("add observer, type: %{public}s, size: %{public}d", cbType.c_str(),
            static_cast<int32_t>(activeCallbacks_[cbType].size()));
    } else {
        BGTASK_LOGI("observer exist");
    }
}

std::shared_ptr<callback<void(const ContinuousTaskActiveInfo&)>> AniBackgroundTaskSubscriber::GetActiveObserverObject(
    const std::string& cbType, std::shared_ptr<callback<void(const ContinuousTaskActiveInfo&)>> taiheCallback)
{
    if (taiheCallback == nullptr) {
        BGTASK_LOGI("null observer");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    auto iter = activeCallbacks_.find(cbType);
    if (iter == activeCallbacks_.end()) {
        BGTASK_LOGW("null callback Type: %{public}s", cbType.c_str());
        return nullptr;
    }

    for (const auto& observer : iter->second) {
        if (*observer == *taiheCallback) {
            return observer;
        }
    }
    return nullptr;
}

void AniBackgroundTaskSubscriber::RemoveActiveObserverObject(const std::string& cbType,
    std::shared_ptr<callback<void(const ContinuousTaskActiveInfo&)>> taiheCallback)
{
    if (taiheCallback == nullptr) {
        BGTASK_LOGI("null observer");
        return;
    }
 
    auto observer = GetActiveObserverObject(cbType, taiheCallback);
    if (observer != nullptr) {
        std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
        activeCallbacks_[cbType].erase(observer);
        if (activeCallbacks_[cbType].empty()) {
            activeCallbacks_.erase(cbType);
        }
    }
}

bool AniBackgroundTaskSubscriber::IsEmpty()
{
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    return cancelCallbacks_.empty() && suspendCallbacks_.empty() && activeCallbacks_.empty();
}

void AniBackgroundTaskSubscriber::RemoveJsObserverObjects(const std::string& cbType)
{
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    if (cbType == "continuousTaskCancel") {
        cancelCallbacks_.clear();
    } else if (cbType == "continuousTaskSuspend") {
        suspendCallbacks_.clear();
    } else if (cbType == "continuousTaskActive") {
        activeCallbacks_.clear();
    }
}

void AniBackgroundTaskSubscriber::SetFlag(uint32_t flag, bool isSubscriber)
{
    std::lock_guard<std::mutex> lock(flagLock_);
    if (isSubscriber) {
        flag_ |= flag;
    } else {
        flag_ &= ~flag;
    }
}

void AniBackgroundTaskSubscriber::GetFlag(int32_t &flag)
{
    std::lock_guard<std::mutex> lock(flagLock_);
    flag = static_cast<int32_t>(flag_);
}

} // BackgroundTaskMgr
} // OHOS