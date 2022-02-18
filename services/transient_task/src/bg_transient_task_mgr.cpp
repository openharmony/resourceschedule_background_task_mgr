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

#include "bg_transient_task_mgr.h"

#include <file_ex.h>
#include <ipc_skeleton.h>
#include <sstream>
#include <system_ability.h>
#include <system_ability_definition.h>

#include "bundle_mgr_proxy.h"
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "want.h"

#include "background_task_mgr_service.h"
#include "bgtaskmgr_inner_errors.h"
#include "time_provider.h"
#include "transient_task_app_info.h"
#include "transient_task_log.h"

using namespace std;

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static const std::string All_BGTASKMGR_OPTION = "All";
static const std::string LOW_BATTARY_OPTION = "BATTARY_LOW";
static const std::string OKAY_BATTARY_OPTION = "BATTARY_OKAY";
static const std::string SCREEN_ON_OPTION = "SCREEN_ON";
static const std::string SCREEN_OFF_OPTION = "SCREEN_OFF";
static const std::string CANCEL_DUMP_OPTION = "DUMP_CANCEL";

constexpr int32_t BG_INVALID_REMAIN_TIME = -1;
constexpr int32_t WATCHDOG_DELAY_TIME = 6 * MSEC_PER_SEC;
constexpr int32_t SERVICE_WAIT_TIME = 2000;
}

BgTransientTaskMgr::BgTransientTaskMgr() {}
BgTransientTaskMgr::~BgTransientTaskMgr() {}

void BgTransientTaskMgr::Init()
{
    runner_ = AppExecFwk::EventRunner::Create(true);
    if (!runner_) {
        BGTASK_LOGE("Failed to init due to create runner error");
        return;
    }
    handler_ = std::make_shared<AppExecFwk::EventHandler>(runner_);
    if (!handler_) {
        BGTASK_LOGE("Failed to init due to create handler error");
    }
    callbackDeathRecipient_ =
        new ExpiredCallbackDeathRecipient(DelayedSingleton<BackgroundTaskMgrService>::GetInstance().get());
    susriberDeathRecipient_ =
        new SubscriberDeathRecipient(DelayedSingleton<BackgroundTaskMgrService>::GetInstance().get());

    InitNecessaryState();
}

void BgTransientTaskMgr::InitNecessaryState()
{
    sptr<ISystemAbilityManager> systemAbilityManager
        = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr
        || systemAbilityManager->CheckSystemAbility(APP_MGR_SERVICE_ID) == nullptr
        || systemAbilityManager->CheckSystemAbility(COMMON_EVENT_SERVICE_ID) == nullptr
        || systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) == nullptr) {
        isReady_.store(false);
        BGTASK_LOGI("request system service is not ready yet!");
        std::function <void()> InitNecessaryStateFunc = std::bind(&BgTransientTaskMgr::InitNecessaryState, this);
        handler_->PostTask(InitNecessaryStateFunc, SERVICE_WAIT_TIME);
        return;
    }

    deviceInfoManeger_ = make_shared<DeviceInfoManager>();
    timerManager_ = make_shared<TimerManager>(DelayedSingleton<BackgroundTaskMgrService>::GetInstance().get());
    decisionMaker_ = make_shared<DecisionMaker>(timerManager_, deviceInfoManeger_);
    watchdog_ = make_shared<Watchdog>(DelayedSingleton<BackgroundTaskMgrService>::GetInstance().get(), decisionMaker_);

    inputManager_ = make_shared<InputManager>();
    inputManager_->RegisterEventListener(deviceInfoManeger_);
    inputManager_->RegisterEventListener(decisionMaker_);
    isReady_.store(true);
}

