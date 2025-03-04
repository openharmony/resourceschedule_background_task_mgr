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
#include "background_task_mgr_service.h"

#include <sstream>
#include <unistd.h>
#include <fcntl.h>

#include "app_mgr_client.h"
#include "bundle_constants.h"
#include "bundle_manager_helper.h"
#include "common_event_support.h"
#include "common_event_manager.h"
#include "common_utils.h"
#include "errors.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "hisysevent.h"
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

#include "bgtask_common.h"
#include "bgtaskmgr_inner_errors.h"
#include "continuous_task_record.h"
#include "continuous_task_log.h"
#include "system_event_observer.h"
#include "data_storage_helper.h"
#ifdef SUPPORT_GRAPHICS
#include "locale_config.h"
#endif // SUPPORT_GRAPHICS
#include "background_mode.h"
#include "background_sub_mode.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static const char *g_taskPromptResNames[] = {
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

static const char *g_taskPromptResNamesSubMode[] = {
    "ohos_bgsubmode_prompt_car_key",
};

static constexpr char SEPARATOR[] = "_";
static constexpr char DUMP_PARAM_LIST_ALL[] = "--all";
static constexpr char DUMP_PARAM_CANCEL_ALL[] = "--cancel_all";
static constexpr char DUMP_PARAM_CANCEL[] = "--cancel";
static constexpr char BGMODE_PERMISSION[] = "ohos.permission.KEEP_BACKGROUND_RUNNING";
static constexpr char BG_TASK_RES_BUNDLE_NAME[] = "com.ohos.backgroundtaskmgr.resources";
static constexpr char BG_TASK_SUB_MODE_TYPE[] = "subMode";
static constexpr uint32_t SYSTEM_APP_BGMODE_WIFI_INTERACTION = 64;
static constexpr uint32_t PC_BGMODE_TASK_KEEPING = 256;
static constexpr int32_t DELAY_TIME = 2000;
static constexpr int32_t RECLAIM_MEMORY_DELAY_TIME = 20 * 60 * 1000;
static constexpr int32_t MAX_DUMP_PARAM_NUMS = 3;
static constexpr uint32_t INVALID_BGMODE = 0;
static constexpr uint32_t BG_MODE_INDEX_HEAD = 1;
static constexpr uint32_t BGMODE_NUMS = 10;
static constexpr uint32_t VOIP_SA_UID = 7022;
static constexpr uint32_t HEALTHSPORT_SA_UID = 7259;
#ifdef DFEATURE_PRODUCT_WATCH
static constexpr uint32_t HEALTHSPORT_SA_UID = 7500;
#endif // DFEATURE_PRODUCT_WATCH
static constexpr uint32_t ALL_MODES = 0xFF;

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

