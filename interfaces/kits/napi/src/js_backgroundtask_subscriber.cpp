/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
 
#include "js_backgroundtask_subscriber.h"
#include "continuous_task_log.h"
#include "common.h"
#include "js_runtime_utils.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
 
namespace OHOS {
namespace BackgroundTaskMgr {
using namespace OHOS::AbilityRuntime;
 
JsBackgroundTaskSubscriber::JsBackgroudTaskSystemAbilityStatusChange::JsBackgroudTaskSystemAbilityStatusChange(
    std::shared_ptr<JsBackgroundTaskSubscriber> subscriber) : subscriber_(subscriber)
{
    BGTASK_LOGD("JsBackgroudTaskSystemAbilityStatusChange constructor");
}
 
JsBackgroundTaskSubscriber::JsBackgroudTaskSystemAbilityStatusChange::~JsBackgroudTaskSystemAbilityStatusChange()
{
    BGTASK_LOGD("~JsBackgroudTaskSystemAbilityStatusChange");
}
 
void JsBackgroundTaskSubscriber::JsBackgroudTaskSystemAbilityStatusChange::OnAddSystemAbility(
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
 
void JsBackgroundTaskSubscriber::JsBackgroudTaskSystemAbilityStatusChange::OnRemoveSystemAbility(
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
 
JsBackgroundTaskSubscriber::JsBackgroundTaskSubscriber(napi_env env) : env_(env)
{
    BGTASK_LOGD("JsBackgroundTaskSubscriber constructor");
}
 
JsBackgroundTaskSubscriber::~JsBackgroundTaskSubscriber()
{
    BGTASK_LOGD("~JsBackgroundTaskSubscriber");
}
 
void JsBackgroundTaskSubscriber::SubscriberBgtaskSaStatusChange()
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
    BGTASK_LOGI("JsBackgroundTaskSubscriber::InitSaStatusListener ok");
}
 
void JsBackgroundTaskSubscriber::UnSubscriberBgtaskSaStatusChange()
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

void JsBackgroundTaskSubscriber::OnContinuousTaskStart(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("OnContinuousTaskStart abilityname %{public}s continuousTaskId %{public}d",
        continuousTaskCallbackInfo->GetAbilityName().c_str(),
        continuousTaskCallbackInfo->GetContinuousTaskId());
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [self = weak_from_this(), continuousTaskCallbackInfo](napi_env env, NapiAsyncTask &task, int32_t status) {
            auto jsObserver = self.lock();
            if (jsObserver == nullptr) {
                BGTASK_LOGE("null observer");
                return;
            }
            BGTASK_LOGI("OnContinuousTaskStart js thread %{public}s",
                continuousTaskCallbackInfo->GetAbilityName().c_str());
            jsObserver->HandleOnContinuousTaskStart(continuousTaskCallbackInfo);
        });
    napi_ref callback = nullptr;
    NapiAsyncTask::Schedule("JsBackgroundTaskSubscriber::OnContinuousTaskStart", env_,
        std::make_unique<NapiAsyncTask>(callback, nullptr, std::move(complete)));
}

void JsBackgroundTaskSubscriber::HandleOnContinuousTaskStart(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("HandleOnContinuousTaskStart called");
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    auto iter = jsObserverObjectMap_.find("subscribeContinuousTaskState");
    if (iter == jsObserverObjectMap_.end()) {
        BGTASK_LOGW("null callback Type");
        return;
    }
    std::set<std::shared_ptr<NativeReference>> jsObserverObjectSet_ = iter->second;
    for (auto &item : jsObserverObjectSet_) {
        napi_value jsCallbackObj = item->GetNapiValue();
        napi_value callFunction;
        napi_get_named_property(env_, jsCallbackObj, "onContinuousTaskStart", &callFunction);
        napi_value jsContinuousTaskStateInfo = Common::GetNapiCallBackInfo(env_, continuousTaskCallbackInfo);
        if (jsContinuousTaskStateInfo == nullptr) {
            BGTASK_LOGE("GetNapiCallBackInfo fail.");
            return;
        }
        napi_value argv[1] = { jsContinuousTaskStateInfo };
        napi_value callResult = nullptr;
        napi_value undefined = nullptr;
        napi_get_undefined(env_, &undefined);
        napi_status status = napi_call_function(env_, undefined, callFunction, 1, argv, &callResult);
        if (status != napi_ok) {
            BGTASK_LOGE("call onContinuousTaskStart func failed %{public}d.", status);
        }
    }
}

void JsBackgroundTaskSubscriber::OnContinuousTaskUpdate(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("OnContinuousTaskUpdate abilityname %{public}s continuousTaskId %{public}d",
        continuousTaskCallbackInfo->GetAbilityName().c_str(),
        continuousTaskCallbackInfo->GetContinuousTaskId());
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [self = weak_from_this(), continuousTaskCallbackInfo](napi_env env, NapiAsyncTask &task, int32_t status) {
            auto jsObserver = self.lock();
            if (jsObserver == nullptr) {
                BGTASK_LOGE("null observer");
                return;
            }
            BGTASK_LOGI("OnContinuousTaskUpdate js thread %{public}s",
                continuousTaskCallbackInfo->GetAbilityName().c_str());
            jsObserver->HandleOnContinuousTaskUpdate(continuousTaskCallbackInfo);
        });
    napi_ref callback = nullptr;
    NapiAsyncTask::Schedule("JsBackgroundTaskSubscriber::OnContinuousTaskUpdate", env_,
        std::make_unique<NapiAsyncTask>(callback, nullptr, std::move(complete)));
}

void JsBackgroundTaskSubscriber::HandleOnContinuousTaskUpdate(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("HandleOnContinuousTaskUpdate called");
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    auto iter = jsObserverObjectMap_.find("subscribeContinuousTaskState");
    if (iter == jsObserverObjectMap_.end()) {
        BGTASK_LOGW("null callback Type");
        return;
    }
    std::set<std::shared_ptr<NativeReference>> jsObserverObjectSet_ = iter->second;
    for (auto &item : jsObserverObjectSet_) {
        napi_value jsCallbackObj = item->GetNapiValue();
        napi_value callFunction;
        napi_get_named_property(env_, jsCallbackObj, "onContinuousTaskUpdate", &callFunction);
        napi_value jsContinuousTaskStateInfo = Common::GetNapiCallBackInfo(env_, continuousTaskCallbackInfo);
        if (jsContinuousTaskStateInfo == nullptr) {
            BGTASK_LOGE("GetNapiCallBackInfo fail.");
            return;
        }
        napi_value argv[1] = { jsContinuousTaskStateInfo };
        napi_value callResult = nullptr;
        napi_value undefined = nullptr;
        napi_get_undefined(env_, &undefined);
        napi_status status = napi_call_function(env_, undefined, callFunction, 1, argv, &callResult);
        if (status != napi_ok) {
            BGTASK_LOGE("call onContinuousTaskUpdate func failed %{public}d.", status);
        }
    }
}
 
void JsBackgroundTaskSubscriber::OnContinuousTaskStop(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("OnContinuousTaskStop abilityname %{public}s continuousTaskId %{public}d cancelReason %{public}d",
        continuousTaskCallbackInfo->GetAbilityName().c_str(),
        continuousTaskCallbackInfo->GetContinuousTaskId(),
        continuousTaskCallbackInfo->GetCancelReason());
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [self = weak_from_this(), continuousTaskCallbackInfo](napi_env env, NapiAsyncTask &task, int32_t status) {
            auto jsObserver = self.lock();
            if (jsObserver == nullptr) {
                BGTASK_LOGE("null observer");
                return;
            }
            BGTASK_LOGI("OnContinuousTaskStop js thread %{public}s",
                continuousTaskCallbackInfo->GetAbilityName().c_str());
            if (continuousTaskCallbackInfo->IsCancelCallBackSelf()) {
                jsObserver->HandleOnContinuousTaskStop(continuousTaskCallbackInfo);
            }
            jsObserver->HandleSubscribeOnContinuousTaskStop(continuousTaskCallbackInfo);
        });
    napi_ref callback = nullptr;
    NapiAsyncTask::Schedule("JsBackgroundTaskSubscriber::OnContinuousTaskStop", env_,
        std::make_unique<NapiAsyncTask>(callback, nullptr, std::move(complete)));
}

