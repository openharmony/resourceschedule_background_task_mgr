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

#include "resource_type.h"
#include "time_provider.h"
#include "bundle_manager_helper.h"
#include "efficiency_resource_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {

static constexpr char BG_EFFICIENCY_RESOURCES_MGR_NAME[] = "BgEfficiencyResourcesMgr";
static constexpr char DUMP_PARAM_LIST_ALL[] = "--all";
static constexpr char DUMP_PARAM_RESET_ALL[] = "--reset_all";
static constexpr char DUMP_PARAM_RESET_APP[] = "--resetapp";
static constexpr char DUMP_PARAM_RESET_PROC[] = "--resetproc";
static constexpr int32_t DELAY_TIME = 500;
static constexpr int32_t MAX_DUMP_PARAM_NUMS = 4;
static constexpr uint32_t MAX_RESOURCES_TYPE_NUM = 7;
static constexpr uint32_t MAX_RESOURCE_NUMBER = (1 << MAX_RESOURCES_TYPE_NUM) - 1;

BgEfficiencyResourcesMgr::BgEfficiencyResourcesMgr() {
}

BgEfficiencyResourcesMgr::~BgEfficiencyResourcesMgr() {}

bool BgEfficiencyResourcesMgr::Init()
{
    subscriberMgr_ = DelayedSingleton<ResourcesSubscriberMgr>::GetInstance();
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
    HandlePersistenceData();
    InitNecessaryState();
    BGTASK_LOGI("BgEfficiencyResourcesMgr finish Init");
    return true;
}

void BgEfficiencyResourcesMgr::InitNecessaryState()
{
    sptr<ISystemAbilityManager> systemAbilityManager
        = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr
        || systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) == nullptr
        || systemAbilityManager->CheckSystemAbility(APP_MGR_SERVICE_ID) == nullptr) {
        BGTASK_LOGW("necessary system service is not ready yet!");
        auto task = [this]() { this->InitNecessaryState(); };
        handler_->PostTask(task, DELAY_TIME);
        return;
    }
    RegisterAppStateObserver();
    BGTASK_LOGI("necessary system service has been accessiable!");
    BGTASK_LOGW("appResourceApplyMap_.size(): %{public}u, resourceApplyMap_.size():  %{public}u!",
        appResourceApplyMap_.size(), resourceApplyMap_.size());
    isSysReady_.store(true);
}

void BgEfficiencyResourcesMgr::RegisterAppStateObserver()
{
    appStateObserver_ = DelayedSingleton<AppStateObserver>::GetInstance();
    if (appStateObserver_ != nullptr) {
        appStateObserver_->SetBgEfficiencyResourcesMgr(shared_from_this());
    }
}

void BgEfficiencyResourcesMgr::HandlePersistenceData()
{
    recordStorage_ = std::make_shared<ResourceRecordStorage>();
    BGTASK_LOGI("ResourceRecordStorage service restart, restore data");
    recordStorage_->RestoreResourceRecord(appResourceApplyMap_, resourceApplyMap_);
    BGTASK_LOGD("ResourceRecordStorage service finish, restore data");
    auto appMgrClient = std::make_shared<AppExecFwk::AppMgrClient>();
    if (appMgrClient == nullptr || appMgrClient->ConnectAppMgrService() != ERR_OK) {
        BGTASK_LOGW("ResourceRecordStorage connect to app mgr service failed");
        return;
    }
    std::vector<AppExecFwk::RunningProcessInfo> allAppProcessInfos;
    appMgrClient->GetAllRunningProcesses(allAppProcessInfos);
    BGTASK_LOGI("HandlePersistenceData locked before");
    CheckPersistenceData(allAppProcessInfos);
    RecoverDelayedTask(true, resourceApplyMap_);
    RecoverDelayedTask(false, appResourceApplyMap_);
    recordStorage_->RefreshResourceRecord(appResourceApplyMap_, resourceApplyMap_);
}

void BgEfficiencyResourcesMgr::EraseRecordIf(ResourceRecordMap &infoMap,
    const std::function<bool(ResourceRecordPair)> &fun)
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
    BGTASK_LOGI("CheckPersistenceData restart, check existing uid and pid");
    std::set<int32_t> runningUid;
    std::set<int32_t> runningPid;
    std::for_each(allProcesses.begin(), allProcesses.end(), [&runningUid, &runningPid](const auto &iter) {
        runningUid.emplace(iter.uid_);
        runningPid.emplace(iter.pid_);
    } );
    auto removeUid = [&runningUid](const auto &iter) { return runningUid.find(iter.first) == runningUid.end(); };
    EraseRecordIf(appResourceApplyMap_, removeUid);
    auto removePid = [&runningPid](const auto &iter)  { return runningPid.find(iter.first) == runningPid.end(); };
    EraseRecordIf(resourceApplyMap_, removePid);
}

void BgEfficiencyResourcesMgr::RecoverDelayedTask(bool isProcess, ResourceRecordMap& infoMap)
{
    BGTASK_LOGI("RecoverDelayedTask restart");
    const auto &mgr = shared_from_this();
    for (auto iter = infoMap.begin(); iter != infoMap.end(); iter ++) {
        auto &resourceList = iter->second->resourceUnitList_;
        int32_t mapKey = iter->first;
        for (auto resourceIter = resourceList.begin(); resourceIter != resourceList.end(); resourceIter++) {
            if (resourceIter->isPersist_) {
                continue;
            }
            auto task = [mgr, mapKey, isProcess] () {
                mgr->ResetTimeOutResource(mapKey, isProcess);
            };
            int32_t timeOut = static_cast<int32_t>(resourceIter->endTime_ - TimeProvider::GetCurrentTime());
            BGTASK_LOGI("RecoverDelayedTask %{public}lld, %{public}lld, %{public}u after", resourceIter->endTime_,
                TimeProvider::GetCurrentTime(), static_cast<uint32_t>(resourceIter->endTime_ - TimeProvider::GetCurrentTime()));
            handler_->PostTask(task, std::max(0, timeOut));
        }
    }
}

ErrCode BgEfficiencyResourcesMgr::RemoveAppRecord(int32_t uid)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("Efficiency resources manager is not ready, RemoveAppRecord failed.");
        return ERR_BGTASK_SERVICE_NOT_READY;
    }
    BGTASK_LOGI("RemoveAppRecord locked before");
    ErrCode res = ERR_OK;
    handler_->PostSyncTask([&]() {
        this->EraseRecordIf(appResourceApplyMap_, [uid](const auto &iter) { return iter.first == uid; });
        res = recordStorage_->RefreshResourceRecord(appResourceApplyMap_, resourceApplyMap_);
    });
    return res;
}

ErrCode BgEfficiencyResourcesMgr::RemoveProcessRecord(int32_t pid)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("Efficiency resources manager is not ready, RemoveProcessRecord failed..");
        return ERR_BGTASK_SERVICE_NOT_READY;
    }
    BGTASK_LOGI("RemoveProcessRecord locked before");
    ErrCode res = ERR_OK;
    handler_->PostSyncTask([&]() {
        this->EraseRecordIf(resourceApplyMap_, [pid](const auto &iter) { return iter.first == pid; });
        res = recordStorage_->RefreshResourceRecord(appResourceApplyMap_, resourceApplyMap_);
    });
    return res;
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
        || (resourceInfo->IsApply() && !resourceInfo->IsPersist() && resourceInfo->GetTimeOut() <= 0)) {
        BGTASK_LOGE("efficiency resources params invalid!");
        return false;
    }
    return true;
}

ErrCode BgEfficiencyResourcesMgr::ApplyEfficiencyResources(
        const sptr<EfficiencyResourceInfo> &resourceInfo, bool &isSuccess)
{
    if(resourceInfo->IsApply()) {
        BGTASK_LOGI("ApplyEfficiencyResources apply, resource number: %{public}u, "\
            "isPersist: %{public}d, timeOut: %{public}d, isProcess: %{public}d", resourceInfo->GetResourceNumber(),
            resourceInfo->IsPersist(), resourceInfo->GetTimeOut(), resourceInfo->IsProcess());
    } else {
        BGTASK_LOGI("ApplyEfficiencyResources reset, resource number: %{public}u, "\
            "isPersist: %{public}d, timeOut: %{public}d, isProcess: %{public}d", resourceInfo->GetResourceNumber(),
            resourceInfo->IsPersist(), resourceInfo->GetTimeOut(), resourceInfo->IsProcess());
    }
    BGTASK_LOGW("ApplyEfficiencyResources before");
    if (!isSysReady_.load()) {
        BGTASK_LOGW("Efficiency resources manager is not ready.");
        return ERR_BGTASK_SERVICE_NOT_READY;
    }

    if (!CheckResourceInfo(resourceInfo)) {
        return ERR_BGTASK_INVALID_PARAM;
    }

    auto uid = IPCSkeleton::GetCallingUid();
    auto pid = IPCSkeleton::GetCallingPid();
    BGTASK_LOGI("ApplyEfficiencyResources enter uid : %{public}d, pid : %{public}d", uid, pid);
    std::string bundleName = "";
    if (!IsCallingInfoLegal(uid, pid, bundleName)) {
        BGTASK_LOGI("apply efficiency resources failed, calling info is illegal.");
        return ERR_BGTASK_INVALID_PARAM;
    }
    isSuccess = true;
    std::shared_ptr<ResourceCallbackInfo> callbackInfo = std::make_shared<ResourceCallbackInfo>(uid,
        pid, resourceInfo->GetResourceNumber(), bundleName);
    if (resourceInfo->IsApply()) {
        handler_->PostSyncTask([this, &callbackInfo, &resourceInfo]() {
            this->ApplyEfficiencyResourcesInner(callbackInfo, resourceInfo);
        });
    } else {
        handler_->PostSyncTask([this, &callbackInfo, &resourceInfo]() {
            this->ResetEfficiencyResourcesInner(callbackInfo, resourceInfo->IsProcess());
        });
    }
    recordStorage_->RefreshResourceRecord(appResourceApplyMap_, resourceApplyMap_);
    return ERR_OK;
}

void BgEfficiencyResourcesMgr::ApplyEfficiencyResourcesInner(const std::shared_ptr<ResourceCallbackInfo>
    &callbackInfo, const sptr<EfficiencyResourceInfo> &resourceInfo)
{
    BGTASK_LOGI("ApplyEfficiencyResourcesInner locked before");
    int32_t mapKey = !resourceInfo->IsProcess() ? callbackInfo->GetUid() : callbackInfo->GetPid();
    auto &infoMap = !resourceInfo->IsProcess() ? appResourceApplyMap_ : resourceApplyMap_;
    BGTASK_LOGI("ApplyEfficiencyResourcesInner info.size(): %{public}u", infoMap.size());
    auto iter = infoMap.find(mapKey);
    if (iter == infoMap.end()) {
        infoMap.emplace(mapKey, std::make_shared<ResourceApplicationRecord>(callbackInfo->GetUid(),
            callbackInfo->GetPid(), callbackInfo->GetResourceNumber(), callbackInfo->GetBundleName()));
    } else {
        iter->second->reason_ = resourceInfo->GetReason();
        iter->second->resourceNumber_ |= callbackInfo->GetResourceNumber();
    }
    iter = infoMap.find(mapKey);
    BGTASK_LOGI("ApplyEfficiencyResourcesInner info.size(): %{public}u, list.size(): %{public}u",
        infoMap.size(), iter->second->resourceUnitList_.size());
    BGTASK_LOGI("UpdateResourcesEndtime before");
    UpdateResourcesEndtime(callbackInfo, iter->second, resourceInfo->IsPersist(),
        resourceInfo->GetTimeOut(), resourceInfo->IsProcess());
    BGTASK_LOGI("UpdateResourcesEndtime end");
    subscriberMgr_->OnResourceChanged(callbackInfo, !resourceInfo->IsProcess() ?
        EfficiencyResourcesEventType::APP_RESOURCE_APPLY : EfficiencyResourcesEventType::RESOURCE_APPLY);
    BGTASK_LOGI("UpdateResourcesEndtime end");
    recordStorage_->RefreshResourceRecord(appResourceApplyMap_, resourceApplyMap_);
}