bool BgContinuousTaskMgr::Init(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
{
    BGTASK_LOGI("BgContinuousTaskMgr service init start");
    if (runner == nullptr) {
        BGTASK_LOGE("BgContinuousTaskMgr runner create failed!");
        return false;
    }
    handler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    if (handler_ == nullptr) {
        BGTASK_LOGE("BgContinuousTaskMgr handler create failed!");
        return false;
    }
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    bgTaskUid_ = IPCSkeleton::GetCallingUid();
    BGTASK_LOGI("BgContinuousTaskMgr service uid is: %{public}d", bgTaskUid_);
    IPCSkeleton::SetCallingIdentity(identity);
    auto registerTask = [this]() { this->InitNecessaryState(); };
    handler_->PostSyncTask(registerTask);
    auto self = shared_from_this();
    auto reclaimTask = [self]() {
        if (self) {
            self->ReclaimProcessMemory(getpid());
        }
    };
    handler_->PostTask(reclaimTask, RECLAIM_MEMORY_DELAY_TIME);
    return true;
}

void BgContinuousTaskMgr::ReclaimProcessMemory(int32_t pid)
{
    BGTASK_LOGI("BgContinuousTaskMgr reclaimProcessMemory pid: %{public}d start.", pid);
    std::string path = "/proc/" + std::to_string(pid) + "/reclaim";
    std::string contentStr = "1";
    int fd = open(path.c_str(), O_WRONLY);
    if (fd < 0) {
        BGTASK_LOGE("BgContinuousTaskMgr ReclaimProcessMemory open file failed!");
        return;
    }
    int res = write(fd, contentStr.c_str(), contentStr.length());
    if (res == -1) {
        BGTASK_LOGE("BgContinuousTaskMgr ReclaimProcessMemory write file failed!");
    }
    close(fd);
    BGTASK_LOGI("BgContinuousTaskMgr reclaimProcessMemory pid: %{public}d end.", pid);
}

void BgContinuousTaskMgr::Clear()
{
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    Notification::NotificationHelper::UnSubscribeNotification(*subscriber_);
#endif
    if (systemEventListener_ != nullptr) {
        systemEventListener_->Unsubscribe();
    }
    UnregisterAppStateObserver();
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
    BGTASK_LOGW("RegisterNotificationSubscriber start");
    if (!RegisterNotificationSubscriber()) {
        BGTASK_LOGE("RegisterNotificationSubscriber failed");
        return;
    }
    BGTASK_LOGW("RegisterAppStateObserver start");
    if (!RegisterAppStateObserver()) {
        BGTASK_LOGE("RegisterAppStateObserver failed");
        return;
    }
    BGTASK_LOGW("RegisterSysCommEventListener start");
    if (!RegisterSysCommEventListener()) {
        BGTASK_LOGE("RegisterSysCommEventListener failed");
        return;
    }
    BGTASK_LOGW("RegisterConfigurationObserver start");
    if (!RegisterConfigurationObserver()) {
        BGTASK_LOGE("RegisterConfigurationObserver failed");
        return;
    }
    BGTASK_LOGW("InitRequiredResourceInfo start");
    InitRequiredResourceInfo();
}

void BgContinuousTaskMgr::HandlePersistenceData()
{
    BGTASK_LOGI("service restart, restore data");
    DelayedSingleton<DataStorageHelper>::GetInstance()->RestoreTaskRecord(continuousTaskInfosMap_);
    auto appMgrClient = std::make_shared<AppExecFwk::AppMgrClient>();
    std::vector<AppExecFwk::RunningProcessInfo> allAppProcessInfos;
    if (appMgrClient == nullptr) {
        BGTASK_LOGE("connect to app mgr service failed");
        return;
    }
    if (appMgrClient->GetAllRunningProcesses(allAppProcessInfos) != ERR_OK) {
        BGTASK_LOGE("get all running process failed");
        return;
    }
    CheckPersistenceData(allAppProcessInfos);
    DelayedSingleton<DataStorageHelper>::GetInstance()->RefreshTaskRecord(continuousTaskInfosMap_);
}

bool BgContinuousTaskMgr::CheckProcessUidInfo(const std::vector<AppExecFwk::RunningProcessInfo> &allProcesses,
    int32_t uid)
{
    for (const auto &runningProcessInfo : allProcesses) {
        if (runningProcessInfo.uid_ == uid) {
            return true;
        }
    }
    return false;
}

void BgContinuousTaskMgr::CheckPersistenceData(const std::vector<AppExecFwk::RunningProcessInfo> &allProcesses)
{
    auto iter = continuousTaskInfosMap_.begin();
    int32_t maxNotificationId = -1;
    int32_t maxContinuousTaskId = -1;

    while (iter != continuousTaskInfosMap_.end()) {
        bool pidIsAlive = checkPidCondition(allProcesses, iter->second->GetPid());
        int32_t notificationId = iter->second->GetNotificationId();
        if (notificationId > maxNotificationId) {
            maxNotificationId = notificationId;
        }
        if (iter->second->continuousTaskId_ > maxContinuousTaskId) {
            maxContinuousTaskId = iter->second->continuousTaskId_;
        }

        if (pidIsAlive) {
            if (iter->second->GetNotificationId() == -1) {
                BGTASK_LOGI("notification id is -1, continue");
                iter++;
                continue;
            }
            if (cachedBundleInfos_.find(iter->second->GetUid()) == cachedBundleInfos_.end()) {
                std::string mainAbilityLabel = GetMainAbilityLabel(iter->second->GetBundleName(),
                    iter->second->GetUserId());
                SetCachedBundleInfo(iter->second->GetUid(), iter->second->GetUserId(),
                    iter->second->GetBundleName(), mainAbilityLabel);
            }
            SendContinuousTaskNotification(iter->second);
            BGTASK_LOGI("restore notification id %{public}d", iter->second->GetNotificationId());
            iter++;
        } else {
            BGTASK_LOGI("process %{public}d die, not restore notification id %{public}d", iter->second->GetPid(),
                iter->second->GetNotificationId());
            iter = continuousTaskInfosMap_.erase(iter);
        }
    }
    if (maxNotificationId != -1) {
        BGTASK_LOGI("set maxNotificationId %{public}d", maxNotificationId);
        NotificationTools::SetNotificationIdIndex(maxNotificationId);
    }
    if (maxContinuousTaskId != -1) {
        BGTASK_LOGI("set maxContinuousTaskId %{public}d", maxContinuousTaskId);
        continuousTaskIdIndex_ = maxContinuousTaskId;
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
        BGTASK_LOGW("init required resource info failed");
    }
    HandlePersistenceData();
    isSysReady_.store(true);
    DelayedSingleton<BackgroundTaskMgrService>::GetInstance()->SetReady(ServiceReadyState::CONTINUOUS_SERVICE_READY);
    BGTASK_LOGI("SetReady CONTINUOUS_SERVICE_READY");
}

bool BgContinuousTaskMgr::RegisterNotificationSubscriber()
{
    bool res = true;
#ifdef DISTRIBUTED_NOTIFICATION_ENABLE
    subscriber_ = std::make_shared<TaskNotificationSubscriber>();
    if (Notification::NotificationHelper::SubscribeNotificationSelf(*subscriber_) != ERR_OK) {
        BGTASK_LOGE("SubscribeNotificationSelf failed!");
        res = false;
    }
#endif
    return res;
}

__attribute__((no_sanitize("cfi"))) bool BgContinuousTaskMgr::RegisterAppStateObserver()
{
    appStateObserver_ = new (std::nothrow) AppStateObserver(); // must be sprt
    if (!appStateObserver_) {
        BGTASK_LOGE("appStateObserver_ null");
        return false;
    }
    auto res = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->
        RegisterApplicationStateObserver(appStateObserver_);
    if (res != ERR_OK) {
        BGTASK_LOGE("RegisterApplicationStateObserver error");
        return false;
    }
    appStateObserver_->SetEventHandler(handler_);
    return true;
}

void BgContinuousTaskMgr::UnregisterAppStateObserver()
{
    if (!appStateObserver_) {
        return;
    }
    auto res = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->
        UnregisterApplicationStateObserver(appStateObserver_);
    if (res != ERR_OK) {
        BGTASK_LOGE("UnregisterApplicationStateObserver error");
        return;
    }
    appStateObserver_ = nullptr;
    BGTASK_LOGI("UnregisterApplicationStateObserver ok");
}

__attribute__((no_sanitize("cfi"))) bool BgContinuousTaskMgr::RegisterConfigurationObserver()
{
    auto appMgrClient = std::make_shared<AppExecFwk::AppMgrClient>();
    if (appMgrClient == nullptr) {
        BGTASK_LOGE("connect to app mgr service failed");
        return false;
    }
    configChangeObserver_ = sptr<AppExecFwk::IConfigurationObserver>(
        new (std::nothrow) ConfigChangeObserver(handler_, shared_from_this()));
    if (appMgrClient->RegisterConfigurationObserver(configChangeObserver_) != ERR_OK) {
        BGTASK_LOGE("register configuration observer failed");
        return false;
    }
    return true;
}

std::shared_ptr<Global::Resource::ResourceManager> BgContinuousTaskMgr::GetBundleResMgr(
    const AppExecFwk::BundleInfo &bundleInfo)
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    if (!resourceManager) {
        BGTASK_LOGE("create resourceManager failed");
        return nullptr;
    }
    for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
        std::string moduleResPath = hapModuleInfo.hapPath.empty() ? hapModuleInfo.resourcePath : hapModuleInfo.hapPath;
        if (moduleResPath.empty()) {
            continue;
        }
        BGTASK_LOGD("GetBundleResMgr, moduleResPath: %{private}s", moduleResPath.c_str());
        if (!resourceManager->AddResource(moduleResPath.c_str())) {
            BGTASK_LOGW("GetBundleResMgr AddResource failed");
        }
    }
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
#ifdef SUPPORT_GRAPHICS
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(Global::I18n::LocaleConfig::GetSystemLanguage(), status);
    resConfig->SetLocaleInfo(locale);
#endif // SUPPORT_GRAPHICS
    resourceManager->UpdateResConfig(*resConfig);
    return resourceManager;
}

bool BgContinuousTaskMgr::GetNotificationPrompt()
{
    continuousTaskText_.clear();
    continuousTaskSubText_.clear();
    AppExecFwk::BundleInfo bundleInfo;
    if (!BundleManagerHelper::GetInstance()->GetBundleInfo(BG_TASK_RES_BUNDLE_NAME,
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo)) {
        BGTASK_LOGE("get background task res: %{public}s bundle info failed", BG_TASK_RES_BUNDLE_NAME);
        return false;
    }
    auto resourceManager = GetBundleResMgr(bundleInfo);
    if (resourceManager == nullptr) {
        BGTASK_LOGE("Get bgtask resource hap manager failed");
        return false;
    }
    std::string taskText {""};
    for (std::string name : g_taskPromptResNames) {
        resourceManager->GetStringByName(name.c_str(), taskText);
        if (taskText.empty()) {
            BGTASK_LOGE("get continuous task notification text failed!");
            return false;
        }
        BGTASK_LOGI("get taskText: %{public}s", taskText.c_str());
        continuousTaskText_.push_back(taskText);
    }
    std::string taskSubText {""};
    for (std::string name : g_taskPromptResNamesSubMode) {
        resourceManager->GetStringByName(name.c_str(), taskSubText);
        if (taskSubText.empty()) {
            BGTASK_LOGE("get continuous task notification sub text failed!");
            return false;
        }
        BGTASK_LOGI("get sub taskSubText: %{public}s", taskSubText.c_str());
        continuousTaskSubText_.push_back(taskSubText);
    }
    return true;
}

__attribute__((no_sanitize("cfi"))) bool BgContinuousTaskMgr::RegisterSysCommEventListener()
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
    const std::string &bundleName, const std::string &appName)
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

ErrCode BgContinuousTaskMgr::CheckBgmodeType(uint32_t configuredBgMode, uint32_t requestedBgModeId,
    bool isNewApi, uint64_t fullTokenId)
{
    if (!isNewApi) {
        if (configuredBgMode == INVALID_BGMODE) {
            BGTASK_LOGE("ability without background mode config");
            return ERR_BGMODE_NULL_OR_TYPE_ERR;
        } else {
            return ERR_OK;
        }
    } else {
        uint32_t recordedBgMode = BG_MODE_INDEX_HEAD << (requestedBgModeId - 1);
        if (recordedBgMode == SYSTEM_APP_BGMODE_WIFI_INTERACTION &&
            !BundleManagerHelper::GetInstance()->IsSystemApp(fullTokenId)) {
            BGTASK_LOGE("wifiInteraction background mode only support for system app");
            return ERR_BGTASK_NOT_SYSTEM_APP;
        }
        if (recordedBgMode == PC_BGMODE_TASK_KEEPING && !SUPPORT_TASK_KEEPING) {
            BGTASK_LOGE("task keeping is not supported, please set param "
                "persist.sys.bgtask_support_task_keeping.");
            return ERR_BGTASK_KEEPING_TASK_VERIFY_ERR;
        }
        if (requestedBgModeId == INVALID_BGMODE || (configuredBgMode &
            (BG_MODE_INDEX_HEAD << (requestedBgModeId - 1))) == 0) {
            BGTASK_LOGE("requested background mode is not declared in config file, configuredBgMode: %{public}d",
                configuredBgMode);
            return ERR_BGTASK_INVALID_BGMODE;
        }
    }
    return ERR_OK;
}

uint32_t BgContinuousTaskMgr::GetBackgroundModeInfo(int32_t uid, const std::string &abilityName)
{
    if (cachedBundleInfos_.find(uid) != cachedBundleInfos_.end()) {
        auto cachedBundleInfo = cachedBundleInfos_.at(uid);
        if (cachedBundleInfo.abilityBgMode_.find(abilityName) !=
            cachedBundleInfo.abilityBgMode_.end()) {
            return cachedBundleInfo.abilityBgMode_.at(abilityName);
        }
    }
    BGTASK_LOGI("get background mode info, uid: %{public}d, abilityName: %{public}s", uid, abilityName.c_str());
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
        if (taskParam->isBatchApi_ && taskParam->bgModeIds_.empty()) {
            BGTASK_LOGE("bgModeIds_ is empty");
            return false;
        }
        if (taskParam->abilityId_ < 0) {
            BGTASK_LOGE("abilityId_ is invalid");
        }
    } else {
        if (taskParam->abilityName_.empty()) {
            BGTASK_LOGE("continuous task params invalid!");
            return false;
        }
    }
    return true;
}

ErrCode BgContinuousTaskMgr::CheckBgmodeTypeForInner(uint32_t requestedBgModeId)
{
    if (requestedBgModeId == INVALID_BGMODE || requestedBgModeId > BGMODE_NUMS) {
        BGTASK_LOGE("requested background mode is not declared in config file!");
        return ERR_BGTASK_INVALID_BGMODE;
    }
    return ERR_OK;
}

ErrCode BgContinuousTaskMgr::RequestBackgroundRunningForInner(const sptr<ContinuousTaskParamForInner> &taskParam)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }

    if (!taskParam) {
        BGTASK_LOGE("continuous task param is null!");
        return ERR_BGTASK_CHECK_TASK_PARAM;
    }
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    // webview sdk申请长时任务，上下文在应用。callkit sa 申请长时时，上下文在sa;
    if (callingUid != VOIP_SA_UID && callingUid != HEALTHSPORT_SA_UID && callingUid != taskParam->uid_) {
        BGTASK_LOGE("continuous task param uid %{public}d is invalid, real %{public}d", taskParam->uid_, callingUid);
        return ERR_BGTASK_CHECK_TASK_PARAM;
    }
    BGTASK_LOGI("continuous task param uid %{public}d, real %{public}d", taskParam->uid_, callingUid);
    if (taskParam->isStart_) {
        return StartBackgroundRunningForInner(taskParam);
    }
    return StopBackgroundRunningForInner(taskParam);
}

ErrCode BgContinuousTaskMgr::StartBackgroundRunningForInner(const sptr<ContinuousTaskParamForInner> &taskParam)
{
    ErrCode result = ERR_OK;
    int32_t uid = taskParam->uid_;
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    std::string bundleName = BundleManagerHelper::GetInstance()->GetClientBundleName(uid);
    std::string abilityName = "Webview" + std::to_string(taskParam->bgModeId_);
    int32_t userId = -1;

#ifdef HAS_OS_ACCOUNT_PART
    AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId);
#else // HAS_OS_ACCOUNT_PART
    GetOsAccountIdFromUid(uid, userId);
#endif // HAS_OS_ACCOUNT_PART

    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>(bundleName,
        abilityName, uid, callingPid, taskParam->bgModeId_);
    continuousTaskRecord->isNewApi_ = true;
    continuousTaskRecord->isFromWebview_ = true;
    continuousTaskRecord->userId_ = userId;
    continuousTaskRecord->fullTokenId_ = fullTokenId;

    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Service::StartBackgroundRunningInner");
    handler_->PostSyncTask([this, continuousTaskRecord, &result]() mutable {
        result = this->StartBackgroundRunningInner(continuousTaskRecord);
        }, AppExecFwk::EventQueue::Priority::HIGH);

    return result;
}

ErrCode BgContinuousTaskMgr::StartBackgroundRunning(const sptr<ContinuousTaskParam> &taskParam)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    if (!CheckTaskParam(taskParam)) {
        return ERR_BGTASK_CHECK_TASK_PARAM;
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
    std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord = std::make_shared<ContinuousTaskRecord>(bundleName,
        taskParam->abilityName_, callingUid, callingPid, taskParam->bgModeId_, taskParam->isBatchApi_,
        taskParam->bgModeIds_, taskParam->abilityId_);
    InitRecordParam(continuousTaskRecord, taskParam, userId);
    if (taskParam->wantAgent_ != nullptr && taskParam->wantAgent_->GetPendingWant() != nullptr) {
        auto target = taskParam->wantAgent_->GetPendingWant()->GetTarget();
        auto want = taskParam->wantAgent_->GetPendingWant()->GetWant(target);
        if (want != nullptr) {
            std::shared_ptr<WantAgentInfo> info = std::make_shared<WantAgentInfo>();
            info->bundleName_ = want->GetOperation().GetBundleName();
            info->abilityName_ = want->GetOperation().GetAbilityName();
            continuousTaskRecord->wantAgentInfo_ = info;
            result = CheckSubMode(want, continuousTaskRecord);
            if (result != ERR_OK) {
                return result;
            }
        }
    }
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Service::StartBackgroundRunningInner");
    handler_->PostSyncTask([this, continuousTaskRecord, &result]() mutable {
        result = this->StartBackgroundRunningInner(continuousTaskRecord);
        }, AppExecFwk::EventQueue::Priority::HIGH);
    taskParam->notificationId_ = continuousTaskRecord->GetNotificationId();
    taskParam->continuousTaskId_ = continuousTaskRecord->continuousTaskId_;
    return result;
}

void BgContinuousTaskMgr::InitRecordParam(std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord,
    const sptr<ContinuousTaskParam> &taskParam, int32_t userId)
{
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    continuousTaskRecord->wantAgent_ = taskParam->wantAgent_;
    continuousTaskRecord->userId_ = userId;
    continuousTaskRecord->isNewApi_ = taskParam->isNewApi_;
    continuousTaskRecord->appName_ = taskParam->appName_;
    continuousTaskRecord->fullTokenId_ = fullTokenId;
    continuousTaskRecord->isSystem_ = BundleManagerHelper::GetInstance()->IsSystemApp(fullTokenId);
}

