/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
 
#ifndef BACKGROUND_TASK_MGR_JS_BACKGROUND_SUBSCRIBER_H
#define BACKGROUND_TASK_MGR_JS_BACKGROUND_SUBSCRIBER_H
 
#include <memory>
#include <atomic>
#include "background_task_mgr_helper.h"
#include "system_ability_status_change_stub.h"
 
namespace OHOS {
namespace BackgroundTaskMgr {
class JsBackgroundTaskSubscriber : public OHOS::BackgroundTaskMgr::BackgroundTaskSubscriber,
    public std::enable_shared_from_this<JsBackgroundTaskSubscriber> {
public:
    explicit JsBackgroundTaskSubscriber(napi_env env);
    virtual ~JsBackgroundTaskSubscriber();
    void OnContinuousTaskStart(const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) override;
    void HandleOnContinuousTaskStart(const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo);
    void OnContinuousTaskUpdate(const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) override;
    void HandleOnContinuousTaskUpdate(const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo);
    void HandleSubscribeOnContinuousTaskStop(
        const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo);
    void OnContinuousTaskStop(const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) override;
    void HandleOnContinuousTaskStop(const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo);
    void OnContinuousTaskSuspend(
        const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) override;
    void HandleOnContinuousTaskSuspend(const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo);
    void OnContinuousTaskActive(
        const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) override;
    void HandleOnContinuousTaskActive(const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo);
    void CallJsFunction(const napi_value value, const char *methodName, const napi_value *argv, const size_t argc);
    void AddJsObserverObject(const std::string cbType, const napi_value &jsObserverObject);
    void RemoveJsObserverObject(const std::string cbType, const napi_value &jsObserverObject);
    void RemoveJsObserverObjects(const std::string cbType);
    std::shared_ptr<NativeReference> GetObserverObject(const std::string cbType, const napi_value &jsObserverObject);
    bool IsEmpty();
    bool IsTypeEmpty(const std::string &cbType);
    void SubscriberBgtaskSaStatusChange();
    void UnSubscriberBgtaskSaStatusChange();
    void SetFlag(uint32_t flag, bool isSubscriber);
    void GetFlag(int32_t &flag) override;
 
private:
    class JsBackgroudTaskSystemAbilityStatusChange : public SystemAbilityStatusChangeStub {
    public:
        explicit JsBackgroudTaskSystemAbilityStatusChange(std::shared_ptr<JsBackgroundTaskSubscriber> subscriber_);
        virtual ~JsBackgroudTaskSystemAbilityStatusChange();
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    private:
        std::weak_ptr<JsBackgroundTaskSubscriber> subscriber_;
    };
 
private:
    napi_env env_;
    std::mutex jsObserverObjectSetLock_;
    std::map<std::string, std::set<std::shared_ptr<NativeReference>>> jsObserverObjectMap_;
    sptr<JsBackgroudTaskSystemAbilityStatusChange> jsSaListner_ = nullptr;
    std::atomic<bool> needRestoreSubscribeStatus_ = false;
    std::mutex flagLock_;
}; // JsBackgroundTaskSubscriber
} // BackgroundTaskMgr
} // OHOS
#endif // BACKGROUND_TASK_MGR_JS_BACKGROUND_SUBSCRIBER_H