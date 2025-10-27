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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_BG_CONTINUOUS_TASK_MGR_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_BG_CONTINUOUS_TASK_MGR_H

#include <memory>
#include <mutex>

#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "resource_manager.h"
#include "singleton.h"

#include "app_state_observer.h"
#include "bgtaskmgr_inner_errors.h"
#include "bundle_info.h"
#include "continuous_task_callback_info.h"
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
#include "task_notification_subscriber.h"
#endif
#include "continuous_task_info.h"
#include "background_task_mode.h"
#include "continuous_task_param.h"
#include "continuous_task_record.h"
#include "continuous_task_request.h"
#include "background_task_submode.h"
#include "ibackground_task_subscriber.h"
#include "remote_death_recipient.h"
#include "system_event_observer.h"
#include "config_change_observer.h"
#include "want.h"
#include "banner_notification_record.h"
#include "iexpired_callback.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
    static constexpr uint32_t SA_ID_VOIP_CALL_MANAGER = 65968;
    static constexpr uint32_t SA_ID_HEALTH_SPORT = 9527;
}
class BackgroundTaskMgrService;
class DataStorageHelper;
class AuthExpiredCallbackDeathRecipient;

enum class ContinuousTaskEventTriggerType: uint32_t {
    TASK_START,
    TASK_UPDATE,
    TASK_CANCEL,
    TASK_SUSPEND,
    TASK_ACTIVE,
};


struct CachedBundleInfo {
    std::unordered_map<std::string, uint32_t> abilityBgMode_ {};
    std::string appName_ {""};
};

struct SubscriberInfo {
    SubscriberInfo(sptr<IBackgroundTaskSubscriber> subscriber, int uid, int pid, bool isHap, uint32_t flag)
        : subscriber_(subscriber), uid_(uid), pid_(pid), isHap_(isHap), flag_(flag) {};
    sptr<IBackgroundTaskSubscriber> subscriber_;
    int uid_;
    int pid_;
    bool isHap_ {false};
    uint32_t flag_ {0};
};

