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

#include "bg_continuous_task_mgr.h"

#include <sstream>

#include "app_mgr_client.h"
#include "bundle_constants.h"
#include "bundle_manager_helper.h"
#include "common_event_support.h"
#include "common_event_manager.h"
#include "errors.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#ifdef HAS_OS_ACCOUNT_PART
#include "os_account_manager.h"
#endif // HAS_OS_ACCOUNT_PART
#include "notification_tools.h"
#include "parameters.h"
#include "running_process_info.h"
#include "string_wrapper.h"
#include "system_ability_definition.h"

#include "bgtaskmgr_inner_errors.h"
#include "continuous_task_record.h"
#include "continuous_task_log.h"
#include "system_event_observer.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static const char *CONTI_TASK_TEXT_RES_NAMES[] = {
    "ohos_bgmode_prompt_data_transfer",
    "ohos_bgmode_prompt_audio_playback",
    "ohos_bgmode_prompt_audio_recording",
    "ohos_bgmode_prompt_location",
    "ohos_bgmode_prompt_bluetooth_interaction",
    "ohos_bgmode_prompt_multidevice_connection",
    "ohos_bgmode_prompt_wifi_interaction",
    "ohos_bgmode_prompt_voip",
    "ohos_bgmode_prompt_task_keeping",
    "ohos_bgmode_prompt_default_value",
};

static constexpr char BG_CONTINUOUS_TASK_MGR_NAME[] = "BgContinuousTaskMgr";
static constexpr char SEPARATOR[] = "_";
static constexpr char DUMP_PARAM_LIST_ALL[] = "--all";
static constexpr char DUMP_PARAM_CANCEL_ALL[] = "--cancel_all";
static constexpr char DUMP_PARAM_CANCEL[] = "--cancel";
static constexpr char BGMODE_PERMISSION[] = "ohos.permission.KEEP_BACKGROUND_RUNNING";
static constexpr char BG_TASK_RES_BUNDLE_NAME[] = "ohos.backgroundtaskmgr.resources";
static constexpr char DEVICE_TYPE_PC[] = "pc";
static constexpr uint32_t SYSTEM_APP_BGMODE_WIFI_INTERACTION = 64;
static constexpr uint32_t SYSTEM_APP_BGMODE_VOIP = 128;
static constexpr uint32_t PC_BGMODE_TASK_KEEPING = 256;
static constexpr int32_t DEFAULT_NOTIFICATION_ID = 0;
static constexpr int32_t DELAY_TIME = 2000;
static constexpr int32_t MAX_DUMP_PARAM_NUMS = 3;
static constexpr int32_t UNSET_UID = -1;
static constexpr uint32_t INVALID_BGMODE = 0;
static constexpr uint32_t BG_MODE_INDEX_HEAD = 1;
static constexpr uint32_t BGMODE_NUMS = 10;

#ifndef HAS_OS_ACCOUNT_PART
constexpr int32_t DEFAULT_OS_ACCOUNT_ID = 0; // 0 is the default id when there is no os_account part
constexpr int32_t UID_TRANSFORM_DIVISOR = 200000;
static void GetOsAccountIdFromUid(int32_t uid, int32_t &osAccountId)
{
    osAccountId = uid / UID_TRANSFORM_DIVISOR;
}
#endif // HAS_OS_ACCOUNT_PART
}

BgContinuousTaskMgr::BgContinuousTaskMgr() {}

BgContinuousTaskMgr::~BgContinuousTaskMgr() {}

bool BgContinuousTaskMgr::Init()
{
    runner_ = AppExecFwk::EventRunner::Create(BG_CONTINUOUS_TASK_MGR_NAME);
    if (runner_ == nullptr) {
        BGTASK_LOGE("BgContinuousTaskMgr runner create failed!");
        return false;
    }
    handler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner_);
    if (handler_ == nullptr) {
        BGTASK_LOGE("BgContinuousTaskMgr handler create failed!");
        return false;
    }
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    bgTaskUid_ = IPCSkeleton::GetCallingUid();
    BGTASK_LOGI("BgContinuousTaskMgr service uid is: %{public}d", bgTaskUid_);
    IPCSkeleton::SetCallingIdentity(identity);
    dataStorage_ = std::make_shared<DataStorage>();
    auto registerTask = [this]() { this->InitNecessaryState(); };
    handler_->PostSyncTask(registerTask);
    return true;
}

void BgContinuousTaskMgr::Clear()
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    Notification::NotificationHelper::UnSubscribeNotification(*subscriber_);
#endif
    if (systemEventListener_ != nullptr) {
        systemEventListener_->Unsubscribe();
    }
    if (appStateObserver_ != nullptr) {
        appStateObserver_->Unsubscribe();
    }
}

void BgContinuousTaskMgr::InitNecessaryState()
{
    sptr<ISystemAbilityManager> systemAbilityManager
        = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr
        || systemAbilityManager->CheckSystemAbility(APP_MGR_SERVICE_ID) == nullptr
        || systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) == nullptr
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
        || systemAbilityManager->CheckSystemAbility(ADVANCED_NOTIFICATION_SERVICE_ABILITY_ID) == nullptr
