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

#include "decision_maker.h"

#include <climits>

#include "bg_transient_task_mgr.h"
#include "bgtask_common.h"
#include "transient_task_log.h"
#include "time_provider.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "hisysevent.h"
#include "data_storage_helper.h"
#include "bgtask_config.h"

using namespace std;

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
    const std::string SUSPEND_MANAGER_CONFIG_FILE = "/etc/efficiency_manager/suspend_manager_config.json";
}
DecisionMaker::DecisionMaker(const shared_ptr<TimerManager>& timerManager, const shared_ptr<DeviceInfoManager>& device)
{
    lock_guard<mutex> lock(lock_);
    timerManager_ = timerManager;
    deviceInfoManager_ = device;

    if (!GetAppMgrProxy()) {
        BGTASK_LOGE("GetAppMgrProxy failed");
        return;
    }
}

DecisionMaker::~DecisionMaker()
{
    lock_guard<mutex> lock(lock_);
    if (appMgrProxy_ && observer_) {
        appMgrProxy_->UnregisterApplicationStateObserver(iface_cast<AppExecFwk::IApplicationStateObserver>(observer_));
    }
    appMgrProxy_ = nullptr;
    observer_ = nullptr;
    recipient_ = nullptr;
}

bool DecisionMaker::GetAppMgrProxy()
{
    if (appMgrProxy_ != nullptr) {
        return true;
    }

    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        BGTASK_LOGE("GetSystemAbilityManager failed.");
        return false;
    }

    sptr<IRemoteObject> remoteObject =
        systemAbilityManager->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        BGTASK_LOGE("GetSystemAbility failed.");
        return false;
    }

    appMgrProxy_ = iface_cast<AppExecFwk::IAppMgr>(remoteObject);
    if ((appMgrProxy_ == nullptr) || (appMgrProxy_->AsObject() == nullptr)) {
        BGTASK_LOGE("iface_cast remoteObject failed.");
        return false;
    }
    observer_ = new (std::nothrow) ApplicationStateObserver(*this);
    if (observer_ == nullptr) {
        return false;
    }
    appMgrProxy_->RegisterApplicationStateObserver(iface_cast<AppExecFwk::IApplicationStateObserver>(observer_));

    recipient_ = new (std::nothrow) AppMgrDeathRecipient(*this);
    if (recipient_ == nullptr) {
        return false;
    }
    appMgrProxy_->AsObject()->AddDeathRecipient(recipient_);
    return true;
}

void DecisionMaker::ResetAppMgrProxy()
{
    if ((appMgrProxy_ != nullptr) && (appMgrProxy_->AsObject() != nullptr)) {
        appMgrProxy_->AsObject()->RemoveDeathRecipient(recipient_);
    }
    appMgrProxy_ = nullptr;
}

void DecisionMaker::AppMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    lock_guard<mutex> lock(decisionMaker_.lock_);
    decisionMaker_.ResetAppMgrProxy();
    decisionMaker_.GetAppMgrProxy();
}

void DecisionMaker::ApplicationStateObserver::OnExtensionStateChanged(
    const AppExecFwk::AbilityStateData &abilityStateData)
{
    bool isForeground =
        abilityStateData.abilityState == static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_FOREGROUND);
    bool isBackground =
        abilityStateData.abilityState == static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_BACKGROUND);
    if (isForeground || isBackground) {
        HandleStateChange(abilityStateData.bundleName, abilityStateData.uid, isForeground, isBackground);
    }
}

void DecisionMaker::ApplicationStateObserver::OnForegroundApplicationChanged(
    const AppExecFwk::AppStateData &appStateData)
{
    bool isForeground =
        appStateData.state == static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND) ||
        appStateData.state == static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_FOCUS);
    bool isBackground =
        appStateData.state == static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_BACKGROUND);
    if (isForeground || isBackground) {
        HandleStateChange(appStateData.bundleName, appStateData.uid, isForeground, isBackground);
    }
}

void DecisionMaker::ApplicationStateObserver::HandleStateChange(
    const std::string &bundleName, int32_t uid, bool isForeground, bool isBackground)
{
    lock_guard<mutex> lock(decisionMaker_.lock_);
    auto key = std::make_shared<KeyInfo>(bundleName, uid);

    if (isForeground) {
        auto it = decisionMaker_.pkgDelaySuspendInfoMap_.find(key);
        if (it != decisionMaker_.pkgDelaySuspendInfoMap_.end()) {
            auto pkgInfo = it->second;
            BGTASK_LOGI("pkgname: %{public}s, uid: %{public}d is foreground, stop accounting",
                bundleName.c_str(), uid);
            pkgInfo->StopAccountingAll();
        }
        auto itBg = decisionMaker_.pkgBgDurationMap_.find(key);
        if (itBg != decisionMaker_.pkgBgDurationMap_.end()) {
            decisionMaker_.pkgBgDurationMap_.erase(itBg);
        }
    } else if (isBackground) {
        decisionMaker_.pkgBgDurationMap_[key] = TimeProvider::GetCurrentTime();
        auto it = decisionMaker_.pkgDelaySuspendInfoMap_.find(key);
        if (it == decisionMaker_.pkgDelaySuspendInfoMap_.end()) {
            return;
        }
        auto pkgInfo = it->second;
        if (decisionMaker_.CanStartAccountingLocked(pkgInfo)) {
            BGTASK_LOGI("pkgname: %{public}s, uid: %{public}d is background, start accounting",
                bundleName.c_str(), uid);
            pkgInfo->StartAccounting();
        }
    }
}

int DecisionMaker::GetAllowRequestTime()
{
    static int time = 0;

    if (time != 0) {
        return time;
    }
    if (!DelayedSingleton<DataStorageHelper>::GetInstance()->ParseFastSuspendDozeTime(
        SUSPEND_MANAGER_CONFIG_FILE, time)) {
        time = ALLOW_REQUEST_TIME_BG;
    }
    BGTASK_LOGI("time = %{public}d", time);
    return time;
}

ErrCode DecisionMaker::CheckQuotaTime(const std::shared_ptr<PkgDelaySuspendInfo>& pkgInfo, const std::string &name,
    int32_t uid, const std::shared_ptr<KeyInfo>& key, bool &needSetTime)
{
    ErrCode ret = pkgInfo->IsAllowRequest();
    if (ret == ERR_BGTASK_TIME_INSUFFICIENT) {
        bool isExemptedApp = DelayedSingleton<BgtaskConfig>::GetInstance()->
            IsTransientTaskExemptedQuatoApp(name);
        BGTASK_LOGI("pkgname %{public}s has no quota time, isExemptedApp %{public}d", name.c_str(), isExemptedApp);
        if (isExemptedApp) {
            needSetTime = true;
            return ERR_OK;
        } else {
            return ERR_BGTASK_TIME_INSUFFICIENT;
        }
    }
    if (ret != ERR_OK) {
        BGTASK_LOGE("Request not allow by its info");
        return ret;
    }
    return ERR_OK;
}

