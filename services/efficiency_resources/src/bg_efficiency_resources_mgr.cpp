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

#include <set>
#include <algorithm>

#include "bg_efficiency_resources_mgr.h"
#include "app_mgr_client.h"
#include "event_runner.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "bgtaskmgr_inner_errors.h"

#include "efficiency_resource_log.h"
#include "resource_type.h"
#include "time_provider.h"
#include "bundle_manager_helper.h"
namespace OHOS {
namespace BackgroundTaskMgr {

static constexpr char BG_EFFICIENCY_RESOURCES_MGR_NAME[] = "BgEfficiencyResourcesMgr";
static constexpr char DUMP_PARAM_LIST_ALL[] = "--all";
static constexpr char DUMP_PARAM_RESET_ALL[] = "--reset_all";
static constexpr char DUMP_PARAM_RESET_APP[] = "--resetapp";
static constexpr char DUMP_PARAM_RESET_PROC[] = "--resetproc";
static constexpr int32_t DELAY_TIME = 2000;
static constexpr int32_t MAX_DUMP_PARAM_NUMS = 4;
static constexpr int32_t MAX_RESOURCES_TYPE_NUM = 7;
static constexpr int32_t MAX_RESOURCE_NUMBER = (1 << MAX_RESOURCES_TYPE_NUM) - 1;

BgEfficiencyResourcesMgr::BgEfficiencyResourcesMgr() {
    subscriberMgr_ = DelayedSingleton<ResourcesSubscriberMgr>::GetInstance();
}

BgEfficiencyResourcesMgr::~BgEfficiencyResourcesMgr() {}

bool BgEfficiencyResourcesMgr::Init()
{
    runner_ = AppExecFwk::EventRunner::Create(BG_EFFICIENCY_RESOURCES_MGR_NAME);
    if (runner_ == nullptr) {
        BGTASK_LOGE("BgEfficiencyResourcesMgr runner create failed!");
        return false;
    }
    handler_ = std::make_shared<AppExecFwk::EventHandler>(runner_);
    if (handler_ == nullptr) {
        BGTASK_LOGE("BgEfficiencyResourcesMgr handler create failed!");
        return false;
    }
    InitNecessaryState();
    return true;
}

void BgEfficiencyResourcesMgr::InitNecessaryState()
{
    sptr<ISystemAbilityManager> systemAbilityManager
        = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr
        || systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) == nullptr
        || systemAbilityManager->CheckSystemAbility(APP_MGR_SERVICE_ID) == nullptr) {
        BGTASK_LOGW("request system service is not ready yet!");
        auto task = [this](){this->InitNecessaryState(); };
        handler_->PostTask(task, DELAY_TIME);
        return;
    }
    if (!RegisterAppStateObserver()) {
        BGTASK_LOGE("BgEfficiencyResourcesMgr RegisterAppStateObserver failed");
        return;
    }
    HandlePersistenceData();
    isSysReady_.store(true);
}

bool BgEfficiencyResourcesMgr::RegisterAppStateObserver()
{
    bool res = true;
    appStateObserver_ = DelayedSingleton<AppStateObserver>::GetInstance();
    if (appStateObserver_ != nullptr) {
        appStateObserver_->SetBgEfficiencyResourcesMgr(shared_from_this());
        res = appStateObserver_->Subscribe();
    }
    return res;
}

//To be reconstruction
void BgEfficiencyResourcesMgr::HandlePersistenceData()
{
    recordStorage_ = std::make_shared<ResourceRecordStorage>();
    BGTASK_LOGI("service restart, restore data");
    recordStorage_->RestoreResourceRecord(appResourceApplyMap_, resourceApplyMap_);
    auto appMgrClient = std::make_shared<AppExecFwk::AppMgrClient>();
    if (appMgrClient == nullptr || appMgrClient->ConnectAppMgrService() != ERR_OK) {
        BGTASK_LOGW("connect to app mgr service failed");
        return;
    }
    std::vector<AppExecFwk::RunningProcessInfo> allAppProcessInfos;
    appMgrClient->GetAllRunningProcesses(allAppProcessInfos);
    std::lock_guard<std::mutex> lock(callbackLock_);
    CheckPersistenceData(allAppProcessInfos);
    RecoverDelayedTask(true, resourceApplyMap_);
    RecoverDelayedTask(false, appResourceApplyMap_);
    recordStorage_->RefreshResourceRecord(appResourceApplyMap_, resourceApplyMap_);
}

void BgEfficiencyResourcesMgr::EraseRecordIf(ResourceRecordMap &infoMap,const 
    std::function<bool(ResourceRecordPair)> &fun)
{
    for (auto iter = infoMap.begin(); iter != infoMap.end();) {
        if (fun(*iter)) {
            infoMap.erase(iter++);
        } else {
            iter++;
        }
    }
}

void BgEfficiencyResourcesMgr::CheckPersistenceData(const std::vector<AppExecFwk::RunningProcessInfo> &allProcesses)
{
    std::set<int32_t> runningUid;
    std::set<int32_t> runningPid;
    std::for_each(allProcesses.begin(), allProcesses.end(), [&runningUid, &runningPid](const auto &iter) {
        runningUid.emplace(iter.uid_);
        runningPid.emplace(iter.pid_);
    } );
    auto removeUid = [&runningUid](const auto &iter) { return runningUid.find(iter.first) == runningUid.end(); };
    EraseRecordIf(appResourceApplyMap_, removeUid);
    auto removePid = [&runningPid](const auto &iter)  { return runningPid.find(iter.first) == runningPid.end(); };
    EraseRecordIf(appResourceApplyMap_, removePid);
}


void BgEfficiencyResourcesMgr::RecoverDelayedTask(bool isProcess, ResourceRecordMap& infoMap)
{
    const auto &mgr = shared_from_this();
    for (auto iter = infoMap.begin(); iter != infoMap.end(); iter ++) {
        auto &resourceList = iter->second->resourceUnitList_;
        int32_t mapKey = iter->first; 
        for (auto resourceIter = resourceList.begin(); resourceIter != resourceList.end(); resourceIter++) {
            if (resourceIter->isPersist_) {
                continue;
            }
            uint32_t timeOutBit = resourceIter->resourceIndex_;
            auto task = [mgr, mapKey, timeOutBit, isProcess] () {
                mgr->ResetTimeOutResource(mapKey, timeOutBit, isProcess);
            };
            int32_t timeOut = static_cast<int32_t>(resourceIter->endTime_-TimeProvider::GetCurrentTime());
            handler_->PostTask(task, std::max(0, timeOut));
        }
    }    
}

void BgEfficiencyResourcesMgr::RemoveAppRecord(int32_t uid)
{
    std::lock_guard<std::mutex> lock(callbackLock_);
    EraseRecordIf(appResourceApplyMap_, [uid](const auto &iter) { return iter.first == uid; });
    recordStorage_->RefreshResourceRecord(appResourceApplyMap_, resourceApplyMap_);
}

void BgEfficiencyResourcesMgr::RemoveProcessRecord(int32_t pid)
{   
    std::lock_guard<std::mutex> lock(callbackLock_);
    EraseRecordIf(resourceApplyMap_, [pid](const auto &iter) { return iter.first == pid; });
    recordStorage_->RefreshResourceRecord(appResourceApplyMap_, resourceApplyMap_);
}

void BgEfficiencyResourcesMgr::Clear()
{
    if (appStateObserver_ != nullptr) {
        appStateObserver_->Unsubscribe();
    }
}

bool CheckResourceInfo(const sptr<EfficiencyResourceInfo> &resourceInfo)
{
    if (!resourceInfo) {
        BGTASK_LOGE("apply efficiency resource request params is null!");
        return false;
    }
    if (resourceInfo->GetResourceNumber() == 0 || resourceInfo->GetResourceNumber() > MAX_RESOURCE_NUMBER
        || (!resourceInfo->IsPersist() && resourceInfo->GetTimeOut() <= 0)) {
        BGTASK_LOGE("efficiency resources params invalid!");
        return false;
    }
    return true;
}

ErrCode BgEfficiencyResourcesMgr::ApplyEfficiencyResources(
        const sptr<EfficiencyResourceInfo> &resourceInfo, bool &isSuccess)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("Efficiency resources manager is not ready.");
        return ERR_BGTASK_SERVICE_NOT_READY;
    }

    if (!CheckResourceInfo(resourceInfo)) {
        return ERR_BGTASK_INVALID_PARAM;
    }

    auto uid = IPCSkeleton::GetCallingUid();
    auto pid = IPCSkeleton::GetCallingPid();
    std::string bundleName = "";
    if (!IsCallingInfoLegal(uid, pid, bundleName)) {
        BGTASK_LOGI("apply efficiency resources failed, calling info is illegal.");
        return ERR_BGTASK_INVALID_PARAM;
    }
    BGTASK_LOGI("apply efficiency resources uid : %{public}d, pid : %{public}d, bundle name : %{public}s", uid,
        pid, bundleName.c_str());
    isSuccess = true;
    std::shared_ptr<ResourceCallbackInfo> callbackInfo = std::make_shared<ResourceCallbackInfo>(uid,
        pid, resourceInfo->GetResourceNumber(), bundleName);
    if (resourceInfo->IsApply()) {
        ApplyEfficiencyResourcesInner(callbackInfo, resourceInfo);
    } else {
        ResetEfficiencyResourcesInner(callbackInfo, resourceInfo->IsProcess());
    }
    recordStorage_->RefreshResourceRecord(appResourceApplyMap_, resourceApplyMap_);
    return ERR_OK;
}

void BgEfficiencyResourcesMgr::ApplyEfficiencyResourcesInner(const std::shared_ptr<ResourceCallbackInfo> 
    &callbackInfo, const sptr<EfficiencyResourceInfo> &resourceInfo)
{
    std::lock_guard<std::mutex> lock(callbackLock_);
    int32_t mapKey = !resourceInfo->IsProcess() ? callbackInfo->GetUid() : callbackInfo->GetPid();
    auto &infoMap = !resourceInfo->IsProcess() ? appResourceApplyMap_ : resourceApplyMap_;
    auto iter = infoMap.find(mapKey);
    if (iter == infoMap.end()) {
        infoMap.emplace(mapKey, std::make_shared<ResourceApplicationRecord>(callbackInfo->GetUid(),
            callbackInfo->GetPid(), callbackInfo->GetResourceNumber(), callbackInfo->GetBundleName()));
    } else {
        iter->second->reason_ = resourceInfo->GetReason();
        iter->second->resourceNumber_ &= callbackInfo->GetResourceNumber();
    }
    UpdateResourcesEndtime(callbackInfo, iter->second, resourceInfo->IsPersist(),
        resourceInfo->GetTimeOut(), resourceInfo->IsProcess());
    subscriberMgr_->OnResourceChanged(callbackInfo, !resourceInfo->IsProcess() ?
        EfficiencyResourcesEventType::APP_RESOURCE_APPLY : EfficiencyResourcesEventType::RESOURCE_APPLY);
    recordStorage_->RefreshResourceRecord(appResourceApplyMap_, resourceApplyMap_);
}

void BgEfficiencyResourcesMgr::UpdateResourcesEndtime(const std::shared_ptr<ResourceCallbackInfo> 
    &callbackInfo, std::shared_ptr<ResourceApplicationRecord> &record, bool isPersist, int32_t timeOut, bool isProcess){
    uint32_t timeOutBit = 0;
    for (int resourceIndex = 0; resourceIndex < MAX_RESOURCES_TYPE_NUM; ++resourceIndex) {
        if ((callbackInfo->GetResourceNumber() & (1 << resourceIndex)) != 0) {
            auto task = [resourceIndex](const auto &it){
                return it.resourceIndex_ == resourceIndex;
            };
            auto resourceUnitIter = std::find_if(record->resourceUnitList_.begin(),
                record->resourceUnitList_.end(), task);
            int64_t lastTime = 0; 
            if (resourceUnitIter == record->resourceUnitList_.end()) {
                record->resourceUnitList_.emplace_back(PersistTime{resourceIndex, isPersist,
                    TimeProvider::GetCurrentTime() + timeOut});
            } else {
                resourceUnitIter->isPersist_ = resourceUnitIter->isPersist_ || isPersist;
                lastTime = resourceUnitIter->endTime_;
                resourceUnitIter->endTime_ = std::max(resourceUnitIter->endTime_,
                    TimeProvider::GetCurrentTime() + timeOut);
            }
            resourceUnitIter = std::find_if(record->resourceUnitList_.begin(), record->resourceUnitList_.end(), task);
            if (resourceUnitIter->isPersist_) {
                resourceUnitIter->endTime_ = 0;
            } else if (resourceUnitIter->endTime_ > lastTime) {
                timeOutBit |= (1 << resourceIndex);
            }
        }
    }
    int32_t mapKey = !isProcess ? callbackInfo->GetUid() : callbackInfo->GetPid();
    const auto& mgr = shared_from_this();
    auto task = [mgr, mapKey, timeOutBit, isProcess] () {
        mgr->ResetTimeOutResource(mapKey, timeOutBit, isProcess);
    };
    handler_->PostTask(task, timeOut);
    subscriberMgr_->OnResourceChanged(callbackInfo, !isProcess ? 
        EfficiencyResourcesEventType::APP_RESOURCE_RESET : EfficiencyResourcesEventType::RESOURCE_RESET);
}

void BgEfficiencyResourcesMgr::ResetTimeOutResource(int32_t mapKey, uint32_t timeOutBit, bool isProcess){
    if (!isSysReady_.load()) {
        BGTASK_LOGW("Efficiency resources manager is not ready.");
        return;
    }
    std::lock_guard<std::mutex> lock(callbackLock_);
    auto &infoMap = !isProcess ? appResourceApplyMap_ : resourceApplyMap_;
    auto type = !isProcess ? EfficiencyResourcesEventType::APP_RESOURCE_RESET : 
        EfficiencyResourcesEventType::RESOURCE_RESET;
    auto iter = infoMap.find(mapKey);
    if (iter == infoMap.end()) {
        BGTASK_LOGD("Efficiency resource does not exist.");
        return;
    }
    auto &resourceRecord = iter->second;
    uint32_t resetZeros = 0;
    for (int resourceIndex = 0; resourceIndex < MAX_RESOURCES_TYPE_NUM; ++resourceIndex) {
        if ((resourceRecord->resourceNumber_ & 1 << resourceIndex) ==0 || 
            (timeOutBit & 1 << resourceIndex) == 0) {
            continue;
        }
        auto resourceUnitIter = std::find_if(resourceRecord->resourceUnitList_.begin(), 
            resourceRecord->resourceUnitList_.end(), [resourceIndex](const auto &it)
            { return it.resourceIndex_ == resourceIndex; });
        if (resourceUnitIter != resourceRecord->resourceUnitList_.end()) {
            auto endTime = resourceUnitIter->endTime_;
            if (TimeProvider::GetCurrentTime() >= endTime) {
                resetZeros |= 1 << resourceIndex;
            }
        }
    }
    resourceRecord->resourceNumber_ ^= resetZeros;
    auto callbackInfo = std::make_shared<ResourceCallbackInfo>(resourceRecord->uid_, resourceRecord->pid_, resetZeros,
        resourceRecord->bundleName_);
    subscriberMgr_->OnResourceChanged(callbackInfo, type);
    if(resourceRecord->resourceNumber_ == 0) {
        infoMap.erase(iter);
    }
}

ErrCode BgEfficiencyResourcesMgr::ResetAllEfficiencyResources()
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("Efficiency resources manager is not ready.");
        return ERR_BGTASK_SERVICE_NOT_READY;
    }

    auto uid = IPCSkeleton::GetCallingUid();
    auto pid = IPCSkeleton::GetCallingPid();
    std::string bundleName = "";
    if (!IsCallingInfoLegal(uid, pid, bundleName)) {
        BGTASK_LOGI("reset efficiency resources failed, calling info is illegal.");
        return ERR_BGTASK_INVALID_PARAM;
    }
    BGTASK_LOGI("reset efficiency resources uid : %{public}d, pid : %{public}d, bundle name : %{public}s", uid,
        pid, bundleName.c_str());
    std::shared_ptr<ResourceCallbackInfo> callbackInfo = std::make_shared<ResourceCallbackInfo>(uid,
        pid, MAX_RESOURCE_NUMBER, bundleName);
    ResetEfficiencyResourcesInner(callbackInfo, false);
    return ERR_OK;
}