bool BgTransientTaskMgr::GetBundleNamesForUid(int32_t uid, std::string &bundleName)
{
    sptr<ISystemAbilityManager> systemMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemMgr == nullptr) {
        BGTASK_LOGE("Fail to get system ability mgr");
        return false;
    }

    sptr<IRemoteObject> remoteObject = systemMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        BGTASK_LOGE("Fail to get bundle manager proxy");
        return false;
    }

    sptr<OHOS::AppExecFwk::IBundleMgr> bundleMgrProxy = iface_cast<OHOS::AppExecFwk::IBundleMgr>(remoteObject);
    if (bundleMgrProxy == nullptr) {
        BGTASK_LOGE("Bundle mgr proxy is nullptr");
        return false;
    }

    if (!bundleMgrProxy->GetBundleNameForUid(uid, bundleName)) {
        BGTASK_LOGE("Get bundle name failed");
        return false;
    }
    return true;
}

bool BgTransientTaskMgr::IsCallingInfoLegal(int32_t uid, int32_t pid, std::string &name,
    const sptr<IExpiredCallback>& callback)
{
    if (!VerifyCallingInfo(uid, pid)) {
        BGTASK_LOGE("pid or uid is invalid.");
        return false;
    }

    if (!GetBundleNamesForUid(uid, name)) {
        BGTASK_LOGE("GetBundleNamesForUid fail.");
        return false;
    }

    if (callback == nullptr) {
        BGTASK_LOGE("callback is null.");
        return false;
    }

    if (callback->AsObject() == nullptr) {
        BGTASK_LOGE("remote in callback is null.");
        return false;
    }
    return true;
}

ErrCode BgTransientTaskMgr::RequestSuspendDelay(const std::u16string& reason,
    const sptr<IExpiredCallback>& callback, std::shared_ptr<DelaySuspendInfo> &delayInfo)
{
    if (!isReady_.load()) {
        BGTASK_LOGE("Transient task manager is not ready.");
        return ERR_BGTASK_SERVICE_NOT_READY;
    }
    auto uid = IPCSkeleton::GetCallingUid();
    auto pid = IPCSkeleton::GetCallingPid();
    std::string name = "";
    if (!IsCallingInfoLegal(uid, pid, name, callback)) {
        BGTASK_LOGE("Request suspend delay failed, calling info is illegal.");
        return ERR_BGTASK_INVALID_PARAM;
    }
    BGTASK_LOGI("request suspend delay pkg : %{public}s, reason : %{public}s, uid : %{public}d, pid : %{public}d",
        name.c_str(), Str16ToStr8(reason).c_str(), uid, pid);

    auto infoEx = make_shared<DelaySuspendInfoEx>(pid);
    delayInfo = infoEx;
    auto remote = callback->AsObject();
    lock_guard<mutex> lock(expiredCallbackLock_);
    auto findCallback = [&callback](const auto& callbackMap) {
        return callback->AsObject() == callbackMap.second->AsObject();
    };

    auto callbackIter = find_if(expiredCallbackMap_.begin(), expiredCallbackMap_.end(), findCallback);
    if (callbackIter != expiredCallbackMap_.end()) {
        BGTASK_LOGE("request suspend failed, callback is already exists.");
        return ERR_BGTASK_OBJECT_EXISTS;
    }

    auto keyInfo = make_shared<KeyInfo>(name, uid, pid);
    if (!decisionMaker_->Decide(keyInfo, infoEx)) {
        BGTASK_LOGE("request suspend failed, callback is already exists, not allow.");
        return ERR_BGTASK_NOT_ALLOWED;
    }
    BGTASK_LOGI("request suspend success, pkg : %{public}s, uid : %{public}d, requestId: %{public}d,"
        "delayTime: %{public}d", name.c_str(), uid, infoEx->GetRequestId(), infoEx->GetActualDelayTime());
    expiredCallbackMap_[infoEx->GetRequestId()] = callback;
    keyInfoMap_[infoEx->GetRequestId()] = keyInfo;
    if (callbackDeathRecipient_ != nullptr) {
        (void)remote->AddDeathRecipient(callbackDeathRecipient_);
    }
    handler_->PostSyncTask([&]() {
        auto appInfo = make_shared<TransientTaskAppInfo>(name, uid, pid);
        NotifyTransientTaskSuscriber(appInfo, TransientTaskEventType::TASK_START);
    });
    return ERR_OK;
}

