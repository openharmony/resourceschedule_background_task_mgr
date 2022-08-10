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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_BG_EFFICIENCY_RESOURCES_MGR_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_BG_EFFICIENCY_RESOURCES_MGR_H

#include <memory>
#include <mutex>
#include <list>
<<<<<<< Updated upstream
=======
#include <unordered_map>

>>>>>>> Stashed changes
#include "ipc_skeleton.h"
#include "singleton.h"
#include "bgtaskmgr_inner_errors.h"
#include "event_runner.h"
#include "event_handler.h"
#include "event_info.h"
#include "remote_death_recipient.h"

#include "efficiency_resource_info.h"
<<<<<<< Updated upstream
#include "application_record_storage.h"
=======
#include "resource_record_storage.h"
>>>>>>> Stashed changes
#include "ibackground_task_subscriber.h"
#include "resource_callback_info.h"
#include "bundle_manager_helper.h"
#include "resources_application_record.h"
<<<<<<< Updated upstream
#include "remote_death_recipient.h"

namespace OHOS {
namespace BackgroundTaskMgr {


enum class EfficiencyResourcesEventType: uint32_t {
    APP_RESOURCE_APPLY,
    APP_RESOURCE_RESET,
    RESOURCE_APPLY,
    RESOURCE_RESET,
};

=======
#include "resources_subscriber_mgr.h"
#include "app_state_observer.h"
#include "running_process_info.h"
namespace OHOS {
namespace BackgroundTaskMgr {

>>>>>>> Stashed changes
class BgEfficiencyResourcesMgr : public DelayedSingleton<BgEfficiencyResourcesMgr>,
                            public std::enable_shared_from_this<BgEfficiencyResourcesMgr> {
public:
    ErrCode ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    bool Init();
    void InitNecessaryState();
    void Clear();
    ErrCode ApplyEfficiencyResources(const sptr<EfficiencyResourceInfo> &resourceInfo, bool &isSuccess);
    ErrCode ResetAllEfficiencyResources();
<<<<<<< Updated upstream
    ErrCode AddSubscriber(const sptr<IBackgroundTaskSubscriber> &subscriber);
    ErrCode RemoveSubscriber(const sptr<IBackgroundTaskSubscriber> &subscriber);
=======
    ErrCode AddSubscriber(const sptr<IBackgroundTaskSubscriber>& subscriber);
    ErrCode RemoveSubscriber(const sptr<IBackgroundTaskSubscriber>& subscriber);
    void RemoveProcessRecord(int32_t pid);
    void RemoveAppRecord(int32_t uid);
    void HandlePersistenceData();
    void CheckPersistenceData(const std::vector<AppExecFwk::RunningProcessInfo> &allProcesses);
>>>>>>> Stashed changes

private:
    ErrCode ApplyEfficiencyResourcesInner(const std::shared_ptr<ResourceCallbackInfo> &callbackInfo,
        const sptr<EfficiencyResourceInfo> &resourceInfo);
<<<<<<< Updated upstream
    void BgEfficiencyResourcesMgr::UpdateResourcesEndtime(const std::shared_ptr<ResourceCallbackInfo> 
        &callbackInfo, std::shared_ptr<ResourceApplicationRecord> &record, bool isPersist, int32_t timeOut);
    ErrCode ResetEfficiencyResourcesInner(const std::shared_ptr<ResourceCallbackInfo> &callbackInfo,
        bool isProcess);
    ErrCode ResetAllEfficiencyResourcesInner(const std::shared_ptr<ResourceCallbackInfo> &callbackInfo,
        bool isProcess);
=======
    void UpdateResourcesEndtime(const std::shared_ptr<ResourceCallbackInfo> &callbackInfo,
        std::shared_ptr<ResourceApplicationRecord> &record, bool isPersist, int32_t timeOut, bool isProcess);
    void ResetEfficiencyResourcesInner(const std::shared_ptr<ResourceCallbackInfo> &callbackInfo,
        bool isProcess);
    ErrCode ResetAllEfficiencyResourcesInner(const std::shared_ptr<ResourceCallbackInfo> &callbackInfo,
        bool isProcess);
    void RemoveRelativeProcessRecord(int32_t uid, uint32_t resourceNumber);
>>>>>>> Stashed changes
    ErrCode ShellDumpInner(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    void DumpAllApplicationInfo(std::vector<std::string> &dumpInfo);
    void DumpResetAllResource(const std::vector<std::string> &dumpOption);
    void DumpResetResource(const std::vector<std::string> &dumpOption, bool cleanApp, bool cleanAll);
<<<<<<< Updated upstream
    void DumpApplicationInfoMap(std::unordered_map<std::string,
        std::shared_ptr<ResourceApplicationRecord>> &infoMap, std::vector<std::string> &dumpInfo,
        std::stringstream &stream, const char *headInfo);
    void ResetTimeOutResource(std::string mapKey, uint32_t timeOutBit, bool isProcess);
    bool RemoveSingleResourceRecord(std::unordered_map<std::string,
        std::shared_ptr<ResourceApplicationRecord>> &infoMap, const std::string &mapKey, 
        uint32_t cleanResource, EfficiencyResourcesEventType type);
    bool GetBundleNamesForUid(int32_t uid, std::string &bundleName);
    bool VerifyCallingInfo(int32_t uid, int32_t pid);
    bool IsCallingInfoLegal(int32_t uid, int32_t pid, std::string &bundleName);
    void OnResourceChanged(const std::shared_ptr<ResourceCallbackInfo> &callbackInfo,
        EfficiencyResourcesEventType type);
    void OnRemoteSubscriberDied(const wptr<IRemoteObject> &object);
    
=======
    void DumpApplicationInfoMap(std::unordered_map<int32_t,
        std::shared_ptr<ResourceApplicationRecord>> &infoMap, std::vector<std::string> &dumpInfo,
        std::stringstream &stream, const char *headInfo);
    void ResetTimeOutResource(int32_t mapKey, uint32_t timeOutBit, bool isProcess);
    bool RemoveTargetResourceRecord(std::unordered_map<int32_t,
        std::shared_ptr<ResourceApplicationRecord>> &infoMap, int32_t mapKey, 
        uint32_t cleanResource, EfficiencyResourcesEventType type);
    bool GetBundleNamesForUid(int32_t uid, std::string &bundleName);
    bool IsCallingInfoLegal(int32_t uid, int32_t pid, std::string &bundleName);
    bool RegisterAppStateObserver();

>>>>>>> Stashed changes
private:
    std::atomic<bool> isSysReady_ {false};
    std::shared_ptr<AppExecFwk::EventRunner> runner_ {nullptr};
    std::shared_ptr<AppExecFwk::EventHandler> handler_ {nullptr};
<<<<<<< Updated upstream
    std::list<sptr<IBackgroundTaskSubscriber>> bgTaskSubscribers_ {};
    std::map<sptr<IRemoteObject>, sptr<RemoteDeathRecipient>> subscriberRecipients_ {};
    std::list<sptr<IBackgroundTaskSubscriber>> subscriberList_;
    sptr<OHOS::AppExecFwk::IBundleMgr> bundleMgrProxy_;
    std::unordered_map<std::string, std::shared_ptr<ResourceApplicationRecord>>appResourceApplyMap_ {};
    std::unordered_map<std::string, std::shared_ptr<ResourceApplicationRecord>> resourceApplyMap_ {};
    std::mutex callbackLock_;
    std::mutex subscriberLock_;
    sptr<RemoteDeathRecipient> susriberDeathRecipient_ {nullptr};
    std::shared_ptr<ApplicationRecordStorage> dataStorage_ {nullptr};
=======
    std::mutex callbackLock_;
    std::unordered_map<int32_t, std::shared_ptr<ResourceApplicationRecord>> appResourceApplyMap_ {};
    std::unordered_map<int32_t, std::shared_ptr<ResourceApplicationRecord>> resourceApplyMap_ {};
    std::shared_ptr<ResourceRecordStorage> recordStorage_ {nullptr};
    std::shared_ptr<AppStateObserver> appStateObserver_ {nullptr};
    std::shared_ptr<ResourcesSubscriberMgr> subscriberMgr_ {nullptr};

>>>>>>> Stashed changes
    DECLARE_DELAYED_SINGLETON(BgEfficiencyResourcesMgr);
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_BG_EFFICIENCY_RESOURCES_MGR_H