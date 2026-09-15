/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "app_mgr_helper.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
AppMgrHelper::AppMgrHelper() {}

AppMgrHelper::~AppMgrHelper() {}

bool AppMgrHelper::GetAllRunningProcesses(std::vector<AppExecFwk::RunningProcessInfo>& allAppProcessInfos)
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    if (!Connect()) {
        return false;
    }
    if (appMgrProxy_->GetAllRunningProcesses(allAppProcessInfos) != ERR_OK) {
        BGTASK_LOGE("failed to get all running process.");
        return false;
    }
    return true;
}

bool AppMgrHelper::GetForegroundApplications(std::vector<AppExecFwk::AppStateData> &fgApps)
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    if (!Connect()) {
        return false;
    }
    if (appMgrProxy_->GetForegroundApplications(fgApps) != ERR_OK) {
        return false;
    }
    return true;
}

bool AppMgrHelper::SubscribeConfigurationObserver(const sptr<AppExecFwk::IConfigurationObserver> &observer)
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    if (!Connect()) {
        return false;
    }
    int32_t res = appMgrProxy_->RegisterConfigurationObserver(observer);
    if (res != ERR_OK) {
        BGTASK_LOGE("failed to register configuration state observer, ret: %{public}d", res);
        return false;
    }
    return true;
}

bool AppMgrHelper::SubscribeObserver(const sptr<AppExecFwk::IApplicationStateObserver> &observer)
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    if (!Connect()) {
        return false;
    }
    int32_t res = appMgrProxy_->RegisterApplicationStateObserver(observer);
    if (res != ERR_OK) {
        BGTASK_LOGE("failed to register application state observer, ret: %{public}d", res);
        return false;
    }
    return true;
}

bool AppMgrHelper::UnsubscribeObserver(const sptr<AppExecFwk::IApplicationStateObserver> &observer)
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    if (!Connect()) {
        return false;
    }
    if (appMgrProxy_->UnregisterApplicationStateObserver(observer) != ERR_OK) {
        return false;
    }
    return true;
}

bool AppMgrHelper::Connect()
{
    if (appMgrProxy_ != nullptr) {
        return true;
    }

    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        BGTASK_LOGE("failed to get SystemAbilityManager");
        return false;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        BGTASK_LOGE("failed to get App Manager Service");
        return false;
    }

    appMgrProxy_ = iface_cast<AppExecFwk::IAppMgr>(remoteObject);
    if (!appMgrProxy_ || !appMgrProxy_->AsObject()) {
        BGTASK_LOGE("failed to get app mgr proxy");
        return false;
    }
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS

bool BgContinuousTaskMgr::CheckLiveViewAndMediaControllerByUid(int32_t uid, bool &liveViewState,
    bool &mediaControllerState)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    std::vector<sptr<Notification::NotificationRequest>> notificationRequests;
    if (Notification::NotificationHelper::GetActiveNotifications(notificationRequests) != ERR_OK) {
        BGTASK_LOGE("GetActiveNotifications fail.");
        return false;
    }
    for (Notification::NotificationRequest *var : notificationRequests) {
        if (!var) {
            BGTASK_LOGE("var is null.");
            return false;
        }
        auto notificationType = var->GetNotificationType();
        if ((uid == var->GetOwnerUid()) && notificationType == Notification::NotificationContent::Type::LIVE_VIEW) {
            liveViewState = true;
        }
    }
#endif
    auto iter = avSessionNotification_.find(uid);
    if (iter != avSessionNotification_.end()) {
        mediaControllerState = iter->second;
    }
    return true;
}

void BgContinuousTaskMgr::ReportTaskAdjustEventByUid(int32_t uid)
{
    bool liveViewState = false;
    bool mediaControllerState = false;
    if (!CheckLiveViewAndMediaControllerByUid(uid, liveViewState, mediaControllerState)) {
        return;
    }
    nlohmann::json payload = nlohmann::json::object();
    payload["uid"] = uid;
    payload["liveViewState"] = liveViewState;
    payload["mediaControllerState"] = mediaControllerState;
    auto pidForModes = nlohmann::json::array();
    std::map<pid_t, std::set<uint32_t>> pidModeMap;
    for (const auto &task : continuousTaskInfosMap_) {
        if (task.second == nullptr || task.second->GetUid() != uid) {
            continue;
        }
        pid_t pid = task.second->pid_;
        const auto bgModeIds = task.second->bgModeIds_;
        auto modeSet = pidModeMap[pid];
        for (uint32_t mode : bgModeIds) {
            modeSet.insert(mode);
        }
    }
    nlohmann::json result = nlohmann::json::array();
    for (auto &[pid, modeSet] : pidModeMap) {
        nlohmann::json item;
        nlohmann::json modes = nlohmann::json::array();
        for (uint32_t mode : modeSet) {
            modes.push_back(mode);
        }
        item[std::to_string(pid)] = modes;
        pidForModes.push_back(item);
    }
    payload["pidForModes"] = pidForModes;
    ReportDataInProcess(ResourceSchedule::ResType::RES_TYPE_BGTASK_ADJUST_EVENT, -1, payload);
}

void BgContinuousTaskMgr::ReportTaskAdjustEventByTask(const std::shared_ptr<ContinuousTaskRecord> record,
    ContinuousTaskEventTriggerType changeEventType)
{
    bool liveViewState = false;
    bool mediaControllerState = false;
    if (!CheckLiveViewAndMediaControllerByUid(record->uid_, liveViewState, mediaControllerState)) {
        return;
    }
    nlohmann::json payload = nlohmann::json::object();
    payload["uid"] = record->uid_;
    payload["liveViewState"] = liveViewState;
    payload["mediaControllerState"] = mediaControllerState;
    auto pidForModes = nlohmann::json::array();
    std::map<pid_t, std::set<uint32_t>> pidModeMap;
    std::string taskInfoMapKey = std::to_string(record->uid_) + SEPARATOR
        + record->abilityName_ + SEPARATOR + std::to_string(record->abilityId_);
    if (record->isByRequestObject_) {
        taskInfoMapKey = taskInfoMapKey + SEPARATOR + std::to_string(record->continuousTaskId_);
    }
    for (const auto &task : continuousTaskInfosMap_) {
        if (task.second == nullptr || task.second->GetUid() != record->uid_) {
            continue;
        }
        pid_t pid = task.second->pid_;
        auto bgModeIds = task.second->bgModeIds_;
        auto modeSet = pidModeMap[pid];
        if (task.first == taskInfoMapKey) {
            if (changeEventType == ContinuousTaskEventTriggerType::TASK_CANCEL ||
                changeEventType == ContinuousTaskEventTriggerType::TASK_SUSPEND) {
                continue;
            } else if (changeEventType == ContinuousTaskEventTriggerType::TASK_UPDATE) {
                bgModeIds = record->bgModeIds_;
            }
        }
        for (uint32_t mode : bgModeIds) {
            modeSet.insert(mode);
        }
    }
    nlohmann::json result = nlohmann::json::array();
    for (auto &[pid, modeSet] : pidModeMap) {
        nlohmann::json item;
        nlohmann::json modes = nlohmann::json::array();
        for (uint32_t mode : modeSet) {
            modes.push_back(mode);
        }
        item[std::to_string(pid)] = modes;
        pidForModes.push_back(item);
    }
    payload["pidForModes"] = pidForModes;
    ReportDataInProcess(ResourceSchedule::ResType::RES_TYPE_BGTASK_ADJUST_EVENT, -1, payload);
}

bool BgContinuousTaskMgr::CheckReportTaskAdjustEvent(int32_t uid)
{
    const std::vector<uint32_t> checkMode = {
        BackgroundMode::USB_CONNECTION
    };
    for (const auto &task : continuousTaskInfosMap_) {
        if (task.second == nullptr || task.second->GetUid() != uid) {
            continue;
        }
        const std::vector<uint32_t> bgModeIds = task.second->bgModeIds_;
        if (CommonUtils::CheckApplyMode(checkMode, bgModeIds)) {
            return true;
        }
    }
    return false;
}