void JsBackgroundTaskSubscriber::HandleSubscribeOnContinuousTaskStop(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("HandleSubscribeOnContinuousTaskStop called");
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    auto iter = jsObserverObjectMap_.find("subscribeContinuousTaskState");
    if (iter == jsObserverObjectMap_.end()) {
        BGTASK_LOGW("null callback Type subscribeContinuousTaskState");
        return;
    }
    std::set<std::shared_ptr<NativeReference>> jsObserverObjectSet_ = iter->second;
    for (auto &item : jsObserverObjectSet_) {
        napi_value jsCallbackObj = item->GetNapiValue();
        napi_value callFunction;
        napi_get_named_property(env_, jsCallbackObj, "onContinuousTaskStop", &callFunction);
        napi_value jsContinuousTaskStateInfo = Common::GetNapiCallBackInfo(env_, continuousTaskCallbackInfo);
        if (jsContinuousTaskStateInfo == nullptr) {
            BGTASK_LOGE("GetNapiCallBackInfo fail.");
            return;
        }
        napi_value argv[1] = { jsContinuousTaskStateInfo };
        napi_value callResult = nullptr;
        napi_value undefined = nullptr;
        napi_get_undefined(env_, &undefined);
        napi_status status = napi_call_function(env_, undefined, callFunction, 1, argv, &callResult);
        if (status != napi_ok) {
            BGTASK_LOGE("call subscriber onContinuousTaskStop func failed %{public}d.", status);
        }
    }
}
 