ErrCode BgContinuousTaskMgr::CheckSubMode(const std::shared_ptr<AAFwk::Want> want,
    std::shared_ptr<ContinuousTaskRecord> record)
{
    if (want->HasParameter(BG_TASK_SUB_MODE_TYPE)) {
        if (CommonUtils::CheckExistMode(record->bgModeIds_, BackgroundMode::BLUETOOTH_INTERACTION) &&
            want->GetIntParam(BG_TASK_SUB_MODE_TYPE, 0) == BackgroundSubMode::CAR_KEY) {
            record->bgSubModeIds_.push_back(BackgroundSubMode::CAR_KEY);
        } else {
            BGTASK_LOGE("subMode is invaild.");
            return ERR_BGTASK_CHECK_TASK_PARAM;
        }
    }
    return ERR_OK;
}

ErrCode BgContinuousTaskMgr::UpdateBackgroundRunning(const sptr<ContinuousTaskParam> &taskParam)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    if (!BundleManagerHelper::GetInstance()->CheckPermission(BGMODE_PERMISSION)) {
        BGTASK_LOGE("background mode permission is not passed");
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    ErrCode result = ERR_OK;
    int32_t callingUid = IPCSkeleton::GetCallingUid();

    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Service::UpdateBackgroundRunningInner");
    std::string taskInfoMapKey = std::to_string(callingUid) + SEPARATOR + taskParam->abilityName_ + SEPARATOR +
        std::to_string(taskParam->abilityId_);
    auto self = shared_from_this();
    handler_->PostSyncTask([self, &taskInfoMapKey, &result, taskParam]() mutable {
        if (!self) {
            BGTASK_LOGE("self is null");
            result = ERR_BGTASK_SERVICE_INNER_ERROR;
            return;
        }
        result = self->UpdateBackgroundRunningInner(taskInfoMapKey, taskParam);
        }, AppExecFwk::EventQueue::Priority::HIGH);
    return result;
}

ErrCode BgContinuousTaskMgr::UpdateBackgroundRunningInner(const std::string &taskInfoMapKey,
    const sptr<ContinuousTaskParam> &taskParam)
{
    ErrCode ret;

    auto iter = continuousTaskInfosMap_.find(taskInfoMapKey);
    if (iter == continuousTaskInfosMap_.end()) {
        BGTASK_LOGW("continuous task is not exist: %{public}s, use start befor update", taskInfoMapKey.c_str());
        return ERR_BGTASK_OBJECT_NOT_EXIST;
    }

    auto continuousTaskRecord = iter->second;
    auto oldModes = continuousTaskRecord->bgModeIds_;

    BGTASK_LOGI("continuous task mode %{public}d, old modes: %{public}s, new modes %{public}s, isBatchApi %{public}d,"
        " abilityId %{public}d", continuousTaskRecord->bgModeId_,
        continuousTaskRecord->ToString(continuousTaskRecord->bgModeIds_).c_str(),
        continuousTaskRecord->ToString(taskParam->bgModeIds_).c_str(),
        continuousTaskRecord->isBatchApi_, continuousTaskRecord->abilityId_);
    // update continuoustask by same modes.
    if (CommonUtils::CheckModesSame(oldModes, taskParam->bgModeIds_)) {
        BGTASK_LOGI("uid: %{public}d, bundleName: %{public}s, abilityId: %{public}d have same modes.",
            continuousTaskRecord->uid_, continuousTaskRecord->bundleName_.c_str(), continuousTaskRecord->abilityId_);
        return ERR_OK;
    }
    if (!CommonUtils::CheckExistMode(taskParam->bgModeIds_, BackgroundMode::BLUETOOTH_INTERACTION) &&
        !continuousTaskRecord->bgSubModeIds_.empty()) {
        continuousTaskRecord->bgSubModeIds_.clear();
    }
    uint32_t configuredBgMode = GetBackgroundModeInfo(continuousTaskRecord->uid_, continuousTaskRecord->abilityName_);
    for (auto it =  taskParam->bgModeIds_.begin(); it != taskParam->bgModeIds_.end(); it++) {
        ret = CheckBgmodeType(configuredBgMode, *it, true, continuousTaskRecord->fullTokenId_);
        if (ret != ERR_OK) {
            BGTASK_LOGE("CheckBgmodeType error, config mode: %{public}u, apply mode: %{public}u.", configuredBgMode,
                *it);
            return ret;
        }
    }
    continuousTaskRecord->bgModeIds_ = taskParam->bgModeIds_;
    continuousTaskRecord->isBatchApi_ = taskParam->isBatchApi_;

    // old and new task hava mode: DATA_TRANSFER, not update notification
    if (CommonUtils::CheckExistMode(oldModes, BackgroundMode::DATA_TRANSFER) &&
        CommonUtils::CheckExistMode(continuousTaskRecord->bgModeIds_, BackgroundMode::DATA_TRANSFER)) {
        BGTASK_LOGI("uid: %{public}d, bundleName: %{public}s, abilityId: %{public}d have same mode: DATA_TRANSFER",
            continuousTaskRecord->uid_, continuousTaskRecord->bundleName_.c_str(), continuousTaskRecord->abilityId_);
    } else {
        ret = SendContinuousTaskNotification(continuousTaskRecord);
        if (ret != ERR_OK) {
            BGTASK_LOGE("publish error");
            return ret;
        }
    }
    OnContinuousTaskChanged(iter->second, ContinuousTaskEventTriggerType::TASK_UPDATE);
    taskParam->notificationId_ = continuousTaskRecord->GetNotificationId();
    taskParam->continuousTaskId_ = continuousTaskRecord->GetContinuousTaskId();
    return RefreshTaskRecord();
}

