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
 
#ifndef BACKGROUND_TASK_MGR_TAIHE_BACKGROUND_SUBSCRIBER_H
#define BACKGROUND_TASK_MGR_TAIHE_BACKGROUND_SUBSCRIBER_H
 
#include <memory>
#include <atomic>
#include "background_task_mgr_helper.h"
#include "system_ability_status_change_stub.h"
#include "ohos.resourceschedule.backgroundTaskManager.impl.hpp"
#include "ani.h"
#include "event_handler.h"

namespace OHOS {
namespace BackgroundTaskMgr {
using namespace taihe;
using namespace ohos::resourceschedule::backgroundTaskManager;
class AniBackgroundTaskSubscriber : public OHOS::BackgroundTaskMgr::BackgroundTaskSubscriber,
    public std::enable_shared_from_this<AniBackgroundTaskSubscriber> {
public:
    explicit AniBackgroundTaskSubscriber();
    virtual ~AniBackgroundTaskSubscriber();
    void OnContinuousTaskStop(const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) override;
    void HandleOnContinuousTaskStop(const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo);
    void OnContinuousTaskSuspend(
        const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) override;
    void HandleOnContinuousTaskSuspend(const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo);
    void OnContinuousTaskActive(
        const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) override;
    void HandleOnContinuousTaskActive(const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo);
    void AddCancelObserverObject(const std::string& cbType,
        std::shared_ptr<callback<void(const ContinuousTaskCancelInfo&)>> taiheCallback);
    std::shared_ptr<callback<void(const ContinuousTaskCancelInfo&)>> GetCancelObserverObject(
        const std::string& cbType, std::shared_ptr<callback<void(const ContinuousTaskCancelInfo&)>> taiheCallback);
    void RemoveCancelObserverObject(const std::string& cbType,
        std::shared_ptr<callback<void(const ContinuousTaskCancelInfo&)>> taiheCallback);    
    void AddSuspendObserverObject(const std::string& cbType,
        std::shared_ptr<callback<void(const ContinuousTaskSuspendInfo&)>> taiheCallback);
    std::shared_ptr<callback<void(const ContinuousTaskSuspendInfo&)>>GetSuspendObserverObject(
        const std::string& cbType, std::shared_ptr<callback<void(const ContinuousTaskSuspendInfo&)>> taiheCallback);
    void RemoveSuspendObserverObject(const std::string& cbType,
        std::shared_ptr<callback<void(const ContinuousTaskSuspendInfo&)>> taiheCallback);   
    void AddActiveObserverObject(const std::string& cbType,
        std::shared_ptr<callback<void(const ContinuousTaskActiveInfo&)>> taiheCallback);
    std::shared_ptr<callback<void(const ContinuousTaskActiveInfo&)>> GetActiveObserverObject(
        const std::string& cbType, std::shared_ptr<callback<void(const ContinuousTaskActiveInfo&)>> taiheCallback);
    void RemoveActiveObserverObject(const std::string& cbType,
        std::shared_ptr<callback<void(const ContinuousTaskActiveInfo&)>> taiheCallback);
    void RemoveJsObserverObjects(const std::string& cbType);
    bool IsEmpty();
    void SubscriberBgtaskSaStatusChange();
    void UnSubscriberBgtaskSaStatusChange();
    void SetFlag(uint32_t flag, bool isSubscriber);
    void GetFlag(int32_t &flag) override;
 
private:
    class JsBackgroudTaskSystemAbilityStatusChange : public SystemAbilityStatusChangeStub {
    public:
        explicit JsBackgroudTaskSystemAbilityStatusChange(std::shared_ptr<AniBackgroundTaskSubscriber> subscriber_);
        virtual ~JsBackgroudTaskSystemAbilityStatusChange();
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    private:
        std::weak_ptr<AniBackgroundTaskSubscriber> subscriber_;
    };
 
private:
    std::mutex jsObserverObjectSetLock_;
    std::map<std::string, std::set<std::shared_ptr<callback<void(const ContinuousTaskCancelInfo&)>>>> cancelCallbacks_;
    std::map<std::string, std::set<std::shared_ptr<callback<void(const ContinuousTaskActiveInfo&)>>>> activeCallbacks_;
    std::map<std::string, std::set<std::shared_ptr<callback<void(
        const ContinuousTaskSuspendInfo&)>>>> suspendCallbacks_;

    sptr<JsBackgroudTaskSystemAbilityStatusChange> jsSaListner_ = nullptr;
    std::atomic<bool> needRestoreSubscribeStatus_ = false;
    std::mutex flagLock_;
}; // AniBackgroundTaskSubscriber
} // BackgroundTaskMgr
} // OHOS
#endif // BACKGROUND_TASK_MGR_TAIHE_BACKGROUND_SUBSCRIBER_H