ErrCode DecisionMaker::Decide(const std::shared_ptr<KeyInfo>& key, const std::shared_ptr<DelaySuspendInfoEx>& delayInfo)
{
    lock_guard<mutex> lock(lock_);
    if (key == nullptr || delayInfo == nullptr) {
        BGTASK_LOGE("Invalid key or delayInfo");
        return ERR_BGTASK_NO_MEMORY;
    }

    ResetDayQuotaLocked();
    auto findBgDurationIt = pkgBgDurationMap_.find(key);
    if (findBgDurationIt != pkgBgDurationMap_.end()) {
        if (TimeProvider::GetCurrentTime() - findBgDurationIt->second > GetAllowRequestTime()) {
            BGTASK_LOGI("Request not allow after entering background for a valid duration, %{public}s",
                key->ToString().c_str());
            return ERR_BGTASK_NOT_IN_PRESET_TIME;
        }
    }
    const string &name = key->GetPkg();
    int32_t uid = key->GetUid();
    auto findInfoIt = pkgDelaySuspendInfoMap_.find(key);
    if (findInfoIt == pkgDelaySuspendInfoMap_.end()) {
        pkgDelaySuspendInfoMap_[key] = make_shared<PkgDelaySuspendInfo>(name, uid, timerManager_);
    }
    auto pkgInfo = pkgDelaySuspendInfoMap_[key];
    bool needSetTime = false;
    ErrCode ret = CheckQuotaTime(pkgInfo, name, uid, key, needSetTime);
    if (ret != ERR_OK) {
        return ret;
    }
    delayInfo->SetRequestId(NewDelaySuspendRequestId());
    pkgInfo->AddRequest(delayInfo, GetDelayTime(), needSetTime);
    auto appInfo = make_shared<TransientTaskAppInfo>(name, uid, key->GetPid());
    DelayedSingleton<BgTransientTaskMgr>::GetInstance()
        ->HandleTransientTaskSuscriberTask(appInfo, TransientTaskEventType::TASK_START);
    if (pkgInfo->GetRequestSize() == 1) {
        suspendController_.RequestSuspendDelay(key);
        auto info = make_shared<TransientTaskAppInfo>(name, uid);
        DelayedSingleton<BgTransientTaskMgr>::GetInstance()
        ->HandleTransientTaskSuscriberTask(info, TransientTaskEventType::APP_TASK_START);
    }
    if (CanStartAccountingLocked(pkgInfo)) {
        pkgInfo->StartAccounting(delayInfo->GetRequestId());
    }
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::BACKGROUND_TASK, "TRANSIENT_TASK_APPLY",
        HiviewDFX::HiSysEvent::EventType::STATISTIC, "APP_UID", key->GetUid(), "APP_PID", key->GetPid(),
        "APP_NAME", key->GetPkg(), "TASKID", delayInfo->GetRequestId(), "VALUE", delayInfo->GetActualDelayTime());
    return ERR_OK;
}

ErrCode DecisionMaker::PauseTransientTaskTimeForInner(int32_t uid, const std::string &name)
{
    lock_guard<mutex> lock(lock_);
    auto key = std::make_shared<KeyInfo>(name, uid);
    auto itBg = pkgBgDurationMap_.find(key);
    if (itBg == pkgBgDurationMap_.end()) {
        BGTASK_LOGE("pkgname: %{public}s, uid: %{public}d is foreground applicaion.", name.c_str(), uid);
        return ERR_BGTASK_FOREGROUND;
    }
    auto it = pkgDelaySuspendInfoMap_.find(key);
    if (it == pkgDelaySuspendInfoMap_.end()) {
        BGTASK_LOGE("pkgname: %{public}s, uid: %{public}d not request transient task.", name.c_str(), uid);
        return ERR_BGTASK_NOREQUEST_TASK;
    }
    auto pkgInfo = it->second;
    pkgInfo->StopAccountingAll();
    return ERR_OK;
}

ErrCode DecisionMaker::StartTransientTaskTimeForInner(int32_t uid, const std::string &name)
{
    lock_guard<mutex> lock(lock_);
    auto key = std::make_shared<KeyInfo>(name, uid);
    auto itBg = pkgBgDurationMap_.find(key);
    if (itBg == pkgBgDurationMap_.end()) {
        BGTASK_LOGE("pkgname: %{public}s, uid: %{public}d is foreground applicaion.", name.c_str(), uid);
        return ERR_BGTASK_FOREGROUND;
    }
    auto it = pkgDelaySuspendInfoMap_.find(key);
    if (it == pkgDelaySuspendInfoMap_.end()) {
        BGTASK_LOGE("pkgname: %{public}s, uid: %{public}d not request transient task.", name.c_str(), uid);
        return ERR_BGTASK_NOREQUEST_TASK;
    }
    auto pkgInfo = it->second;
    if (CanStartAccountingLocked(pkgInfo)) {
        pkgInfo->StartAccounting();
    } else {
        BGTASK_LOGE("pkgname: %{public}s, uid: %{public}d can't can startAccountingLocked.", name.c_str(), uid);
        return ERR_BGTASK_FOREGROUND;
    }
    return ERR_OK;
}

