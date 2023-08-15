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

#include "bg_efficiency_resources_mgr.h"

#include <set>
#include <algorithm>
#include <vector>

#include "event_runner.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "bgtaskmgr_inner_errors.h"

#include "resource_type.h"
#include "time_provider.h"
#include "bundle_manager_helper.h"
#include "efficiency_resource_log.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
    const std::string DUMP_PARAM_LIST_ALL = "--all";
    const std::string DUMP_PARAM_RESET_ALL = "--reset_all";
    const std::string DUMP_PARAM_RESET_APP = "--resetapp";
    const std::string DUMP_PARAM_RESET_PROC = "--resetproc";
    const int32_t MAX_DUMP_PARAM_NUMS = 4;
    const uint32_t APP_MGR_READY = 1;
    const uint32_t BUNDLE_MGR_READY = 2;
    const uint32_t ALL_DEPENDS_READY = 3;
    const uint32_t FREEZE_ALL_RESOURCES = 0;
    const uint32_t MAX_RESOURCES_TYPE_NUM = ResourceTypeName.size();
    const uint32_t MAX_RESOURCE_MASK = (1 << ResourceTypeName.size()) - 1;
}
BgEfficiencyResourcesMgr::BgEfficiencyResourcesMgr() {}

BgEfficiencyResourcesMgr::~BgEfficiencyResourcesMgr() {}

bool BgEfficiencyResourcesMgr::Init(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
{
    subscriberMgr_ = DelayedSingleton<ResourcesSubscriberMgr>::GetInstance();
    if (runner == nullptr) {
        BGTASK_LOGE("efficiency resources mgr runner create failed!");
        return false;
    }
    handler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    if (!handler_) {
        BGTASK_LOGE("efficiency resources mgr handler create failed!");
        return false;
    }
    HandlePersistenceData();
    BGTASK_LOGI("efficiency resources mgr finish Init");
    return true;
}

void BgEfficiencyResourcesMgr::InitNecessaryState()
{
    RegisterAppStateObserver();
    BGTASK_LOGI("necessary system service has been accessiable!");
    BGTASK_LOGD("app resource record size: %{public}d, process  resource record size:  %{public}d!",
        static_cast<int32_t>(appResourceApplyMap_.size()), static_cast<int32_t>(procResourceApplyMap_.size()));
    isSysReady_.store(true);
}

void BgEfficiencyResourcesMgr::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    BGTASK_LOGI("add system ability, systemAbilityId : %{public}d", systemAbilityId);
    std::lock_guard<std::mutex> lock(sysAbilityLock_);
    switch (systemAbilityId) {
        case APP_MGR_SERVICE_ID:
            BGTASK_LOGI("app mgr service is ready!");
            dependsReady_ |= APP_MGR_READY;
            break;
        case BUNDLE_MGR_SERVICE_SYS_ABILITY_ID:
            BGTASK_LOGI("bundle mgr service is ready!");
            dependsReady_ |= BUNDLE_MGR_READY;
            break;
        default:
            break;
    }
    if (dependsReady_ == ALL_DEPENDS_READY) {
        BGTASK_LOGI("necessary system service has been satisfied!");
        auto task = [this]() { this->InitNecessaryState(); };
        handler_->PostSyncTask(task);
    }
}

void BgEfficiencyResourcesMgr::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    BGTASK_LOGI("remove system ability, systemAbilityId : %{public}d", systemAbilityId);
    std::lock_guard<std::mutex> lock(sysAbilityLock_);
    switch (systemAbilityId) {
        case APP_MGR_SERVICE_ID:
            BGTASK_LOGI("app mgr service is removed!");
            dependsReady_ &= (~APP_MGR_READY);
            break;
        case BUNDLE_MGR_SERVICE_SYS_ABILITY_ID:
            BGTASK_LOGI("bundle mgr service is removed!");
            dependsReady_ &= (~BUNDLE_MGR_READY);
            break;
        default:
            break;
    }
    if (dependsReady_ != ALL_DEPENDS_READY) {
        BGTASK_LOGI("necessary system service has been unsatisfied");
        isSysReady_.store(false);
    }
}

void BgEfficiencyResourcesMgr::RegisterAppStateObserver()
{
    appStateObserver_ = DelayedSingleton<AppStateObserver>::GetInstance();
    if (appStateObserver_) {
        appStateObserver_->SetBgEfficiencyResourcesMgr(shared_from_this());
    }
}