void JsBackgroundTaskSubscriber::HandleOnContinuousTaskStop(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("HandleOnContinuousTaskStop called");
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    auto iter = jsObserverObjectMap_.find("continuousTaskCancel");
    if (iter == jsObserverObjectMap_.end()) {
        BGTASK_LOGW("null callback Type");
        return;
    }
    std::set<std::shared_ptr<NativeReference>> jsObserverObjectSet_ = iter->second;
    for (auto &item : jsObserverObjectSet_) {
        napi_value jsCallbackObj = item->GetNapiValue();
        napi_value jsContinuousTaskCancelInfo = nullptr;

        napi_create_object(env_, &jsContinuousTaskCancelInfo);

        napi_value value = nullptr;
        napi_create_int32(env_, continuousTaskCallbackInfo->GetCancelReason(), &value);
        napi_set_named_property(env_, jsContinuousTaskCancelInfo, "reason", value);

        napi_create_int32(env_, continuousTaskCallbackInfo->GetContinuousTaskId(), &value);
        napi_set_named_property(env_, jsContinuousTaskCancelInfo, "id", value);

        napi_value argv[1] = { jsContinuousTaskCancelInfo };
        napi_value callResult = nullptr;
        napi_value undefined = nullptr;
        napi_get_undefined(env_, &undefined);
        napi_status status = napi_call_function(env_, undefined, jsCallbackObj, 1, argv, &callResult);
        if (status != napi_ok) {
            BGTASK_LOGE("call continuousTaskCancel func failed %{public}d.", status);
        }
    }
}

void JsBackgroundTaskSubscriber::OnContinuousTaskSuspend(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("OnContinuousTaskSuspend abilityname: %{public}s, continuousTaskId: %{public}d,"
        "suspendReason: %{public}d, suspendState: %{public}d", continuousTaskCallbackInfo->GetAbilityName().c_str(),
        continuousTaskCallbackInfo->GetContinuousTaskId(), continuousTaskCallbackInfo->GetSuspendReason(),
        continuousTaskCallbackInfo->GetSuspendState());
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [self = weak_from_this(), continuousTaskCallbackInfo](napi_env env, NapiAsyncTask &task, int32_t status) {
            auto jsObserver = self.lock();
            if (jsObserver == nullptr) {
                BGTASK_LOGE("null observer");
                return;
            }
            BGTASK_LOGD("OnContinuousTaskSuspend js thread %{public}s",
                continuousTaskCallbackInfo->GetAbilityName().c_str());
            jsObserver->HandleOnContinuousTaskSuspend(continuousTaskCallbackInfo);
        });
    napi_ref callback = nullptr;
    NapiAsyncTask::Schedule("JsBackgroundTaskSubscriber::OnContinuousTaskSuspend", env_,
        std::make_unique<NapiAsyncTask>(callback, nullptr, std::move(complete)));
}

void JsBackgroundTaskSubscriber::HandleOnContinuousTaskSuspend(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("HandleOnContinuousTaskSuspend called");
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    auto iter = jsObserverObjectMap_.find("continuousTaskSuspend");
    if (iter == jsObserverObjectMap_.end()) {
        BGTASK_LOGW("null callback Type: continuousTaskSuspend");
        return;
    }
    std::set<std::shared_ptr<NativeReference>> jsObserverObjectSet_ = iter->second;
    for (auto &item : jsObserverObjectSet_) {
        napi_value jsCallbackObj = item->GetNapiValue();
        napi_value jsContinuousTaskSuspendInfo = nullptr;

        napi_create_object(env_, &jsContinuousTaskSuspendInfo);

        // set continuousTaskId
        napi_value continuousTaskId = nullptr;
        napi_create_int32(env_, continuousTaskCallbackInfo->GetContinuousTaskId(), &continuousTaskId);
        napi_set_named_property(env_, jsContinuousTaskSuspendInfo, "continuousTaskId", continuousTaskId);

        // set suspendState
        napi_value suspendState = nullptr;
        napi_get_boolean(env_, continuousTaskCallbackInfo->GetSuspendState(), &suspendState);
        napi_set_named_property(env_, jsContinuousTaskSuspendInfo, "suspendState", suspendState);

        // set suspendReason
        napi_value suspendReason = nullptr;
        napi_create_int32(env_, -1, &suspendReason);
        napi_set_named_property(env_, jsContinuousTaskSuspendInfo, "suspendReason", suspendReason);

        napi_value argv[1] = { jsContinuousTaskSuspendInfo };
        napi_value callResult = nullptr;
        napi_value undefined = nullptr;
        napi_get_undefined(env_, &undefined);
        napi_status status = napi_call_function(env_, undefined, jsCallbackObj, 1, argv, &callResult);
        if (status != napi_ok) {
            BGTASK_LOGE("call js func failed %{public}d.", status);
        }
    }
}

void JsBackgroundTaskSubscriber::OnContinuousTaskActive(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("OnContinuousTaskActive abilityname: %{public}s, continuousTaskId: %{public}d",
        continuousTaskCallbackInfo->GetAbilityName().c_str(), continuousTaskCallbackInfo->GetContinuousTaskId());
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [self = weak_from_this(), continuousTaskCallbackInfo](napi_env env, NapiAsyncTask &task, int32_t status) {
            auto jsObserver = self.lock();
            if (jsObserver == nullptr) {
                BGTASK_LOGE("null observer");
                return;
            }
            BGTASK_LOGI("OnContinuousTaskActive js thread %{public}s",
                continuousTaskCallbackInfo->GetAbilityName().c_str());
            jsObserver->HandleOnContinuousTaskActive(continuousTaskCallbackInfo);
        });
    napi_ref callback = nullptr;
    NapiAsyncTask::Schedule("JsBackgroundTaskSubscriber::OnContinuousTaskActive", env_,
        std::make_unique<NapiAsyncTask>(callback, nullptr, std::move(complete)));
}

void JsBackgroundTaskSubscriber::HandleOnContinuousTaskActive(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("HandleOnContinuousTaskActive called");
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    auto iter = jsObserverObjectMap_.find("continuousTaskActive");
    if (iter == jsObserverObjectMap_.end()) {
        BGTASK_LOGW("null callback Type: continuousTaskActive");
        return;
    }
    std::set<std::shared_ptr<NativeReference>> jsObserverObjectSet_ = iter->second;
    for (auto &item : jsObserverObjectSet_) {
        napi_value jsCallbackObj = item->GetNapiValue();
        napi_value jsContinuousTaskActiveInfo = nullptr;

        napi_create_object(env_, &jsContinuousTaskActiveInfo);

        // set continuousTaskId
        napi_value continuousTaskId = nullptr;
        napi_create_int32(env_, continuousTaskCallbackInfo->GetContinuousTaskId(), &continuousTaskId);
        napi_set_named_property(env_, jsContinuousTaskActiveInfo, "id", continuousTaskId);

        napi_value argv[1] = { jsContinuousTaskActiveInfo };
        napi_value callResult = nullptr;
        napi_value undefined = nullptr;
        napi_get_undefined(env_, &undefined);
        napi_status status = napi_call_function(env_, undefined, jsCallbackObj, 1, argv, &callResult);
        if (status != napi_ok) {
            BGTASK_LOGE("call js func failed %{public}d.", status);
        }
    }
}

