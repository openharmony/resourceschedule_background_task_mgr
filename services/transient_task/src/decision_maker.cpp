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

using namespace std;

namespace OHOS {
namespace BackgroundTaskMgr {
DecisionMaker::DecisionMaker(const shared_ptr<TimerManager>& timerManager, const shared_ptr<DeviceInfoManager>& device)
{
    timerManager_ = timerManager;
    deviceInfoManager_ = device;

    if (!GetAppMgrProxy()) {
        BGTASK_LOGE("GetAppMgrProxy failed");
        return;
    }
}

DecisionMaker::~DecisionMaker()
{
    if (!GetAppMgrProxy()) {
        BGTASK_LOGE("GetAppMgrProxy failed");
        return;
    }
    appMgrProxy_->UnregisterApplicationStateObserver(iface_cast<AppExecFwk::IApplicationStateObserver>(observer_));
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
    decisionMaker_.ResetAppMgrProxy();
    decisionMaker_.GetAppMgrProxy();
}

void DecisionMaker::ApplicationStateObserver::OnForegroundApplicationChanged(
    const AppExecFwk::AppStateData &appStateData)
{
    lock_guard<mutex> lock(decisionMaker_.lock_);

    auto key = std::make_shared<KeyInfo>(appStateData.bundleName, appStateData.uid);
    if (appStateData.state == (int32_t)AppExecFwk::ApplicationState::APP_STATE_FOREGROUND ||
        appStateData.state == (int32_t)AppExecFwk::ApplicationState::APP_STATE_FOCUS) {
        auto it = decisionMaker_.pkgDelaySuspendInfoMap_.find(key);
        if (it != decisionMaker_.pkgDelaySuspendInfoMap_.end()) {
            auto pkgInfo = it->second;
            pkgInfo->StopAccountingAll();
        }
        auto itBg = decisionMaker_.pkgBgDurationMap_.find(key);
        if (itBg != decisionMaker_.pkgBgDurationMap_.end()) {
            decisionMaker_.pkgBgDurationMap_.erase(itBg);
        }
    } else if (appStateData.state == (int32_t)AppExecFwk::ApplicationState::APP_STATE_BACKGROUND) {
        decisionMaker_.pkgBgDurationMap_[key] = TimeProvider::GetCurrentTime();
        auto it = decisionMaker_.pkgDelaySuspendInfoMap_.find(key);
        if (it == decisionMaker_.pkgDelaySuspendInfoMap_.end()) {
            return;
        }
        auto pkgInfo = it->second;
        if (decisionMaker_.CanStartAccountingLocked(pkgInfo)) {
            pkgInfo->StartAccounting();
        }
    }
}

bool DecisionMaker::Decide(const std::shared_ptr<KeyInfo>& key, const std::shared_ptr<DelaySuspendInfoEx>& delayInfo)
{
    lock_guard<mutex> lock(lock_);
    if (key == nullptr) {
        BGTASK_LOGE("Invalid key");
        return false;
    }

    ResetDayQuotaLocked();
    auto findBgDurationIt = pkgBgDurationMap_.find(key);
    if (findBgDurationIt != pkgBgDurationMap_.end()) {
        if (TimeProvider::GetCurrentTime() - findBgDurationIt->second > ALLOW_REQUEST_TIME_BG) {
            BGTASK_LOGI("Request not allow after entering background for a valid duration, %{public}s",
                key->ToString().c_str());
            return false;
        }
    }

    const string &name = key->GetPkg();
    int32_t uid = key->GetUid();
    auto findInfoIt = pkgDelaySuspendInfoMap_.find(key);
    if (findInfoIt == pkgDelaySuspendInfoMap_.end()) {
        pkgDelaySuspendInfoMap_[key] = make_shared<PkgDelaySuspendInfo>(name, uid, timerManager_);
    }

    auto pkgInfo = pkgDelaySuspendInfoMap_[key];
    if (!pkgInfo->IsAllowRequest()) {
        BGTASK_LOGI("Request not allow by its info");
        return false;
    }

    if (delayInfo == nullptr) {
        BGTASK_LOGE("Invalid delayInfo");
        return false;
    }
    delayInfo->SetRequestId(NewDelaySuspendRequestId());
    pkgInfo->AddRequest(delayInfo, GetDelayTime());
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
    return true;
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
        BGTASK_LOGD("Remove requestId: %{public}d", requestId);
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
        requestId_ = INIT_REQUEST_ID;
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
            pkgInfo->StopAccountingAll();
        }
    }
}

void DecisionMaker::HandleScreenOff()
{
    lock_guard<mutex> lock(lock_);
    for (auto &p : pkgDelaySuspendInfoMap_) {
        auto pkgInfo = p.second;
        if (CanStartAccountingLocked(pkgInfo)) {
            pkgInfo->StartAccounting();
        }
    }
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS