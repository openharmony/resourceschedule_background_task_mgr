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
#include "continuous_task_param.h"
#include "continuous_task_record.h"
#include "ibackground_task_subscriber.h"
#include "remote_death_recipient.h"
#include "system_event_observer.h"
#include "config_change_observer.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundTaskMgrService;
class DataStorageHelper;

enum class ContinuousTaskEventTriggerType: uint32_t {
    TASK_START,
    TASK_UPDATE,
    TASK_CANCEL,
};


struct CachedBundleInfo {
    std::unordered_map<std::string, uint32_t> abilityBgMode_ {};
    std::string appName_ {""};
};

class BgContinuousTaskMgr : public DelayedSingleton<BgContinuousTaskMgr>,
                            public std::enable_shared_from_this<BgContinuousTaskMgr> {
public:
    ErrCode StartBackgroundRunning(const sptr<ContinuousTaskParam> &taskParam);
    ErrCode UpdateBackgroundRunning(const sptr<ContinuousTaskParam> &taskParam);
    ErrCode StopBackgroundRunning(const std::string &abilityName, int32_t abilityId);
    ErrCode RequestBackgroundRunningForInner(const sptr<ContinuousTaskParamForInner> &taskParam);
    ErrCode AddSubscriber(const sptr<IBackgroundTaskSubscriber> &subscriber);
    ErrCode RemoveSubscriber(const sptr<IBackgroundTaskSubscriber> &subscriber);
    ErrCode ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    ErrCode GetContinuousTaskApps(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list);
    bool StopContinuousTaskByUser(const std::string &mapKey);
    void OnAccountsStateChanged(int32_t id);
    void OnBundleInfoChanged(const std::string &action, const std::string &bundleName, int32_t uid);
    void OnAbilityStateChanged(int32_t uid, const std::string &abilityName, int32_t abilityId);
    void OnProcessDied(int32_t uid, int32_t pid);
    void OnRemoteSubscriberDied(const wptr<IRemoteObject> &object);
    bool Init(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    void InitNecessaryState();
    void InitRequiredResourceInfo();
    void Clear();
    int32_t GetBgTaskUid();
    void StopContinuousTask(int32_t uid, int32_t pid, uint32_t taskType);
    void OnConfigurationChanged(const AppExecFwk::Configuration &configuration);

private:
    ErrCode StartBackgroundRunningInner(std::shared_ptr<ContinuousTaskRecord> &continuousTaskRecordPtr);
    ErrCode UpdateBackgroundRunningInner(const std::string &taskInfoMapKey, std::vector<uint32_t> &updateModes);
    ErrCode StartBackgroundRunningForInner(const sptr<ContinuousTaskParamForInner> &taskParam);
    ErrCode StopBackgroundRunningInner(int32_t uid, const std::string &abilityName, int32_t abilityId);
    ErrCode StopBackgroundRunningForInner(const sptr<ContinuousTaskParamForInner> &taskParam);
    ErrCode AddSubscriberInner(const sptr<IBackgroundTaskSubscriber> &subscriber);
    ErrCode RemoveSubscriberInner(const sptr<IBackgroundTaskSubscriber> &subscriber);
    ErrCode ShellDumpInner(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    ErrCode SendContinuousTaskNotification(std::shared_ptr<ContinuousTaskRecord> &ContinuousTaskRecordPtr);
    ErrCode GetContinuousTaskAppsInner(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list);
    void HandlePersistenceData();
    void CheckPersistenceData(const std::vector<AppExecFwk::RunningProcessInfo> &allProcesses,
        const std::set<std::string> &allLabels);
    void DumpAllTaskInfo(std::vector<std::string> &dumpInfo);
    void DumpCancelTask(const std::vector<std::string> &dumpOption, bool cleanAll);
    bool RemoveContinuousTaskRecord(const std::string &mapKey);
    bool AddAppNameInfos(const AppExecFwk::BundleInfo &bundleInfo, CachedBundleInfo &cachedBundleInfo);
    bool CheckProcessUidInfo(const std::vector<AppExecFwk::RunningProcessInfo> &allProcesses, int32_t uid);
    uint32_t GetBackgroundModeInfo(int32_t uid, const std::string &abilityName);
    bool AddAbilityBgModeInfos(const AppExecFwk::BundleInfo &bundleInfo, CachedBundleInfo &cachedBundleInfo);
    bool RegisterNotificationSubscriber();
    bool RegisterSysCommEventListener();
    bool RegisterAppStateObserver();
    bool RegisterConfigurationObserver();
    bool GetNotificationPrompt();
    bool SetCachedBundleInfo(int32_t uid, int32_t userId, const std::string &bundleName, const std::string &appName);
    void HandleStopContinuousTask(int32_t uid, int32_t pid, uint32_t taskType);
    void RemoveSpecifiedBgTask(uint32_t taskType);
    void OnRemoteSubscriberDiedInner(const wptr<IRemoteObject> &object);
    void OnContinuousTaskChanged(const std::shared_ptr<ContinuousTaskRecord> continuousTaskInfo,
        ContinuousTaskEventTriggerType changeEventType);
    ErrCode CheckBgmodeType(uint32_t configuredBgMode, uint32_t requestedBgModeId, bool isNewApi, uint64_t fullTokenId);
    ErrCode CheckBgmodeTypeForInner(uint32_t requestedBgModeId);
    int32_t RefreshTaskRecord();
    void HandleAppContinuousTaskStop(int32_t uid);
    bool checkPidCondition(const std::vector<AppExecFwk::RunningProcessInfo> &allProcesses, int32_t pid);
    bool checkNotificationCondition(const std::set<std::string> &notificationLabels, const std::string &label);
    std::shared_ptr<Global::Resource::ResourceManager> GetBundleResMgr(const AppExecFwk::BundleInfo &bundleInfo);
    std::string GetMainAbilityLabel(const std::string &bundleName, int32_t userId);

private:
    std::atomic<bool> isSysReady_ {false};
    std::string deviceType_ {""};
    int32_t bgTaskUid_ {-1};
    std::shared_ptr<AppExecFwk::EventHandler> handler_ {nullptr};
    std::unordered_map<std::string, std::shared_ptr<ContinuousTaskRecord>> continuousTaskInfosMap_ {};
    std::unordered_map<std::string, std::shared_ptr<ContinuousTaskRecord>> continuousTaskInfosMapForBatch_ {};
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    std::shared_ptr<TaskNotificationSubscriber> subscriber_ {nullptr};
#endif
    std::shared_ptr<SystemEventObserver> systemEventListener_ {nullptr};
    std::shared_ptr<AppStateObserver> appStateObserver_ {nullptr};
    sptr<AppExecFwk::IConfigurationObserver> configChangeObserver_ {nullptr};
    std::list<sptr<IBackgroundTaskSubscriber>> bgTaskSubscribers_ {};
    std::map<sptr<IRemoteObject>, sptr<RemoteDeathRecipient>> subscriberRecipients_ {};
    std::unordered_map<int32_t, CachedBundleInfo> cachedBundleInfos_ {};
    std::vector<std::string> continuousTaskText_ {};

    DECLARE_DELAYED_SINGLETON(BgContinuousTaskMgr);
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_BG_CONTINUOUS_TASK_MGR_H