ErrCode BgContinuousTaskMgr::StartBackgroundRunningInner(std::shared_ptr<ContinuousTaskRecord> &continuousTaskRecord)
{
    std::string taskInfoMapKey = std::to_string(continuousTaskRecord->uid_) + SEPARATOR
        + continuousTaskRecord->abilityName_ + SEPARATOR + std::to_string(continuousTaskRecord->abilityId_);
    if (continuousTaskInfosMap_.find(taskInfoMapKey) != continuousTaskInfosMap_.end()) {
        BGTASK_LOGW("continuous task is already exist: %{public}s", taskInfoMapKey.c_str());
        return ERR_BGTASK_OBJECT_EXISTS;
    }
    BGTASK_LOGI("continuous task mode: %{public}u, modes %{public}s, isBatchApi %{public}d, uid %{public}d,"
        " abilityId %{public}d", continuousTaskRecord->bgModeId_,
        continuousTaskRecord->ToString(continuousTaskRecord->bgModeIds_).c_str(),
        continuousTaskRecord->isBatchApi_, continuousTaskRecord->uid_, continuousTaskRecord->abilityId_);
    if (!continuousTaskRecord->isFromWebview_
        && cachedBundleInfos_.find(continuousTaskRecord->uid_) == cachedBundleInfos_.end()) {
        std::string mainAbilityLabel = GetMainAbilityLabel(continuousTaskRecord->bundleName_,
            continuousTaskRecord->userId_);
        SetCachedBundleInfo(continuousTaskRecord->uid_, continuousTaskRecord->userId_,
            continuousTaskRecord->bundleName_, mainAbilityLabel);
    }

    ErrCode ret;
    if (continuousTaskRecord->isFromWebview_) {
        ret = CheckBgmodeTypeForInner(continuousTaskRecord->bgModeId_);
    } else {
        uint32_t configuredBgMode = GetBackgroundModeInfo(continuousTaskRecord->uid_,
            continuousTaskRecord->abilityName_);
        for (auto it = continuousTaskRecord->bgModeIds_.begin(); it != continuousTaskRecord->bgModeIds_.end(); it++) {
            ret = CheckBgmodeType(configuredBgMode, *it, continuousTaskRecord->isNewApi_,
                continuousTaskRecord->fullTokenId_);
            if (ret != ERR_OK) {
                BGTASK_LOGE("CheckBgmodeType invalid!");
                return ret;
            }
        }
    }

    if (!continuousTaskRecord->isFromWebview_) {
        ret = SendContinuousTaskNotification(continuousTaskRecord);
        if (ret != ERR_OK) {
            BGTASK_LOGE("publish error");
            return ret;
        }
    }
    continuousTaskRecord->continuousTaskId_ = ++continuousTaskIdIndex_;
    continuousTaskInfosMap_.emplace(taskInfoMapKey, continuousTaskRecord);
    OnContinuousTaskChanged(continuousTaskRecord, ContinuousTaskEventTriggerType::TASK_START);
    return RefreshTaskRecord();
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
    if (continuousTaskText_.empty()) {
        BGTASK_LOGE("get notification prompt info failed, continuousTaskText_ is empty");
        return ERR_BGTASK_NOTIFICATION_VERIFY_FAILED;
    }
    std::string appName {""};
    if (cachedBundleInfos_.find(continuousTaskRecord->uid_) != cachedBundleInfos_.end()) {
        appName = cachedBundleInfos_.at(continuousTaskRecord->uid_).appName_;
    }
    if (appName.empty()) {
        BGTASK_LOGE("appName is empty");
        return ERR_BGTASK_NOTIFICATION_VERIFY_FAILED;
    }

    std::string notificationText {""};
    for (auto mode : continuousTaskRecord->bgModeIds_) {
        if (mode == BackgroundMode::AUDIO_PLAYBACK || ((mode == BackgroundMode::VOIP ||
            mode == BackgroundMode::AUDIO_RECORDING) && continuousTaskRecord->IsSystem())) {
            continue;
        }
        BGTASK_LOGD("mode %{public}d", mode);
        uint32_t index = GetBgModeNameIndex(mode, continuousTaskRecord->isNewApi_);
        if (index < continuousTaskText_.size()) {
            notificationText += continuousTaskText_.at(index);
            notificationText += "\n";
        } else {
            BGTASK_LOGI("index is invalid");
            return ERR_BGTASK_NOTIFICATION_VERIFY_FAILED;
        }
    }
    if (CheckSubModeNotificationText(notificationText, continuousTaskRecord) != ERR_OK) {
        return ERR_BGTASK_NOTIFICATION_VERIFY_FAILED;
    }
    if (notificationText.empty()) {
        if (continuousTaskRecord->GetNotificationId() != -1) {
            NotificationTools::GetInstance()->CancelNotification(
                continuousTaskRecord->GetNotificationLabel(), continuousTaskRecord->GetNotificationId());
            continuousTaskRecord->notificationId_ = -1;
        }
        return ERR_OK;
    }
    BGTASK_LOGD("notificationText %{public}s", notificationText.c_str());
    return NotificationTools::GetInstance()->PublishNotification(continuousTaskRecord,
        appName, notificationText, bgTaskUid_);
}

ErrCode BgContinuousTaskMgr::CheckSubModeNotificationText(std::string &notificationText,
    const std::shared_ptr<ContinuousTaskRecord> record)
{
    if (record->bgSubModeIds_.size() > 0) {
        if (continuousTaskSubText_.empty()) {
            BGTASK_LOGE("get subMode notification prompt info failed, continuousTaskSubText_ is empty");
            return ERR_BGTASK_NOTIFICATION_VERIFY_FAILED;
        }
        notificationText = "";
        for (auto subMode : record->bgSubModeIds_) {
            BGTASK_LOGD("subMode %{public}d", subMode);
            uint32_t index = subMode - 1;
            if (index < continuousTaskSubText_.size()) {
                notificationText += continuousTaskSubText_.at(index);
                notificationText += "\n";
            } else {
                BGTASK_LOGI("sub index is invalid");
                return ERR_BGTASK_NOTIFICATION_VERIFY_FAILED;
            }
        }
    }
    return ERR_OK;
}

ErrCode BgContinuousTaskMgr::StopBackgroundRunningForInner(const sptr<ContinuousTaskParamForInner> &taskParam)
{
    ErrCode result = ERR_OK;
    int32_t uid = taskParam->uid_;
    int32_t abilityId = taskParam->abilityId_;
    std::string abilityName = "Webview" + std::to_string(taskParam->bgModeId_);

    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Service::StopBackgroundRunningInner");
    handler_->PostSyncTask([this, uid, abilityName, abilityId, &result]() {
        result = this->StopBackgroundRunningInner(uid, abilityName, abilityId);
        }, AppExecFwk::EventQueue::Priority::HIGH);

    return result;
}

ErrCode BgContinuousTaskMgr::StopBackgroundRunning(const std::string &abilityName, int32_t abilityId)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    if (abilityName.empty()) {
        BGTASK_LOGE("abilityName is empty!");
        return ERR_BGTASK_INVALID_PARAM;
    }
    if (abilityId < 0) {
        BGTASK_LOGE("abilityId is Invalid!");
    }
    int32_t callingUid = IPCSkeleton::GetCallingUid();

    ErrCode result = ERR_OK;

    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Service::StopBackgroundRunningInner");
    handler_->PostSyncTask([this, callingUid, abilityName, abilityId, &result]() {
        result = this->StopBackgroundRunningInner(callingUid, abilityName, abilityId);
        }, AppExecFwk::EventQueue::Priority::HIGH);

    return result;
}

ErrCode BgContinuousTaskMgr::StopBackgroundRunningInner(int32_t uid, const std::string &abilityName,
    int32_t abilityId)
{
    std::string mapKey = std::to_string(uid) + SEPARATOR + abilityName + SEPARATOR + std::to_string(abilityId);

    auto iter = continuousTaskInfosMap_.find(mapKey);
    if (iter == continuousTaskInfosMap_.end()) {
        BGTASK_LOGW("%{public}s continuous task not exists", mapKey.c_str());
        return ERR_BGTASK_OBJECT_NOT_EXIST;
    }
    BGTASK_LOGI("TASK STOP: user: %{public}s stop continuous task", mapKey.c_str());
    ErrCode result = ERR_OK;
    if (iter->second->GetNotificationId() != -1) {
        result = NotificationTools::GetInstance()->CancelNotification(
            iter->second->GetNotificationLabel(), iter->second->GetNotificationId());
    }
    RemoveContinuousTaskRecord(mapKey);
    return result;
}

void BgContinuousTaskMgr::StopContinuousTask(int32_t uid, int32_t pid, uint32_t taskType, const std::string &key)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }
    auto self = shared_from_this();
    auto task = [self, uid, pid, taskType, key]() {
        if (self) {
            self->HandleStopContinuousTask(uid, pid, taskType, key);
        }
    };
    handler_->PostTask(task);
}

void BgContinuousTaskMgr::HandleStopContinuousTask(int32_t uid, int32_t pid, uint32_t taskType, const std::string &key)
{
    BGTASK_LOGI("StopContinuousTask taskType: %{public}d, key %{public}s", taskType, key.c_str());
    if (taskType == BackgroundMode::DATA_TRANSFER) {
        RemoveContinuousTaskRecordByUidAndMode(uid, taskType);
        return;
    }
    if (taskType == ALL_MODES) {
        RemoveContinuousTaskRecordByUid(uid);
        return;
    }
    if (continuousTaskInfosMap_.find(key) == continuousTaskInfosMap_.end()) {
        BGTASK_LOGW("remove TaskInfo failure, no matched task: %{public}s", key.c_str());
        return;
    }
    NotificationTools::GetInstance()->CancelNotification(continuousTaskInfosMap_[key]->GetNotificationLabel(),
        continuousTaskInfosMap_[key]->GetNotificationId());
    SetReason(key, FREEZE_CANCEL);
    RemoveContinuousTaskRecord(key);
}