void BgEfficiencyResourcesMgr::UpdateResourcesEndtime(const std::shared_ptr<ResourceCallbackInfo>
    &callbackInfo, std::shared_ptr<ResourceApplicationRecord> &record, bool isPersist, int32_t timeOut, bool isProcess)
{
    for (uint32_t resourceIndex = 0; resourceIndex < MAX_RESOURCES_TYPE_NUM; ++resourceIndex) {
        if ((callbackInfo->GetResourceNumber() & (1 << resourceIndex)) != 0) {
            auto task = [resourceIndex](const auto &it) {
                return it.resourceIndex_ == resourceIndex;
            }; 
            auto resourceUnitIter = std::find_if(record->resourceUnitList_.begin(),
                record->resourceUnitList_.end(), task);
            int64_t lastTime = 0;
            if (resourceUnitIter == record->resourceUnitList_.end()) {
                record->resourceUnitList_.emplace_back(PersistTime {resourceIndex, isPersist,
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
            }
        }
    }
    BGTASK_LOGI("UpdateResourcesEndtime list.size(): %{public}u", record->resourceUnitList_.size());
    if (isPersist) {
        return;
    }
    int32_t mapKey = !isProcess ? callbackInfo->GetUid() : callbackInfo->GetPid();
    const auto& mgr = shared_from_this();
    auto task = [mgr, mapKey, isProcess] () {
        mgr->ResetTimeOutResource(mapKey, isProcess);
    };
    handler_->PostTask(task, timeOut);
}

void BgEfficiencyResourcesMgr::ResetTimeOutResource(int32_t mapKey, bool isProcess)
{
    if (!isProcess) {
        BGTASK_LOGI("ResetTimeOutResource reset efficiency rsources uid: %{public}d",
            mapKey);
    } else {
        BGTASK_LOGI("ResetTimeOutResource reset efficiency rsources pid: %{public}d",
            mapKey);
    }
    auto &infoMap = !isProcess ? appResourceApplyMap_ : resourceApplyMap_;
    auto type = !isProcess ? EfficiencyResourcesEventType::APP_RESOURCE_RESET :
        EfficiencyResourcesEventType::RESOURCE_RESET;
    auto iter = infoMap.find(mapKey);  
    if (iter == infoMap.end()) {
        BGTASK_LOGI("Efficiency resource does not exist.");
        return;
    }
    auto &resourceRecord = iter->second;
    uint32_t resetZeros = 0;
    const auto &resourceUnitList = resourceRecord->resourceUnitList_;
    for (auto iter = resourceUnitList.begin(); iter != resourceUnitList.end(); ++iter) {
        if (iter->isPersist_) {
            continue;
        }
        auto endTime = iter->endTime_;
        if(TimeProvider::GetCurrentTime() >= endTime) {
            resetZeros |= 1 << iter->resourceIndex_;
        }
    }
    // for (uint32_t resourceIndex = 0; resourceIndex < MAX_RESOURCES_TYPE_NUM; ++resourceIndex) {
    //     if ((resourceRecord->resourceNumber_ & 1 << resourceIndex) ==0 ||
    //         (timeOutBit & 1 << resourceIndex) == 0) {
    //         continue;
    //     }
    //     auto resourceUnitIter = std::find_if(resourceRecord->resourceUnitList_.begin(),
    //         resourceRecord->resourceUnitList_.end(), [resourceIndex](const auto &it) {
    //             return it.resourceIndex_ == resourceIndex;
    //         });
    //     if (resourceUnitIter != resourceRecord->resourceUnitList_.end()) {
    //         auto endTime = resourceUnitIter->endTime_;
    //         if (TimeProvider::GetCurrentTime() >= endTime) {
    //             resetZeros |= 1 << resourceIndex;
    //         }
    //     }
    // }
    BGTASK_LOGI("ResetTimeOutResource resetZeros: %{public}u, resourceNumber: %{public}u, result: %{public}u,",
        resetZeros, resourceRecord->resourceNumber_, resourceRecord->resourceNumber_ ^ resetZeros);
    resourceRecord->resourceNumber_ ^= resetZeros;
    if (resetZeros) {
        return;
    }
    RemoveListRecord(resourceRecord->resourceUnitList_, resetZeros);
    auto callbackInfo = std::make_shared<ResourceCallbackInfo>(resourceRecord->uid_, resourceRecord->pid_, resetZeros,
        resourceRecord->bundleName_);
    subscriberMgr_->OnResourceChanged(callbackInfo, type);
    if(resourceRecord->resourceNumber_ == 0) {
        infoMap.erase(iter);
    }
}

ErrCode BgEfficiencyResourcesMgr::ResetAllEfficiencyResources()
{
    BGTASK_LOGW("ResetAllEfficiencyResources before");
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
    std::shared_ptr<ResourceCallbackInfo> callbackInfo = std::make_shared<ResourceCallbackInfo>(uid,
        pid, MAX_RESOURCE_NUMBER, bundleName);
    BGTASK_LOGI("reset efficiency resources uid : %{public}d, pid : %{public}d, resource number : %{public}d", uid,
        pid, callbackInfo->GetResourceNumber());
    handler_->PostSyncTask([this, &callbackInfo]() {
        this->ResetEfficiencyResourcesInner(callbackInfo, false);
    });
    return ERR_OK;
}

void BgEfficiencyResourcesMgr::RemoveRelativeProcessRecord(int32_t uid, uint32_t resourceNumber)
{
    for (auto iter = resourceApplyMap_.begin(); iter != resourceApplyMap_.end(); iter++) {
        if(iter->second->uid_ == uid && (resourceNumber & iter->second->resourceNumber_) != 0) {
            uint32_t eraseBit = (resourceNumber & iter->second->resourceNumber_);
            iter->second->resourceNumber_ ^= eraseBit;
            RemoveListRecord(iter->second->resourceUnitList_, eraseBit);
        }
    }
    auto findEmptyResource = [](const auto &iter) { return iter.second->resourceNumber_ == 0; };
    EraseRecordIf(resourceApplyMap_, findEmptyResource);
}

void BgEfficiencyResourcesMgr::ResetEfficiencyResourcesInner(
    const std::shared_ptr<ResourceCallbackInfo> &callbackInfo, bool isProcess)
{
    BGTASK_LOGI("ResetEfficiencyResourcesInner locked before");
    if (!isProcess) {
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

ErrCode BgEfficiencyResourcesMgr::AddSubscriber(const sptr<IBackgroundTaskSubscriber> &subscriber)
{
    BGTASK_LOGW("AddSubscriber Efficiency resources manager is already ready.");
    return subscriberMgr_->AddSubscriber(subscriber);
}

ErrCode BgEfficiencyResourcesMgr::RemoveSubscriber(const sptr<IBackgroundTaskSubscriber> &subscriber)
{
    BGTASK_LOGW("RemoveSubscriber Efficiency resources manager is already ready.");
    return subscriberMgr_->RemoveSubscriber(subscriber);
}

ErrCode BgEfficiencyResourcesMgr::ShellDump(const std::vector<std::string> &dumpOption,
    std::vector<std::string> &dumpInfo)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGE("manager is not ready");
        return ERR_BGTASK_SERVICE_NOT_READY;
    }
    handler_->PostSyncTask([&]() {
        this->ShellDumpInner(dumpOption, dumpInfo);
    });
    return ERR_OK;
}

ErrCode BgEfficiencyResourcesMgr::ShellDumpInner(const std::vector<std::string> &dumpOption,
    std::vector<std::string> &dumpInfo)
{
    BGTASK_LOGI("ShellDumpInner locked before");
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
    if (appResourceApplyMap_.empty() && resourceApplyMap_.empty()) {
        dumpInfo.emplace_back("No running efficiency resources\n");
        return;
    }
    DumpApplicationInfoMap(appResourceApplyMap_, dumpInfo, stream, "app efficiency resource: \n");
    DumpApplicationInfoMap(resourceApplyMap_, dumpInfo, stream, "process efficiency resource: \n");
}

void BgEfficiencyResourcesMgr::DumpApplicationInfoMap(std::unordered_map<int32_t,
    std::shared_ptr<ResourceApplicationRecord>> &infoMap, std::vector<std::string> &dumpInfo,
    std::stringstream &stream, const char *headInfo)
{
    stream.str("");
    stream.clear();
    stream << headInfo << "\n";
    uint32_t index = 1;
    for (auto iter = infoMap.begin(); iter != infoMap.end(); iter++) {
        stream << "No." << index << "\n";
        stream << "\tefficiencyResourceKey: " << iter->first << "\n";
        stream << "\tefficiencyResourceValue:" << "\n";
        stream << "\t\tbundleName: " << iter->second->GetBundleName() << "\n";
        stream << "\t\tuid: " << iter->second->GetUid() << "\n";
        stream << "\t\tpid: " << iter->second->GetPid() << "\n";
        stream << "\t\tresourceNumber: " << iter->second->GetResourceNumber() << "\n";
        stream << "\t\treason: " << iter->second->GetReason() << "\n";
        int64_t curTime = TimeProvider::GetCurrentTime();
        iter->second->resourceUnitList_.sort();
        auto &resourceUnitList = iter->second->resourceUnitList_;
        for(auto unitIter = resourceUnitList.begin();
            unitIter != resourceUnitList.end(); ++unitIter) {
            stream << "\t\t\tresource type: " << ResourceTypeName[unitIter->resourceIndex_] << "\n";
            stream << "\t\t\tisPersist " << (unitIter->isPersist_ ? "true" : "false") << "\n";
            if (!unitIter->isPersist_) {
                stream << "\t\t\tremainTime " << unitIter->endTime_ - curTime << "\n";
            }
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

void BgEfficiencyResourcesMgr::DumpResetResource(const std::vector<std::string> &dumpOption,
    bool cleanApp, bool cleanAll)
{
    auto &infoMap = cleanApp ? appResourceApplyMap_ : resourceApplyMap_;
    auto type = cleanApp ? EfficiencyResourcesEventType::APP_RESOURCE_RESET : EfficiencyResourcesEventType::RESOURCE_RESET;

    if (cleanAll) {
        for (auto iter = infoMap.begin(); iter != infoMap.end(); ++iter) {
            std::shared_ptr<ResourceCallbackInfo> callbackInfo = std::make_shared<ResourceCallbackInfo>
                (iter->second->GetUid(), iter->second->GetPid(), iter->second->GetResourceNumber(), 
                iter->second->GetBundleName());
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

bool BgEfficiencyResourcesMgr::RemoveTargetResourceRecord(std::unordered_map<int32_t,
    std::shared_ptr<ResourceApplicationRecord>> &infoMap, int32_t mapKey, uint32_t cleanResource,
    EfficiencyResourcesEventType type)
{
    BGTASK_LOGD("resource record key: %{public}d", mapKey);
    BGTASK_LOGD("resource record size(): %{public}d", infoMap.size());
    auto iter = infoMap.find(mapKey);
    if (iter == infoMap.end() || (iter->second->resourceNumber_ & cleanResource) == 0) {
        BGTASK_LOGW("remove single resource record failure, no matched task: %{public}d", mapKey);
        return false;
    }
    auto callbackInfo = std::make_shared<ResourceCallbackInfo>(iter->second->GetUid(),
        iter->second->GetPid(), cleanResource, iter->second->GetBundleName());
    uint32_t eraseBit = (iter->second->resourceNumber_ & cleanResource);
    iter->second->resourceNumber_ ^= eraseBit;

    RemoveListRecord(iter->second->resourceUnitList_, eraseBit);
    if (iter->second->resourceNumber_ == 0) {
        infoMap.erase(iter);
    }
    subscriberMgr_->OnResourceChanged(callbackInfo, type);
    return true;
}

void BgEfficiencyResourcesMgr::RemoveListRecord(std::list<PersistTime> &resourceUnitList, uint32_t eraseBit)
{
    BGTASK_LOGD("start RemoveListRecord: %{public}u", resourceUnitList.size());
    if (eraseBit == 0) {
        return;
    }
    for (auto it = resourceUnitList.begin(); it != resourceUnitList.end(); ++it) {
        if(((1 << it->resourceIndex_) & eraseBit) != 0) {
            it = resourceUnitList.erase(it);
        } else {
            ++it;
        }
    }
}

ErrCode BgEfficiencyResourcesMgr::GetEfficiencyResourcesInfos(std::vector<std::shared_ptr<
    ResourceCallbackInfo>> &appList, std::vector<std::shared_ptr<ResourceCallbackInfo>> &procList)
{
    // if (!isSysReady_.load()) {
    //     BGTASK_LOGW("efficiency resources manager is not ready");
    //     return ERR_BGTASK_SYS_NOT_READY;
    // }
    handler_->PostSyncTask([this, &appList, &procList]() {
        this->GetEfficiencyResourcesInfosInner(appResourceApplyMap_, appList);
        this->GetEfficiencyResourcesInfosInner(resourceApplyMap_, procList);
    }, AppExecFwk::EventQueue::Priority::HIGH);

    return ERR_OK;
}

void BgEfficiencyResourcesMgr::GetEfficiencyResourcesInfosInner(const ResourceRecordMap &infoMap,
    std::vector<std::shared_ptr<ResourceCallbackInfo>> &list)
{
    if (infoMap.empty()) {
        return;
    }
    BGTASK_LOGI("GetEfficiencyResourcesInfosInner ResourceRecordMap.size(): %{public}u ", infoMap.size());
    for (auto &record : infoMap) {
        auto appInfo = std::make_shared<ResourceCallbackInfo>(record.second->uid_, record.second->pid_,
            record.second->resourceNumber_, record.second->bundleName_);
        list.push_back(appInfo);
    }
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