#endif
        || systemAbilityManager->CheckSystemAbility(COMMON_EVENT_SERVICE_ID) == nullptr) {
        BGTASK_LOGW("request system service is not ready yet!");
        auto task = [this]() { this->InitNecessaryState(); };
        handler_->PostTask(task, DELAY_TIME);
        return;
    }

    if (!RegisterNotificationSubscriber()) {
        BGTASK_LOGE("RegisterNotificationSubscriber failed");
        return;
    }
    if (!RegisterAppStateObserver()) {
        BGTASK_LOGE("RegisterAppStateObserver failed");
        return;
    }
    if (!RegisterSysCommEventListener()) {
        BGTASK_LOGE("RegisterSysCommEventListener failed");
        return;
    }
    deviceType_ = OHOS::system::GetParameter("const.build.characteristics", "");
    BGTASK_LOGI("current device type is: %{public}s", deviceType_.c_str());
    InitRequiredResourceInfo();
}

void BgContinuousTaskMgr::HandlePersistenceData()
{
    if (dataStorage_ == nullptr) {
        BGTASK_LOGE("get data storage failed");
        return;
    }
    BGTASK_LOGI("service restart, restore data");
    dataStorage_->RestoreTaskRecord(continuousTaskInfosMap_);
    auto appMgrClient = std::make_shared<AppExecFwk::AppMgrClient>();
    std::vector<AppExecFwk::RunningProcessInfo> allAppProcessInfos;
    if (appMgrClient == nullptr || appMgrClient->ConnectAppMgrService() != ERR_OK) {
        BGTASK_LOGW("connect to app mgr service failed");
        return;
    }
    appMgrClient->GetAllRunningProcesses(allAppProcessInfos);
    std::set<std::string> labels;
    NotificationTools::GetInstance()->GetAllActiveNotificationsLabels(labels);
    CheckPersistenceData(allAppProcessInfos, labels);
    dataStorage_->RefreshTaskRecord(continuousTaskInfosMap_);
}

void BgContinuousTaskMgr::CheckPersistenceData(const std::vector<AppExecFwk::RunningProcessInfo> &allProcesses,
    const std::set<std::string> &allLabels)
{
    int32_t recordPid;
    std::string recordLabel;
    auto iter = continuousTaskInfosMap_.begin();
    while (iter != continuousTaskInfosMap_.end()) {
        recordPid = iter->second->GetPid();
        recordLabel = iter->second->GetNotificationLabel();
        if (checkPidCondition(allProcesses, recordPid) && checkNotificationCondition(allLabels, recordLabel)) {
            BGTASK_LOGI("target continuous task exist");
            iter++;
            continue;
        }

        if (!checkPidCondition(allProcesses, recordPid) && checkNotificationCondition(allLabels, recordLabel)) {
            BGTASK_LOGI("pid: %{public}d not exist, label: %{public}s exist", recordPid, recordLabel.c_str());
            NotificationTools::GetInstance()->CancelNotification(recordLabel, DEFAULT_NOTIFICATION_ID);
            iter = continuousTaskInfosMap_.erase(iter);
            continue;
        }

        if (checkPidCondition(allProcesses, recordPid) && !checkNotificationCondition(allLabels, recordLabel)) {
            BGTASK_LOGI("pid: %{public}d exist, label: %{public}s not exist", recordPid, recordLabel.c_str());
            iter = continuousTaskInfosMap_.erase(iter);
            continue;
        }

        if (!checkPidCondition(allProcesses, recordPid) && !checkNotificationCondition(allLabels, recordLabel)) {
            BGTASK_LOGI("pid: %{public}d not exist, label: %{public}s not exist", recordPid, recordLabel.c_str());
            iter = continuousTaskInfosMap_.erase(iter);
            continue;
        }
    }
}

bool BgContinuousTaskMgr::checkPidCondition(const std::vector<AppExecFwk::RunningProcessInfo> &allProcesses,
    int32_t pid)
{
    auto findPid = [pid](const auto &target) {
        return pid == target.pid_;
    };
    auto findPidIter = find_if(allProcesses.begin(), allProcesses.end(), findPid);
    return findPidIter != allProcesses.end();
}

bool BgContinuousTaskMgr::checkNotificationCondition(const std::set<std::string> &notificationLabels,
    const std::string &label)
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    auto findLabel = [label](const auto &target) {
        return label == target;
    };
    auto findLabelIter = find_if(notificationLabels.begin(), notificationLabels.end(), findLabel);
    return findLabelIter != notificationLabels.end();
#else
    return true;
#endif
}

void BgContinuousTaskMgr::InitRequiredResourceInfo()
{
    if (!GetNotificationPrompt()) {
        BGTASK_LOGW("init required resource info failed. will try again");
        isSysReady_.store(false);
        auto task = [this]() { this->InitRequiredResourceInfo(); };
        handler_->PostTask(task, DELAY_TIME);
        return;
    }
    HandlePersistenceData();
    isSysReady_.store(true);
}

bool BgContinuousTaskMgr::RegisterNotificationSubscriber()
{
    bool res = true;
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    subscriber_ = std::make_shared<TaskNotificationSubscriber>();
    if (Notification::NotificationHelper::SubscribeNotification(*subscriber_) != ERR_OK) {
        BGTASK_LOGE("SubscribeNotification failed!");
        res = false;
    }
#endif
    return res;
}

bool BgContinuousTaskMgr::RegisterAppStateObserver()
{
    bool res = true;
    appStateObserver_ = DelayedSingleton<AppStateObserver>::GetInstance();
    if (appStateObserver_ != nullptr) {
        appStateObserver_->SetEventHandler(handler_);
        appStateObserver_->SetBgContinuousTaskMgr(shared_from_this());
        res = appStateObserver_->Subscribe();
    }
    return res;
}