void BgContinuousTaskMgr::RemoveContinuousTaskRecordByUid(int32_t uid)
{
    auto iter = continuousTaskInfosMap_.begin();
    while (iter != continuousTaskInfosMap_.end()) {
        if (iter->second->GetUid() != uid) {
            ++iter;
            continue;
        }
        BGTASK_LOGW("erase key %{public}s", iter->first.c_str());
        iter->second->reason_ = FREEZE_CANCEL;
        OnContinuousTaskChanged(iter->second, ContinuousTaskEventTriggerType::TASK_CANCEL);
        NotificationTools::GetInstance()->CancelNotification(iter->second->GetNotificationLabel(),
            iter->second->GetNotificationId());
        iter = continuousTaskInfosMap_.erase(iter);
        RefreshTaskRecord();
    }
}

void BgContinuousTaskMgr::RemoveContinuousTaskRecordByUidAndMode(int32_t uid, uint32_t mode)
{
    auto iter = continuousTaskInfosMap_.begin();
    while (iter != continuousTaskInfosMap_.end()) {
        if (iter->second->GetUid() != uid) {
            ++iter;
            continue;
        }
        auto findModeIter = std::find(iter->second->bgModeIds_.begin(), iter->second->bgModeIds_.end(), mode);
        if (findModeIter == iter->second->bgModeIds_.end()) {
            ++iter;
            continue;
        }
        BGTASK_LOGW("erase key %{public}s", iter->first.c_str());
        iter->second->reason_ = FREEZE_CANCEL;
        OnContinuousTaskChanged(iter->second, ContinuousTaskEventTriggerType::TASK_CANCEL);
        NotificationTools::GetInstance()->CancelNotification(iter->second->GetNotificationLabel(),
            iter->second->GetNotificationId());
        iter = continuousTaskInfosMap_.erase(iter);
        RefreshTaskRecord();
    }
}

ErrCode BgContinuousTaskMgr::AddSubscriber(const std::shared_ptr<SubscriberInfo> subscriberInfo)
{
    if (subscriberInfo == nullptr || subscriberInfo->subscriber_ == nullptr ||
        subscriberInfo->subscriber_->AsObject() == nullptr) {
        BGTASK_LOGE("subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }

    handler_->PostSyncTask([=]() {
        AddSubscriberInner(subscriberInfo);
    });
    return ERR_OK;
}

ErrCode BgContinuousTaskMgr::AddSubscriberInner(const std::shared_ptr<SubscriberInfo> subscriberInfo)
{
    BGTASK_LOGD("BgContinuousTaskMgr enter");
    auto remoteObj = subscriberInfo->subscriber_->AsObject();
    auto findSuscriber = [&remoteObj](const auto& target) {
        return remoteObj == target->subscriber_->AsObject();
    };

    auto subscriberIter = find_if(bgTaskSubscribers_.begin(), bgTaskSubscribers_.end(), findSuscriber);
    if (subscriberIter != bgTaskSubscribers_.end()) {
        BGTASK_LOGW("target subscriber already exist");
        return ERR_BGTASK_OBJECT_EXISTS;
    }
    bgTaskSubscribers_.emplace_back(subscriberInfo);

    if (!susriberDeathRecipient_) {
        susriberDeathRecipient_ = new (std::nothrow) RemoteDeathRecipient(
            [this](const wptr<IRemoteObject> &remote) { this->OnRemoteSubscriberDied(remote); });
    }
    if (susriberDeathRecipient_) {
        remoteObj->AddDeathRecipient(susriberDeathRecipient_);
    }
    BGTASK_LOGI("continuous subscribers size %{public}d", static_cast<int32_t>(bgTaskSubscribers_.size()));
    return ERR_OK;
}

ErrCode BgContinuousTaskMgr::RemoveSubscriber(const sptr<IBackgroundTaskSubscriber> &subscriber)
{
    if (subscriber == nullptr) {
        BGTASK_LOGE("subscriber is null.");
        return ERR_BGTASK_INVALID_PARAM;
    }

    handler_->PostSyncTask([=]() {
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
    auto findSubscriber = [&remote](const auto &info) {
        return remote == info->subscriber_->AsObject();
    };

    auto subscriberIter = find_if(bgTaskSubscribers_.begin(), bgTaskSubscribers_.end(), findSubscriber);
    if (subscriberIter == bgTaskSubscribers_.end()) {
        BGTASK_LOGE("subscriber to remove is not exists.");
        return ERR_BGTASK_INVALID_PARAM;
    }
    if (susriberDeathRecipient_) {
        remote->RemoveDeathRecipient(susriberDeathRecipient_);
    }
    bgTaskSubscribers_.erase(subscriberIter);
    BGTASK_LOGI("Remove continuous task subscriber succeed");
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
            record.second->pid_, record.second->abilityName_, record.second->isFromWebview_, record.second->isBatchApi_,
            record.second->bgModeIds_, record.second->abilityId_, record.second->fullTokenId_);
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
    for (iter = continuousTaskInfosMap_.begin(); iter != continuousTaskInfosMap_.end(); ++iter) {
        stream.str("");
        stream.clear();
        stream << "No." << index;
        stream << "\tcontinuousTaskKey: " << iter->first << "\n";
        stream << "\tcontinuousTaskValue:" << "\n";
        stream << "\t\tbundleName: " << iter->second->GetBundleName() << "\n";
        stream << "\t\tabilityName: " << iter->second->GetAbilityName() << "\n";
        stream << "\t\tisFromWebview: " << (iter->second->IsFromWebview() ? "true" : "false") << "\n";
        stream << "\t\tisFromNewApi: " << (iter->second->IsNewApi() ? "true" : "false") << "\n";
        stream << "\t\tbackgroundMode: " << g_continuousTaskModeName[GetBgModeNameIndex(
            iter->second->GetBgModeId(), iter->second->IsNewApi())] << "\n";
        stream << "\t\tisBatchApi: " << (iter->second->isBatchApi_ ? "true" : "false") << "\n";
        stream << "\t\tbackgroundModes: " << iter->second->ToString(iter->second->bgModeIds_) << "\n";
        stream << "\t\tbackgroundSubModes: " << iter->second->ToString(iter->second->bgSubModeIds_) << "\n";
        stream << "\t\tuid: " << iter->second->GetUid() << "\n";
        stream << "\t\tuserId: " << iter->second->GetUserId() << "\n";
        stream << "\t\tpid: " << iter->second->GetPid() << "\n";
        stream << "\t\tnotificationLabel: " << iter->second->GetNotificationLabel() << "\n";
        stream << "\t\tnotificationId: " << iter->second->GetNotificationId() << "\n";
        stream << "\t\tcontinuousTaskId: " << iter->second->continuousTaskId_ << "\n";
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
                item.second->GetNotificationId());
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
            iter->second->GetNotificationId());
        RemoveContinuousTaskRecord(taskKey);
    }
}

void BgContinuousTaskMgr::SetReason(const std::string &mapKey, int32_t reason)
{
    auto iter = continuousTaskInfosMap_.find(mapKey);
    if (iter == continuousTaskInfosMap_.end()) {
        BGTASK_LOGW("SetReason failure, no matched task: %{public}s", mapKey.c_str());
    } else {
        auto record = iter->second;
        record->reason_ = reason;
    }
}

bool BgContinuousTaskMgr::RemoveContinuousTaskRecord(const std::string &mapKey)
{
    if (continuousTaskInfosMap_.find(mapKey) == continuousTaskInfosMap_.end()) {
        BGTASK_LOGW("remove TaskInfo failure, no matched task: %{public}s", mapKey.c_str());
        return false;
    }
    BGTASK_LOGI("erase task info: %{public}s", mapKey.c_str());
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
        return false;
    }
    bool result = true;
    SetReason(mapKey, REMOVE_NOTIFICATION_CANCEL);
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
        if ((*iter)->subscriber_->AsObject() == objectProxy) {
            BGTASK_LOGI("OnRemoteSubscriberDiedInner erase it");
            iter = bgTaskSubscribers_.erase(iter);
        } else {
            iter++;
        }
    }
    BGTASK_LOGI("continuous subscriber die, list size is %{public}d", static_cast<int>(bgTaskSubscribers_.size()));
}

