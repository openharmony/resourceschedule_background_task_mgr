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

#include "bg_transient_task_mgr.h"

#include <file_ex.h>
#include <ipc_skeleton.h>
#include <sstream>
#include <system_ability.h>
#include <system_ability_definition.h>

#include "accesstoken_kit.h"
#include "bundle_mgr_proxy.h"
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "want.h"

#include "background_task_mgr_service.h"
#include "bgtask_hitrace_chain.h"
#include "bgtaskmgr_inner_errors.h"
#include "time_provider.h"
#include "transient_task_log.h"
#include "hitrace_meter.h"

using namespace std;

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static const std::string ALL_BGTASKMGR_OPTION = "All";
static const std::string LOW_BATTARY_OPTION = "BATTARY_LOW";
static const std::string OKAY_BATTARY_OPTION = "BATTARY_OKAY";
static const std::string CANCEL_DUMP_OPTION = "DUMP_CANCEL";
static const std::string PAUSE_DUMP_OPTION = "PAUSE";
static const std::string START_DUMP_OPTION = "START";
static const int32_t DUMP_PARAM_INDEX_TWO = 2;

constexpr int32_t BG_INVALID_REMAIN_TIME = -1;
constexpr int32_t WATCHDOG_DELAY_TIME = 6 * MSEC_PER_SEC;
constexpr int32_t SERVICE_WAIT_TIME = 2000;

const std::set<std::string> SUSPEND_NATIVE_OPERATE_CALLER = {
    "resource_schedule_service",
    "hidumper_service",
};
}

#ifdef BGTASK_MGR_UNIT_TEST
#define WEAK_FUNC __attribute__((weak))
#else
#define WEAK_FUNC
#endif

BgTransientTaskMgr::BgTransientTaskMgr() {}
BgTransientTaskMgr::~BgTransientTaskMgr() {}

void BgTransientTaskMgr::Init(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
{
    BGTASK_LOGI("BgTransientTaskMgr service init start");
    if (runner == nullptr) {
        BGTASK_LOGE("Failed to init due to create runner error");
        return;
    }
    handler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    if (!handler_) {
        BGTASK_LOGE("Failed to init due to create handler error");
    }
    callbackDeathRecipient_ = new (std::nothrow)
        ExpiredCallbackDeathRecipient(DelayedSingleton<BackgroundTaskMgrService>::GetInstance().get());
    susriberDeathRecipient_ = new (std::nothrow)
        SubscriberDeathRecipient(DelayedSingleton<BackgroundTaskMgrService>::GetInstance().get());

    InitNecessaryState(runner);
}

void BgTransientTaskMgr::InitNecessaryState(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
{
    sptr<ISystemAbilityManager> systemAbilityManager
        = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr
        || systemAbilityManager->CheckSystemAbility(APP_MGR_SERVICE_ID) == nullptr
        || systemAbilityManager->CheckSystemAbility(COMMON_EVENT_SERVICE_ID) == nullptr
        || systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) == nullptr) {
        isReady_.store(false);
        BGTASK_LOGI("request system service is not ready yet!");
        auto InitNecessaryStateFunc = [this, runner] { this->InitNecessaryState(runner); };
        handler_->PostTask(InitNecessaryStateFunc, SERVICE_WAIT_TIME);
        return;
    }

    deviceInfoManeger_ = make_shared<DeviceInfoManager>();
    timerManager_ = make_shared<TimerManager>(DelayedSingleton<BackgroundTaskMgrService>::GetInstance().get(), runner);
    decisionMaker_ = make_shared<DecisionMaker>(timerManager_, deviceInfoManeger_);
    watchdog_ = make_shared<Watchdog>(DelayedSingleton<BackgroundTaskMgrService>::GetInstance().get(),
        decisionMaker_, runner);

    inputManager_ = make_shared<InputManager>(runner);
    if (inputManager_ == nullptr) {
        BGTASK_LOGE("Fail to make inputManager");
        return;
    }
    inputManager_->RegisterEventHub();
    inputManager_->RegisterEventListener(deviceInfoManeger_);
    inputManager_->RegisterEventListener(decisionMaker_);
    isReady_.store(true);
    DelayedSingleton<BackgroundTaskMgrService>::GetInstance()->SetReady(ServiceReadyState::TRANSIENT_SERVICE_READY);
    BGTASK_LOGI("SetReady TRANSIENT_SERVICE_READY");
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

    if (bundleMgrProxy->GetNameForUid(uid, bundleName) != ERR_OK) {
        BGTASK_LOGE("Get bundle name failed");
        return false;
    }
    return true;
}

ErrCode BgTransientTaskMgr::IsCallingInfoLegal(int32_t uid, int32_t pid, std::string &name,
    const sptr<IExpiredCallback>& callback)
{
    if (!VerifyCallingInfo(uid, pid)) {
        BGTASK_LOGE("pid or uid is invalid.");
        return ERR_BGTASK_INVALID_PID_OR_UID;
    }

    if (!GetBundleNamesForUid(uid, name)) {
        BGTASK_LOGE("GetBundleNamesForUid fail.");
        return ERR_BGTASK_INVALID_BUNDLE_NAME;
    }

    if (callback == nullptr) {
        BGTASK_LOGE("callback is null.");
        return ERR_BGTASK_INVALID_CALLBACK;
    }

    if (callback->AsObject() == nullptr) {
        BGTASK_LOGE("remote in callback is null.");
        return ERR_BGTASK_INVALID_CALLBACK;
    }
    return ERR_OK;
}

ErrCode BgTransientTaskMgr::RequestSuspendDelay(const std::u16string& reason,
    const sptr<IExpiredCallback>& callback, std::shared_ptr<DelaySuspendInfo> &delayInfo)
{
    BgTaskHiTraceChain traceChain(__func__);
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::TransientTask::Service::RequestSuspendDelay");

    if (!isReady_.load()) {
        BGTASK_LOGW("Transient task manager is not ready.");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    auto uid = IPCSkeleton::GetCallingUid();
    auto pid = IPCSkeleton::GetCallingPid();
    std::string name = "";
    ErrCode ret = IsCallingInfoLegal(uid, pid, name, callback);
    if (ret != ERR_OK) {
        BGTASK_LOGI("Request suspend delay failed, calling info is illegal.");
        return ret;
    }
    BGTASK_LOGD("request suspend delay pkg : %{public}s, reason : %{public}s, uid : %{public}d, pid : %{public}d",
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
        BGTASK_LOGI("%{public}s request suspend failed, callback is already exists.", name.c_str());
        return ERR_BGTASK_CALLBACK_EXISTS;
    }

    auto keyInfo = make_shared<KeyInfo>(name, uid, pid);
    ret = decisionMaker_->Decide(keyInfo, infoEx);
    if (ret != ERR_OK) {
        BGTASK_LOGI("%{public}s request suspend failed.", name.c_str());
        return ret;
    }
    BGTASK_LOGI("request suspend success, pkg : %{public}s, uid : %{public}d, pid : %{public}d, requestId: %{public}d,"
        "delayTime: %{public}d", name.c_str(), uid, pid, infoEx->GetRequestId(), infoEx->GetActualDelayTime());
    expiredCallbackMap_[infoEx->GetRequestId()] = callback;
    keyInfoMap_[infoEx->GetRequestId()] = keyInfo;
    if (callbackDeathRecipient_ != nullptr) {
        (void)remote->AddDeathRecipient(callbackDeathRecipient_);
    }

    return ERR_OK;
}

bool WEAK_FUNC BgTransientTaskMgr::CheckProcessName()
{
    Security::AccessToken::AccessTokenID tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo callingTokenInfo;
    Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(tokenId, callingTokenInfo);
    if (SUSPEND_NATIVE_OPERATE_CALLER.find(callingTokenInfo.processName) == SUSPEND_NATIVE_OPERATE_CALLER.end()) {
        return false;
    }
    return true;
}

ErrCode BgTransientTaskMgr::PauseTransientTaskTimeForInner(int32_t uid)
{
    if (!isReady_.load()) {
        BGTASK_LOGW("Transient task manager is not ready.");
        return ERR_BGTASK_SYS_NOT_READY;
    }

    if (!CheckProcessName()) {
        return ERR_BGTASK_INVALID_PROCESS_NAME;
    }

    if (uid < 0) {
        BGTASK_LOGE("PauseTransientTaskTimeForInner uid is invalid.");
        return ERR_BGTASK_INVALID_PID_OR_UID;
    }

    std::string name = "";
    if (!GetBundleNamesForUid(uid, name)) {
        BGTASK_LOGE("GetBundleNamesForUid fail, uid : %{public}d.", uid);
        return ERR_BGTASK_SERVICE_INNER_ERROR;
    }

    ErrCode ret = decisionMaker_->PauseTransientTaskTimeForInner(uid, name);
    if (ret != ERR_OK) {
        BGTASK_LOGE("pkgname: %{public}s, uid: %{public}d PauseTransientTaskTimeForInner fail.",
            name.c_str(), uid);
        return ret;
    }
    lock_guard<mutex> lock(transientUidLock_);
    transientPauseUid_.insert(uid);
    return ERR_OK;
}

ErrCode BgTransientTaskMgr::StartTransientTaskTimeForInner(int32_t uid)
{
    if (!isReady_.load()) {
        BGTASK_LOGW("Transient task manager is not ready.");
        return ERR_BGTASK_SYS_NOT_READY;
    }

    if (!CheckProcessName()) {
        return ERR_BGTASK_INVALID_PROCESS_NAME;
    }

    if (uid < 0) {
        BGTASK_LOGE("StartTransientTaskTimeForInner uid is invalid.");
        return ERR_BGTASK_INVALID_PID_OR_UID;
    }

    std::string name = "";
    if (!GetBundleNamesForUid(uid, name)) {
        BGTASK_LOGE("GetBundleNamesForUid fail, uid : %{public}d.", uid);
        return ERR_BGTASK_SERVICE_INNER_ERROR;
    }

    ErrCode ret = decisionMaker_->StartTransientTaskTimeForInner(uid, name);
    if (ret != ERR_OK) {
        BGTASK_LOGE("pkgname: %{public}s, uid: %{public}d StartTransientTaskTimeForInner fail.",
            name.c_str(), uid);
        return ret;
    }
    lock_guard<mutex> lock(transientUidLock_);
    transientPauseUid_.erase(uid);
    return ERR_OK;
}

void BgTransientTaskMgr::HandleTransientTaskSuscriberTask(const shared_ptr<TransientTaskAppInfo>& appInfo,
    const TransientTaskEventType type)
{
    if (handler_ == nullptr) {
        BGTASK_LOGE("HandleTransientTaskSuscriberTask handler is not init.");
        return;
    }
    handler_->PostTask([=]() {
        NotifyTransientTaskSuscriber(appInfo, type);
    });
}

void BgTransientTaskMgr::NotifyTransientTaskSuscriber(const shared_ptr<TransientTaskAppInfo>& appInfo,
    const TransientTaskEventType type)
{
    if (appInfo == nullptr) {
        BGTASK_LOGE("NotifyTransientTaskSuscriber failed, appInfo is null.");
        return;
    }
    if (subscriberList_.empty()) {
        BGTASK_LOGI("Transient Task Subscriber List is empty");
        return;
    }
    const TransientTaskAppInfo& appInfoRef = *appInfo;
    switch (type) {
        case TransientTaskEventType::TASK_START:
            for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); iter++) {
                (*iter)->OnTransientTaskStart(appInfoRef);
            }
            break;
        case TransientTaskEventType::TASK_END:
            for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); iter++) {
                (*iter)->OnTransientTaskEnd(appInfoRef);
            }
            break;
        case TransientTaskEventType::TASK_ERR:
            for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); iter++) {
                (*iter)->OnTransientTaskErr(appInfoRef);
            }
            break;
        case TransientTaskEventType::APP_TASK_START:
            for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); iter++) {
                (*iter)->OnAppTransientTaskStart(appInfoRef);
            }
            break;
        case TransientTaskEventType::APP_TASK_END:
            for (auto iter = subscriberList_.begin(); iter != subscriberList_.end(); iter++) {
                (*iter)->OnAppTransientTaskEnd(appInfoRef);
            }
            break;
        default:
            break;
    }
}

ErrCode BgTransientTaskMgr::CancelSuspendDelay(int32_t requestId)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::TransientTask::Service::CancelSuspendDelay");

    if (!isReady_.load()) {
        BGTASK_LOGE("Transient task manager is not ready.");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    auto uid = IPCSkeleton::GetCallingUid();
    auto pid = IPCSkeleton::GetCallingPid();
    if (!VerifyCallingInfo(uid, pid)) {
        BGTASK_LOGE("cancel suspend delay failed, pid or uid is invalid.");
        return ERR_BGTASK_INVALID_PID_OR_UID;
    }

    std::string name = "";
    if (!GetBundleNamesForUid(uid, name)) {
        BGTASK_LOGW("GetBundleNamesForUid fail, uid : %{public}d.", uid);
        return ERR_BGTASK_SERVICE_INNER_ERROR;
    }
    BGTASK_LOGI("cancel suspend delay pkg : %{public}s, uid : %{public}d, pid : %{public}d, requestId : %{public}d",
        name.c_str(), uid, pid, requestId);

    lock_guard<mutex> lock(expiredCallbackLock_);
    if (!VerifyRequestIdLocked(name, uid, requestId)) {
        BGTASK_LOGE(" cancel suspend delay failed, requestId is illegal.");
        return ERR_BGTASK_INVALID_REQUEST_ID;
    }

    return CancelSuspendDelayLocked(requestId);
}

ErrCode BgTransientTaskMgr::CancelSuspendDelayLocked(int32_t requestId)
{
    watchdog_->RemoveWatchdog(requestId);
    decisionMaker_->RemoveRequest(keyInfoMap_[requestId], requestId);
    keyInfoMap_.erase(requestId);

    auto iter = expiredCallbackMap_.find(requestId);
    if (iter == expiredCallbackMap_.end()) {
        BGTASK_LOGE("CancelSuspendDelayLocked Callback not found.");
        return ERR_BGTASK_CALLBACK_NOT_EXIST;
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
    BGTASK_LOGI("force cancel suspend delay, keyInfo: %{public}s", keyInfoIter->second->ToString().c_str());
    CancelSuspendDelayLocked(requestId);
}

ErrCode BgTransientTaskMgr::GetRemainingDelayTime(int32_t requestId, int32_t &delayTime)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::TransientTask::Service::GetRemainingDelayTime");

    if (!isReady_.load()) {
        BGTASK_LOGW("Transient task manager is not ready.");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    auto uid = IPCSkeleton::GetCallingUid();
    auto pid = IPCSkeleton::GetCallingPid();
    if (!VerifyCallingInfo(uid, pid)) {
        BGTASK_LOGE("get remain time failed, uid or pid is invalid");
        delayTime = BG_INVALID_REMAIN_TIME;
        return ERR_BGTASK_INVALID_PID_OR_UID;
    }

    std::string name = "";
    if (!GetBundleNamesForUid(uid, name)) {
        BGTASK_LOGE("GetBundleNamesForUid fail.");
        delayTime = BG_INVALID_REMAIN_TIME;
        return ERR_BGTASK_SERVICE_INNER_ERROR;
    }
    BGTASK_LOGI("get remain time pkg : %{public}s, uid : %{public}d, requestId : %{public}d",
        name.c_str(), uid, requestId);

    lock_guard<mutex> lock(expiredCallbackLock_);
    if (!VerifyRequestIdLocked(name, uid, requestId)) {
        BGTASK_LOGE("get remain time failed, requestId is illegal.");
        delayTime = BG_INVALID_REMAIN_TIME;
        return ERR_BGTASK_INVALID_REQUEST_ID;
    }

    delayTime = decisionMaker_->GetRemainingDelayTime(keyInfoMap_[requestId], requestId);
    return ERR_OK;
}

ErrCode BgTransientTaskMgr::GetAllTransientTasks(int32_t &remainingQuota,
    std::vector<std::shared_ptr<DelaySuspendInfo>> &list)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::TransientTask::Service::GetAllTransientTasks");
    if (!isReady_.load()) {
        BGTASK_LOGW("Transient task manager is not ready.");
        return ERR_BGTASK_TRANSIENT_SYS_NOT_READY;
    }
    auto uid = IPCSkeleton::GetCallingUid();
    auto pid = IPCSkeleton::GetCallingPid();
    if (!VerifyCallingInfo(uid, pid)) {
        BGTASK_LOGE("get remain time failed, uid: %{public}d or pid: %{public}d is invalid", uid, pid);
        return ERR_BGTASK_INVALID_PID_OR_UID;
    }
    std::string name = "";
    if (!GetBundleNamesForUid(uid, name)) {
        BGTASK_LOGE("GetBundleNamesForUid fail.");
        return ERR_BGTASK_SERVICE_INNER_ERROR;
    }
    auto keyInfo = std::make_shared<KeyInfo>(name, uid, pid);
    remainingQuota = decisionMaker_->GetQuota(keyInfo);
    lock_guard<mutex> lock(expiredCallbackLock_);
    if (keyInfoMap_.empty()) {
        BGTASK_LOGD("not have transient task, pkg : %{public}s, uid : %{public}d", name.c_str(), uid);
        return ERR_OK;
    }
    for (const auto &record : keyInfoMap_) {
        if (!record.second) {
            continue;
        }
        if (record.second->GetPkg() != name || record.second->GetUid() != uid) {
            continue;
        }
        auto info = std::make_shared<DelaySuspendInfo>();
        info->SetRequestId(record.first);
        info->SetActualDelayTime(decisionMaker_->GetRemainingDelayTime(record.second, record.first));
        list.push_back(info);
    }
    BGTASK_LOGI("get transient task, pkg : %{public}s, uid : %{public}d", name.c_str(), uid);
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
        BGTASK_LOGE("subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }
    auto remote = subscriber->AsObject();
    if (remote == nullptr) {
        BGTASK_LOGE("request suspend delay failed, remote in subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }

    handler_->PostSyncTask([=]() {
        auto findSuscriber = [&remote](const auto& subscriberList) {
            return remote == subscriberList->AsObject();
        };
        auto subscriberIter = find_if(subscriberList_.begin(), subscriberList_.end(), findSuscriber);
        if (subscriberIter != subscriberList_.end()) {
            BGTASK_LOGE("request subscriber is already exists.");
            return;
        }

        if (susriberDeathRecipient_ != nullptr) {
            remote->AddDeathRecipient(susriberDeathRecipient_);
        }
        subscriberList_.emplace_back(subscriber);
        BGTASK_LOGI("subscribe transient task success.");
    });
    return ERR_OK;
}

ErrCode BgTransientTaskMgr::UnsubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber)
{
    if (subscriber == nullptr) {
        BGTASK_LOGE("subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }
    auto remote = subscriber->AsObject();
    if (remote == nullptr) {
        BGTASK_LOGE("request suspend delay failed, remote in subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }

    handler_->PostSyncTask([=]() {
        auto findSuscriber = [&remote](const auto& subscriberList) {
            return remote == subscriberList->AsObject();
        };
        auto subscriberIter = find_if(subscriberList_.begin(), subscriberList_.end(), findSuscriber);
        if (subscriberIter == subscriberList_.end()) {
            BGTASK_LOGE("request subscriber is not exists.");
            return;
        }
        remote->RemoveDeathRecipient(susriberDeathRecipient_);
        subscriberList_.erase(subscriberIter);
        BGTASK_LOGI("unsubscribe transient task success.");
    });
    return ERR_OK;
}

ErrCode BgTransientTaskMgr::GetTransientTaskApps(std::vector<std::shared_ptr<TransientTaskAppInfo>> &list)
{
    lock_guard<mutex> lock(expiredCallbackLock_);
    if (keyInfoMap_.empty()) {
        return ERR_OK;
    }

    for (auto record : keyInfoMap_) {
        auto findInfo = [&record](const auto& info) {
            return (record.second->GetPkg() == info->GetPackageName()) &&
                (record.second->GetUid() == info->GetUid());
        };
        auto findInfoIter = std::find_if(list.begin(), list.end(), findInfo);
        if (findInfoIter == list.end()) {
            auto appInfo = make_shared<TransientTaskAppInfo>(record.second->GetPkg(),
                record.second->GetUid());
            list.push_back(appInfo);
        }
    }
    return ERR_OK;
}

ErrCode BgTransientTaskMgr::SetBgTaskConfig(const std::string &configData, int32_t sourceType)
{
    if (!isReady_.load()) {
        BGTASK_LOGE("Transient task manager is not ready.");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    if (!CheckProcessName()) {
        return ERR_BGTASK_INVALID_PROCESS_NAME;
    }
    BGTASK_LOGD("SetBgTaskConfig configData: %{public}s, sourceType: %{public}d.", configData.c_str(), sourceType);
    bool addResult = DelayedSingleton<BgtaskConfig>::GetInstance()->AddExemptedQuatoData(configData, sourceType);
    if (!addResult) {
        BGTASK_LOGE("AddExemptedQuatoData fail.");
        return ERR_PARAM_NUMBER_ERR;
    }
    return ERR_OK;
}

ErrCode BgTransientTaskMgr::ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo)
{
    if (!isReady_.load()) {
        BGTASK_LOGE("Transient task manager is not ready.");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    bool result = false;
    if (dumpOption[1] == ALL_BGTASKMGR_OPTION) {
        result = DumpAllRequestId(dumpInfo);
    } else if (dumpOption[1] == LOW_BATTARY_OPTION) {
        deviceInfoManeger_->SetDump(true);
        SendLowBatteryEvent(dumpInfo);
        result = true;
    } else if (dumpOption[1] == OKAY_BATTARY_OPTION) {
        deviceInfoManeger_->SetDump(true);
        SendOkayBatteryEvent(dumpInfo);
        result = true;
    } else if (dumpOption[1] == CANCEL_DUMP_OPTION) {
        deviceInfoManeger_->SetDump(false);
        result = true;
    } else if (dumpOption[1] == PAUSE_DUMP_OPTION) {
        DumpTaskTime(dumpOption, true, dumpInfo);
        result = true;
    } else if (dumpOption[1] == START_DUMP_OPTION) {
        DumpTaskTime(dumpOption, false, dumpInfo);
        result = true;
    } else {
        dumpInfo.push_back("Error transient dump command!\n");
    }

    return result ? ERR_OK : ERR_BGTASK_PERMISSION_DENIED;
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

void BgTransientTaskMgr::OnAppCacheStateChanged(int32_t uid, int32_t pid, const std::string &bundleName)
{
    if (!isReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }
    if (!VerifyCallingInfo(uid, pid)) {
        BGTASK_LOGE("pid or uid is invalid.");
        return;
    }
    auto keyInfo = make_shared<KeyInfo>(bundleName, uid, pid);
    vector<int32_t> requestIdList = decisionMaker_->GetRequestIdListByKey(keyInfo);
    if (requestIdList.empty()) {
        BGTASK_LOGD("pkgname: %{public}s, uid: %{public}d not request transient task.",
            bundleName.c_str(), uid);
        return;
    }
    lock_guard<mutex> lock(expiredCallbackLock_);
    for (auto &requestId : requestIdList) {
        BGTASK_LOGI("OnAppCacheStateChanged cancel task, bundlename: %{public}s, uid: %{public}d, pid: %{public}d,"
            " requestId: %{public}d.", bundleName.c_str(), uid, pid, requestId);
        CancelSuspendDelayLocked(requestId);
    }
}

void BgTransientTaskMgr::DumpTaskTime(const std::vector<std::string> &dumpOption, bool pause,
    std::vector<std::string> &dumpInfo)
{
    int32_t uid = std::atoi(dumpOption[DUMP_PARAM_INDEX_TWO].c_str());
    ErrCode ret = ERR_OK;
    if (pause) {
        ret = PauseTransientTaskTimeForInner(uid);
        if (ret != ERR_OK) {
            dumpInfo.push_back("pause transient tasl fail!\n");
        } else {
            dumpInfo.push_back("pause transient tasl success!\n");
        }
    } else {
        ret = StartTransientTaskTimeForInner(uid);
        if (ret != ERR_OK) {
            dumpInfo.push_back("start transient tasl fail!\n");
        } else {
            dumpInfo.push_back("start transient tasl success!\n");
        }
    }
}

bool BgTransientTaskMgr::DumpAllRequestId(std::vector<std::string> &dumpInfo)
{
    lock_guard<mutex> lock(expiredCallbackLock_);
    if (keyInfoMap_.empty()) {
        dumpInfo.push_back("No Transient Task!\n");
        return true;
    }
    std::stringstream stream;
    int32_t index = 1;
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

void BgTransientTaskMgr::HandleSuspendManagerDie()
{
    lock_guard<mutex> lock(transientUidLock_);
    if (!transientPauseUid_.empty()) {
        for (auto iter = transientPauseUid_.begin(); iter != transientPauseUid_.end(); iter++) {
            int32_t uid = *iter;
            std::string name = "";
            if (!GetBundleNamesForUid(uid, name)) {
                BGTASK_LOGE("GetBundleNamesForUid fail, uid : %{public}d.", uid);
                continue;
            }
            ErrCode ret = decisionMaker_->StartTransientTaskTimeForInner(uid, name);
            if (ret != ERR_OK) {
                BGTASK_LOGE("transient task uid: %{public}d, restart fail.", uid);
            }
        }
        transientPauseUid_.clear();
    }
}

void BgTransientTaskMgr::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    if (!isReady_.load()) {
        BGTASK_LOGE("Transient task manager is not ready.");
        return;
    }
    switch (systemAbilityId) {
        case SUSPEND_MANAGER_SYSTEM_ABILITY_ID:
            {
                BGTASK_LOGI("remove suspend manager system ability, systemAbilityId: %{public}d", systemAbilityId);
                auto task = [this]() { this->HandleSuspendManagerDie(); };
                handler_->PostTask(task);
            }
            break;
        default:
            break;
    }
}

std::set<int32_t>& BgTransientTaskMgr::GetTransientPauseUid()
{
    return transientPauseUid_;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS