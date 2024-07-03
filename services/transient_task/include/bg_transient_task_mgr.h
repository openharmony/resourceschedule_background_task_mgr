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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_BG_TRANSIENT_TASK_MGR_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_BG_TRANSIENT_TASK_MGR_H

#include <functional>
#include <mutex>
#include <memory>
#include <set>

#include <iremote_object.h>
#include <nocopyable.h>
#include <singleton.h>
#include <string_ex.h>
#include <system_ability.h>

#include "background_task_mgr_stub.h"
#include "decision_maker.h"
#include "delay_suspend_info.h"
#include "device_info_manager.h"
#include "event_handler.h"
#include "event_info.h"
#include "ibackground_task_mgr.h"
#include "iexpired_callback.h"
#include "ibackground_task_subscriber.h"
#include "timer_manager.h"
#include "transient_task_app_info.h"
#include "watchdog.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class SubscriberDeathRecipient;
class ExpiredCallbackDeathRecipient;

enum class TransientTaskEventType: uint32_t {
    TASK_START,
    TASK_END,
    APP_TASK_START,
    APP_TASK_END,
};
class BgTransientTaskMgr {
    DECLARE_DELAYED_SINGLETON(BgTransientTaskMgr);

public:
    void Init(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ErrCode RequestSuspendDelay(const std::u16string& reason,
        const sptr<IExpiredCallback>& callback, std::shared_ptr<DelaySuspendInfo> &delayInfo);
    ErrCode CancelSuspendDelay(int32_t requestId);
    ErrCode GetRemainingDelayTime(int32_t requestId, int32_t &delayTime);
    ErrCode SubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber);
    ErrCode UnsubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber);
    ErrCode GetTransientTaskApps(std::vector<std::shared_ptr<TransientTaskAppInfo>> &list);
    ErrCode PauseTransientTaskTimeForInner(int32_t uid);
    ErrCode StartTransientTaskTimeForInner(int32_t uid);
    ErrCode ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);

    void ForceCancelSuspendDelay(int32_t requestId);
    void HandleExpiredCallbackDeath(const wptr<IRemoteObject>& remote);
    void HandleSubscriberDeath(const wptr<IRemoteObject>& remote);
    void HandleRequestExpired(const int32_t requestId);
    void HandleTransientTaskSuscriberTask(const shared_ptr<TransientTaskAppInfo>& appInfo,
        const TransientTaskEventType type);
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId);
    void HandleSuspendManagerDie();
    std::set<int32_t>& GetTransientPauseUid();

private:
    ErrCode IsCallingInfoLegal(int32_t uid, int32_t pid, std::string &name,
        const sptr<IExpiredCallback>& callback);
    bool GetBundleNamesForUid(int32_t uid, std::string &bundleName);
    bool VerifyCallingInfo(int32_t uid, int32_t pid);
    bool VerifyRequestIdLocked(const std::string& name, int32_t uid, int32_t requestId);
    bool CheckProcessName();
    ErrCode CancelSuspendDelayLocked(int32_t requestId);
    void NotifyTransientTaskSuscriber(const shared_ptr<TransientTaskAppInfo>& appInfo,
        const TransientTaskEventType type);
    bool DumpAllRequestId(std::vector<std::string> &dumpInfo);
    void DumpTaskTime(const std::vector<std::string> &dumpOption, bool pause, std::vector<std::string> &dumpInfo);
    void SendLowBatteryEvent(std::vector<std::string> &dumpInfo);
    void SendOkayBatteryEvent(std::vector<std::string> &dumpInfo);
    void InitNecessaryState(const std::shared_ptr<AppExecFwk::EventRunner>& runner);

    std::atomic<bool> isReady_ {false};
    std::mutex suscriberLock_;
    sptr<SubscriberDeathRecipient> susriberDeathRecipient_ {nullptr};
    std::mutex expiredCallbackLock_;
    std::map<int32_t, sptr<IExpiredCallback>> expiredCallbackMap_;
    std::map<int32_t, std::shared_ptr<KeyInfo>> keyInfoMap_;
    sptr<ExpiredCallbackDeathRecipient> callbackDeathRecipient_ {nullptr};
    std::list<sptr<IBackgroundTaskSubscriber>> subscriberList_;

    std::shared_ptr<TimerManager> timerManager_ {nullptr};
    std::shared_ptr<Watchdog> watchdog_ {nullptr};
    std::shared_ptr<InputManager> inputManager_ {nullptr};
    std::shared_ptr<DeviceInfoManager> deviceInfoManeger_ {nullptr};
    std::shared_ptr<DecisionMaker> decisionMaker_ {nullptr};
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    std::mutex transientUidLock_;
    std::set<int32_t> transientPauseUid_ {};
};

class SubscriberDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    explicit SubscriberDeathRecipient(const wptr<BackgroundTaskMgrService>& service);
    ~SubscriberDeathRecipient() override;
    DISALLOW_COPY_AND_MOVE(SubscriberDeathRecipient);
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;

private:
    wptr<BackgroundTaskMgrService> service_;
};

class ExpiredCallbackDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    explicit ExpiredCallbackDeathRecipient(const wptr<BackgroundTaskMgrService>& service);
    ~ExpiredCallbackDeathRecipient() override;
    DISALLOW_COPY_AND_MOVE(ExpiredCallbackDeathRecipient);
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;

private:
    wptr<BackgroundTaskMgrService> service_;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_BG_TRANSIENT_TASK_MGR_H