void DecisionMaker::RemoveRequest(const std::shared_ptr<KeyInfo>& key, const int32_t requestId)
{
    lock_guard<mutex> lock(lock_);
    if (key == nullptr) {
        BGTASK_LOGE("Invalid key");
        return;
    }

    auto findInfoIt = pkgDelaySuspendInfoMap_.find(key);
    if (findInfoIt != pkgDelaySuspendInfoMap_.end()) {
        auto pkgInfo = findInfoIt->second;
        pkgInfo->RemoveRequest(requestId);
        auto appInfo = make_shared<TransientTaskAppInfo>(key->GetPkg(), key->GetUid(), key->GetPid());
        DelayedSingleton<BgTransientTaskMgr>::GetInstance()
            ->HandleTransientTaskSuscriberTask(appInfo, TransientTaskEventType::TASK_END);
        if (pkgInfo->IsRequestEmpty()) {
            suspendController_.CancelSuspendDelay(key);
            auto info = make_shared<TransientTaskAppInfo>(key->GetPkg(), key->GetUid());
            DelayedSingleton<BgTransientTaskMgr>::GetInstance()
                ->HandleTransientTaskSuscriberTask(info, TransientTaskEventType::APP_TASK_END);
        }
        BGTASK_LOGI("Remove requestId: %{public}d", requestId);
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::BACKGROUND_TASK, "TRANSIENT_TASK_CANCEL",
            HiviewDFX::HiSysEvent::EventType::STATISTIC, "APP_UID", key->GetUid(), "APP_PID", key->GetPid(),
            "APP_NAME", key->GetPkg(), "TASKID", requestId);
    }
}

int32_t DecisionMaker::GetRemainingDelayTime(const std::shared_ptr<KeyInfo>& key, const int32_t requestId)
{
    lock_guard<mutex> lock(lock_);
    if (key == nullptr) {
        BGTASK_LOGE("GetRemainingDelayTime, key is null.");
        return -1;
    }

    auto it = pkgDelaySuspendInfoMap_.find(key);
    if (it != pkgDelaySuspendInfoMap_.end()) {
        auto pkgInfo = it->second;
        return pkgInfo->GetRemainDelayTime(requestId);
    }
    return -1;
}

vector<int32_t> DecisionMaker::GetRequestIdListByKey(const std::shared_ptr<KeyInfo>& key)
{
    lock_guard<mutex> lock(lock_);
    vector<int32_t> requestIdList;
    if (key == nullptr) {
        BGTASK_LOGE("GetRequestListByKey, key is null.");
        return requestIdList;
    }
    auto it = pkgDelaySuspendInfoMap_.find(key);
    if (it != pkgDelaySuspendInfoMap_.end()) {
        auto pkgInfo = it->second;
        for (const auto &task : pkgInfo->GetRequestList()) {
            requestIdList.emplace_back(task->GetRequestId());
        }
    } else {
        BGTASK_LOGD("pkgname: %{public}s, uid: %{public}d not request transient task.",
            key->GetPkg().c_str(), key->GetUid());
    }
    return requestIdList;
}

int32_t DecisionMaker::GetQuota(const std::shared_ptr<KeyInfo>& key)
{
    lock_guard<mutex> lock(lock_);
    if (key == nullptr) {
        BGTASK_LOGE("GetQuota, key is null.");
        return -1;
    }

    auto it = pkgDelaySuspendInfoMap_.find(key);
    if (it != pkgDelaySuspendInfoMap_.end()) {
        auto pkgInfo = it->second;
        pkgInfo->UpdateQuota();
        return pkgInfo->GetQuota();
    }
    return INIT_QUOTA;
}

bool DecisionMaker::IsFrontApp(const string& pkgName, int32_t uid)
{
    lock_guard<mutex> lock(lock_);
    if (!GetAppMgrProxy()) {
        BGTASK_LOGE("GetAppMgrProxy failed");
        return false;
    }
    vector<AppExecFwk::AppStateData> fgAppList;
    appMgrProxy_->GetForegroundApplications(fgAppList);
    for (auto fgApp : fgAppList) {
        if (fgApp.bundleName == pkgName && fgApp.uid == uid) {
            return true;
        }
    }
    return false;
}

bool DecisionMaker::CanStartAccountingLocked(const std::shared_ptr<PkgDelaySuspendInfo>& pkgInfo)
{
    if (!deviceInfoManager_->IsScreenOn()) {
        return true;
    }

    if (!GetAppMgrProxy()) {
        BGTASK_LOGE("GetAppMgrProxy failed");
        return false;
    }
    vector<AppExecFwk::AppStateData> fgAppList;
    appMgrProxy_->GetForegroundApplications(fgAppList);
    for (auto fgApp : fgAppList) {
        if (fgApp.bundleName == pkgInfo->GetPkg() && fgApp.uid == pkgInfo->GetUid()) {
            return false;
        }
    }
    return true;
}

int32_t DecisionMaker::GetDelayTime()
{
    return deviceInfoManager_->IsLowPowerMode() ? DELAY_TIME_LOW_POWER : DELAY_TIME_NORMAL;
}

int32_t DecisionMaker::NewDelaySuspendRequestId()
{
    if (requestId_ == INT_MAX) {
        requestId_ = initRequestId_;
    }
    return requestId_++;
}

void DecisionMaker::ResetDayQuotaLocked()
{
    int64_t currentTime = TimeProvider::GetCurrentTime();
    if (!IsAfterOneDay(lastRequestTime_, currentTime)) {
        return;
    }
    for (auto iter = pkgDelaySuspendInfoMap_.begin(); iter != pkgDelaySuspendInfoMap_.end();) {
        auto pkgInfo = iter->second;
        if (pkgInfo->IsRequestEmpty()) {
            iter = pkgDelaySuspendInfoMap_.erase(iter);
        } else {
            pkgInfo->UpdateQuota(true);
            iter++;
        }
    }
    lastRequestTime_ = currentTime;
}

bool DecisionMaker::IsAfterOneDay(int64_t lastRequestTime, int64_t currentTime)
{
    if (currentTime - lastRequestTime > QUOTA_UPDATE) {
        return true;
    }
    return false;
}

void DecisionMaker::OnInputEvent(const EventInfo& eventInfo)
{
    if (!eventInfo.GetEventId()) {
        return;
    }

    switch (eventInfo.GetEventId()) {
        case EVENT_SCREEN_ON:
            HandleScreenOn();
            break;
        case EVENT_SCREEN_OFF:
            HandleScreenOff();
            break;
        default:
            break;
    }
}

void DecisionMaker::HandleScreenOn()
{
    lock_guard<mutex> lock(lock_);
    if (!GetAppMgrProxy()) {
        BGTASK_LOGE("GetAppMgrProxy failed");
        return;
    }
    vector<AppExecFwk::AppStateData> fgAppList;
    appMgrProxy_->GetForegroundApplications(fgAppList);
    for (auto fgApp : fgAppList) {
        auto key = std::make_shared<KeyInfo>(fgApp.bundleName, fgApp.uid);
        auto it = pkgDelaySuspendInfoMap_.find(key);
        if (it != pkgDelaySuspendInfoMap_.end()) {
            auto pkgInfo = it->second;
            BGTASK_LOGI("screen is on and uid: %{public}d is foreground app, stop accounting", fgApp.uid);
            pkgInfo->StopAccountingAll();
        }
    }
}

void DecisionMaker::HandleScreenOff()
{
    lock_guard<mutex> lock(lock_);
    std::set<int32_t> &transientPauseUid = DelayedSingleton<BgTransientTaskMgr>::GetInstance()
        ->GetTransientPauseUid();
    for (const auto &p : pkgDelaySuspendInfoMap_) {
        auto pkgInfo = p.second;
        auto findUid = [&pkgInfo](const auto &target) {
            return pkgInfo->GetUid() == target;
        };
        auto findUidIter = find_if(transientPauseUid.begin(), transientPauseUid.end(), findUid);
        if (findUidIter != transientPauseUid.end()) {
            BGTASK_LOGD("transient task freeze, not can start.");
            continue;
        }
        if (CanStartAccountingLocked(pkgInfo)) {
            BGTASK_LOGI("screen is off and uid: %{public}d is not freeze, start accounting", pkgInfo->GetUid());
            pkgInfo->StartAccounting();
        }
    }
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS