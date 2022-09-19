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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_EFFICIENCY_RES_MGR_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_EFFICIENCY_RES_MGR_H

#include <memory>
#include <list>
#include <unordered_map>

#include "ipc_skeleton.h"
#include "singleton.h"
#include "event_runner.h"
#include "event_handler.h"
#include "event_info.h"
#include "running_process_info.h"
#include "app_mgr_client.h"

#include "efficiency_resource_info.h"
#include "ibackground_task_subscriber.h"
#include "resource_callback_info.h"
#include "bundle_manager_helper.h"
#include "resource_application_record.h"
#include "resources_subscriber_mgr.h"
#include "app_state_observer.h"
#include "data_storage_helper.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class DataStorageHelper;
class BgEfficiencyResourcesMgr : public DelayedSingleton<BgEfficiencyResourcesMgr>,
                            public std::enable_shared_from_this<BgEfficiencyResourcesMgr> {
using ResourceRecordMap = std::unordered_map<int32_t, std::shared_ptr<ResourceApplicationRecord>>;
using ResourceRecordPair = std::pair<const int32_t, std::shared_ptr<
    OHOS::BackgroundTaskMgr::ResourceApplicationRecord>>;
public:
    ErrCode ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    bool Init();
    void InitNecessaryState();
    void Clear();
    ErrCode ApplyEfficiencyResources(const sptr<EfficiencyResourceInfo> &resourceInfo);
    ErrCode ResetAllEfficiencyResources();
    ErrCode GetEfficiencyResourcesInfos(std::vector<std::shared_ptr<ResourceCallbackInfo>> &appList,
        std::vector<std::shared_ptr<ResourceCallbackInfo>> &procList);
    ErrCode AddSubscriber(const sptr<IBackgroundTaskSubscriber> &subscriber);
    ErrCode RemoveSubscriber(const sptr<IBackgroundTaskSubscriber> &subscriber);
    ErrCode RemoveProcessRecord(int32_t uid, int32_t pid, const std::string &bundleName);
    ErrCode RemoveAppRecord(int32_t uid, const std::string &bundleName);
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId);
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId);

private:
    void ApplyEfficiencyResourcesInner(std::shared_ptr<ResourceCallbackInfo> callbackInfo,
        const sptr<EfficiencyResourceInfo> &resourceInfo);
    void UpdateResourcesEndtime(const std::shared_ptr<ResourceCallbackInfo> &callbackInfo,
        std::shared_ptr<ResourceApplicationRecord> &record, const sptr<EfficiencyResourceInfo> &resourceInfo);
    void ResetEfficiencyResourcesInner(const std::shared_ptr<ResourceCallbackInfo> &callbackInfo,
        bool isProcess);
    ErrCode ResetAllEfficiencyResourcesInner(const std::shared_ptr<ResourceCallbackInfo> &callbackInfo,
        bool isProcess);
    void RemoveRelativeProcessRecord(int32_t uid, uint32_t resourceNumber);
    ErrCode ShellDumpInner(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    void DumpAllApplicationInfo(std::vector<std::string> &dumpInfo);
    void DumpResetAllResource(const std::vector<std::string> &dumpOption);
    void DumpResetResource(const std::vector<std::string> &dumpOption, bool cleanApp, bool cleanAll);
    void DumpApplicationInfoMap(std::unordered_map<int32_t,
        std::shared_ptr<ResourceApplicationRecord>> &infoMap, std::vector<std::string> &dumpInfo,
        std::stringstream &stream, const char *headInfo);
    void ResetTimeOutResource(int32_t mapKey, bool isProcess);
    bool RemoveTargetResourceRecord(std::unordered_map<int32_t,
        std::shared_ptr<ResourceApplicationRecord>> &infoMap, int32_t mapKey,
        uint32_t cleanResource, EfficiencyResourcesEventType type);
    bool GetBundleNamesForUid(int32_t uid, std::string &bundleName);
    bool IsCallingInfoLegal(int32_t uid, int32_t pid, std::string &bundleName);
    void EraseRecordIf(ResourceRecordMap &infoMap, const std::function<bool(ResourceRecordPair)> &fun);
    void RecoverDelayedTask(bool isProcess, ResourceRecordMap& infoMap);
    void RegisterAppStateObserver();
    void HandlePersistenceData();
    void CheckPersistenceData(const std::vector<AppExecFwk::RunningProcessInfo> &allProcesses);
    void RemoveListRecord(std::list<PersistTime> &resourceUnitList, uint32_t eraseBit);
    void GetEfficiencyResourcesInfosInner(const ResourceRecordMap &infoMap,
        std::vector<std::shared_ptr<ResourceCallbackInfo>> &list);
    bool CheckRunningResourcesApply(const int32_t uid, const std::string &bundleName);
    bool CheckAlivedApp(int32_t uid);
    int32_t GetUserIdByUid(int32_t uid);

private:
    std::atomic<bool> isSysReady_ {false};
    std::shared_ptr<AppExecFwk::EventRunner> runner_ {nullptr};
    std::shared_ptr<AppExecFwk::EventHandler> handler_ {nullptr};
    std::unordered_map<int32_t, std::shared_ptr<ResourceApplicationRecord>> appResourceApplyMap_ {};
    std::unordered_map<int32_t, std::shared_ptr<ResourceApplicationRecord>> procResourceApplyMap_ {};
    std::shared_ptr<AppStateObserver> appStateObserver_ {nullptr};
    std::shared_ptr<ResourcesSubscriberMgr> subscriberMgr_ {nullptr};
    std::unique_ptr<AppExecFwk::AppMgrClient> appMgrClient_ {nullptr};
    uint32_t dependsReady_ = 0;

    DECLARE_DELAYED_SINGLETON(BgEfficiencyResourcesMgr);
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_EFFICIENCY_RES_MGR_H