bool BgContinuousTaskMgr::GetNotificationPrompt()
{
    AppExecFwk::BundleInfo bundleInfo;
    if (!BundleManagerHelper::GetInstance()->GetBundleInfo(BG_TASK_RES_BUNDLE_NAME,
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo)) {
        BGTASK_LOGE("get background task res: %{public}s bundle info failed", BG_TASK_RES_BUNDLE_NAME);
        return false;
    }
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    if (!resourceManager) {
        BGTASK_LOGE("create resourceManager failed");
        return false;
    }
    for (auto moduleResPath : bundleInfo.moduleResPaths) {
        if (!moduleResPath.empty()) {
            if (!resourceManager->AddResource(moduleResPath.c_str())) {
                BGTASK_LOGE("AddResource failed");
            }
        }
    }
    std::string taskText {""};
    for (std::string name : CONTI_TASK_TEXT_RES_NAMES) {
        resourceManager->GetStringByName(name.c_str(), taskText);
        if (taskText.empty()) {
            BGTASK_LOGE("get continuous task notification text failed!");
            return false;
        }
        BGTASK_LOGI("get taskText: %{public}s", taskText.c_str());
        continuousTaskText_.push_back(taskText);
    }
    return true;
}

bool BgContinuousTaskMgr::RegisterSysCommEventListener()
{
    bool res = true;
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REPLACED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_REMOVED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_FULLY_REMOVED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_ADDED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    EventFwk::CommonEventSubscribeInfo commonEventSubscribeInfo(matchingSkills);
    systemEventListener_ = std::make_shared<SystemEventObserver>(commonEventSubscribeInfo);
    if (systemEventListener_ != nullptr) {
        systemEventListener_->SetEventHandler(handler_);
        systemEventListener_->SetBgContinuousTaskMgr(shared_from_this());
        res = systemEventListener_->Subscribe();
    }
    return res;
}

int32_t BgContinuousTaskMgr::GetBgTaskUid()
{
    return bgTaskUid_;
}

bool BgContinuousTaskMgr::SetCachedBundleInfo(int32_t uid, int32_t userId,
    std::string &bundleName, const std::string &appName)
{
    AppExecFwk::BundleInfo bundleInfo;
    if (!BundleManagerHelper::GetInstance()->GetBundleInfo(bundleName,
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, userId)) {
        BGTASK_LOGE("get bundle info: %{public}s failure!", bundleName.c_str());
        return false;
    }

    CachedBundleInfo cachedBundleInfo = CachedBundleInfo();
    cachedBundleInfo.appName_ = appName;
    if (AddAbilityBgModeInfos(bundleInfo, cachedBundleInfo)) {
        cachedBundleInfos_.emplace(uid, cachedBundleInfo);
        return true;
    }
    return false;
}

bool BgContinuousTaskMgr::AddAbilityBgModeInfos(const AppExecFwk::BundleInfo &bundleInfo,
    CachedBundleInfo &cachedBundleInfo)
{
    for (auto abilityInfo : bundleInfo.abilityInfos) {
        if (abilityInfo.backgroundModes != INVALID_BGMODE) {
            cachedBundleInfo.abilityBgMode_.emplace(abilityInfo.name, abilityInfo.backgroundModes);
            BGTASK_LOGI("abilityName: %{public}s, abilityNameHash: %{public}s, Background Mode: %{public}u.",
                abilityInfo.name.c_str(), std::to_string(std::hash<std::string>()(abilityInfo.name)).c_str(),
                abilityInfo.backgroundModes);
        }
    }
    if (cachedBundleInfo.abilityBgMode_.empty()) {
        return false;
    }
    return true;
}

bool BgContinuousTaskMgr::checkBgmodeType(uint32_t configuredBgMode, uint32_t requestedBgModeId,
    bool isNewApi, int32_t uid)
{
    if (!isNewApi) {
        if (configuredBgMode == INVALID_BGMODE) {
            BGTASK_LOGE("ability without background mode config");
            return false;
        } else {
            return true;
        }
    } else {
        uint32_t recordedBgMode = BG_MODE_INDEX_HEAD << (requestedBgModeId - 1);
        if ((recordedBgMode == SYSTEM_APP_BGMODE_WIFI_INTERACTION || recordedBgMode == SYSTEM_APP_BGMODE_VOIP)
            && !BundleManagerHelper::GetInstance()->IsSystemApp(uid)) {
            BGTASK_LOGE("voip and wifiInteraction background mode only support for system app");
            return false;
        }
        if (recordedBgMode == PC_BGMODE_TASK_KEEPING
            && deviceType_ != DEVICE_TYPE_PC) {
            BGTASK_LOGE("task keeping background mode only support for pc device");
            return false;
        }
        if (requestedBgModeId == INVALID_BGMODE || (configuredBgMode
            & (BG_MODE_INDEX_HEAD << (requestedBgModeId - 1))) == 0) {
            BGTASK_LOGE("requested background mode is not declared in config file!");
            return false;
        }
    }
    return true;
}

uint32_t BgContinuousTaskMgr::GetBackgroundModeInfo(int32_t uid, std::string &abilityName)
{
    if (cachedBundleInfos_.find(uid) != cachedBundleInfos_.end()) {
        auto cachedBundleInfo = cachedBundleInfos_.at(uid);
        if (cachedBundleInfo.abilityBgMode_.find(abilityName) !=
            cachedBundleInfo.abilityBgMode_.end()) {
            return cachedBundleInfo.abilityBgMode_.at(abilityName);
        }
    }
    return INVALID_BGMODE;
}