void JsBackgroundTaskSubscriber::AddJsObserverObject(const std::string cbType, const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        BGTASK_LOGI("null observer");
        return;
    }
 
    if (GetObserverObject(cbType, jsObserverObject) == nullptr) {
        napi_ref ref = nullptr;
        napi_create_reference(env_, jsObserverObject, 1, &ref);
        std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
        jsObserverObjectMap_[cbType].emplace(
            std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref)));
        BGTASK_LOGI("add observer, type: %{public}s, size: %{public}d", cbType.c_str(),
            static_cast<int32_t>(jsObserverObjectMap_[cbType].size()));
    } else {
        BGTASK_LOGI("observer exist");
    }
}

std::shared_ptr<NativeReference> JsBackgroundTaskSubscriber::GetObserverObject(
    const std::string cbType, const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        BGTASK_LOGI("null observer");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    auto iter = jsObserverObjectMap_.find(cbType);
    if (iter == jsObserverObjectMap_.end()) {
        BGTASK_LOGW("null callback Type: %{public}s", cbType.c_str());
        return nullptr;
    }
    std::set<std::shared_ptr<NativeReference>> jsObserverObjectSet_ = iter->second;
    for (auto &observer : jsObserverObjectSet_) {
        if (observer == nullptr) {
            BGTASK_LOGI("null observer");
            continue;
        }
 
        napi_value value = observer->GetNapiValue();
        if (value == nullptr) {
            BGTASK_LOGI("null value");
            continue;
        }
 
        bool isEqual = false;
        napi_strict_equals(env_, value, jsObserverObject, &isEqual);
        if (isEqual) {
            return observer;
        }
    }
    return nullptr;
}
 
bool JsBackgroundTaskSubscriber::IsEmpty()
{
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    return jsObserverObjectMap_.empty();
}

bool JsBackgroundTaskSubscriber::IsTypeEmpty(const std::string &cbType)
{
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    auto iter = jsObserverObjectMap_.find(cbType);
    return iter == jsObserverObjectMap_.end();
}

void JsBackgroundTaskSubscriber::RemoveJsObserverObjects(const std::string cbType)
{
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    auto iter = jsObserverObjectMap_.find(cbType);
    if (iter != jsObserverObjectMap_.end()) {
        jsObserverObjectMap_.erase(cbType);
    }
}

void JsBackgroundTaskSubscriber::RemoveJsObserverObject(const std::string cbType, const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        BGTASK_LOGI("null observer");
        return;
    }
    auto observer = GetObserverObject(cbType, jsObserverObject);
    if (observer != nullptr) {
        std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
        int32_t size = static_cast<int32_t>(jsObserverObjectMap_[cbType].size());
        if (size == 1) {
            jsObserverObjectMap_.erase(cbType);
        } else {
            jsObserverObjectMap_[cbType].erase(observer);
        }
    }
}

void JsBackgroundTaskSubscriber::SetFlag(uint32_t flag, bool isSubscriber)
{
    std::lock_guard<std::mutex> lock(flagLock_);
    if (isSubscriber) {
        flag_ = flag_ |= flag;
    } else {
        flag_ = flag_ & ~flag;
    }
}

void JsBackgroundTaskSubscriber::GetFlag(int32_t &flag)
{
    std::lock_guard<std::mutex> lock(flagLock_);
    flag = static_cast<int32_t>(flag_);
}
} // BackgroundTaskMgr
} // OHOS