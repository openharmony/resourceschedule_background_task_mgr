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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_DECISION_MAKER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_DECISION_MAKER_H

#include <array>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "app_mgr_constants.h"
#include "app_mgr_proxy.h"
#include "application_state_observer_stub.h"
#include "delay_suspend_info_ex.h"
#include "device_info_manager.h"
#include "input_manager.h"
#include "iremote_object.h"
#include "key_info.h"
#include "pkg_delay_suspend_info.h"
#include "suspend_controller.h"
#include "timer_manager.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class DecisionMaker : public IEventListener {
public:
    DecisionMaker(const std::shared_ptr<TimerManager>& timerManager, const std::shared_ptr<DeviceInfoManager>& device);
    ~DecisionMaker() override;
    bool Decide(const std::shared_ptr<KeyInfo>& key, const std::shared_ptr<DelaySuspendInfoEx>& delayInfo);
    void RemoveRequest(const std::shared_ptr<KeyInfo>& key, const int32_t requestId);
    int32_t GetRemainingDelayTime(const std::shared_ptr<KeyInfo>& key, const int32_t requestId);
    void OnInputEvent(const EventInfo& eventInfo) override;
    int32_t GetQuota(const std::shared_ptr<KeyInfo>& key);
    bool IsFrontApp(const string& pkgName, int32_t uid);
    void ResetAppMgrProxy();

private:
    class ApplicationStateObserver : public AppExecFwk::ApplicationStateObserverStub {
    public:
        explicit ApplicationStateObserver(DecisionMaker &decisionMaker) : decisionMaker_(decisionMaker) {}
        ~ApplicationStateObserver() {}
        void OnForegroundApplicationChanged(const AppExecFwk::AppStateData &appStateData) override;
        void OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData) override
        {}
        void OnExtensionStateChanged(const AppExecFwk::AbilityStateData &abilityStateData) override
        {}
        void OnProcessCreated(const AppExecFwk::ProcessData &processData) override
        {}
        void OnProcessDied(const AppExecFwk::ProcessData &processData) override
        {}

    private:
        DecisionMaker &decisionMaker_;
    };

private:
    class AppMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit AppMgrDeathRecipient(DecisionMaker &decisionMaker) : decisionMaker_(decisionMaker) {}

        ~AppMgrDeathRecipient() {}

        void OnRemoteDied(const wptr<IRemoteObject> &object) override;

    private:
        DecisionMaker &decisionMaker_;
    };

private:
    int32_t NewDelaySuspendRequestId();
    int32_t GetDelayTime();
    void HandleScreenOn();
    void HandleScreenOff();
    void HandlePowerConnect();
    void HandlePowerDisconnect();
    void HandlePkgRemove(const EventInfo& eventInfo);

    bool GetAppMgrProxy();
    void ResetDayQuotaLocked();
    bool IsAfterOneDay(int64_t lastRequestTime, int64_t currentTime);
    bool CanStartAccountingLocked(const std::shared_ptr<PkgDelaySuspendInfo>& pkgInfo);

    const int32_t INIT_REQUEST_ID = 1;
    int32_t requestId_ {INIT_REQUEST_ID};
    std::mutex lock_;
    int64_t lastRequestTime_ {0};
    SuspendController suspendController_;
    std::shared_ptr<TimerManager> timerManager_ {nullptr};
    std::shared_ptr<DeviceInfoManager> deviceInfoManager_ {nullptr};
    std::map<std::shared_ptr<KeyInfo>, std::shared_ptr<PkgDelaySuspendInfo>, KeyInfoComp> pkgDelaySuspendInfoMap_;
    std::map<std::shared_ptr<KeyInfo>, int32_t, KeyInfoComp> pkgBgDurationMap_;
    sptr<AppExecFwk::IAppMgr> appMgrProxy_ {nullptr};
    sptr<AppMgrDeathRecipient> recipient_;
    sptr<ApplicationStateObserver> observer_ {nullptr};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_DECISION_MAKER_H