void BgContinuousTaskMgr::OnAbilityStateChanged(int32_t uid, const std::string &abilityName, int32_t abilityId)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }
    auto iter = continuousTaskInfosMap_.begin();
    while (iter != continuousTaskInfosMap_.end()) {
        if (iter->second->uid_ == uid && iter->second->abilityName_ == abilityName &&
            iter->second->abilityId_ == abilityId) {
            auto record = iter->second;
            BGTASK_LOGI("OnAbilityStateChanged uid: %{public}d, bundleName: %{public}s abilityName: %{public}s"
                "bgModeId: %{public}d, abilityId: %{public}d", uid, record->bundleName_.c_str(),
                record->abilityName_.c_str(), record->bgModeId_, record->abilityId_);
            record->reason_ = SYSTEM_CANCEL;
            OnContinuousTaskChanged(record, ContinuousTaskEventTriggerType::TASK_CANCEL);
            if (!iter->second->isFromWebview_) {
                NotificationTools::GetInstance()->CancelNotification(
                    record->GetNotificationLabel(), record->GetNotificationId());
            }
            iter = continuousTaskInfosMap_.erase(iter);
            HandleAppContinuousTaskStop(record->uid_);
            RefreshTaskRecord();
        } else {
            iter++;
        }
    }
}

void BgContinuousTaskMgr::OnAppStopped(int32_t uid)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }
    auto iter = continuousTaskInfosMap_.begin();
    while (iter != continuousTaskInfosMap_.end()) {
        if (iter->second->uid_ == uid) {
            auto record = iter->second;
            BGTASK_LOGI("OnAppStopped uid: %{public}d, bundleName: %{public}s abilityName: %{public}s"
                "bgModeId: %{public}d, abilityId: %{public}d", uid, record->bundleName_.c_str(),
                record->abilityName_.c_str(), record->bgModeId_, record->abilityId_);
            record->reason_ = SYSTEM_CANCEL;
            OnContinuousTaskChanged(record, ContinuousTaskEventTriggerType::TASK_CANCEL);
            if (record->GetNotificationId() != -1) {
                NotificationTools::GetInstance()->CancelNotification(
                    record->GetNotificationLabel(), record->GetNotificationId());
            }
            iter = continuousTaskInfosMap_.erase(iter);
            HandleAppContinuousTaskStop(record->uid_);
            RefreshTaskRecord();
        } else {
            iter++;
        }
    }
}

uint32_t BgContinuousTaskMgr::GetModeNumByTypeIds(const std::vector<uint32_t> &typeIds)
{
    uint32_t modeNum = 0;
    for (auto mode : typeIds) {
        modeNum |= (1 << (mode - 1));
    }
    return modeNum;
}

bool BgContinuousTaskMgr::CanNotifyHap(const std::shared_ptr<SubscriberInfo> subscriberInfo,
    const std::shared_ptr<ContinuousTaskCallbackInfo> &callbackInfo)
{
    if (subscriberInfo->isHap_ && subscriberInfo->uid_ == callbackInfo->GetCreatorUid() &&
        (callbackInfo->GetCancelReason() == REMOVE_NOTIFICATION_CANCEL ||
        callbackInfo->GetCancelReason() == FREEZE_CANCEL)) {
        return true;
    }
    return false;
}

void BgContinuousTaskMgr::NotifySubscribers(ContinuousTaskEventTriggerType changeEventType,
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    if (continuousTaskCallbackInfo == nullptr) {
        BGTASK_LOGD("continuousTaskCallbackInfo is null");
        return;
    }
    const ContinuousTaskCallbackInfo& taskCallbackInfoRef = *continuousTaskCallbackInfo;
    switch (changeEventType) {
        case ContinuousTaskEventTriggerType::TASK_START:
            for (auto iter = bgTaskSubscribers_.begin(); iter != bgTaskSubscribers_.end(); ++iter) {
                BGTASK_LOGD("continuous task start callback trigger");
                if (!(*iter)->isHap_ && (*iter)->subscriber_) {
                    (*iter)->subscriber_->OnContinuousTaskStart(taskCallbackInfoRef);
                }
            }
            break;
        case ContinuousTaskEventTriggerType::TASK_UPDATE:
            for (auto iter = bgTaskSubscribers_.begin(); iter != bgTaskSubscribers_.end(); ++iter) {
                BGTASK_LOGD("continuous task update callback trigger");
                if (!(*iter)->isHap_ && (*iter)->subscriber_) {
                    (*iter)->subscriber_->OnContinuousTaskUpdate(taskCallbackInfoRef);
                }
            }
            break;
        case ContinuousTaskEventTriggerType::TASK_CANCEL:
            for (auto iter = bgTaskSubscribers_.begin(); iter != bgTaskSubscribers_.end(); ++iter) {
                BGTASK_LOGD("continuous task stop callback trigger");
                if (!(*iter)->isHap_ && (*iter)->subscriber_) {
                    // notify all sa
                    (*iter)->subscriber_->OnContinuousTaskStop(taskCallbackInfoRef);
                } else if (CanNotifyHap(*iter, continuousTaskCallbackInfo) && (*iter)->subscriber_) {
                    // notify self hap
                    BGTASK_LOGI("uid %{public}d is hap and uid is same, need notify cancel", (*iter)->uid_);
                    (*iter)->subscriber_->OnContinuousTaskStop(taskCallbackInfoRef);
                }
            }
            break;
    }
}

