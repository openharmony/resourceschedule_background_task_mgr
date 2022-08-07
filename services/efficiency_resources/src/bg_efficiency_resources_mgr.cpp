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
#include "app_mgr_client.h"
#include "event_runner.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "bgtaskmgr_inner_errors.h"

#include "efficiency_resource_log.h"
#include "resource_type.h"
#include "time_provider.h"

namespace OHOS {
namespace BackgroundTaskMgr {

static constexpr char BG_EFFICIENCY_RESOURCES_MGR_NAME[] = "BgEfficiencyResourcesMgr";
static constexpr char SEPARATOR[] = "_";
static constexpr char DUMP_PARAM_LIST_ALL[] = "--all";
static constexpr char DUMP_PARAM_RESET_ALL[] = "--reset_all";
static constexpr char DUMP_PARAM_RESET_APP[] = "--resetapp";
static constexpr char DUMP_PARAM_RESET_PROC[] = "--resetproc";
static constexpr int32_t DELAY_TIME = 2000;
static constexpr int32_t MAX_DUMP_PARAM_NUMS = 4;
static constexpr int32_t MAX_RESOURCES_TYPE = 7;
static constexpr int32_t MAX_RESOURCE_NUMBER = (1 << MAX_RESOURCES_TYPE) - 1;

BgEfficiencyResourcesMgr::BgEfficiencyResourcesMgr() {}

BgEfficiencyResourcesMgr::~BgEfficiencyResourcesMgr() {}

bool BgEfficiencyResourcesMgr::Init()
{
    runner_ = AppExecFwk::EventRunner::Create(BG_EFFICIENCY_RESOURCES_MGR_NAME);
    if (runner_ == nullptr) {
        BGTASK_LOGE("BgContinuousTaskMgr runner create failed!");
        return false;
    }
    handler_ = std::make_shared<AppExecFwk::EventHandler>(runner_);
    if (handler_ == nullptr) {
        BGTASK_LOGE("BgContinuousTaskMgr handler create failed!");
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
        || systemAbilityManager->CheckSystemAbility(COMMON_EVENT_SERVICE_ID) == nullptr) {
        BGTASK_LOGW("request system service is not ready yet!");
        auto task = [this](){this->InitNecessaryState(); };
        handler_->PostTask(task, DELAY_TIME);
        return;
    }
    isSysReady_.store(true);
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

    if (resourceInfo->GetResourceNumber() == 0 || resourceInfo->GetResourceNumber() > MAX_RESOURCE_NUMBER
        || resourceInfo->GetTimeOut() <= 0) {
        BGTASK_LOGE("continuous task params invalid!");
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
    ErrCode result = ERR_OK; 
    if (resourceInfo->IsApply()) {
        result = ApplyEfficiencyResourcesInner(callbackInfo, resourceInfo);
    } else {
        result = ResetEfficiencyResourcesInner(callbackInfo, resourceInfo->IsProcess());
    }
    return result;
}

ErrCode BgEfficiencyResourcesMgr::ApplyEfficiencyResourcesInner(const std::shared_ptr<ResourceCallbackInfo> 
    &callbackInfo, const sptr<EfficiencyResourceInfo> &resourceInfo)
{

    std::lock_guard<std::mutex> lock(callbackLock_);
    std::string mapKey {""};
    std::unordered_map<std::string, std::shared_ptr<ResourceApplicationRecord>> &infoMap = appResourceApplyMap_;
    if (resourceInfo->IsProcess()) {
        mapKey = std::to_string(callbackInfo->GetUid()) + SEPARATOR + callbackInfo->GetBundleName();
    } else {
        mapKey = std::to_string(callbackInfo->GetPid()) + SEPARATOR + callbackInfo->GetBundleName();
        infoMap = resourceApplyMap_;
    }
    auto appIter = infoMap.find(mapKey);
    if (appIter == infoMap.end()) {
        infoMap.emplace(mapKey, std::make_shared<ResourceApplicationRecord>(callbackInfo->GetUid(),
            callbackInfo->GetPid(), callbackInfo->GetResourceNumber(), callbackInfo->GetBundleName()));
    } else {
        appIter->second->reason_ = resourceInfo->GetReason();
        appIter->second->resourceNumber_ &= callbackInfo->GetResourceNumber();
    }
    UpdateResourcesEndtime(callbackInfo, appIter->second, resourceInfo->IsPersist(), resourceInfo->GetTimeOut());
    OnResourceChanged(callbackInfo, resourceInfo->IsProcess() ? EfficiencyResourcesEventType::RESOURCE_APPLY :
        EfficiencyResourcesEventType::APP_RESOURCE_APPLY);
}

void BgEfficiencyResourcesMgr::UpdateResourcesEndtime(const std::shared_ptr<ResourceCallbackInfo> 
    &callbackInfo, std::shared_ptr<ResourceApplicationRecord> &record, bool isPersist, int32_t timeOut){
    uint32_t timeOutBit = 0;
    for (int resourceIndex = 0; resourceIndex < MAX_RESOURCES_TYPE; ++resourceIndex) {
        if ((callbackInfo->GetResourceNumber() & (1 << resourceIndex)) != 0) {
            auto resourceUnitIter = record->resourceUnitMap_.find(resourceIndex);
            int64_t lastTime = 0; 
            if (resourceUnitIter == record->resourceUnitMap_.end()) {
                record->resourceUnitMap_.emplace(resourceIndex, PersistTime{isPersist,
                    TimeProvider::GetCurrentTime() + timeOut});
            } else {
                resourceUnitIter->second.isPersist_ = resourceUnitIter->second.isPersist_ || isPersist;
                lastTime = resourceUnitIter->second.endTime_;
                resourceUnitIter->second.endTime_ = std::max(resourceUnitIter->second.endTime_,
                    TimeProvider::GetCurrentTime() + timeOut);
            }
            resourceUnitIter = record->resourceUnitMap_.find(resourceIndex);
            if (resourceUnitIter->second.isPersist_) {
                resourceUnitIter->second.endTime_ = 0;
            } else if (resourceUnitIter->second.endTime_ > lastTime) {
                timeOutBit |= (1 << resourceIndex);
                std::string mapKey = std::to_string(callbackInfo->GetUid()) + SEPARATOR + callbackInfo->GetBundleName();
                if (record->IsProcess()){
                    mapKey = std::to_string(callbackInfo->GetPid()) + SEPARATOR + callbackInfo->GetBundleName()
                }
                auto task = [mgr=shared_from_this(), mapKey, timeOutBit, isProcess=record->IsProcess()] () {
                    mgr->ResetTimeOutResource(mapKey, timeOutBit, isProcess);
                };
                handler_->PostTask(task, timeOut);
            }
        }
    }
    OnResourceChanged(callbackInfo, record->IsProcess() ? EfficiencyResourcesEventType::RESOURCE_RESET :
        EfficiencyResourcesEventType::APP_RESOURCE_RESET);
}

void BgEfficiencyResourcesMgr::ResetTimeOutResource(std::string mapKey, uint32_t timeOutBit, bool isProcess){
    if (!isSysReady_.load()) {
        BGTASK_LOGW("Efficiency resources manager is not ready.");
        return;
    }
    std::lock_guard<std::mutex> lock(callbackLock_);
    for (int resourceIndex = 0; resourceIndex < MAX_RESOURCES_TYPE; ++resourceIndex) {
        if ((timeOutBit & (1 << resourceIndex)) == 0) {
            continue;
        }
        auto &infoMap = !isProcess ? appResourceApplyMap_ : resourceApplyMap_;
        auto type = isProcess ? EfficiencyResourcesEventType::RESOURCE_RESET : EfficiencyResourcesEventType::APP_RESOURCE_RESET;
        auto iter = infoMap.find(mapKey);
        uint32_t resetZeros = 0;
        if (iter != infoMap.end() && iter->second->resourceUnitMap_.count(resourceIndex) > 0) {
            auto endTime = iter->second->resourceUnitMap_[resourceIndex].endTime_;
            if (TimeProvider::GetCurrentTime() >= endTime) {
                resetZeros |= 1 << resourceIndex;
                iter->second->resourceNumber_ ^= 1 << resourceIndex;
            }
        }
        auto callbackInfo = std::make_shared<ResourceCallbackInfo>(iter->second->uid_, iter->second->pid_, resetZeros,
            iter->second->bundleName_);
        OnResourceChanged(callbackInfo, type);
        if(iter->second->resourceNumber_ == 0) {
            infoMap.erase(iter);
        }
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
    ErrCode result = ERR_OK; 
    result = ResetEfficiencyResourcesInner(callbackInfo, false);
    return result;
}

ErrCode BgEfficiencyResourcesMgr::ResetEfficiencyResourcesInner(
    const std::shared_ptr<ResourceCallbackInfo> &callbackInfo, bool isProcess)
{
    std::lock_guard<std::mutex> lock(callbackLock_);
    if(!isProcess) {
        auto type = EfficiencyResourcesEventType::APP_RESOURCE_RESET;
        std::string mapKey = std::to_string(callbackInfo->GetUid()) + SEPARATOR + callbackInfo->GetBundleName();
        RemoveSingleResourceRecord(appResourceApplyMap_, mapKey, callbackInfo->GetResourceNumber(), type);
        for(auto iter = resourceApplyMap_.begin(); iter != resourceApplyMap_.end(); iter++){
            if(iter->second->uid_ == callbackInfo->GetUid() && iter->second->bundleName_ == callbackInfo->GetBundleName()
                && (callbackInfo->GetResourceNumber() & iter->second->resourceNumber_) != 0) {
                    iter->second->resourceNumber_ ^= (callbackInfo->GetResourceNumber() & iter->second->resourceNumber_);
            }
        }
        auto findEmptyResource = [](auto &iter) { return iter->second->resourceNumber_ == 0; };
        remove_if(resourceApplyMap_.begin(), resourceApplyMap_.end(), findEmptyResource);
    } else {
        auto type = EfficiencyResourcesEventType::RESOURCE_RESET;
        std::string mapKey = std::to_string(callbackInfo->GetPid()) + SEPARATOR + callbackInfo->GetBundleName();
        RemoveSingleResourceRecord(resourceApplyMap_, mapKey, callbackInfo->GetResourceNumber(), type);
    }
}

void BgEfficiencyResourcesMgr::OnResourceChanged(const std::shared_ptr<ResourceCallbackInfo> &callbackInfo,
    EfficiencyResourcesEventType type)
{
    if (callbackInfo == nullptr) {
        BGTASK_LOGW("ContinuousTaskRecord is null");
        return;
    }

    if (subscriberList_.empty()) {
        BGTASK_LOGI("Background Task Subscriber List is empty");
        return;
    }
    switch (type) {
        case EfficiencyResourcesEventType::APP_RESOURCE_APPLY:
            for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); ++iter) {
                (*iter)->OnAppEfficiencyResourcesApply(callbackInfo);
            }
            break;
        case EfficiencyResourcesEventType::RESOURCE_APPLY:
            for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); ++iter) {
                (*iter)->OnEfficiencyResourcesApply(callbackInfo);
            }
            break;
        case EfficiencyResourcesEventType::APP_RESOURCE_RESET:
            for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); ++iter) {
                (*iter)->OnAppEfficiencyResourcesReset(callbackInfo);
            }
            break;
        case EfficiencyResourcesEventType::RESOURCE_RESET:
            for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); ++iter) {
                (*iter)->OnEfficiencyResourcesReset(callbackInfo);
            }
            break;
    }
}

bool BgEfficiencyResourcesMgr::IsCallingInfoLegal(int32_t uid, int32_t pid, std::string &bundleName)
{
    if (!VerifyCallingInfo(uid, pid)) {
        BGTASK_LOGE("pid or uid is invalid.");
        return false;
    }

    if (!bundleMgrProxy_->GetBundleNameForUid(uid, bundleName)) {
        BGTASK_LOGE("IsCallingInfoLegal GetBundleNameForUid fail.");
        return false;
    }
    return true;
}

bool BgEfficiencyResourcesMgr::VerifyCallingInfo(int32_t uid, int32_t pid)
{
    return (uid >= 0) && (pid >= 0);
}

ErrCode BgEfficiencyResourcesMgr::AddSubscriber(const sptr<IBackgroundTaskSubscriber>& subscriber)
{
    if (subscriber == NULL) {
        BGTASK_LOGI("subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }
    auto remote = subscriber->AsObject();
    if (remote == nullptr) {
        BGTASK_LOGE("remote in subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }

    std::lock_guard<std::mutex> subcriberLock(subscriberLock_);
    auto subscriberIter = find_if(subscriberList_.begin(), subscriberList_.end(),[&remote](const auto &subscriber){
        return subscriber->AsObject() == remote;
    });
    if (subscriberIter != subscriberList_.end()) {
        BGTASK_LOGE("subscriber has already exist");
        return ERR_BGTASK_OBJECT_EXISTS;
    }
    subscriberList_.emplace_back(subscriber);

    if (subscriberRecipients_.find(subscriber->AsObject()) != subscriberRecipients_.end()) {
        return ERR_BGTASK_OBJECT_EXISTS;
    }
    sptr<RemoteDeathRecipient> deathRecipient = new (std::nothrow) RemoteDeathRecipient(
        [this](const wptr<IRemoteObject> &remote) { this->OnRemoteSubscriberDied(remote); });
    if (!deathRecipient) {
        BGTASK_LOGE("create death recipient failed");
        return ERR_BGTASK_INVALID_PARAM;
    }
    subscriber->AsObject()->AddDeathRecipient(deathRecipient);
    subscriberRecipients_.emplace(subscriber->AsObject(), deathRecipient);
    subscriber->OnConnected();
    BGTASK_LOGD("Add continuous task subscriber succeed");
    return ERR_OK;
}

void BgEfficiencyResourcesMgr::OnRemoteSubscriberDied(const wptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> subcriberLock(subscriberLock_);
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }
    if (object == nullptr) {
        BGTASK_LOGE("remote object is null.");
        return;
    }
    sptr<IRemoteObject> objectProxy = object.promote();
    if (!objectProxy) {
        BGTASK_LOGE("get remote object failed");
        return;
    }
    auto iter = bgTaskSubscribers_.begin();
    while (iter != bgTaskSubscribers_.end()) {
        if ((*iter)->AsObject() == objectProxy) {
            iter = bgTaskSubscribers_.erase(iter);
        } else {
            iter++;
        }
    }
    subscriberRecipients_.erase(objectProxy);
}

ErrCode BgEfficiencyResourcesMgr::RemoveSubscriber(const sptr<IBackgroundTaskSubscriber>& subscriber)
{
    if (subscriber == nullptr) {
        BGTASK_LOGE("subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }
    auto remote = subscriber->AsObject();
    if (remote == nullptr) {
        BGTASK_LOGE("apply efficiency resources failed, remote in subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }

    ErrCode result = ERR_OK;
    std::lock_guard<std::mutex> subcriberLock(subscriberLock_);

    handler_->PostSyncTask([&]() {
        auto findSuscriber = [&remote](const auto& subscriberList) {
            return remote == subscriberList->AsObject();
        };
        auto subscriberIter = find_if(subscriberList_.begin(), subscriberList_.end(), findSuscriber);
        if (subscriberIter == subscriberList_.end()) {
            BGTASK_LOGE("request subscriber is not exists.");
            result = ERR_BGTASK_OBJECT_EXISTS;
            return;
        }
        remote->RemoveDeathRecipient(susriberDeathRecipient_);
        subscriberList_.erase(subscriberIter);
        BGTASK_LOGI("unsubscribe transient task success.");
    });
    return result;
}

bool BgEfficiencyResourcesMgr::GetBundleNamesForUid(int32_t uid, std::string &bundleName)
{
    if (!bundleMgrProxy_->GetBundleNameForUid(uid, bundleName)) {
        BGTASK_LOGE("Get bundle name failed");
        return false;
    }
    return true;
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

void BgEfficiencyResourcesMgr::DumpApplicationInfoMap(std::unordered_map<std::string, 
    std::shared_ptr<ResourceApplicationRecord>> &infoMap, std::vector<std::string> &dumpInfo,
    std::stringstream &stream, const char *headInfo)
{
    uint32_t index = 1;
    for (auto iter = infoMap.begin(); iter != infoMap.end(); iter++) {
        stream.str("");
        stream.clear();
        stream << headInfo << "\n";
        stream << "No." << index;
        stream << "\efficiencyResourceKey: " << iter->first << "\n";
        stream << "\tefficiencyResourceValue:" << "\n";
        stream << "\t\tbundleName: " << iter->second->GetBundleName() << "\n";
        stream << "\t\tuid: " << iter->second->GetUid() << "\n";
        stream << "\t\tpid: " << iter->second->GetPid() << "\n";
        stream << "\t\tisprocess: " << (iter->second->IsProcess() ? "true" : "false") << "\n";
        stream << "\t\resourceNumber: " << iter->second->GetResourceNumber() << "\n";
        stream << "\t\treason: " << iter->second->GetReason() << "\n";
        int64_t curTime = TimeProvider::GetCurrentTime();
        for(auto unitIter = iter->second->resourceUnitMap_.begin(); 
            unitIter != iter->second->resourceUnitMap_.end(); ++unitIter) {
            if (curTime <= unitIter->second.endTime_) {
                stream << "\t\t\tresource type: " << ResourceTypeName[unitIter->first] << "\n";
                stream << "\t\t\tisPersist " << (unitIter->second.isPersist_ ? "true" : "false") << "\n";
                stream << "\t\t\tremainTime " << curTime - unitIter->second.endTime_ << "\n";
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

void BgEfficiencyResourcesMgr::DumpResetResource(const std::vector<std::string> &dumpOption, bool cleanApp, bool cleanAll)
{
    EfficiencyResourcesEventType type = EfficiencyResourcesEventType::APP_RESOURCE_RESET;
    std::unordered_map<std::string, std::shared_ptr<ResourceApplicationRecord>> &infoMap = appResourceApplyMap_;
    if (!cleanApp) {
        type = EfficiencyResourcesEventType::RESOURCE_RESET;
        infoMap = resourceApplyMap_;
    }
    if (cleanAll) {
        for (auto iter = infoMap.begin(); iter != infoMap.end(); ++iter) {
            std::shared_ptr<ResourceCallbackInfo> callbackInfo = std::make_shared<ResourceCallbackInfo>(iter->second->GetUid(),
                iter->second->GetPid(), iter->second->GetResourceNumber(), iter->second->GetBundleName());
            OnResourceChanged(callbackInfo, type);
        }
        infoMap.clear();
    } else {
        if (dumpOption.size() < MAX_DUMP_PARAM_NUMS) {
            BGTASK_LOGW("invalid dump param");
            return;
        }
        std::string mapKey = dumpOption[2];
        uint32_t cleanResource = std::stoi(dumpOption[3]);
        RemoveSingleResourceRecord(infoMap, mapKey, cleanResource, type);
    }
}

bool BgEfficiencyResourcesMgr::RemoveSingleResourceRecord(std::unordered_map<std::string, std::shared_ptr<ResourceApplicationRecord>> &infoMap, 
    const std::string &mapKey, uint32_t cleanResource, EfficiencyResourcesEventType type)
{
    BGTASK_LOGD("resource record key: %{public}s", mapKey.c_str());
    auto iter = infoMap.find(mapKey);
    if (iter== infoMap.end() || (iter->second->resourceNumber_ & cleanResource) == 0) {
        BGTASK_LOGW("remove single resource record failure, no matched task: %{public}s", mapKey.c_str());
        return false;
    }
    auto callbackInfo = std::make_shared<ResourceCallbackInfo>(iter->second->GetUid(),
            iter->second->GetUid(), cleanResource, iter->second->GetBundleName());
    iter->second->resourceNumber_ ^= (iter->second->resourceNumber_ & cleanResource);
    if (iter->second->resourceNumber_ == 0) {
        infoMap.erase(iter);
    }
    OnResourceChanged(callbackInfo, type);
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