void BgEfficiencyResourcesMgr::HandlePersistenceData()
{
    BGTASK_LOGD("ResourceRecordStorage service restart, restore data");

    if (appMgrClient_ == nullptr) {
        appMgrClient_ = std::make_unique<AppExecFwk::AppMgrClient>();
        if (!appMgrClient_ || appMgrClient_->ConnectAppMgrService() != ERR_OK) {
            BGTASK_LOGW("ResourceRecordStorage connect to app mgr service failed");
            DelayedSingleton<DataStorageHelper>::GetInstance()->RefreshResourceRecord(
                appResourceApplyMap_, procResourceApplyMap_);
            return;
        }
    }
    std::vector<AppExecFwk::RunningProcessInfo> allAppProcessInfos;
    appMgrClient_->GetAllRunningProcesses(allAppProcessInfos);
    BGTASK_LOGI("start to recovery delayed task of apps and processes");
    DelayedSingleton<DataStorageHelper>::GetInstance()->RestoreResourceRecord(
        appResourceApplyMap_, procResourceApplyMap_);
    CheckPersistenceData(allAppProcessInfos);
    RecoverDelayedTask(true, procResourceApplyMap_);
    RecoverDelayedTask(false, appResourceApplyMap_);
    DelayedSingleton<DataStorageHelper>::GetInstance()->RefreshResourceRecord(
        appResourceApplyMap_, procResourceApplyMap_);
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
    BGTASK_LOGI("efficiency resources check existing uid and pid");
    std::set<int32_t> runningUid;
    std::set<int32_t> runningPid;
    std::for_each(allProcesses.begin(), allProcesses.end(), [&runningUid, &runningPid](const auto &iter) {
        runningUid.emplace(iter.uid_);
        runningPid.emplace(iter.pid_);
    });
    auto removeUid = [&runningUid](const auto &iter) { return runningUid.find(iter.first) == runningUid.end(); };
    EraseRecordIf(appResourceApplyMap_, removeUid);
    auto removePid = [&runningPid](const auto &iter)  { return runningPid.find(iter.first) == runningPid.end(); };
    EraseRecordIf(procResourceApplyMap_, removePid);
}

void BgEfficiencyResourcesMgr::RecoverDelayedTask(bool isProcess, ResourceRecordMap& infoMap)
{
    BGTASK_LOGD("start to recovery delayed task");
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
            handler_->PostTask(task, std::max(0, timeOut));
        }
    }
}

ErrCode BgEfficiencyResourcesMgr::RemoveAppRecord(int32_t uid, const std::string &bundleName, bool resetAll)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("Efficiency resources manager is not ready, RemoveAppRecord failed");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    BGTASK_LOGD("app died, uid: %{public}d, bundleName: %{public}s", uid, bundleName.c_str());
    handler_->PostTask([this, uid, bundleName, resetAll]() {
        int resourceNumber = resetAll ? MAX_RESOURCE_MASK : (MAX_RESOURCE_MASK ^ ResourceType::WORK_SCHEDULER ^
            ResourceType::TIMER);
        std::shared_ptr<ResourceCallbackInfo> callbackInfo = std::make_shared<ResourceCallbackInfo>(uid,
            0, resourceNumber, bundleName);
        this->ResetEfficiencyResourcesInner(callbackInfo, false);
    });
    return ERR_OK;
}

ErrCode BgEfficiencyResourcesMgr::RemoveProcessRecord(int32_t uid, int32_t pid, const std::string &bundleName)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("Efficiency resources manager is not ready, remove process record failed");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    BGTASK_LOGD("process died, uid: %{public}d, pid: %{public}d, bundleName: %{public}s",
        uid, pid, bundleName.c_str());
    handler_->PostTask([this, uid, pid, bundleName]() {
        std::shared_ptr<ResourceCallbackInfo> callbackInfo = std::make_shared<ResourceCallbackInfo>(uid,
            pid, MAX_RESOURCE_MASK ^ ResourceType::WORK_SCHEDULER ^ ResourceType::TIMER, bundleName);
        this->ResetEfficiencyResourcesInner(callbackInfo, true);
        if (!this->CheckAlivedApp(uid)) {
            this->ResetEfficiencyResourcesInner(callbackInfo, false);
        }
    });
    return ERR_OK;
}

bool BgEfficiencyResourcesMgr::CheckAlivedApp(int32_t uid)
{
    BGTASK_LOGD("start check app alive or not");
    if (!appMgrClient_ || appMgrClient_->ConnectAppMgrService() != ERR_OK) {
        BGTASK_LOGE("ResourceRecordStorage connect to app mgr service failed");
        return true;
    }
    std::vector<AppExecFwk::RunningProcessInfo> allAppProcessInfos {};
    appMgrClient_->GetAllRunningProcesses(allAppProcessInfos);
    for (const auto &info : allAppProcessInfos) {
        if (info.uid_ == uid) {
            return true;
        }
    }
    return false;
}

void BgEfficiencyResourcesMgr::Clear()
{
}

bool CheckResourceInfo(const sptr<EfficiencyResourceInfo> &resourceInfo)
{
    if (!resourceInfo) {
        BGTASK_LOGE("apply efficiency resource request params is null!");
        return false;
    }
    if (resourceInfo->GetResourceNumber() == 0 || resourceInfo->GetResourceNumber() > MAX_RESOURCE_MASK
        || (resourceInfo->IsApply() && !resourceInfo->IsPersist() && resourceInfo->GetTimeOut() == 0)) {
        BGTASK_LOGE("efficiency resources params invalid!");
        return false;
    }
    return true;
}

ErrCode BgEfficiencyResourcesMgr::ApplyEfficiencyResources(
    const sptr<EfficiencyResourceInfo> &resourceInfo)
{
    BGTASK_LOGD("start bgtaskefficiency");
    if (!isSysReady_.load()) {
        BGTASK_LOGW("Efficiency resources manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }

    if (!CheckResourceInfo(resourceInfo)) {
        return ERR_BGTASK_RESOURCES_EXCEEDS_MAX;
    }

    auto uid = IPCSkeleton::GetCallingUid();
    auto pid = IPCSkeleton::GetCallingPid();
    std::string bundleName = "";
    if (!IsCallingInfoLegal(uid, pid, bundleName)) {
        BGTASK_LOGI("apply efficiency resources failed, calling info is illegal");
        return ERR_BGTASK_INVALID_PID_OR_UID;
    }
    uint64_t tokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!BundleManagerHelper::GetInstance()->IsSystemApp(tokenId)) {
        BGTASK_LOGE("apply efficiency resources failed,  %{public}s is not system app", bundleName.c_str());
        return ERR_BGTASK_NOT_SYSTEM_APP;
    }

    auto exemptedResourceType = GetExemptedResourceType(resourceInfo->GetResourceNumber(), uid, bundleName);
    resourceInfo->SetResourceNumber(exemptedResourceType);
    if (exemptedResourceType == 0) {
        BGTASK_LOGE("apply efficiency resources failed, no permitted resource type");
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    ApplyResourceForPkgAndProc(uid, pid, bundleName, resourceInfo);
    return ERR_OK;
}

void BgEfficiencyResourcesMgr::ApplyResourceForPkgAndProc(int32_t uid, int32_t pid, const std::string &bundleName,
    const sptr<EfficiencyResourceInfo> &resourceInfo)
{
    if (!resourceInfo->IsProcess()) {
        SendResourceApplyTask(uid, pid, bundleName, resourceInfo);
        return;
    }
    // Only cpu can apply for process
    if ((resourceInfo->GetResourceNumber() & ResourceType::CPU) !=0) {
        sptr<EfficiencyResourceInfo> procResourceInfo = new (std::nothrow) EfficiencyResourceInfo(*resourceInfo);
        procResourceInfo->SetResourceNumber(ResourceType::CPU);
        SendResourceApplyTask(uid, pid, bundleName, procResourceInfo);
    }
    int resourceNumber = resourceInfo->GetResourceNumber() & (~ResourceType::CPU);
    if (resourceNumber != 0) {
        sptr<EfficiencyResourceInfo> appResourceInfo = new (std::nothrow) EfficiencyResourceInfo(*resourceInfo);
        appResourceInfo->SetResourceNumber(resourceNumber);
        appResourceInfo->SetProcess(false);
        SendResourceApplyTask(uid, pid, bundleName, appResourceInfo);
    }
}

void BgEfficiencyResourcesMgr::SendResourceApplyTask(int32_t uid, int32_t pid, const std::string &bundleName,
    const sptr<EfficiencyResourceInfo> &resourceInfo)
{
    std::shared_ptr<ResourceCallbackInfo> callbackInfo = std::make_shared<ResourceCallbackInfo>(uid,
        pid, resourceInfo->GetResourceNumber(), bundleName);
    if (resourceInfo->IsApply()) {
        handler_->PostTask([this, callbackInfo, resourceInfo]() {
            this->ApplyEfficiencyResourcesInner(callbackInfo, resourceInfo);
        });
    } else {
        handler_->PostTask([this, callbackInfo, resourceInfo]() {
            this->ResetEfficiencyResourcesInner(callbackInfo, resourceInfo->IsProcess());
        });
    }
}

void BgEfficiencyResourcesMgr::ApplyEfficiencyResourcesInner(std::shared_ptr<ResourceCallbackInfo>
    callbackInfo, const sptr<EfficiencyResourceInfo> &resourceInfo)
{
    BGTASK_LOGI("apply efficiency resources, uid:%{public}d, pid %{public}d, resource number: %{public}u,"\
        "isPersist: %{public}d, timeOut: %{public}u, isProcess: %{public}d", callbackInfo->GetUid(),
        callbackInfo->GetPid(), resourceInfo->GetResourceNumber(), resourceInfo->IsPersist(),
        resourceInfo->GetTimeOut(), resourceInfo->IsProcess());
    int32_t mapKey = resourceInfo->IsProcess() ? callbackInfo->GetPid() : callbackInfo->GetUid();
    auto &infoMap = resourceInfo->IsProcess() ? procResourceApplyMap_ : appResourceApplyMap_;
    uint32_t preResourceNumber = 0;
    auto iter = infoMap.find(mapKey);
    if (iter == infoMap.end()) {
        infoMap.emplace(mapKey, std::make_shared<ResourceApplicationRecord>(callbackInfo->GetUid(),
            callbackInfo->GetPid(), callbackInfo->GetResourceNumber(), callbackInfo->GetBundleName()));
        iter = infoMap.find(mapKey);
    } else {
        preResourceNumber = iter->second->resourceNumber_;
        iter->second->resourceNumber_ |= callbackInfo->GetResourceNumber();
    }
    BGTASK_LOGD("start to update resources end time");
    UpdateResourcesEndtime(callbackInfo, iter->second, resourceInfo);
    uint32_t diffResourceNumber = iter->second->resourceNumber_ ^ (preResourceNumber & iter->second->resourceNumber_);
    if (diffResourceNumber == 0) {
        BGTASK_LOGD("after update end time, diff between resourcesNumbers is zero");
        DelayedSingleton<DataStorageHelper>::GetInstance()->RefreshResourceRecord(
            appResourceApplyMap_, procResourceApplyMap_);
        return;
    }

    callbackInfo->SetResourceNumber(diffResourceNumber);
    BGTASK_LOGI("after update end time, callbackInfo resource number is %{public}u,"\
        " uid: %{public}d, bundle name: %{public}s", callbackInfo->GetResourceNumber(),
        callbackInfo->GetUid(), callbackInfo->GetBundleName().c_str());
    if (resourceInfo->IsProcess()) {
        subscriberMgr_->OnResourceChanged(callbackInfo, EfficiencyResourcesEventType::RESOURCE_APPLY);
    } else {
        subscriberMgr_->OnResourceChanged(callbackInfo, EfficiencyResourcesEventType::APP_RESOURCE_APPLY);
    }
    DelayedSingleton<DataStorageHelper>::GetInstance()->RefreshResourceRecord(
        appResourceApplyMap_, procResourceApplyMap_);
}

void BgEfficiencyResourcesMgr::UpdateResourcesEndtime(const std::shared_ptr<ResourceCallbackInfo>
    &callbackInfo, std::shared_ptr<ResourceApplicationRecord> &record,
    const sptr<EfficiencyResourceInfo> &resourceInfo)
{
    for (uint32_t resourceIndex = 0; resourceIndex < MAX_RESOURCES_TYPE_NUM; ++resourceIndex) {
        if ((callbackInfo->GetResourceNumber() & (1 << resourceIndex)) == 0) {
            continue;
        }
        auto task = [resourceIndex](const auto &it) {
            return it.resourceIndex_ == resourceIndex;
        };
        auto resourceUnitIter = std::find_if(record->resourceUnitList_.begin(),
            record->resourceUnitList_.end(), task);
        int64_t endtime = TimeProvider::GetCurrentTime() + static_cast<int64_t>(resourceInfo->GetTimeOut());
        if (resourceUnitIter == record->resourceUnitList_.end()) {
            if (resourceInfo->IsPersist()) {
                endtime = 0;
            }
            record->resourceUnitList_.emplace_back(PersistTime {resourceIndex, resourceInfo->IsPersist(),
                endtime, resourceInfo->GetReason()});
        } else {
            resourceUnitIter->reason_ = resourceInfo->GetReason();
            resourceUnitIter->isPersist_ = resourceUnitIter->isPersist_ || resourceInfo->IsPersist();
            if (resourceUnitIter->isPersist_) {
                resourceUnitIter->endTime_ = 0;
            } else {
                resourceUnitIter->endTime_ = std::max(resourceUnitIter->endTime_,
                    endtime);
            }
        }
    }
    BGTASK_LOGD("update end time of resource");
    if (resourceInfo->IsPersist()) {
        return;
    }
    const bool isProcess = resourceInfo->IsProcess();
    int32_t mapKey = isProcess ? callbackInfo->GetPid() : callbackInfo->GetUid();
    const auto& mgr = shared_from_this();
    auto task = [mgr, mapKey, isProcess] () {
        mgr->ResetTimeOutResource(mapKey, isProcess);
    };
    handler_->PostTask(task, resourceInfo->GetTimeOut());
}

void BgEfficiencyResourcesMgr::ResetTimeOutResource(int32_t mapKey, bool isProcess)
{
    BGTASK_LOGD("ResetTimeOutResource reset efficiency rsources, mapkey: %{public}d",
        mapKey);
    auto &infoMap = isProcess ? procResourceApplyMap_ : appResourceApplyMap_;
    auto type = isProcess ? EfficiencyResourcesEventType::RESOURCE_RESET :
        EfficiencyResourcesEventType::APP_RESOURCE_RESET;
    auto iter = infoMap.find(mapKey);
    if (iter == infoMap.end()) {
        BGTASK_LOGI("efficiency resource does not exist");
        return;
    }
    auto &resourceRecord = iter->second;
    uint32_t eraseBit = 0;
    for (auto recordIter = resourceRecord->resourceUnitList_.begin();
        recordIter != resourceRecord->resourceUnitList_.end(); ++recordIter) {
        if (recordIter->isPersist_) {
            continue;
        }
        auto endTime = recordIter->endTime_;
        if (TimeProvider::GetCurrentTime() >= endTime) {
            eraseBit |= 1 << recordIter->resourceIndex_;
        }
    }
    BGTASK_LOGD("ResetTimeOutResource eraseBit: %{public}u, resourceNumber: %{public}u, result: %{public}u",
        eraseBit, resourceRecord->resourceNumber_, resourceRecord->resourceNumber_ ^ eraseBit);
    if (eraseBit == 0) {
        BGTASK_LOGD("try to reset time out resources, but find nothing to reset");
        return;
    }
    resourceRecord->resourceNumber_ ^= eraseBit;
    RemoveListRecord(resourceRecord->resourceUnitList_, eraseBit);
    auto callbackInfo = std::make_shared<ResourceCallbackInfo>(resourceRecord->uid_, resourceRecord->pid_, eraseBit,
        resourceRecord->bundleName_);
    BGTASK_LOGI("after reset time out resources, callbackInfo resource number is %{public}u,"\
        " uid: %{public}d, bundle name: %{public}s", callbackInfo->GetResourceNumber(),
        callbackInfo->GetUid(), callbackInfo->GetBundleName().c_str());
    subscriberMgr_->OnResourceChanged(callbackInfo, type);
    if (resourceRecord->resourceNumber_ == 0) {
        infoMap.erase(iter);
    }
}

ErrCode BgEfficiencyResourcesMgr::ResetAllEfficiencyResources()
{
    BGTASK_LOGD("start to reset all efficiency resources");
    if (!isSysReady_.load()) {
        BGTASK_LOGW("efficiency resources manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }

    auto uid = IPCSkeleton::GetCallingUid();
    auto pid = IPCSkeleton::GetCallingPid();
    std::string bundleName = "";
    if (!IsCallingInfoLegal(uid, pid, bundleName)) {
        BGTASK_LOGE("reset efficiency resources failed, calling info is illegal");
        return ERR_BGTASK_INVALID_PID_OR_UID;
    }
    uint64_t tokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!BundleManagerHelper::GetInstance()->IsSystemApp(tokenId)) {
        BGTASK_LOGE("reset efficiency resources failed, %{public}s is not system app", bundleName.c_str());
        return ERR_BGTASK_NOT_SYSTEM_APP;
    }

    auto exemptedResourceType = GetExemptedResourceType(MAX_RESOURCE_MASK, uid, bundleName);
    if (exemptedResourceType == 0) {
        BGTASK_LOGE("reset efficiency resources failed, no permitted resource type");
        return ERR_BGTASK_PERMISSION_DENIED;
    }

    handler_->PostTask([this, exemptedResourceType, uid, pid, bundleName]() {
        std::shared_ptr<ResourceCallbackInfo> callbackInfo = std::make_shared<ResourceCallbackInfo>(uid,
            pid, exemptedResourceType, bundleName);
        this->ResetEfficiencyResourcesInner(callbackInfo, false);
    });
    return ERR_OK;
}

void BgEfficiencyResourcesMgr::RemoveRelativeProcessRecord(int32_t uid, uint32_t resourceNumber)
{
    for (auto iter = procResourceApplyMap_.begin(); iter != procResourceApplyMap_.end(); iter++) {
        if (iter->second->uid_ == uid && (resourceNumber & iter->second->resourceNumber_) != 0) {
            uint32_t eraseBit = (resourceNumber & iter->second->resourceNumber_);
            std::shared_ptr<ResourceCallbackInfo> callbackInfo = std::make_shared<ResourceCallbackInfo>(uid,
                iter->second->pid_, eraseBit, iter->second->bundleName_);
            handler_->PostTask([this, callbackInfo]() {
                this->ResetEfficiencyResourcesInner(callbackInfo, true);
            });
        }
    }
}

void BgEfficiencyResourcesMgr::ResetEfficiencyResourcesInner(
    const std::shared_ptr<ResourceCallbackInfo> &callbackInfo, bool isProcess)
{
    BGTASK_LOGI("reset efficiency resources inner,  uid:%{public}d, pid %{public}d,"\
        " resource number: %{public}u, isProcess: %{public}d", callbackInfo->GetUid(),
        callbackInfo->GetPid(), callbackInfo->GetResourceNumber(), isProcess);
    if (isProcess) {
        RemoveTargetResourceRecord(procResourceApplyMap_, callbackInfo->GetPid(),
            callbackInfo->GetResourceNumber(), EfficiencyResourcesEventType::RESOURCE_RESET);
    } else {
        RemoveTargetResourceRecord(appResourceApplyMap_, callbackInfo->GetUid(),
            callbackInfo->GetResourceNumber(), EfficiencyResourcesEventType::APP_RESOURCE_RESET);
        RemoveRelativeProcessRecord(callbackInfo->GetUid(), callbackInfo->GetResourceNumber());
    }
    DelayedSingleton<DataStorageHelper>::GetInstance()->RefreshResourceRecord(
        appResourceApplyMap_, procResourceApplyMap_);
}

bool BgEfficiencyResourcesMgr::IsCallingInfoLegal(int32_t uid, int32_t pid, std::string &bundleName)
{
    if (uid < 0 || pid < 0) {
        BGTASK_LOGE("pid or uid is invalid");
        return false;
    }
    bundleName = BundleManagerHelper::GetInstance()->GetClientBundleName(uid);
    return true;
}

ErrCode BgEfficiencyResourcesMgr::AddSubscriber(const sptr<IBackgroundTaskSubscriber> &subscriber)
{
    BGTASK_LOGI("add subscriber to efficiency resources succeed");
    ErrCode result {};
    handler_->PostSyncTask([this, &result, &subscriber]() {
        result = subscriberMgr_->AddSubscriber(subscriber);
    });
    return result;
}

ErrCode BgEfficiencyResourcesMgr::RemoveSubscriber(const sptr<IBackgroundTaskSubscriber> &subscriber)
{
    BGTASK_LOGI("remove subscriber to efficiency resources succeed");
    ErrCode result {};
    handler_->PostSyncTask([this, &result, &subscriber]() {
        result = subscriberMgr_->RemoveSubscriber(subscriber);
    });
    return result;
}

ErrCode BgEfficiencyResourcesMgr::ShellDump(const std::vector<std::string> &dumpOption,
    std::vector<std::string> &dumpInfo)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGE("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    handler_->PostSyncTask([&]() {
        this->ShellDumpInner(dumpOption, dumpInfo);
    });
    return ERR_OK;
}

ErrCode BgEfficiencyResourcesMgr::ShellDumpInner(const std::vector<std::string> &dumpOption,
    std::vector<std::string> &dumpInfo)
{
    if (dumpOption[1] == DUMP_PARAM_LIST_ALL) {
        DumpAllApplicationInfo(dumpInfo);
    } else if (dumpOption[1] == DUMP_PARAM_RESET_ALL) {
        DumpResetAllResource(dumpInfo);
    } else if (dumpOption[1] == DUMP_PARAM_RESET_APP) {
        DumpResetResource(dumpOption, true, false);
    } else if (dumpOption[1] == DUMP_PARAM_RESET_PROC) {
        DumpResetResource(dumpOption, false, false);
    }
    DelayedSingleton<DataStorageHelper>::GetInstance()->RefreshResourceRecord(
        appResourceApplyMap_, procResourceApplyMap_);
    return ERR_OK;
}

void BgEfficiencyResourcesMgr::DumpAllApplicationInfo(std::vector<std::string> &dumpInfo)
{
    std::stringstream stream;
    if (appResourceApplyMap_.empty() && procResourceApplyMap_.empty()) {
        dumpInfo.emplace_back("No running efficiency resources\n");
        return;
    }
    DumpApplicationInfoMap(appResourceApplyMap_, dumpInfo, stream, "app efficiency resource: \n");
    DumpApplicationInfoMap(procResourceApplyMap_, dumpInfo, stream, "process efficiency resource: \n");
}

void BgEfficiencyResourcesMgr::DumpApplicationInfoMap(std::unordered_map<int32_t,
    std::shared_ptr<ResourceApplicationRecord>> &infoMap, std::vector<std::string> &dumpInfo,
    std::stringstream &stream, const char *headInfo)
{
    uint32_t index = 1;
    stream << headInfo << "\n";
    for (auto iter = infoMap.begin(); iter != infoMap.end(); iter++) {
        stream << "No." << index << "\n";
        stream << "\tefficiencyResourceKey: " << iter->first << "\n";
        stream << "\tefficiencyResourceValue:" << "\n";
        stream << "\t\tbundleName: " << iter->second->GetBundleName() << "\n";
        stream << "\t\tuid: " << iter->second->GetUid() << "\n";
        stream << "\t\tpid: " << iter->second->GetPid() << "\n";
        stream << "\t\tresourceNumber: " << iter->second->GetResourceNumber() << "\n";
        int64_t curTime = TimeProvider::GetCurrentTime();
        auto &resourceUnitList = iter->second->resourceUnitList_;
        for (auto unitIter = resourceUnitList.begin();
            unitIter != resourceUnitList.end(); ++unitIter) {
            stream << "\t\t\tresource type: " << ResourceTypeName[unitIter->resourceIndex_] << "\n";
            stream << "\t\t\tisPersist: " << (unitIter->isPersist_ ? "true" : "false") << "\n";
            if (!unitIter->isPersist_) {
                stream << "\t\t\tremainTime: " << unitIter->endTime_ - curTime << "\n";
            }
            stream << "\t\t\treason: " << unitIter->reason_ << "\n";
        }
        stream << "\n";
        dumpInfo.emplace_back(stream.str());
        stream.str("");
        stream.clear();
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
    auto &infoMap = cleanApp ? appResourceApplyMap_ : procResourceApplyMap_;
    auto type = cleanApp ? EfficiencyResourcesEventType::APP_RESOURCE_RESET
        : EfficiencyResourcesEventType::RESOURCE_RESET;

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
        int32_t mapKey = std::atoi(dumpOption[2].c_str());
        uint32_t cleanResource = static_cast<uint32_t>(std::atoi(dumpOption[3].c_str()));
        RemoveTargetResourceRecord(infoMap, mapKey, cleanResource, type);
    }
}

bool BgEfficiencyResourcesMgr::RemoveTargetResourceRecord(std::unordered_map<int32_t,
    std::shared_ptr<ResourceApplicationRecord>> &infoMap, int32_t mapKey, uint32_t cleanResource,
    EfficiencyResourcesEventType type)
{
    BGTASK_LOGD("resource record key: %{public}d, resource record size(): %{public}d",
        mapKey, static_cast<int32_t>(infoMap.size()));
    auto iter = infoMap.find(mapKey);
    if (iter == infoMap.end() || (iter->second->resourceNumber_ & cleanResource) == 0) {
        BGTASK_LOGW("remove single resource record failure, no matched task: %{public}d", mapKey);
        return false;
    }
    uint32_t eraseBit = (iter->second->resourceNumber_ & cleanResource);
    iter->second->resourceNumber_ ^= eraseBit;
    RemoveListRecord(iter->second->resourceUnitList_, eraseBit);
    auto callbackInfo = std::make_shared<ResourceCallbackInfo>(iter->second->GetUid(),
        iter->second->GetPid(), eraseBit, iter->second->GetBundleName());
    BGTASK_LOGI("remove record from info map, uid: %{public}d, bundle name: %{public}s",
        callbackInfo->GetUid(), callbackInfo->GetBundleName().c_str());
    subscriberMgr_->OnResourceChanged(callbackInfo, type);
    if (iter->second->resourceNumber_ == 0) {
        infoMap.erase(iter);
    }
    return true;
}

void BgEfficiencyResourcesMgr::RemoveListRecord(std::list<PersistTime> &resourceUnitList, uint32_t eraseBit)
{
    BGTASK_LOGD("start remove record from list, eraseBit: %{public}d", eraseBit);
    if (eraseBit == 0) {
        return;
    }
    for (auto it = resourceUnitList.begin(); it != resourceUnitList.end();) {
        if (((1 << it->resourceIndex_) & eraseBit) != 0) {
            it = resourceUnitList.erase(it);
        } else {
            ++it;
        }
    }
}

ErrCode BgEfficiencyResourcesMgr::GetEfficiencyResourcesInfos(std::vector<std::shared_ptr<
    ResourceCallbackInfo>> &appList, std::vector<std::shared_ptr<ResourceCallbackInfo>> &procList)
{
    handler_->PostSyncTask([this, &appList, &procList]() {
        this->GetEfficiencyResourcesInfosInner(appResourceApplyMap_, appList);
        this->GetEfficiencyResourcesInfosInner(procResourceApplyMap_, procList);
        }, AppExecFwk::EventQueue::Priority::HIGH);

    return ERR_OK;
}

void BgEfficiencyResourcesMgr::GetEfficiencyResourcesInfosInner(const ResourceRecordMap &infoMap,
    std::vector<std::shared_ptr<ResourceCallbackInfo>> &list)
{
    if (infoMap.empty()) {
        return;
    }
    BGTASK_LOGD("get efficiency resources info inner function, resources record size(): %{public}d ",
        static_cast<int32_t>(infoMap.size()));
    for (auto &record : infoMap) {
        auto appInfo = std::make_shared<ResourceCallbackInfo>(record.second->uid_, record.second->pid_,
            record.second->resourceNumber_, record.second->bundleName_);
        list.push_back(appInfo);
    }
}

uint32_t BgEfficiencyResourcesMgr::GetExemptedResourceType(uint32_t resourceNumber, const int32_t uid,
    const std::string &bundleName)
{
    const std::vector<int32_t>& resourcesApply = QueryRunningResourcesApply(uid, bundleName);

    // this app is permitted to use all type of resources
    if (std::find(resourcesApply.begin(), resourcesApply.end(), FREEZE_ALL_RESOURCES) != resourcesApply.end()) {
        return resourceNumber;
    }

    uint32_t exemptedResources = 0;
    if (resourcesApply.empty()) {
        return exemptedResources;
    }

    // filter out unpermitted resource type
    for (const auto resourceType : resourcesApply) {
        exemptedResources |= (1 << (resourceType - 1));
    }
    exemptedResources &= resourceNumber;
    BGTASK_LOGD("after filter, uid: %{public}d, bundleName: %{public}s, origin resource number: %{public}u, return "\
        "resource number: %{public}u", uid, bundleName.c_str(), resourceNumber, exemptedResources);

    return exemptedResources;
}

std::vector<int32_t> BgEfficiencyResourcesMgr::QueryRunningResourcesApply(const int32_t uid,
    const std::string &bundleName)
{
    AppExecFwk::ApplicationInfo applicationInfo;
    if (!BundleManagerHelper::GetInstance()->GetApplicationInfo(bundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, GetUserIdByUid(uid), applicationInfo)) {
        BGTASK_LOGE("failed to get applicationInfo from AppExecFwk, bundleName is %{public}s", bundleName.c_str());
        return {};
    }
    BGTASK_LOGD("size of applicationInfo.resourcesApply is %{public}d",
        static_cast<int32_t>(applicationInfo.resourcesApply.size()));
    return applicationInfo.resourcesApply;
}

int32_t BgEfficiencyResourcesMgr::GetUserIdByUid(int32_t uid)
{
    const int32_t BASE_USER_RANGE = 200000;
    return uid / BASE_USER_RANGE;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