class BgContinuousTaskMgr : public DelayedSingleton<BgContinuousTaskMgr>,
                            public std::enable_shared_from_this<BgContinuousTaskMgr> {
public:
    ErrCode StartBackgroundRunning(const sptr<ContinuousTaskParam> &taskParam);
    ErrCode UpdateBackgroundRunning(const sptr<ContinuousTaskParam> &taskParam);
    ErrCode StopBackgroundRunning(const std::string &abilityName, int32_t abilityId, int32_t continuousTaskId = -1);
    ErrCode GetAllContinuousTasks(std::vector<std::shared_ptr<ContinuousTaskInfo>> &list);
    ErrCode GetAllContinuousTasks(std::vector<std::shared_ptr<ContinuousTaskInfo>> &list, bool includeSuspended);
    ErrCode RequestBackgroundRunningForInner(const sptr<ContinuousTaskParamForInner> &taskParam);
    ErrCode RequestGetContinuousTasksByUidForInner(int32_t uid,
        std::vector<std::shared_ptr<ContinuousTaskInfo>> &list);
    ErrCode AddSubscriber(const std::shared_ptr<SubscriberInfo> subscriberInfo);
    ErrCode RemoveSubscriber(const sptr<IBackgroundTaskSubscriber> &subscriber);
    ErrCode ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    ErrCode GetContinuousTaskApps(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list, int32_t uid = -1);
    ErrCode AVSessionNotifyUpdateNotification(int32_t uid, int32_t pid, bool isPublish = false);
    ErrCode DebugContinuousTaskInner(const sptr<ContinuousTaskParamForInner> &taskParam);
    ErrCode IsModeSupported(const sptr<ContinuousTaskParam> &taskParam);
    ErrCode CheckTaskkeepingPermission(const sptr<ContinuousTaskParam> &taskParam,
        uint64_t callingTokenId, const std::string &bundleName, uint64_t fullTokenId);
    ErrCode RequestAuthFromUser(const sptr<ContinuousTaskParam> &taskParam, const sptr<IExpiredCallback> &callback,
        int32_t &notificationId);
    ErrCode CheckSpecialScenarioAuth(uint32_t &authResult);
    ErrCode CheckTaskAuthResult(const std::string &bundleName, int32_t userId, int32_t appIndex);
    bool StopContinuousTaskByUser(const std::string &mapKey, bool isSubNotification = false);
    bool StopBannerContinuousTaskByUser(const std::string &label);
    void OnAccountsStateChanged(int32_t id);
    void OnBundleInfoChanged(const std::string &action, const std::string &bundleName, int32_t uid);
    void OnAbilityStateChanged(int32_t uid, const std::string &abilityName, int32_t abilityId);
    void OnAppStopped(int32_t uid);
    void OnAppStateChanged(int32_t uid, int32_t state);
    void OnRemoteSubscriberDied(const wptr<IRemoteObject> &object);
    bool Init(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    void InitNecessaryState();
    void InitRequiredResourceInfo();
    void Clear();
    int32_t GetBgTaskUid();
    void StopContinuousTask(int32_t uid, int32_t pid, uint32_t taskType, const std::string &key);
    void SuspendContinuousTask(int32_t uid, int32_t pid, int32_t reason, const std::string &key);
    void SuspendContinuousAudioTask(int32_t uid);
    void ActiveContinuousTask(int32_t uid, int32_t pid, const std::string &key);
    void OnConfigurationChanged(const AppExecFwk::Configuration &configuration);
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId);
    void HandleRemoveTaskByMode(uint32_t mode);
    void OnBannerNotificationActionButtonClick(const int32_t buttonType, const int32_t uid,
        const std::string &label);
    void HandleAuthExpiredCallbackDeath(const wptr<IRemoteObject> &remote);
private:
    ErrCode StartBackgroundRunningInner(std::shared_ptr<ContinuousTaskRecord> &continuousTaskRecordPtr);
    ErrCode UpdateBackgroundRunningInner(const std::string &taskInfoMapKey,
        const sptr<ContinuousTaskParam> &taskParam);
    ErrCode UpdateTaskInfo(std::shared_ptr<ContinuousTaskRecord> record, const sptr<ContinuousTaskParam> &taskParam);
    ErrCode UpdateTaskNotification(std::shared_ptr<ContinuousTaskRecord> record,
        const sptr<ContinuousTaskParam> &taskParam);
    ErrCode UpdateBackgroundRunningByTaskIdInner(int32_t uid, const sptr<ContinuousTaskParam> &taskParam);
    ErrCode StartBackgroundRunningForInner(const sptr<ContinuousTaskParamForInner> &taskParam);
    ErrCode StopBackgroundRunningInner(int32_t uid, const std::string &abilityName, int32_t abilityId,
        int32_t continuousTaskId = -1);
    ErrCode StopBackgroundRunningForInner(const sptr<ContinuousTaskParamForInner> &taskParam);
    ErrCode GetAllContinuousTasksInner(int32_t uid, std::vector<std::shared_ptr<ContinuousTaskInfo>> &list,
        bool includeSuspended = true);
    ErrCode CheckIsSysReadyAndPermission(int32_t callingUid);
    ErrCode AddSubscriberInner(const std::shared_ptr<SubscriberInfo> subscriberInfo);
    ErrCode RemoveSubscriberInner(const sptr<IBackgroundTaskSubscriber> &subscriber);
    ErrCode ShellDumpInner(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    ErrCode SendContinuousTaskNotification(std::shared_ptr<ContinuousTaskRecord> &ContinuousTaskRecordPtr);
    ErrCode GetContinuousTaskAppsInner(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list, int32_t uid);
    ErrCode AVSessionNotifyUpdateNotificationInner(int32_t uid, int32_t pid, bool isPublish = false);
    ErrCode StopBackgroundRunningByContext(int32_t uid, const std::string &abilityName, int32_t abilityId);
    ErrCode StopBackgroundRunningByTask(const std::shared_ptr<ContinuousTaskRecord> &task);
    void HandlePersistenceData();
    void RestoreApplyRecord();
    void CheckPersistenceData(const std::vector<AppExecFwk::RunningProcessInfo> &allProcesses);
    void DumpAllTaskInfo(std::vector<std::string> &dumpInfo);
    void DumpCancelTask(const std::vector<std::string> &dumpOption, bool cleanAll);
    bool AddAppNameInfos(const AppExecFwk::BundleInfo &bundleInfo, CachedBundleInfo &cachedBundleInfo);
    bool CheckProcessUidInfo(const std::vector<AppExecFwk::RunningProcessInfo> &allProcesses, int32_t uid);
    uint32_t GetBackgroundModeInfo(int32_t uid, const std::string &abilityName);
    bool AddAbilityBgModeInfos(const AppExecFwk::BundleInfo &bundleInfo, CachedBundleInfo &cachedBundleInfo);
    bool RegisterNotificationSubscriber();
    bool RegisterSysCommEventListener();
    bool RegisterAppStateObserver();
    void UnregisterAppStateObserver();
    bool RegisterConfigurationObserver();
    bool GetNotificationPrompt();
    bool FormatBannerNotificationContext(const std::string &appName, std::string &bannerContent);
    bool SetCachedBundleInfo(int32_t uid, int32_t userId, const std::string &bundleName, const std::string &appName);
    void HandleStopContinuousTask(int32_t uid, int32_t pid, uint32_t taskType, const std::string &key);
    void HandleSuspendContinuousTask(int32_t uid, int32_t pid, int32_t reason, const std::string &key);
    void HandleSuspendContinuousAudioTask(int32_t uid);
    void HandleActiveContinuousTask(int32_t uid, int32_t pid, const std::string &key);
    void HandleActiveNotification(std::shared_ptr<ContinuousTaskRecord> record);
    void OnRemoteSubscriberDiedInner(const wptr<IRemoteObject> &object);
    void OnContinuousTaskChanged(const std::shared_ptr<ContinuousTaskRecord> continuousTaskInfo,
        ContinuousTaskEventTriggerType changeEventType);
    ErrCode CheckBgmodeType(uint32_t configuredBgMode, uint32_t requestedBgModeId, bool isNewApi,
        const std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord);
    bool AllowUseTaskKeeping(const std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord);
    ErrCode AllowUseSpecial(const std::shared_ptr<ContinuousTaskRecord> record);
    ErrCode CheckBgmodeTypeForInner(uint32_t requestedBgModeId);
    void InitRecordParam(std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord,
        const sptr<ContinuousTaskParam> &taskParam, int32_t userId);
    ErrCode CheckSubMode(const std::shared_ptr<AAFwk::Want> want, std::shared_ptr<ContinuousTaskRecord> record);
    ErrCode CheckNotificationText(std::string &notificationText,
        const std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord);
    ErrCode CheckSpecialNotificationText(std::string &notificationText,
        const std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord, uint32_t mode);
    int32_t RefreshTaskRecord();
    void HandleAppContinuousTaskStop(int32_t uid);
    bool checkPidCondition(const std::vector<AppExecFwk::RunningProcessInfo> &allProcesses, int32_t pid);
    bool checkNotificationCondition(const std::set<std::string> &notificationLabels, const std::string &label);
    std::shared_ptr<Global::Resource::ResourceManager> GetBundleResMgr(const AppExecFwk::BundleInfo &bundleInfo);
    std::string GetMainAbilityLabel(const std::string &bundleName, int32_t userId);
    std::string GetNotificationText(const std::shared_ptr<ContinuousTaskRecord> record);
    void RemoveContinuousTaskRecordByUidAndMode(int32_t uid, uint32_t mode);
    void RemoveContinuousTaskRecordByUid(int32_t uid);
    void ReclaimProcessMemory(int32_t pid);
    void SetReason(const std::string &mapKey, int32_t reason);
    uint32_t GetModeNumByTypeIds(const std::vector<uint32_t> &typeIds);
    void NotifySubscribers(ContinuousTaskEventTriggerType changeEventType,
        const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo);
    void NotifySubscribersTaskSuspend(const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo);
    void NotifySubscribersTaskActive(const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo);
    void ReportHisysEvent(ContinuousTaskEventTriggerType changeEventType,
        const std::shared_ptr<ContinuousTaskRecord> &continuousTaskInfo);
    bool CanNotifyHap(const std::shared_ptr<SubscriberInfo> subscriberInfo,
        const std::shared_ptr<ContinuousTaskCallbackInfo> &callbackInfo);
    bool IsExistCallback(int32_t uid, uint32_t type);
    ErrCode CheckCombinedTaskNotification(std::shared_ptr<ContinuousTaskRecord> &record, bool &sendNotification);
    bool StopContinuousTaskByUserInner(const std::string &key, bool isSubNotification);
    bool StopBannerContinuousTaskByUserInner(const std::string &label);
    ErrCode DetermineMatchCombinedTaskNotifacation(std::shared_ptr<ContinuousTaskRecord> recordParam,
        bool &sendNotification);
    ErrCode StartBackgroundRunningSubmit(std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord,
        std::string &taskInfoMapKey);
    ErrCode CheckAbilityTaskNum(const std::shared_ptr<ContinuousTaskRecord> record);
    bool CheckPermissionForInner(const sptr<ContinuousTaskParamForInner> &taskParam, int32_t callingUid);
    ErrCode AllowApplyContinuousTask(const std::shared_ptr<ContinuousTaskRecord> record);
    ErrCode SendLiveViewAndOtherNotification(std::shared_ptr<ContinuousTaskRecord> record);
    ErrCode SendNotification(const std::shared_ptr<ContinuousTaskRecord> subRecord,
        std::shared_ptr<ContinuousTaskRecord> record, const std::string &appName, bool isSubNotification);
    ErrCode SendBannerNotification(std::shared_ptr<ContinuousTaskRecord> record, const sptr<IExpiredCallback> &callback,
        int32_t &notificationId);
    void OnBannerNotificationActionButtonClickInner(const int32_t buttonType, const int32_t uid,
        const std::string &label);
    void CheckSpecialScenarioAuthInner(uint32_t &authResult, const std::string &bundleName,
        int32_t userId, int32_t appIndex);
    ErrCode CheckModeSupportedPermission(const sptr<ContinuousTaskParam> &taskParam);
    ErrCode CheckSpecialModePermission(const sptr<ContinuousTaskParam> &taskParam);
    void HandleAuthExpiredCallbackDeathInner(const wptr<IRemoteObject> &remote);
    ErrCode RefreshAuthRecord();
private:
    std::atomic<bool> isSysReady_ {false};
    int32_t bgTaskUid_ {-1};
    std::shared_ptr<AppExecFwk::EventHandler> handler_ {nullptr};
    std::unordered_map<std::string, std::shared_ptr<ContinuousTaskRecord>> continuousTaskInfosMap_ {};
    std::unordered_map<int32_t, bool> avSessionNotification_ {};

#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    std::shared_ptr<TaskNotificationSubscriber> subscriber_ {nullptr};
#endif
    std::shared_ptr<SystemEventObserver> systemEventListener_ {nullptr};
    sptr<AppStateObserver> appStateObserver_ {nullptr};
    sptr<AppExecFwk::IConfigurationObserver> configChangeObserver_ {nullptr};
    std::list<std::shared_ptr<SubscriberInfo>> bgTaskSubscribers_ {};
    sptr<RemoteDeathRecipient> susriberDeathRecipient_ {nullptr};
    std::unordered_map<int32_t, CachedBundleInfo> cachedBundleInfos_ {};
    std::unordered_map<int32_t, std::vector<uint32_t>> applyTaskOnForeground_ {};
    std::unordered_map<std::string, std::shared_ptr<BannerNotificationRecord>> bannerNotificationRecord_ {};
    std::set<int32_t> appOnForeground_ {};
    std::vector<std::string> continuousTaskText_ {};
    std::vector<std::string> continuousTaskSubText_ {};
    std::vector<std::string> bannerNotificaitonBtn_ {};
    sptr<AuthExpiredCallbackDeathRecipient> authCallbackDeathRecipient_ {nullptr};
    std::map<int32_t, sptr<IExpiredCallback>> expiredCallbackMap_;
    int32_t continuousTaskIdIndex_ = 0;

    DECLARE_DELAYED_SINGLETON(BgContinuousTaskMgr);
};

class AuthExpiredCallbackDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    explicit AuthExpiredCallbackDeathRecipient(const wptr<BackgroundTaskMgrService> &service);
    ~AuthExpiredCallbackDeathRecipient() override;
    DISALLOW_COPY_AND_MOVE(AuthExpiredCallbackDeathRecipient);
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

private:
    wptr<BackgroundTaskMgrService> service_;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_BG_CONTINUOUS_TASK_MGR_H