void BgEfficiencyResourcesMgr::RemoveRelativeProcessRecord(int32_t uid, uint32_t resourceNumber)
{
    for(auto iter = resourceApplyMap_.begin(); iter != resourceApplyMap_.end(); iter++){
        if(iter->second->uid_ == uid && (resourceNumber & iter->second->resourceNumber_) != 0) {
                iter->second->resourceNumber_ ^= (resourceNumber & iter->second->resourceNumber_);
        }
    }
    auto findEmptyResource = [](const auto &iter) { return iter.second->resourceNumber_ == 0; };
    EraseRecordIf(resourceApplyMap_, findEmptyResource);
}

void BgEfficiencyResourcesMgr::ResetEfficiencyResourcesInner(
    const std::shared_ptr<ResourceCallbackInfo> &callbackInfo, bool isProcess)
{
    std::lock_guard<std::mutex> lock(callbackLock_);
    if(!isProcess) {
        RemoveTargetResourceRecord(appResourceApplyMap_, callbackInfo->GetUid(), 
            callbackInfo->GetResourceNumber(), EfficiencyResourcesEventType::APP_RESOURCE_RESET);
        RemoveRelativeProcessRecord(callbackInfo->GetUid(), callbackInfo->GetResourceNumber());
    } else {
        RemoveTargetResourceRecord(resourceApplyMap_, callbackInfo->GetPid(), 
            callbackInfo->GetResourceNumber(), EfficiencyResourcesEventType::RESOURCE_RESET);
    }
    recordStorage_->RefreshResourceRecord(appResourceApplyMap_, resourceApplyMap_);
}

bool BgEfficiencyResourcesMgr::IsCallingInfoLegal(int32_t uid, int32_t pid, std::string &bundleName)
{
    if (uid < 0 || pid < 0) {
        BGTASK_LOGE("pid or uid is invalid.");
        return false;
    }
    bundleName = BundleManagerHelper::GetInstance()->GetClientBundleName(uid);
    return true;
}

ErrCode BgEfficiencyResourcesMgr::AddSubscriber(const sptr<IBackgroundTaskSubscriber>& subscriber)
{
    return subscriberMgr_->AddSubscriber(subscriber);
}

ErrCode BgEfficiencyResourcesMgr::RemoveSubscriber(const sptr<IBackgroundTaskSubscriber>& subscriber)
{
    return subscriberMgr_->RemoveSubscriber(subscriber);
}

ErrCode BgEfficiencyResourcesMgr::ShellDump(const std::vector<std::string> &dumpOption,
    std::vector<std::string> &dumpInfo)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGE("manager is not ready");
        return ERR_BGTASK_SERVICE_NOT_READY;
    }
    ErrCode result = ERR_OK;
    ShellDumpInner(dumpOption, dumpInfo);
    return result;
}

ErrCode BgEfficiencyResourcesMgr::ShellDumpInner(const std::vector<std::string> &dumpOption,
    std::vector<std::string> &dumpInfo)
{
    std::lock_guard<std::mutex> lock(callbackLock_);
    if (dumpOption[1] == DUMP_PARAM_LIST_ALL) {
        DumpAllApplicationInfo(dumpInfo);
    } else if (dumpOption[1] == DUMP_PARAM_RESET_ALL) {
        DumpResetAllResource(dumpInfo);
    } else if (dumpOption[1] == DUMP_PARAM_RESET_APP) {
        DumpResetResource(dumpOption, true, false);
    } else if (dumpOption[1] == DUMP_PARAM_RESET_PROC) {
        DumpResetResource(dumpOption, false, false);
    }
    recordStorage_->RefreshResourceRecord(appResourceApplyMap_, resourceApplyMap_);
    return ERR_OK;
}

void BgEfficiencyResourcesMgr::DumpAllApplicationInfo(std::vector<std::string> &dumpInfo)
{   
    std::stringstream stream;
    if (appResourceApplyMap_.empty()) {
        dumpInfo.emplace_back("No running efficiency resources\n");
        return;
    }
    DumpApplicationInfoMap(appResourceApplyMap_, dumpInfo, stream, "efficiency resource: \n");
    DumpApplicationInfoMap(resourceApplyMap_, dumpInfo, stream, "efficiency resource: \n");
}

void BgEfficiencyResourcesMgr::DumpApplicationInfoMap(std::unordered_map<int32_t, 
    std::shared_ptr<ResourceApplicationRecord>> &infoMap, std::vector<std::string> &dumpInfo,
    std::stringstream &stream, const char *headInfo)
{
    uint32_t index = 1;
    for (auto iter = infoMap.begin(); iter != infoMap.end(); iter++) {
        stream.str("");
        stream.clear();
        stream << headInfo << "\n";
        stream << "No." << index << "\n";
        stream << "\tefficiencyResourceKey: " << iter->first << "\n";
        stream << "\tefficiencyResourceValue:" << "\n";
        stream << "\t\tbundleName: " << iter->second->GetBundleName() << "\n";
        stream << "\t\tuid: " << iter->second->GetUid() << "\n";
        stream << "\t\tpid: " << iter->second->GetPid() << "\n";
        stream << "\t\resourceNumber: " << iter->second->GetResourceNumber() << "\n";
        stream << "\t\treason: " << iter->second->GetReason() << "\n";
        int64_t curTime = TimeProvider::GetCurrentTime();
        for(auto unitIter = iter->second->resourceUnitList_.begin(); 
            unitIter != iter->second->resourceUnitList_.end(); ++unitIter) {
            stream << "\t\t\tresource type: " << ResourceTypeName[unitIter->resourceIndex_] << "\n";
            stream << "\t\t\tisPersist " << (unitIter->isPersist_ ? "true" : "false") << "\n";
            stream << "\t\t\tremainTime " << curTime - unitIter->endTime_ << "\n";
        }
        stream << "\n";
        dumpInfo.emplace_back(stream.str());
        index++;
    }
}

void BgEfficiencyResourcesMgr::DumpResetAllResource(const std::vector<std::string> &dumpOption)
{
    DumpResetResource(dumpOption, true, true);
    DumpResetResource(dumpOption, false, true);
}

void BgEfficiencyResourcesMgr::DumpResetResource(const std::vector<std::string> &dumpOption, bool cleanApp, bool cleanAll)
{
    auto &infoMap = cleanApp ? appResourceApplyMap_ : resourceApplyMap_;
    auto type = cleanApp ? EfficiencyResourcesEventType::APP_RESOURCE_RESET : EfficiencyResourcesEventType::RESOURCE_RESET;

    if (cleanAll) {
        for (auto iter = infoMap.begin(); iter != infoMap.end(); ++iter) {
            std::shared_ptr<ResourceCallbackInfo> callbackInfo = std::make_shared<ResourceCallbackInfo>(iter->second->GetUid(),
                iter->second->GetPid(), iter->second->GetResourceNumber(), iter->second->GetBundleName());
            subscriberMgr_->OnResourceChanged(callbackInfo, type);
        }
        infoMap.clear();
    } else {
        if (dumpOption.size() < MAX_DUMP_PARAM_NUMS) {
            BGTASK_LOGW("invalid dump param");
            return;
        }
        int32_t mapKey = std::stoi(dumpOption[2]);
        uint32_t cleanResource = std::stoi(dumpOption[3]);
        RemoveTargetResourceRecord(infoMap, mapKey, cleanResource, type);
    }
}

bool BgEfficiencyResourcesMgr::RemoveTargetResourceRecord(std::unordered_map<int32_t, std::shared_ptr<ResourceApplicationRecord>> &infoMap, 
    int32_t mapKey, uint32_t cleanResource, EfficiencyResourcesEventType type)
{
    BGTASK_LOGD("resource record key: %{public}d", mapKey);
    auto iter = infoMap.find(mapKey);
    if (iter== infoMap.end() || (iter->second->resourceNumber_ & cleanResource) == 0) {
        BGTASK_LOGW("remove single resource record failure, no matched task: %{public}d", mapKey);
        return false;
    }
    auto callbackInfo = std::make_shared<ResourceCallbackInfo>(iter->second->GetUid(),
            iter->second->GetUid(), cleanResource, iter->second->GetBundleName());
    iter->second->resourceNumber_ ^= (iter->second->resourceNumber_ & cleanResource);
    if (iter->second->resourceNumber_ == 0) {
        infoMap.erase(iter);
    }
    subscriberMgr_->OnResourceChanged(callbackInfo, type);
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