void BgContinuousTaskMgr::ReportHisysEvent(ContinuousTaskEventTriggerType changeEventType,
    const std::shared_ptr<ContinuousTaskRecord> &continuousTaskInfo)
{
    switch (changeEventType) {
        case ContinuousTaskEventTriggerType::TASK_START:
        case ContinuousTaskEventTriggerType::TASK_UPDATE:
            HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::BACKGROUND_TASK, "CONTINUOUS_TASK_APPLY",
                HiviewDFX::HiSysEvent::EventType::STATISTIC, "APP_UID", continuousTaskInfo->GetUid(),
                "APP_PID", continuousTaskInfo->GetPid(), "APP_NAME", continuousTaskInfo->GetBundleName(),
                "ABILITY", continuousTaskInfo->GetAbilityName(),
                "BGMODE", GetModeNumByTypeIds(continuousTaskInfo->bgModeIds_),
                "UIABILITY_IDENTITY", continuousTaskInfo->GetAbilityId());
            break;
        case ContinuousTaskEventTriggerType::TASK_CANCEL:
            HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::BACKGROUND_TASK, "CONTINUOUS_TASK_CANCEL",
                HiviewDFX::HiSysEvent::EventType::STATISTIC, "APP_UID", continuousTaskInfo->GetUid(),
                "APP_PID", continuousTaskInfo->GetPid(), "APP_NAME", continuousTaskInfo->GetBundleName(),
                "ABILITY", continuousTaskInfo->GetAbilityName(),
                "BGMODE", GetModeNumByTypeIds(continuousTaskInfo->bgModeIds_),
                "UIABILITY_IDENTITY", continuousTaskInfo->GetAbilityId(), "STOP_REASON", continuousTaskInfo->reason_);
            break;
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
        continuousTaskInfo->GetUid(), continuousTaskInfo->GetPid(), continuousTaskInfo->GetAbilityName(),
        continuousTaskInfo->IsFromWebview(), continuousTaskInfo->isBatchApi_, continuousTaskInfo->bgModeIds_,
        continuousTaskInfo->abilityId_, continuousTaskInfo->fullTokenId_);
    BGTASK_LOGD("mode %{public}d isBatch %{public}d modes size %{public}u",
        continuousTaskCallbackInfo->GetTypeId(), continuousTaskCallbackInfo->IsBatchApi(),
        static_cast<uint32_t>(continuousTaskCallbackInfo->GetTypeIds().size()));
    continuousTaskCallbackInfo->SetContinuousTaskId(continuousTaskInfo->continuousTaskId_);
    continuousTaskCallbackInfo->SetCancelReason(continuousTaskInfo->reason_);
    NotifySubscribers(changeEventType, continuousTaskCallbackInfo);
    ReportHisysEvent(changeEventType, continuousTaskInfo);
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
                record->reason_ = SYSTEM_CANCEL;
                OnContinuousTaskChanged(record, ContinuousTaskEventTriggerType::TASK_CANCEL);
                NotificationTools::GetInstance()->CancelNotification(
                    record->GetNotificationLabel(), record->GetNotificationId());
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
            record->reason_ = SYSTEM_CANCEL;
            OnContinuousTaskChanged(record, ContinuousTaskEventTriggerType::TASK_CANCEL);
            NotificationTools::GetInstance()->CancelNotification(
                record->GetNotificationLabel(), record->GetNotificationId());
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
        if (!(*iter)->isHap_ && (*iter)->subscriber_) {
            (*iter)->subscriber_->OnAppContinuousTaskStop(uid);
        }
    }
}

int32_t BgContinuousTaskMgr::RefreshTaskRecord()
{
    int32_t ret = DelayedSingleton<DataStorageHelper>::GetInstance()->RefreshTaskRecord(continuousTaskInfosMap_);
    if (ret != ERR_OK) {
        BGTASK_LOGE("refresh data failed");
        return ret;
    }
    return ERR_OK;
}

std::string BgContinuousTaskMgr::GetMainAbilityLabel(const std::string &bundleName, int32_t userId)
{
    AppExecFwk::BundleInfo bundleInfo;
    if (!BundleManagerHelper::GetInstance()->GetBundleInfo(bundleName,
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, userId)) {
        BGTASK_LOGE("Get %{public}s bundle info failed", bundleName.c_str());
        return "";
    }
    auto resourceManager = GetBundleResMgr(bundleInfo);
    if (resourceManager == nullptr) {
        BGTASK_LOGE("Get %{public}s resource manager failed", bundleName.c_str());
        return "";
    }

    AppExecFwk::ApplicationInfo applicationInfo;
    if (!BundleManagerHelper::GetInstance()->GetApplicationInfo(bundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, applicationInfo)) {
        BGTASK_LOGE("failed to get applicationInfo from AppExecFwk, bundleName is %{public}s", bundleName.c_str());
        return "";
    }

    std::string mainAbilityLabel {""};
    resourceManager->GetStringById(static_cast<uint32_t>(applicationInfo.labelId), mainAbilityLabel);
    BGTASK_LOGI("Get main ability label: %{public}s by labelId: %{public}d", mainAbilityLabel.c_str(),
        applicationInfo.labelId);
    mainAbilityLabel = mainAbilityLabel.empty() ? applicationInfo.label : mainAbilityLabel;
    return mainAbilityLabel;
}

void BgContinuousTaskMgr::OnConfigurationChanged(const AppExecFwk::Configuration &configuration)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }
    std::string languageChange = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    if (languageChange.empty()) {
        return;
    }
    BGTASK_LOGI("System language config has changed");
    GetNotificationPrompt();
    cachedBundleInfos_.clear();
    std::map<std::string, std::pair<std::string, std::string>> newPromptInfos;
    auto iter = continuousTaskInfosMap_.begin();
    while (iter != continuousTaskInfosMap_.end()) {
        auto record = iter->second;
        if (!CommonUtils::CheckExistMode(record->bgModeIds_, BackgroundMode::DATA_TRANSFER)) {
            std::string mainAbilityLabel = GetMainAbilityLabel(record->bundleName_, record->userId_);
            std::string notificationText = GetNotificationText(record);
            newPromptInfos.emplace(record->notificationLabel_, std::make_pair(mainAbilityLabel, notificationText));
        }
        iter++;
    }
    NotificationTools::GetInstance()->RefreshContinuousNotifications(newPromptInfos, bgTaskUid_);
}

std::string BgContinuousTaskMgr::GetNotificationText(const std::shared_ptr<ContinuousTaskRecord> record)
{
    std::string notificationText {""};
    for (auto mode : record->bgModeIds_) {
        if (mode == BackgroundMode::AUDIO_PLAYBACK || ((mode == BackgroundMode::VOIP ||
            mode == BackgroundMode::AUDIO_RECORDING) && record->IsSystem())) {
            continue;
        }
        uint32_t index = GetBgModeNameIndex(mode, record->isNewApi_);
        if (index < continuousTaskText_.size()) {
            notificationText += continuousTaskText_.at(index);
            notificationText += "\n";
        }
    }
    if (record->bgSubModeIds_.size() > 0) {
        notificationText = "";
        for (auto subMode : record->bgSubModeIds_) {
            BGTASK_LOGD("subMode %{public}d", subMode);
            uint32_t index = subMode - 1;
            if (index < continuousTaskSubText_.size()) {
                notificationText += continuousTaskSubText_.at(index);
                notificationText += "\n";
            }
        }
    }
    return notificationText;
}

void BgContinuousTaskMgr::HandleRemoveTaskByMode(uint32_t mode)
{
    auto iter = continuousTaskInfosMap_.begin();
    while (iter != continuousTaskInfosMap_.end()) {
        auto record = iter->second;
        if (record->isFromWebview_ && CommonUtils::CheckExistMode(record->bgModeIds_, mode)) {
            BGTASK_LOGI("HandleVoipTaskRemove uid: %{public}d, bundleName: %{public}s, abilityName: %{public}s,"
                " bgModeId: %{public}d, abilityId: %{public}d", record->uid_, record->bundleName_.c_str(),
                record->abilityName_.c_str(), mode, record->abilityId_);
            record->reason_ = SYSTEM_CANCEL;
            OnContinuousTaskChanged(record, ContinuousTaskEventTriggerType::TASK_CANCEL);
            iter = continuousTaskInfosMap_.erase(iter);
            HandleAppContinuousTaskStop(record->uid_);
            RefreshTaskRecord();
        } else {
            iter++;
        }
    }
}

void BgContinuousTaskMgr::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }
    switch (systemAbilityId) {
        case SA_ID_VOIP_CALL_MANAGER:
            {
                BGTASK_LOGI("remove voip system ability, systemAbilityId: %{public}d", systemAbilityId);
                auto task = [this]() { this->HandleRemoveTaskByMode(BackgroundMode::VOIP); };
                handler_->PostTask(task);
            }
            break;
        case SA_ID_HEALTH_SPORT:
            {
                BGTASK_LOGI("remove healthsport system ability, systemAbilityId: %{public}d", systemAbilityId);
                auto task = [this]() { this->HandleRemoveTaskByMode(BackgroundMode::WORKOUT); };
                handler_->PostTask(task);
            }
            break;
        default:
            break;
    }
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
