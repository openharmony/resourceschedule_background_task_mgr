/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
 
void JsBackgroundTaskSubscriber::OnContinuousTaskStop(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("abilityname %{public}s continuousTaskId %{public}d cancelReason %{public}d",
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
            BGTASK_LOGI("js thread %{public}s", continuousTaskCallbackInfo->GetAbilityName().c_str());
            jsObserver->HandleOnContinuousTaskStop(continuousTaskCallbackInfo);
        });
    napi_ref callback = nullptr;
    NapiAsyncTask::Schedule("JsBackgroundTaskSubscriber::OnContinuousTaskStop", env_,
        std::make_unique<NapiAsyncTask>(callback, nullptr, std::move(complete)));
}
 
void JsBackgroundTaskSubscriber::HandleOnContinuousTaskStop(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("called");
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
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
            BGTASK_LOGE("call js func failed %{public}d.", status);
        }
    }
}
 
void JsBackgroundTaskSubscriber::AddJsObserverObject(const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        BGTASK_LOGI("null observer");
        return;
    }
 
    if (GetObserverObject(jsObserverObject) == nullptr) {
        std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
        napi_ref ref = nullptr;
        napi_create_reference(env_, jsObserverObject, 1, &ref);
        jsObserverObjectSet_.emplace(std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref)));
        BGTASK_LOGI("add observer, size %{public}d", static_cast<int32_t>(jsObserverObjectSet_.size()));
    } else {
        BGTASK_LOGI("observer exist");
    }
}
 
std::shared_ptr<NativeReference> JsBackgroundTaskSubscriber::GetObserverObject(const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        BGTASK_LOGI("null observer");
        return nullptr;
    }
 
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
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
    return jsObserverObjectSet_.empty();
}
 
void JsBackgroundTaskSubscriber::RemoveAllJsObserverObjects()
{
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    jsObserverObjectSet_.clear();
}
 
void JsBackgroundTaskSubscriber::RemoveJsObserverObject(const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        BGTASK_LOGI("null observer");
        return;
    }
 
    auto observer = GetObserverObject(jsObserverObject);
    if (observer != nullptr) {
        jsObserverObjectSet_.erase(observer);
    }
}
} // BackgroundTaskMgr
} // OHOS