void BgTransientTaskMgr::NotifyTransientTaskSuscriber(shared_ptr<TransientTaskAppInfo>& appInfo,
    TransientTaskEventType type)
{
    if (appInfo == nullptr) {
        BGTASK_LOGE("NotifyTransientTaskSuscriber failed, appInfo is null.");
        return;
    }
    if (subscriberList_.empty()) {
        BGTASK_LOGI("Transient Task Subscriber List is empty");
        return;
    }
    switch (type) {
        case TransientTaskEventType::TASK_START:
            for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); iter++) {
                (*iter)->OnTransientTaskStart(appInfo);
            }
            break;
        case TransientTaskEventType::TASK_END:
            for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); iter++) {
                (*iter)->OnTransientTaskEnd(appInfo);
            }
            break;
        default:
            break;
    }
}

ErrCode BgTransientTaskMgr::CancelSuspendDelay(int32_t requestId)
{
    if (!isReady_.load()) {
        BGTASK_LOGE("Transient task manager is not ready.");
        return ERR_BGTASK_SERVICE_NOT_READY;
    }
    auto uid = IPCSkeleton::GetCallingUid();
    auto pid = IPCSkeleton::GetCallingPid();
    if (!VerifyCallingInfo(uid, pid)) {
        BGTASK_LOGE(" cancel suspend delay failed, pid or uid is invalid.");
        return ERR_BGTASK_INVALID_PARAM;
    }

    std::string name = "";
    if (!GetBundleNamesForUid(uid, name)) {
        BGTASK_LOGE("GetBundleNamesForUid fail.");
        return ERR_BGTASK_INVALID_PARAM;
    }
    BGTASK_LOGI(" cancel suspend delay pkg : %{public}s, uid : %{public}d, requestId : %{public}d",
        name.c_str(), uid, requestId);

    if (!VerifyRequestIdLocked(name, uid, requestId)) {
        BGTASK_LOGE(" cancel suspend delay failed, requestId is illegal.");
        return ERR_BGTASK_NOT_ALLOWED;
    }

    handler_->PostSyncTask([&]() {
        auto appInfo = make_shared<TransientTaskAppInfo>(name, uid, pid);
        NotifyTransientTaskSuscriber(appInfo, TransientTaskEventType::TASK_END);
    });
    lock_guard<mutex> lock(expiredCallbackLock_);
    return CancelSuspendDelayLocked(requestId);
}

ErrCode BgTransientTaskMgr::CancelSuspendDelayLocked(int32_t requestId)
{
    watchdog_->RemoveWatchdog(requestId);
    decisionMaker_->RemoveRequest(keyInfoMap_[requestId], requestId);
    keyInfoMap_.erase(requestId);

    auto iter = expiredCallbackMap_.find(requestId);
    if (iter == expiredCallbackMap_.end()) {
        BGTASK_LOGE("Callback not found.");
        return ERR_BGTASK_NOT_ALLOWED;
    }
    auto remote = iter->second->AsObject();
    if (remote != nullptr) {
        remote->RemoveDeathRecipient(callbackDeathRecipient_);
    }
    expiredCallbackMap_.erase(iter);
    return ERR_OK;
}

void BgTransientTaskMgr::ForceCancelSuspendDelay(int32_t requestId)
{
    lock_guard<mutex> lock(expiredCallbackLock_);
    auto keyInfoIter = keyInfoMap_.find(requestId);
    if (keyInfoIter == keyInfoMap_.end()) {
        BGTASK_LOGE("force cancel suspend delay failed callback not found.");
        return;
    }
    handler_->PostSyncTask([&]() {
        CancelSuspendDelayLocked(requestId);
        auto appInfo = make_shared<TransientTaskAppInfo>(keyInfoIter->second->GetPkg(),
                                                        keyInfoIter->second->GetUid(),
                                                        keyInfoIter->second->GetPid());
        NotifyTransientTaskSuscriber(appInfo, TransientTaskEventType::TASK_END);
    });
}