bool CheckTaskParam(const sptr<ContinuousTaskParam> &taskParam)
{
    if (!taskParam) {
        BGTASK_LOGE("continuous task params is null!");
        return false;
    }

    if (taskParam->isNewApi_) {
        if (taskParam->wantAgent_ == nullptr || taskParam->abilityName_.empty()) {
            BGTASK_LOGE("continuous task params invalid!");
            return false;
        }
    } else {
        if (taskParam->abilityName_.empty()) {
            BGTASK_LOGE("continuous task params invalid!");
            return false;
        }
    }
    return true;
}

ErrCode BgContinuousTaskMgr::StartBackgroundRunning(const sptr<ContinuousTaskParam> &taskParam)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }

    if (!CheckTaskParam(taskParam)) {
        return ERR_BGTASK_INVALID_PARAM;
    }

    ErrCode result = ERR_OK;

    int32_t callingUid = IPCSkeleton::GetCallingUid();
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    std::string bundleName = BundleManagerHelper::GetInstance()->GetClientBundleName(callingUid);
    int32_t userId = -1;
#ifdef HAS_OS_ACCOUNT_PART
    AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(callingUid, userId);
#else // HAS_OS_ACCOUNT_PART
    GetOsAccountIdFromUid(callingUid, userId);
#endif // HAS_OS_ACCOUNT_PART

    if (!BundleManagerHelper::GetInstance()->CheckPermission(BGMODE_PERMISSION)) {
        BGTASK_LOGE("background mode permission is not passed");
        return ERR_BGTASK_PERMISSION_DENIED;
    }

    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
    continuousTaskRecord->bundleName_ = bundleName;
    continuousTaskRecord->abilityName_ = taskParam->abilityName_;
    continuousTaskRecord->wantAgent_ = taskParam->wantAgent_;
    continuousTaskRecord->userId_ = userId;
    continuousTaskRecord->uid_ = callingUid;
    continuousTaskRecord->pid_ = callingPid;
    continuousTaskRecord->bgModeId_ = taskParam->bgModeId_;
    continuousTaskRecord->isNewApi_ = taskParam->isNewApi_;
    continuousTaskRecord->appName_ = taskParam->appName_;

    if (taskParam->wantAgent_ != nullptr && taskParam->wantAgent_->GetPendingWant() != nullptr) {
        auto target = taskParam->wantAgent_->GetPendingWant()->GetTarget();
        auto want = taskParam->wantAgent_->GetPendingWant()->GetWant(target);
        if (want != nullptr) {
            std::shared_ptr<WantAgentInfo> info = std::make_shared<WantAgentInfo>();
            info->bundleName_ = want->GetOperation().GetBundleName();
            info->abilityName_ = want->GetOperation().GetAbilityName();
            continuousTaskRecord->wantAgentInfo_ = info;
        }
    }

    StartTrace(HITRACE_TAG_OHOS, "BgContinuousTaskMgr::StartBackgroundRunningInner");
    handler_->PostSyncTask([this, continuousTaskRecord, &result]() mutable {
        result = this->StartBackgroundRunningInner(continuousTaskRecord);
        }, AppExecFwk::EventQueue::Priority::HIGH);
    FinishTrace(HITRACE_TAG_OHOS);

    return result;
}

ErrCode BgContinuousTaskMgr::StartBackgroundRunningInner(std::shared_ptr<ContinuousTaskRecord> &continuousTaskRecord)
{
    std::string taskInfoMapKey = std::to_string(continuousTaskRecord->uid_) + SEPARATOR
        + continuousTaskRecord->abilityName_;
    if (continuousTaskInfosMap_.find(taskInfoMapKey) != continuousTaskInfosMap_.end()) {
        BGTASK_LOGW("continuous task is already exist: %{public}s", taskInfoMapKey.c_str());
        return ERR_BGTASK_OBJECT_EXISTS;
    }

    if (cachedBundleInfos_.find(continuousTaskRecord->uid_) == cachedBundleInfos_.end()) {
        SetCachedBundleInfo(continuousTaskRecord->uid_, continuousTaskRecord->userId_,
            continuousTaskRecord->bundleName_, continuousTaskRecord->appName_);
    }

    uint32_t configuredBgMode = GetBackgroundModeInfo(continuousTaskRecord->uid_,
        continuousTaskRecord->abilityName_);
    if (!checkBgmodeType(configuredBgMode, continuousTaskRecord->bgModeId_, continuousTaskRecord->isNewApi_,
        continuousTaskRecord->uid_)) {
        BGTASK_LOGE("background mode invalid!");
        return ERR_BGTASK_INVALID_BGMODE;
    }

    if (SendContinuousTaskNotification(continuousTaskRecord) != ERR_OK) {
        BGTASK_LOGE("publish error");
        return ERR_BGTASK_NOTIFICATION_ERR;
    }
    continuousTaskInfosMap_.emplace(taskInfoMapKey, continuousTaskRecord);
    OnContinuousTaskChanged(continuousTaskRecord, ContinuousTaskEventTriggerType::TASK_START);
    if (RefreshTaskRecord() != ERR_OK) {
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    return ERR_OK;
}

uint32_t GetBgModeNameIndex(uint32_t bgModeId, bool isNewApi)
{
    if (!isNewApi) {
        return BGMODE_NUMS - 1;
    } else {
        return bgModeId - 1;
    }
}

ErrCode BgContinuousTaskMgr::SendContinuousTaskNotification(
    std::shared_ptr<ContinuousTaskRecord> &continuousTaskRecord)
{
    std::string notificationText {""};
    uint32_t index = GetBgModeNameIndex(continuousTaskRecord->GetBgModeId(), continuousTaskRecord->isNewApi_);
    if (index >= 0 && index < BGMODE_NUMS) {
        notificationText = continuousTaskText_.at(index);
    }
    std::string appName {""};
    if (cachedBundleInfos_.find(continuousTaskRecord->uid_) != cachedBundleInfos_.end()) {
        appName = cachedBundleInfos_.at(continuousTaskRecord->uid_).appName_;
    }
    if (appName.empty() || notificationText.empty()) {
        BGTASK_LOGE("get notification prompt info failed");
        return ERR_BGTASK_INVALID_PARAM;
    }

    return NotificationTools::GetInstance()->PublishNotification(continuousTaskRecord,
        appName, notificationText, bgTaskUid_);
}

ErrCode BgContinuousTaskMgr::StopBackgroundRunning(const std::string &abilityName)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    if (abilityName.empty()) {
        BGTASK_LOGE("abilityName is empty!");
        return ERR_BGTASK_INVALID_PARAM;
    }
    int32_t callingUid = IPCSkeleton::GetCallingUid();

    ErrCode result = ERR_OK;

    StartTrace(HITRACE_TAG_OHOS, "BgContinuousTaskMgr::StopBackgroundRunningInner");
    handler_->PostSyncTask([this, callingUid, abilityName, &result]() {
        result = this->StopBackgroundRunningInner(callingUid, abilityName);
        }, AppExecFwk::EventQueue::Priority::HIGH);
    FinishTrace(HITRACE_TAG_OHOS);

    return result;
}

ErrCode BgContinuousTaskMgr::StopBackgroundRunningInner(int32_t uid, const std::string &abilityName)
{
    std::string mapKey = std::to_string(uid) + SEPARATOR + abilityName;

    auto iter = continuousTaskInfosMap_.find(mapKey);
    if (iter == continuousTaskInfosMap_.end()) {
        BGTASK_LOGW("%{public}s continuous task not exists", mapKey.c_str());
        return ERR_BGTASK_INVALID_PARAM;
    }
    ErrCode result = NotificationTools::GetInstance()->CancelNotification(
        iter->second->GetNotificationLabel(), DEFAULT_NOTIFICATION_ID);
    RemoveContinuousTaskRecord(mapKey);
    return result;
}

void BgContinuousTaskMgr::StopContinuousTask(int32_t uid, int32_t pid, uint32_t taskType)
{
    BGTASK_LOGD("StopContinuousTask begin uid: %{public}d, pid: %{public}d, taskType: %{public}d",
        uid, pid, taskType);
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }
    auto task = [this, uid, pid, taskType]() { this->HandleStopContinuousTask(uid, pid, taskType); };
    handler_->PostTask(task);
}

void BgContinuousTaskMgr::HandleStopContinuousTask(int32_t uid, int32_t pid, uint32_t taskType)
{
    // uid == -1 means target type continuoust task required condition is not met, so cancel all this kind of tasks;
    if (uid == UNSET_UID) {
        RemoveSpecifiedBgTask(taskType);
        return;
    }
    auto iter = continuousTaskInfosMap_.begin();
    int32_t uidNum = 0;
    bool uidExist = false;
    while (iter != continuousTaskInfosMap_.end()) {
        int32_t recordUid = iter->second->GetUid();
        uint32_t bgmodeId = iter->second->GetBgModeId();
        if (recordUid != uid) {
            iter++;
            continue;
        }
        uidNum++;
        uidExist = true;
        if (bgmodeId == taskType) {
            OnContinuousTaskChanged(iter->second, ContinuousTaskEventTriggerType::TASK_CANCEL);
            NotificationTools::GetInstance()->CancelNotification(
                iter->second->GetNotificationLabel(), DEFAULT_NOTIFICATION_ID);
            iter = continuousTaskInfosMap_.erase(iter);
            RefreshTaskRecord();
            uidNum--;
        } else {
            iter++;
        }
    }
    if (uidExist && uidNum == 0) {
        HandleAppContinuousTaskStop(uid);
    }
}

void BgContinuousTaskMgr::RemoveSpecifiedBgTask(uint32_t taskType)
{
    std::map<int32_t, int32_t> appTaskNum;
    auto iter = continuousTaskInfosMap_.begin();
    while (iter != continuousTaskInfosMap_.end()) {
        int32_t uid = iter->second->GetUid();
        if (appTaskNum.find(uid) != appTaskNum.end()) {
            appTaskNum[uid]++;
        } else {
            appTaskNum[uid] = 1;
        }
        if (iter->second->GetBgModeId() == taskType) {
            OnContinuousTaskChanged(iter->second, ContinuousTaskEventTriggerType::TASK_CANCEL);
            NotificationTools::GetInstance()->CancelNotification(
                iter->second->GetNotificationLabel(), DEFAULT_NOTIFICATION_ID);
            iter = continuousTaskInfosMap_.erase(iter);
            RefreshTaskRecord();
            appTaskNum[uid]--;
        } else {
            iter++;
        }
    }
    for (auto var : appTaskNum) {
        if (var.second == 0) {
            HandleAppContinuousTaskStop(var.first);
        }
    }
}

ErrCode BgContinuousTaskMgr::AddSubscriber(const sptr<IBackgroundTaskSubscriber> &subscriber)
{
    if (subscriber == nullptr) {
        BGTASK_LOGE("subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }

    handler_->PostTask([=]() {
        AddSubscriberInner(subscriber);
    });
    return ERR_OK;
}

ErrCode BgContinuousTaskMgr::AddSubscriberInner(const sptr<IBackgroundTaskSubscriber> &subscriber)
{
    auto remoteObj = subscriber->AsObject();
    auto findSuscriber = [&remoteObj](const auto& target) {
        return remoteObj == target->AsObject();
    };

    auto subscriberIter = find_if(bgTaskSubscribers_.begin(), bgTaskSubscribers_.end(), findSuscriber);
    if (subscriberIter != bgTaskSubscribers_.end()) {
        BGTASK_LOGW("target subscriber already exist");
        return ERR_BGTASK_OBJECT_EXISTS;
    }

    bgTaskSubscribers_.emplace_back(subscriber);

    if (subscriber->AsObject() == nullptr) {
        return ERR_BGTASK_INVALID_PARAM;
    }
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

ErrCode BgContinuousTaskMgr::RemoveSubscriber(const sptr<IBackgroundTaskSubscriber> &subscriber)
{
    if (subscriber == nullptr) {
        BGTASK_LOGE("subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }

    handler_->PostTask([=]() {
        RemoveSubscriberInner(subscriber);
    });
    return ERR_OK;
}

ErrCode BgContinuousTaskMgr::RemoveSubscriberInner(const sptr<IBackgroundTaskSubscriber> &subscriber)
{
    auto remote = subscriber->AsObject();
    if (remote == nullptr) {
        BGTASK_LOGE("Subscriber' object is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }
    auto findSubscriber = [&remote](const auto &targetSubscriber) {
        return remote == targetSubscriber->AsObject();
    };

    auto subscriberIter = find_if(bgTaskSubscribers_.begin(), bgTaskSubscribers_.end(), findSubscriber);
    if (subscriberIter == bgTaskSubscribers_.end()) {
        BGTASK_LOGE("subscriber to remove is not exists.");
        return ERR_BGTASK_INVALID_PARAM;
    }

    auto iter = subscriberRecipients_.find(remote);
    if (iter != subscriberRecipients_.end()) {
        iter->first->RemoveDeathRecipient(iter->second);
        subscriberRecipients_.erase(iter);
    }
    subscriber->OnDisconnected();
    bgTaskSubscribers_.erase(subscriberIter);
    BGTASK_LOGD("Remove continuous task subscriber succeed");
    return ERR_OK;
}

ErrCode BgContinuousTaskMgr::GetContinuousTaskApps(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }

    ErrCode result = ERR_OK;

    handler_->PostSyncTask([this, &list, &result]() {
        result = this->GetContinuousTaskAppsInner(list);
        }, AppExecFwk::EventQueue::Priority::HIGH);

    return result;
}

ErrCode BgContinuousTaskMgr::GetContinuousTaskAppsInner(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list)
{
    if (continuousTaskInfosMap_.empty()) {
        return ERR_OK;
    }

    for (auto record : continuousTaskInfosMap_) {
        auto appInfo = std::make_shared<ContinuousTaskCallbackInfo>(record.second->bgModeId_, record.second->uid_,
            record.second->pid_, record.second->abilityName_);
        list.push_back(appInfo);
    }
    return ERR_OK;
}

ErrCode BgContinuousTaskMgr::ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    ErrCode result = ERR_OK;
    handler_->PostSyncTask([&]() {
        result = ShellDumpInner(dumpOption, dumpInfo);
    });

    return result;
}

ErrCode BgContinuousTaskMgr::ShellDumpInner(const std::vector<std::string> &dumpOption,
    std::vector<std::string> &dumpInfo)
{
    if (dumpOption[1] == DUMP_PARAM_LIST_ALL) {
        DumpAllTaskInfo(dumpInfo);
    } else if (dumpOption[1] == DUMP_PARAM_CANCEL_ALL) {
        DumpCancelTask(dumpOption, true);
    } else if (dumpOption[1] == DUMP_PARAM_CANCEL) {
        DumpCancelTask(dumpOption, false);
    } else {
        BGTASK_LOGW("invalid dump param");
    }
    return ERR_OK;
}

void BgContinuousTaskMgr::DumpAllTaskInfo(std::vector<std::string> &dumpInfo)
{
    std::stringstream stream;
    if (continuousTaskInfosMap_.empty()) {
        dumpInfo.emplace_back("No running continuous task\n");
        return;
    }
    std::unordered_map<std::string, std::shared_ptr<ContinuousTaskRecord>>::iterator iter;
    uint32_t index = 1;
    for (iter = continuousTaskInfosMap_.begin(); iter != continuousTaskInfosMap_.end(); iter++) {
        stream.str("");
        stream.clear();
        stream << "No." << index;
        stream << "\tcontinuousTaskKey: " << iter->first << "\n";
        stream << "\tcontinuousTaskValue:" << "\n";
        stream << "\t\tbundleName: " << iter->second->GetBundleName() << "\n";
        stream << "\t\tabilityName: " << iter->second->GetAbilityName() << "\n";
        stream << "\t\tisFromNewApi: " << (iter->second->IsNewApi() ? "true" : "false") << "\n";
        stream << "\t\tbackgroundMode: " << ContinuousTaskModeName[GetBgModeNameIndex(
            iter->second->GetBgModeId(), iter->second->IsNewApi())] << "\n";
        stream << "\t\tuid: " << iter->second->GetUid() << "\n";
        stream << "\t\tuserId: " << iter->second->GetUserId() << "\n";
        stream << "\t\tpid: " << iter->second->GetPid() << "\n";
        stream << "\t\tnotificationLabel: " << iter->second->GetNotificationLabel() << "\n";
        if (iter->second->wantAgentInfo_ != nullptr) {
            stream << "\t\twantAgentBundleName: " << iter->second->wantAgentInfo_->bundleName_ << "\n";
            stream << "\t\twantAgentAbilityName: " << iter->second->wantAgentInfo_->abilityName_ << "\n";
        } else {
            stream << "\t\twantAgentBundleName: " << "NULL" << "\n";
            stream << "\t\twantAgentAbilityName: " << "NULL" << "\n";
        }
        stream << "\n";
        dumpInfo.emplace_back(stream.str());
        index++;
    }
}

void BgContinuousTaskMgr::DumpCancelTask(const std::vector<std::string> &dumpOption, bool cleanAll)
{
    if (cleanAll) {
        std::set<int32_t> uids;
        for (auto item : continuousTaskInfosMap_) {
            NotificationTools::GetInstance()->CancelNotification(item.second->GetNotificationLabel(),
                DEFAULT_NOTIFICATION_ID);
            OnContinuousTaskChanged(item.second, ContinuousTaskEventTriggerType::TASK_CANCEL);
            uids.insert(item.second->uid_);
        }
        continuousTaskInfosMap_.clear();
        RefreshTaskRecord();
        for (int32_t uid : uids) {
            HandleAppContinuousTaskStop(uid);
        }
    } else {
        if (dumpOption.size() < MAX_DUMP_PARAM_NUMS) {
            BGTASK_LOGW("invalid dump param");
            return;
        }
        std::string taskKey = dumpOption[2];
        auto iter = continuousTaskInfosMap_.find(taskKey);
        if (iter == continuousTaskInfosMap_.end()) {
            return;
        }
        NotificationTools::GetInstance()->CancelNotification(iter->second->GetNotificationLabel(),
            DEFAULT_NOTIFICATION_ID);
        RemoveContinuousTaskRecord(taskKey);
    }
}

bool BgContinuousTaskMgr::RemoveContinuousTaskRecord(const std::string &mapKey)
{
    BGTASK_LOGD("task info: %{public}s", mapKey.c_str());
    if (continuousTaskInfosMap_.find(mapKey) == continuousTaskInfosMap_.end()) {
        BGTASK_LOGW("remove TaskInfo failure, no matched task: %{public}s", mapKey.c_str());
        return false;
    }
    auto record = continuousTaskInfosMap_.at(mapKey);
    OnContinuousTaskChanged(record, ContinuousTaskEventTriggerType::TASK_CANCEL);
    continuousTaskInfosMap_.erase(mapKey);
    HandleAppContinuousTaskStop(record->uid_);
    RefreshTaskRecord();
    return true;
}

bool BgContinuousTaskMgr::StopContinuousTaskByUser(const std::string &mapKey)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    bool result = true;
    handler_->PostSyncTask([this, mapKey, &result]() {
        result = RemoveContinuousTaskRecord(mapKey);
    });
    return result;
}

void BgContinuousTaskMgr::OnRemoteSubscriberDied(const wptr<IRemoteObject> &object)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }
    if (object == nullptr) {
        BGTASK_LOGE("remote object is null.");
        return;
    }

    handler_->PostSyncTask([this, &object]() { this->OnRemoteSubscriberDiedInner(object); });
}

void BgContinuousTaskMgr::OnRemoteSubscriberDiedInner(const wptr<IRemoteObject> &object)
{
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

void BgContinuousTaskMgr::OnAbilityStateChanged(int32_t uid, const std::string &abilityName)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }
    auto iter = continuousTaskInfosMap_.begin();
    while (iter != continuousTaskInfosMap_.end()) {
        if (iter->second->uid_ == uid && iter->second->abilityName_ == abilityName) {
            auto record = iter->second;
            OnContinuousTaskChanged(record, ContinuousTaskEventTriggerType::TASK_CANCEL);
            NotificationTools::GetInstance()->CancelNotification(
                record->GetNotificationLabel(), DEFAULT_NOTIFICATION_ID);
            iter = continuousTaskInfosMap_.erase(iter);
            HandleAppContinuousTaskStop(record->uid_);
            RefreshTaskRecord();
        } else {
            iter++;
        }
    }
}

void BgContinuousTaskMgr::OnProcessDied(int32_t uid, int32_t pid)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }

    auto iter = continuousTaskInfosMap_.begin();
    while (iter != continuousTaskInfosMap_.end()) {
        if (iter->second->GetPid() == pid) {
            auto record = iter->second;
            OnContinuousTaskChanged(record, ContinuousTaskEventTriggerType::TASK_CANCEL);
            NotificationTools::GetInstance()->CancelNotification(
                record->GetNotificationLabel(), DEFAULT_NOTIFICATION_ID);
            iter = continuousTaskInfosMap_.erase(iter);
            HandleAppContinuousTaskStop(record->uid_);
            RefreshTaskRecord();
        } else {
            iter++;
        }
    }
}

void BgContinuousTaskMgr::OnContinuousTaskChanged(const std::shared_ptr<ContinuousTaskRecord> continuousTaskInfo,
    ContinuousTaskEventTriggerType changeEventType)
{
    if (continuousTaskInfo == nullptr) {
        BGTASK_LOGW("ContinuousTaskRecord is null");
        return;
    }

    if (bgTaskSubscribers_.empty()) {
        BGTASK_LOGI("Background Task Subscriber List is empty");
        return;
    }

    std::shared_ptr<ContinuousTaskCallbackInfo> continuousTaskCallbackInfo
        = std::make_shared<ContinuousTaskCallbackInfo>(continuousTaskInfo->GetBgModeId(),
        continuousTaskInfo->GetUid(), continuousTaskInfo->GetPid(), continuousTaskInfo->GetAbilityName());

    switch (changeEventType) {
        case ContinuousTaskEventTriggerType::TASK_START:
            for (auto iter = bgTaskSubscribers_.begin(); iter != bgTaskSubscribers_.end(); iter++) {
                BGTASK_LOGD("continuous task start callback trigger");
                (*iter)->OnContinuousTaskStart(continuousTaskCallbackInfo);
            }
            break;
        case ContinuousTaskEventTriggerType::TASK_CANCEL:
            for (auto iter = bgTaskSubscribers_.begin(); iter != bgTaskSubscribers_.end(); iter++) {
                BGTASK_LOGD("continuous task stop callback trigger");
                (*iter)->OnContinuousTaskStop(continuousTaskCallbackInfo);
            }
            break;
    }
}

void BgContinuousTaskMgr::OnBundleInfoChanged(const std::string &action, const std::string &bundleName, int32_t uid)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED
        || action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED
        || action == EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_REMOVED
        || action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_FULLY_REMOVED
        || action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED
        || action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REPLACED) {
        cachedBundleInfos_.erase(uid);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED) {
        cachedBundleInfos_.erase(uid);
        auto iter = continuousTaskInfosMap_.begin();
        while (iter != continuousTaskInfosMap_.end()) {
            if (iter->second->GetUid() == uid) {
                auto record = iter->second;
                OnContinuousTaskChanged(record, ContinuousTaskEventTriggerType::TASK_CANCEL);
                NotificationTools::GetInstance()->CancelNotification(
                    record->GetNotificationLabel(), DEFAULT_NOTIFICATION_ID);
                iter = continuousTaskInfosMap_.erase(iter);
                HandleAppContinuousTaskStop(uid);
                RefreshTaskRecord();
            } else {
                iter++;
            }
        }
    } else {
        BGTASK_LOGW("get unregister common event!");
        return;
    }
}

void BgContinuousTaskMgr::OnAccountsStateChanged(int32_t id)
{
    std::vector<int32_t> activatedOsAccountIds;
#ifdef HAS_OS_ACCOUNT_PART
    if (AccountSA::OsAccountManager::QueryActiveOsAccountIds(activatedOsAccountIds) != ERR_OK) {
        BGTASK_LOGE("query activated account failed");
        return;
    }
#else // HAS_OS_ACCOUNT_PART
    activatedOsAccountIds.push_back(DEFAULT_OS_ACCOUNT_ID);
    BGTASK_LOGI("there is no account part, use default id.");
#endif // HAS_OS_ACCOUNT_PART
    auto iter = continuousTaskInfosMap_.begin();
    while (iter != continuousTaskInfosMap_.end()) {
        auto idIter = find(activatedOsAccountIds.begin(), activatedOsAccountIds.end(), iter->second->GetUserId());
        if (idIter == activatedOsAccountIds.end()) {
            auto record = iter->second;
            OnContinuousTaskChanged(record, ContinuousTaskEventTriggerType::TASK_CANCEL);
            NotificationTools::GetInstance()->CancelNotification(
                record->GetNotificationLabel(), DEFAULT_NOTIFICATION_ID);
            iter = continuousTaskInfosMap_.erase(iter);
            HandleAppContinuousTaskStop(record->uid_);
            RefreshTaskRecord();
        } else {
            iter++;
        }
    }
}

void BgContinuousTaskMgr::HandleAppContinuousTaskStop(int32_t uid)
{
    auto findUid = [uid](const auto &target) {
        return uid == target.second->GetUid();
    };
    auto findUidIter = find_if(continuousTaskInfosMap_.begin(), continuousTaskInfosMap_.end(), findUid);
    if (findUidIter != continuousTaskInfosMap_.end()) {
        return;
    }
    BGTASK_LOGI("All continuous task has stopped of uid: %{public}d, so notify related subsystem", uid);
    for (auto iter = bgTaskSubscribers_.begin(); iter != bgTaskSubscribers_.end(); iter++) {
        (*iter)->OnAppContinuousTaskStop(uid);
    }
}

int32_t BgContinuousTaskMgr::RefreshTaskRecord()
{
    if (dataStorage_ == nullptr) {
        BGTASK_LOGE("data storage object is null");
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    if (dataStorage_->RefreshTaskRecord(continuousTaskInfosMap_) != ERR_OK) {
        BGTASK_LOGE("refresh data failed");
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    return ERR_OK;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