ErrCode BgTransientTaskMgr::GetRemainingDelayTime(int32_t requestId, int32_t &delayTime)
{
    if (!isReady_.load()) {
        BGTASK_LOGE("Transient task manager is not ready.");
        return ERR_BGTASK_SERVICE_NOT_READY;
    }
    auto uid = IPCSkeleton::GetCallingUid();
    auto pid = IPCSkeleton::GetCallingPid();
    if (!VerifyCallingInfo(uid, pid)) {
        BGTASK_LOGE("get remain time failed, uid or pid is invalid");
        delayTime = BG_INVALID_REMAIN_TIME;
        return ERR_BGTASK_INVALID_PARAM;
    }

    std::string name = "";
    if (!GetBundleNamesForUid(uid, name)) {
        BGTASK_LOGE("GetBundleNamesForUid fail.");
        delayTime = BG_INVALID_REMAIN_TIME;
        return ERR_BGTASK_INVALID_PARAM;
    }
    BGTASK_LOGD("get remain time pkg : %{public}s, uid : %{public}d, requestId : %{public}d",
        name.c_str(), uid, requestId);

    if (!VerifyRequestIdLocked(name, uid, requestId)) {
        BGTASK_LOGE("get remain time failed, requestId is illegal.");
        delayTime = BG_INVALID_REMAIN_TIME;
        return ERR_BGTASK_NOT_ALLOWED;
    }

    delayTime = decisionMaker_->GetRemainingDelayTime(keyInfoMap_[requestId], requestId);
    return ERR_OK;
}

bool BgTransientTaskMgr::VerifyCallingInfo(int32_t uid, int32_t pid)
{
    return (uid >= 0) && (pid >= 0);
}

bool BgTransientTaskMgr::VerifyRequestIdLocked(const std::string& name, int32_t uid, int32_t requestId)
{
    auto keyInfoIter = keyInfoMap_.find(requestId);
    if (keyInfoIter == keyInfoMap_.end()) {
        return false;
    }
    return keyInfoIter->second->IsEqual(name, uid);
}

void BgTransientTaskMgr::HandleExpiredCallbackDeath(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        BGTASK_LOGE("expiredCallback death, remote in callback is null.");
        return;
    }

    lock_guard<mutex> lock(expiredCallbackLock_);
    auto findCallback = [&remote](const auto& callbackMap) {
        return callbackMap.second->AsObject() == remote;
    };

    auto callbackIter = find_if(expiredCallbackMap_.begin(), expiredCallbackMap_.end(), findCallback);
    if (callbackIter == expiredCallbackMap_.end()) {
        BGTASK_LOGE("expiredCallback death, remote in callback not found.");
        return;
    }

    watchdog_->RemoveWatchdog(callbackIter->first);
    auto keyInfoIter = keyInfoMap_.find(callbackIter->first);
    expiredCallbackMap_.erase(callbackIter);
    if (keyInfoIter == keyInfoMap_.end()) {
        BGTASK_LOGE("expiredCallback death, keyInfo not found.");
        return;
    }

    BGTASK_LOGI("expiredCallback death, %{public}s, requestId : %{public}d", keyInfoIter->second->ToString().c_str(),
        keyInfoIter->first);
    decisionMaker_->RemoveRequest(keyInfoIter->second, keyInfoIter->first);
    handler_->PostSyncTask([&]() {
        auto appInfo = make_shared<TransientTaskAppInfo>(keyInfoIter->second->GetPkg(),
                                                        keyInfoIter->second->GetUid(),
                                                        keyInfoIter->second->GetPid());
        NotifyTransientTaskSuscriber(appInfo, TransientTaskEventType::TASK_END);
    });
    keyInfoMap_.erase(keyInfoIter);
}

void BgTransientTaskMgr::HandleSubscriberDeath(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        BGTASK_LOGE("suscriber death, remote in suscriber is null.");
        return;
    }

    handler_->PostSyncTask([&]() {
        auto findSuscriber = [&remote](const auto& subscriberList) {
            return remote == subscriberList->AsObject();
        };
        auto subscriberIter = find_if(subscriberList_.begin(), subscriberList_.end(), findSuscriber);
        if (subscriberIter == subscriberList_.end()) {
            BGTASK_LOGE("suscriber death, remote in suscriber not found.");
            return;
        }

        subscriberList_.erase(subscriberIter);
        BGTASK_LOGI("suscriber death, remove it.");
    });
}

void BgTransientTaskMgr::HandleRequestExpired(const int32_t requestId)
{
    BGTASK_LOGI("request expired, id : %{public}d", requestId);

    std::lock_guard<std::mutex> lock(expiredCallbackLock_);
    auto callbackIter = expiredCallbackMap_.find(requestId);
    if (callbackIter == expiredCallbackMap_.end()) {
        BGTASK_LOGE("request expired, callback not found.");
        return;
    }
    callbackIter->second->OnExpired();

    auto keyInfoIter = keyInfoMap_.find(requestId);
    if (keyInfoIter == keyInfoMap_.end()) {
        BGTASK_LOGE("request expired, keyinfo not found.");
        return;
    }
    watchdog_->AddWatchdog(requestId, keyInfoIter->second, WATCHDOG_DELAY_TIME);
}

ErrCode BgTransientTaskMgr::SubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber)
{
    if (subscriber == nullptr) {
        BGTASK_LOGI("subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }
    auto remote = subscriber->AsObject();
    if (remote == nullptr) {
        BGTASK_LOGE("request suspend delay failed, remote in subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask([&]() {
        auto findSuscriber = [&remote](const auto& subscriberList) {
            return remote == subscriberList->AsObject();
        };
        auto subscriberIter = find_if(subscriberList_.begin(), subscriberList_.end(), findSuscriber);
        if (subscriberIter != subscriberList_.end()) {
            BGTASK_LOGE("request subscriber is already exists.");
            result =  ERR_BGTASK_OBJECT_EXISTS;
            return;
        }

        if (susriberDeathRecipient_ != nullptr) {
            remote->AddDeathRecipient(susriberDeathRecipient_);
        }
        subscriberList_.emplace_back(subscriber);
        BGTASK_LOGI("subscribe transient task success.");
    });
    return result;
}

ErrCode BgTransientTaskMgr::UnsubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber)
{
    if (subscriber == nullptr) {
        BGTASK_LOGI("subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }
    auto remote = subscriber->AsObject();
    if (remote == nullptr) {
        BGTASK_LOGE("request suspend delay failed, remote in subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }

    ErrCode result = ERR_OK;
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

ErrCode BgTransientTaskMgr::ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo)
{
    if (!isReady_.load()) {
        BGTASK_LOGE("Transient task manager is not ready.");
        return ERR_BGTASK_SERVICE_NOT_READY;
    }
    bool result = false;
    if (dumpOption[1] == All_BGTASKMGR_OPTION) {
        result = DumpAllRequestId(dumpInfo);
    } else if (dumpOption[1] == LOW_BATTARY_OPTION) {
        deviceInfoManeger_->SetDump(true);
        SendLowBatteryEvent(dumpInfo);
        result = true;
    } else if (dumpOption[1] == OKAY_BATTARY_OPTION) {
        deviceInfoManeger_->SetDump(true);
        SendOkayBatteryEvent(dumpInfo);
        result = true;
    } else if (dumpOption[1] == SCREEN_ON_OPTION) {
        deviceInfoManeger_->SetDump(true);
        SendScreenOnEvent(dumpInfo);
        result = true;
    } else if (dumpOption[1] == SCREEN_OFF_OPTION) {
        deviceInfoManeger_->SetDump(true);
        SendScreenOffEvent(dumpInfo);
        result = true;
    } else if (dumpOption[1] == CANCEL_DUMP_OPTION) {
        deviceInfoManeger_->SetDump(false);
        result = true;
    } else {
        dumpInfo.push_back("Error transient dump command!\n");
    }

    return result ? ERR_OK : ERR_BGTASK_NOT_ALLOWED;
}

void BgTransientTaskMgr::SendScreenOnEvent(std::vector<std::string> &dumpInfo)
{
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    
    data.SetCode(0);
    data.SetData("dump");
    if (EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo)) {
        dumpInfo.push_back("Publish COMMON_EVENT_SCREEN_ON succeed!\n");
    } else {
        dumpInfo.push_back("Publish COMMON_EVENT_SCREEN_ON failed!\n");
    }
}

void BgTransientTaskMgr::SendScreenOffEvent(std::vector<std::string> &dumpInfo)
{
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    
    data.SetCode(0);
    data.SetData("dump");
    if (EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo)) {
        dumpInfo.push_back("Publish COMMON_EVENT_SCREEN_OFF succeed!\n");
    } else {
        dumpInfo.push_back("Publish COMMON_EVENT_SCREEN_OFF failed!\n");
    }
}

void BgTransientTaskMgr::SendLowBatteryEvent(std::vector<std::string> &dumpInfo)
{
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_LOW);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    
    data.SetCode(0);
    data.SetData("dump");
    if (EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo)) {
        dumpInfo.push_back("Publish COMMON_EVENT_BATTERY_LOW succeed!\n");
    } else {
        dumpInfo.push_back("Publish COMMON_EVENT_BATTERY_LOW failed!\n");
    }
}

void BgTransientTaskMgr::SendOkayBatteryEvent(std::vector<std::string> &dumpInfo)
{
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    
    data.SetCode(0);
    data.SetData("dump");
    if (EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo)) {
        dumpInfo.push_back("Publish COMMON_EVENT_BATTERY_OKAY succeed!\n");
    } else {
        dumpInfo.push_back("Publish COMMON_EVENT_BATTERY_OKAY failed!\n");
    }
}

bool BgTransientTaskMgr::DumpAllRequestId(std::vector<std::string> &dumpInfo)
{
    if (keyInfoMap_.empty()) {
        dumpInfo.push_back("No Transient Task!\n");
        return true;
    }
    std::stringstream stream;
    int index = 1;
    for (auto record : keyInfoMap_) {
        stream.clear();
        stream.str("");
        stream << "No." << std::to_string(index++) << "\n";
        stream << "\tRequestId: " << record.first << "\n";
        stream << "\tAppName: " << record.second->GetPkg() << "\n";
        stream << "\tAppUid: " << record.second->GetUid() << "\n";
        stream << "\tAppPid: " << record.second->GetPid() << "\n";
        stream << "\tActualDelayTime: " << decisionMaker_->GetRemainingDelayTime(record.second, record.first) << "\n";
        stream << "\tRemainingQuota: " << decisionMaker_->GetQuota(record.second) << "\n";
        stream << "\n";
        dumpInfo.push_back(stream.str());
    }

    return true;
}

ExpiredCallbackDeathRecipient::ExpiredCallbackDeathRecipient(const wptr<BackgroundTaskMgrService>& service)
    : service_(service) {}

ExpiredCallbackDeathRecipient::~ExpiredCallbackDeathRecipient() {}

void ExpiredCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    auto service = service_.promote();
    if (service == nullptr) {
        BGTASK_LOGE("expired callback died, BackgroundTaskMgrService dead.");
        return;
    }
    service->HandleExpiredCallbackDeath(remote);
}

SubscriberDeathRecipient::SubscriberDeathRecipient(const wptr<BackgroundTaskMgrService>& service)
    : service_(service) {}

SubscriberDeathRecipient::~SubscriberDeathRecipient() {}

void SubscriberDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    auto service = service_.promote();
    if (service == nullptr) {
        BGTASK_LOGE("suscriber died, BackgroundTaskMgrService dead.");
        return;
    }
    service->HandleSubscriberDeath(remote);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS