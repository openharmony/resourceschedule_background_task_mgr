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
#include "running_process_info.h"
#include "string_wrapper.h"
#include "system_ability_definition.h"

#include "bgtask_common.h"
#include "bgtask_config.h"
#include "bgtask_hitrace_chain.h"
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
#include "continuous_task_suspend_reason.h"
#include "bg_continuous_task_dumper.h"

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
static constexpr char DUMP_PARAM_GET[] = "--get";
static constexpr char DUMP_INNER_TASK[] = "--inner_task";
static constexpr char BGMODE_PERMISSION[] = "ohos.permission.KEEP_BACKGROUND_RUNNING";
static constexpr char BGMODE_PERMISSION_SYSTEM[] = "ohos.permission.KEEP_BACKGROUND_RUNNING_SYSTEM";
static constexpr char BG_TASK_RES_BUNDLE_NAME[] = "com.ohos.backgroundtaskmgr.resources";
static constexpr char BG_TASK_SUB_MODE_TYPE[] = "subMode";
static constexpr uint32_t SYSTEM_APP_BGMODE_WIFI_INTERACTION = 64;
static constexpr uint32_t PC_BGMODE_TASK_KEEPING = 256;
static constexpr int32_t DELAY_TIME = 2000;
static constexpr int32_t RECLAIM_MEMORY_DELAY_TIME = 20 * 60 * 1000;
static constexpr int32_t MAX_DUMP_PARAM_NUMS = 3;
static constexpr int32_t ILLEGAL_NOTIFICATION_ID = -2;
static constexpr uint32_t INVALID_BGMODE = 0;
static constexpr uint32_t BG_MODE_INDEX_HEAD = 1;
static constexpr uint32_t BGMODE_NUMS = 10;
static constexpr uint32_t VOIP_SA_UID = 7022;
static constexpr uint32_t AVSESSION_SA_UID = 6700;
static constexpr uint32_t CONTINUOUS_TASK_SUSPEND = 2;
#ifdef FEATURE_PRODUCT_WATCH
static constexpr uint32_t HEALTHSPORT_SA_UID = 7500;
#else
static constexpr uint32_t HEALTHSPORT_SA_UID = 7259;
#endif
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
    FILE *file = fopen(path.c_str(), "w");
    if (file == nullptr) {
        BGTASK_LOGE("Fail to open file: %{private}s, errno: %{public}s", path.c_str(), strerror(errno));
        return;
    }
    size_t res = fwrite(contentStr.c_str(), 1, contentStr.length(), file);
    if (res != contentStr.length()) {
        BGTASK_LOGE("Fail to write file: %{private}s, errno: %{public}s", path.c_str(), strerror(errno));
    }
    int closeResult = fclose(file);
    if (closeResult < 0) {
        BGTASK_LOGE("Fail to close file: %{private}s, errno: %{public}s", path.c_str(), strerror(errno));
    }
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

    if (!RegisterNotificationSubscriber()) {
        return;
    }
    if (!RegisterAppStateObserver()) {
        return;
    }
    if (!RegisterSysCommEventListener()) {
        return;
    }
    if (!RegisterConfigurationObserver()) {
        return;
    }
    InitRequiredResourceInfo();
}

void BgContinuousTaskMgr::HandlePersistenceData()
{
    BGTASK_LOGI("service restart, restore data");
    DelayedSingleton<DataStorageHelper>::GetInstance()->RestoreTaskRecord(continuousTaskInfosMap_);
    auto appMgrClient = std::make_shared<AppExecFwk::AppMgrClient>();
    std::vector<AppExecFwk::RunningProcessInfo> allAppProcessInfos;
    if (appMgrClient->ConnectAppMgrService() != ERR_OK) {
        BGTASK_LOGW("connect to app mgr service failed");
        return;
    }
    appMgrClient->GetAllRunningProcesses(allAppProcessInfos);
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
    if (appMgrClient->ConnectAppMgrService() != ERR_OK) {
        BGTASK_LOGW("connect to app mgr service failed");
        return false;
    }
    configChangeObserver_ = sptr<AppExecFwk::IConfigurationObserver>(
        new (std::nothrow) ConfigChangeObserver(handler_, shared_from_this()));
    if (appMgrClient->RegisterConfigurationObserver(configChangeObserver_) != ERR_OK) {
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
    if (resConfig != nullptr) {
        resConfig->SetLocaleInfo(locale);
    }
#endif // SUPPORT_GRAPHICS
    resourceManager->UpdateResConfig(*resConfig);
    return resourceManager;
}

bool BgContinuousTaskMgr::GetNotificationPrompt()
{
    BgTaskHiTraceChain traceChain(__func__);
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
    BgTaskHiTraceChain traceChain(__func__);
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
    bool isNewApi, const std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord)
{
    BgTaskHiTraceChain traceChain(__func__);
    if (!isNewApi) {
        if (configuredBgMode == INVALID_BGMODE) {
            BGTASK_LOGE("ability without background mode config");
            return ERR_BGMODE_NULL_OR_TYPE_ERR;
        } else {
            return ERR_OK;
        }
    } else {
        uint32_t recordedBgMode = BG_MODE_INDEX_HEAD << (requestedBgModeId - 1);
        if (recordedBgMode == SYSTEM_APP_BGMODE_WIFI_INTERACTION && !continuousTaskRecord->IsSystem()) {
            BGTASK_LOGE("wifiInteraction background mode only support for system app");
            return ERR_BGTASK_NOT_SYSTEM_APP;
        }
        if (recordedBgMode == PC_BGMODE_TASK_KEEPING && !AllowUseTaskKeeping(continuousTaskRecord)) {
            BGTASK_LOGE("task keeping is not supported, please set param persist.sys.bgtask_support_task_keeping.");
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

bool BgContinuousTaskMgr::AllowUseTaskKeeping(const std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord)
{
    if (SUPPORT_TASK_KEEPING) {
        return true;
    }
    std::string bundleName = continuousTaskRecord->GetBundleName();
    if (DelayedSingleton<BgtaskConfig>::GetInstance()->IsTaskKeepingExemptedQuatoApp(bundleName)) {
        return true;
    }
    uint64_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (BundleManagerHelper::GetInstance()->CheckACLPermission(BGMODE_PERMISSION_SYSTEM, callingTokenId) &&
        !BundleManagerHelper::GetInstance()->IsSystemApp(fullTokenId)) {
        return true;
    }
    return false;
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
        if (taskParam->isByRequestObject_ && (taskParam->bgModeIds_.empty() || taskParam->bgSubModeIds_.empty())) {
            BGTASK_LOGE("request param modes or submodes is empty");
            return false;
        }
        if (taskParam->isBatchApi_ && taskParam->bgModeIds_.empty()) {
            BGTASK_LOGE("bgModeIds_ is empty");
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

ErrCode BgContinuousTaskMgr::RequestGetContinuousTasksByUidForInner(int32_t uid,
    std::vector<std::shared_ptr<ContinuousTaskInfo>> &list)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    ErrCode result = ERR_OK;
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Service::RequestGetContinuousTasksByUidForInner");
    handler_->PostSyncTask([this, uid, &list, &result]() {
        result = this->GetAllContinuousTasksInner(uid, list);
        }, AppExecFwk::EventQueue::Priority::HIGH);
    return result;
}

ErrCode BgContinuousTaskMgr::StartBackgroundRunningForInner(const sptr<ContinuousTaskParamForInner> &taskParam)
{
    BgTaskHiTraceChain traceChain(__func__);
    ErrCode result = ERR_OK;
    int32_t uid = taskParam->uid_;
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    if (taskParam->GetPid() != 0) {
        callingPid = taskParam->GetPid();
    }
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
    BgTaskHiTraceChain traceChain(__func__);
    if (!isSysReady_.load()) {
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
    continuousTaskRecord->bgSubModeIds_ = taskParam->bgSubModeIds_;
    continuousTaskRecord->isCombinedTaskNotification_ = taskParam->isCombinedTaskNotification_;
    continuousTaskRecord->combinedNotificationTaskId_ = taskParam->combinedNotificationTaskId_;
    continuousTaskRecord->isByRequestObject_ = taskParam->isByRequestObject_;
}

ErrCode BgContinuousTaskMgr::CheckSubMode(const std::shared_ptr<AAFwk::Want> want,
    std::shared_ptr<ContinuousTaskRecord> record)
{
    if (want->HasParameter(BG_TASK_SUB_MODE_TYPE)) {
        if (CommonUtils::CheckExistMode(record->bgModeIds_, BackgroundMode::BLUETOOTH_INTERACTION) &&
            want->GetIntParam(BG_TASK_SUB_MODE_TYPE, 0) == BackgroundSubMode::CAR_KEY && !record->isByRequestObject_) {
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
    BgTaskHiTraceChain traceChain(__func__);
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    ErrCode result = CheckIsSysReadyAndPermission(callingUid);
    if (result != ERR_OK) {
        return result;
    }

    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Service::UpdateBackgroundRunningInner");
    if (taskParam->isByRequestObject_) {
        // 根据任务id更新
        handler_->PostSyncTask([this, callingUid, taskParam, &result]() {
            result = this->UpdateBackgroundRunningByTaskIdInner(callingUid, taskParam);
            }, AppExecFwk::EventQueue::Priority::HIGH);
    } else {
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
    }
    return result;
}

ErrCode BgContinuousTaskMgr::UpdateTaskInfo(std::shared_ptr<ContinuousTaskRecord> record,
    const sptr<ContinuousTaskParam> &taskParam)
{
    ErrCode ret = UpdateTaskNotification(record, taskParam);
    if (ret != ERR_OK) {
        return ret;
    }
    record->isCombinedTaskNotification_ = taskParam->isCombinedTaskNotification_;
    if (record->suspendState_) {
        std::string taskInfoMapKey = std::to_string(record->uid_) + SEPARATOR + record->abilityName_ + SEPARATOR +
            std::to_string(record->abilityId_) + SEPARATOR + CommonUtils::ModesToString(record->bgModeIds_);
        HandleActiveContinuousTask(record->uid_, record->pid_, taskInfoMapKey);
    }
    OnContinuousTaskChanged(record, ContinuousTaskEventTriggerType::TASK_UPDATE);
    taskParam->notificationId_ = record->GetNotificationId();
    taskParam->continuousTaskId_ = record->GetContinuousTaskId();
    return RefreshTaskRecord();
}

ErrCode BgContinuousTaskMgr::UpdateTaskNotification(std::shared_ptr<ContinuousTaskRecord> record,
    const sptr<ContinuousTaskParam> &taskParam)
{
    auto oldModes = record->bgModeIds_;
    if (CommonUtils::CheckExistMode(oldModes, BackgroundMode::DATA_TRANSFER) &&
        CommonUtils::CheckExistMode(taskParam->bgModeIds_, BackgroundMode::DATA_TRANSFER)) {
        BGTASK_LOGI("uid: %{public}d have same mode: DATA_TRANSFER", record->uid_);
        return ERR_OK;
    }
    std::string mainAbilityLabel = GetMainAbilityLabel(record->bundleName_, record->userId_);
    if (mainAbilityLabel == "") {
        BGTASK_LOGE("uid: %{public}d get main ability label or notification text fail.", record->uid_);
        return ERR_BGTASK_NOTIFICATION_VERIFY_FAILED;
    }
    if (!record->isCombinedTaskNotification_) {
        record->bgModeId_ = taskParam->bgModeId_;
        record->bgModeIds_ = taskParam->bgModeIds_;
        record->bgSubModeIds_ = taskParam->bgSubModeIds_;
    }
    record->wantAgent_ = taskParam->wantAgent_;
    if (record->wantAgent_ != nullptr && record->wantAgent_->GetPendingWant() != nullptr) {
        auto target = record->wantAgent_->GetPendingWant()->GetTarget();
        auto want = record->wantAgent_->GetPendingWant()->GetWant(target);
        if (want != nullptr) {
            std::shared_ptr<WantAgentInfo> info = std::make_shared<WantAgentInfo>();
            info->bundleName_ = want->GetOperation().GetBundleName();
            info->abilityName_ = want->GetOperation().GetAbilityName();
            record->wantAgentInfo_ = info;
        }
    }
    std::map<std::string, std::pair<std::string, std::string>> newPromptInfos;
    if (record->isCombinedTaskNotification_) {
        if (CommonUtils::CheckModesSame(oldModes, taskParam->bgModeIds_)) {
            newPromptInfos.emplace(record->notificationLabel_, std::make_pair(mainAbilityLabel, ""));
            return NotificationTools::GetInstance()->RefreshContinuousNotificationWantAndContext(bgTaskUid_,
                newPromptInfos, record);
        } else {
            return ERR_BGTASK_CONTINUOUS_UPDATE_FAIL_SAME_MODE_AND_MERGED;
        }
    } else {
        std::string notificationText = GetNotificationText(record);
        if (notificationText.empty()) {
            BGTASK_LOGE("notificationText is empty, uid: %{public}d", record->uid_);
        } else {
            newPromptInfos.emplace(record->notificationLabel_, std::make_pair(mainAbilityLabel, notificationText));
            return NotificationTools::GetInstance()->RefreshContinuousNotificationWantAndContext(bgTaskUid_,
                newPromptInfos, record, true);
        }
    }
    return ERR_OK;
}

ErrCode BgContinuousTaskMgr::UpdateBackgroundRunningByTaskIdInner(int32_t uid,
    const sptr<ContinuousTaskParam> &taskParam)
{
    int32_t continuousTaskId = taskParam->updateTaskId_;
    if (continuousTaskId < 0) {
        BGTASK_LOGE("update task fail, taskId: %{public}d", taskParam->updateTaskId_);
        return ERR_BGTASK_CONTINUOUS_TASKID_INVALID;
    }
    auto findTask = [continuousTaskId](const auto &target) {
        return continuousTaskId == target.second->continuousTaskId_ && target.second->isByRequestObject_;
    };
    auto findTaskIter = find_if(continuousTaskInfosMap_.begin(), continuousTaskInfosMap_.end(), findTask);
    if (findTaskIter == continuousTaskInfosMap_.end()) {
        BGTASK_LOGE("uid: %{public}d not have task, taskId: %{public}d", uid, continuousTaskId);
        return ERR_BGTASK_OBJECT_NOT_EXIST;
    }
    auto record = findTaskIter->second;
    uint32_t configuredBgMode = GetBackgroundModeInfo(uid, record->abilityName_);
    ErrCode ret = ERR_OK;
    for (auto it = taskParam->bgModeIds_.begin(); it != taskParam->bgModeIds_.end(); it++) {
        ret = CheckBgmodeType(configuredBgMode, *it, true, record);
        if (ret != ERR_OK) {
            BGTASK_LOGE("CheckBgmodeType error, mode: %{public}u, apply mode: %{public}u.", configuredBgMode, *it);
            return ret;
        }
    }
    return UpdateTaskInfo(record, taskParam);
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
        return ERR_OK;
    }
    if (!CommonUtils::CheckExistMode(taskParam->bgModeIds_, BackgroundMode::BLUETOOTH_INTERACTION) &&
        !continuousTaskRecord->bgSubModeIds_.empty()) {
        continuousTaskRecord->bgSubModeIds_.clear();
    }
    uint32_t configuredBgMode = GetBackgroundModeInfo(continuousTaskRecord->uid_, continuousTaskRecord->abilityName_);
    for (auto it =  taskParam->bgModeIds_.begin(); it != taskParam->bgModeIds_.end(); it++) {
        ret = CheckBgmodeType(configuredBgMode, *it, true, continuousTaskRecord);
        if (ret != ERR_OK) {
            BGTASK_LOGE("CheckBgmodeType error, mode: %{public}u, apply mode: %{public}u.", configuredBgMode, *it);
            return ret;
        }
    }
    continuousTaskRecord->bgModeId_ = taskParam->bgModeId_;
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
            return ret;
        }
    }
    if (continuousTaskInfosMap_[taskInfoMapKey]->suspendState_) {
        HandleActiveContinuousTask(continuousTaskRecord->uid_, continuousTaskRecord->pid_, taskInfoMapKey);
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
    if (continuousTaskRecord->isByRequestObject_) {
        taskInfoMapKey = taskInfoMapKey + SEPARATOR + CommonUtils::ModesToString(continuousTaskRecord->bgModeIds_);
    }
    if (continuousTaskInfosMap_.find(taskInfoMapKey) != continuousTaskInfosMap_.end()) {
        if (continuousTaskInfosMap_[taskInfoMapKey]->suspendState_) {
            HandleActiveContinuousTask(continuousTaskRecord->uid_, continuousTaskRecord->pid_, taskInfoMapKey);
            return ERR_OK;
        }
        BGTASK_LOGD("continuous task is already exist: %{public}s", taskInfoMapKey.c_str());
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
    return StartBackgroundRunningSubmit(continuousTaskRecord, taskInfoMapKey);
}

ErrCode BgContinuousTaskMgr::StartBackgroundRunningSubmit(std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord,
    std::string &taskInfoMapKey)
{
    ErrCode ret;
    if (continuousTaskRecord->isFromWebview_) {
        ret = CheckBgmodeTypeForInner(continuousTaskRecord->bgModeId_);
    } else {
        uint32_t configuredBgMode = GetBackgroundModeInfo(continuousTaskRecord->uid_,
            continuousTaskRecord->abilityName_);
        for (auto it = continuousTaskRecord->bgModeIds_.begin(); it != continuousTaskRecord->bgModeIds_.end(); it++) {
            ret = CheckBgmodeType(configuredBgMode, *it, continuousTaskRecord->isNewApi_, continuousTaskRecord);
            if (ret != ERR_OK) {
                BGTASK_LOGE("CheckBgmodeType invalid!");
                return ret;
            }
        }
    }
    bool sendNotification = true;
    ret = CheckCombinedTaskNotification(continuousTaskRecord, sendNotification);
    if (ret != ERR_OK) {
        return ret;
    }
    if (!continuousTaskRecord->isFromWebview_ && sendNotification) {
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

ErrCode BgContinuousTaskMgr::CheckCombinedTaskNotification(std::shared_ptr<ContinuousTaskRecord> &recordParam,
    bool &sendNotification)
{
    // 无需合并，新申请
    int32_t mergeNotificationTaskId = recordParam->combinedNotificationTaskId_;
    if (recordParam->isByRequestObject_) {
        if (mergeNotificationTaskId == -1) {
            sendNotification = true;
            return ERR_OK;
        } else if (!recordParam->isCombinedTaskNotification_) {
            BGTASK_LOGE("current task not support merge, uid: %{public}d", recordParam->uid_);
            return ERR_BGTASK_CONTINUOUS_NOT_MERGE_CURRENTTASK_COMBINED_FALSE;
        }
    } else {
        sendNotification = true;
        return ERR_OK;
    }
    return DetermineMatchCombinedTaskNotifacation(recordParam, sendNotification);
}

ErrCode BgContinuousTaskMgr::DetermineMatchCombinedTaskNotifacation(std::shared_ptr<ContinuousTaskRecord> recordParam,
    bool &sendNotification)
{
    int32_t mergeNotificationTaskId = recordParam->combinedNotificationTaskId_;
    ErrCode ret = ERR_BGTASK_CONTINUOUS_TASKID_INVALID;
    for (const auto &record : continuousTaskInfosMap_) {
        if (!record.second) {
            BGTASK_LOGE("current task is null");
            continue;
        }
        if (record.second->uid_ != recordParam->uid_) {
            BGTASK_LOGE("current task uid not equal, task uid: %{public}d, param uid: %{public}d", record.second->uid_,
                recordParam->uid_);
            continue;
        }
        if (record.second->GetContinuousTaskId() != mergeNotificationTaskId) {
            BGTASK_LOGE("task id is not equal, task id: %{public}d, param combined task id: %{public}d",
                record.second->GetContinuousTaskId(), mergeNotificationTaskId);
            continue;
        }
        if (!record.second->isCombinedTaskNotification_) {
            BGTASK_LOGE("continuous task not support merge, taskId: %{public}d", mergeNotificationTaskId);
            return ERR_BGTASK_CONTINUOUS_NOT_MERGE_COMBINED_FALSE;
        }
        if (record.second->GetNotificationId() == -1) {
            BGTASK_LOGE("continuous task notification not exist, taskId: %{public}d", mergeNotificationTaskId);
            return ERR_BGTASK_CONTINUOUS_NOT_MERGE_NOTIFICATION_NOT_EXIST;
        }
        if (!CommonUtils::CheckModesSame(record.second->bgModeIds_, recordParam->bgModeIds_)) {
            BGTASK_LOGE("continuous task modes mismatch, taskId: %{public}d", mergeNotificationTaskId);
            return ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_TYPE_MISMATCH;
        }
        if (!CommonUtils::CheckModesSame(record.second->bgSubModeIds_, recordParam->bgSubModeIds_)) {
            BGTASK_LOGE("continuous task submodes mismatch, taskId: %{public}d", mergeNotificationTaskId);
            return ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_TYPE_MISMATCH;
        }
        if (CommonUtils::CheckExistMode(recordParam->bgModeIds_, BackgroundMode::DATA_TRANSFER)) {
            BGTASK_LOGE("continuous task mode: DATA_TRANSFER not support merge, taskId: %{public}d",
                mergeNotificationTaskId);
            return ERR_BGTASK_CONTINUOUS_DATA_TRANSFER_NOT_MERGE_NOTIFICATION;
        } else {
            sendNotification = false;
            recordParam->notificationId_ = record.second->GetNotificationId();
            recordParam->notificationLabel_ = record.second->GetNotificationLabel();
            record.second->combinedNotificationTaskId_ = mergeNotificationTaskId;
            return ERR_OK;
        }
    }
    return ret;
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
    BgTaskHiTraceChain traceChain(__func__);
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
    ErrCode ret = CheckNotificationText(notificationText, continuousTaskRecord);
    if (ret != ERR_OK) {
        return ret;
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
    auto iter = avSessionNotification_.find(continuousTaskRecord->uid_);
    bool isPublish = (iter != avSessionNotification_.end()) ? iter->second : false;
    bool isPublishAvsession = isPublish || (CommonUtils::CheckExistMode(continuousTaskRecord->bgSubModeIds_,
        ContinuousTaskSubmode::SUBMODE_AVSESSION_AUDIO_PLAYBACK) && continuousTaskRecord->isByRequestObject_);
    if (continuousTaskRecord->bgModeIds_.size() == 1 &&
        continuousTaskRecord->bgModeIds_[0] == BackgroundMode::AUDIO_PLAYBACK) {
        if (isPublishAvsession) {
            return ERR_OK;
        }
    }
    return NotificationTools::GetInstance()->PublishNotification(continuousTaskRecord,
        appName, notificationText, bgTaskUid_);
}

ErrCode BgContinuousTaskMgr::CheckNotificationText(std::string &notificationText,
    const std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord)
{
    auto iter = avSessionNotification_.find(continuousTaskRecord->uid_);
    bool isPublish = (iter != avSessionNotification_.end()) ? iter->second : false;
    BGTASK_LOGD("AVSession Notification isPublish: %{public}d", isPublish);
    // 子类型带有avsession,不发通知
    bool isPublishAvsession = isPublish || (CommonUtils::CheckExistMode(continuousTaskRecord->bgSubModeIds_,
        ContinuousTaskSubmode::SUBMODE_AVSESSION_AUDIO_PLAYBACK) && continuousTaskRecord->isByRequestObject_);
    for (auto mode : continuousTaskRecord->bgModeIds_) {
        if ((mode == BackgroundMode::AUDIO_PLAYBACK && isPublishAvsession) || ((mode == BackgroundMode::VOIP ||
            mode == BackgroundMode::AUDIO_RECORDING) && continuousTaskRecord->IsSystem())) {
            continue;
        }
        BGTASK_LOGD("mode %{public}d", mode);
        if (mode == BackgroundMode::BLUETOOTH_INTERACTION &&
            CommonUtils::CheckExistMode(continuousTaskRecord->bgSubModeIds_, BackgroundSubMode::CAR_KEY)) {
            if (continuousTaskSubText_.empty()) {
                BGTASK_LOGE("get subMode notification prompt info failed, continuousTaskSubText_ is empty");
                return ERR_BGTASK_NOTIFICATION_VERIFY_FAILED;
            }
            uint32_t index = BackgroundSubMode::CAR_KEY - 1;
            if (index < continuousTaskSubText_.size()) {
                notificationText += continuousTaskSubText_.at(index);
                notificationText += "\n";
            } else {
                BGTASK_LOGI("sub index is invalid");
                return ERR_BGTASK_NOTIFICATION_VERIFY_FAILED;
            }
            continue;
        }
        uint32_t index = GetBgModeNameIndex(mode, continuousTaskRecord->isNewApi_);
        if (index < continuousTaskText_.size()) {
            notificationText += continuousTaskText_.at(index);
            notificationText += "\n";
        } else {
            BGTASK_LOGI("index is invalid");
            return ERR_BGTASK_NOTIFICATION_VERIFY_FAILED;
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

ErrCode BgContinuousTaskMgr::StopBackgroundRunning(const std::string &abilityName, int32_t abilityId,
    int32_t continuousTaskId)
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
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Service::StopBackgroundRunningInner");
    handler_->PostSyncTask([this, callingUid, abilityName, abilityId, continuousTaskId, &result]() {
        result = this->StopBackgroundRunningInner(callingUid, abilityName, abilityId, continuousTaskId);
        }, AppExecFwk::EventQueue::Priority::HIGH);

    return result;
}

ErrCode BgContinuousTaskMgr::StopBackgroundRunningInner(int32_t uid, const std::string &abilityName,
    int32_t abilityId, int32_t continuousTaskId)
{
    BgTaskHiTraceChain traceChain(__func__);
    if (continuousTaskId != -1) {
        // 新街口取消
        auto findTask = [continuousTaskId](const auto &target) {
            return continuousTaskId == target.second->continuousTaskId_;
        };
        auto findTaskIter = find_if(continuousTaskInfosMap_.begin(), continuousTaskInfosMap_.end(),
            findTask);
        if (findTaskIter == continuousTaskInfosMap_.end()) {
            BGTASK_LOGE("uid: %{public}d not have task, taskId: %{public}d", uid, continuousTaskId);
            return ERR_BGTASK_OBJECT_NOT_EXIST;
        }
        return StopBackgroundRunningByTask(findTaskIter->second);
    } else {
        auto findTask = [uid, abilityName, abilityId](const auto &target) {
            return uid == target.second->uid_ && abilityName == target.second->abilityName_
                && abilityId == target.second->abilityId_;
        };
        auto findTaskIter = find_if(continuousTaskInfosMap_.begin(), continuousTaskInfosMap_.end(),
            findTask);
        if (findTaskIter == continuousTaskInfosMap_.end()) {
            BGTASK_LOGE("uid: %{public}d not have task", uid);
            return ERR_BGTASK_OBJECT_NOT_EXIST;
        }
        return StopBackgroundRunningByContext(uid, abilityName, abilityId);
    }
}

ErrCode BgContinuousTaskMgr::StopBackgroundRunningByContext(int32_t uid, const std::string &abilityName,
    int32_t abilityId)
{
    std::vector<std::shared_ptr<ContinuousTaskRecord>> tasklist {};
    auto iter = continuousTaskInfosMap_.begin();
    while (iter != continuousTaskInfosMap_.end()) {
        if (iter->second->uid_ == uid && iter->second->abilityName_ == abilityName &&
            iter->second->abilityId_ == abilityId) {
            auto record = iter->second;
            tasklist.push_back(record);
        }
        iter++;
    }
    ErrCode ret = ERR_OK;
    for (const auto &record : tasklist) {
        ret = StopBackgroundRunningByTask(record);
        if (ret != ERR_OK) {
            return ret;
        }
    }
    return ret;
}

ErrCode BgContinuousTaskMgr::StopBackgroundRunningByTask(const std::shared_ptr<ContinuousTaskRecord> &task)
{
    if (!task) {
        BGTASK_LOGE("task is null");
        return ERR_BGTASK_CHECK_TASK_PARAM;
    }
    std::string mapKey = std::to_string(task->uid_) + SEPARATOR + task->abilityName_ + SEPARATOR +
        std::to_string(task->abilityId_);
    if (task->isByRequestObject_) {
        mapKey = mapKey + SEPARATOR + CommonUtils::ModesToString(task->bgModeIds_);
    }
    auto iter = continuousTaskInfosMap_.find(mapKey);
    if (iter == continuousTaskInfosMap_.end()) {
        BGTASK_LOGE("not have task, taskKey: %{public}s", mapKey.c_str());
        return ERR_BGTASK_OBJECT_NOT_EXIST;
    }
    auto record = iter->second;
    OnContinuousTaskChanged(record, ContinuousTaskEventTriggerType::TASK_CANCEL);
    int32_t notificationId = record->GetNotificationId();
    BGTASK_LOGI("continuous task key: %{public}s stop, continuousTaskId: %{public}d, "
        "notificationId: %{public}d", mapKey.c_str(), record->GetContinuousTaskId(), notificationId);
    continuousTaskInfosMap_.erase(mapKey);
    ErrCode result = ERR_OK;
    if (notificationId != -1) {
        std::string notificationLabel = record->GetNotificationLabel();
        auto findNotification = [notificationId, notificationLabel](const auto &target) {
            return notificationId == target.second->notificationId_ &&
                notificationLabel == target.second->notificationLabel_;
        };
        auto findNotificationIter = find_if(continuousTaskInfosMap_.begin(), continuousTaskInfosMap_.end(),
            findNotification);
        if (findNotificationIter == continuousTaskInfosMap_.end()) {
            result = NotificationTools::GetInstance()->CancelNotification(notificationLabel, notificationId);
        }
    }
    HandleAppContinuousTaskStop(record->uid_);
    RefreshTaskRecord();
    return result;
}

ErrCode BgContinuousTaskMgr::GetAllContinuousTasks(std::vector<std::shared_ptr<ContinuousTaskInfo>> &list)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    ErrCode result = CheckIsSysReadyAndPermission(callingUid);
    if (result != ERR_OK) {
        return result;
    }
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Service::GetAllContinuousTasks");
    handler_->PostSyncTask([this, callingUid, &list, &result]() {
        result = this->GetAllContinuousTasksInner(callingUid, list);
        }, AppExecFwk::EventQueue::Priority::HIGH);
    return result;
}

ErrCode BgContinuousTaskMgr::GetAllContinuousTasks(
    std::vector<std::shared_ptr<ContinuousTaskInfo>> &list, bool includeSuspended)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    ErrCode result = CheckIsSysReadyAndPermission(callingUid);
    if (result != ERR_OK) {
        return result;
    }
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Service::GetAllContinuousTasksIncludeSuspended");
    handler_->PostSyncTask([this, callingUid, &list, &result, includeSuspended]() {
        result = this->GetAllContinuousTasksInner(callingUid, list, includeSuspended);
        }, AppExecFwk::EventQueue::Priority::HIGH);
    return result;
}

ErrCode BgContinuousTaskMgr::CheckIsSysReadyAndPermission(int32_t callingUid)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    if (!BundleManagerHelper::GetInstance()->CheckPermission(BGMODE_PERMISSION)) {
        BGTASK_LOGE("uid: %{public}d no have permission", callingUid);
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    if (callingUid < 0) {
        BGTASK_LOGE("param callingUid: %{public}d is invaild", callingUid);
        return ERR_BGTASK_INVALID_UID;
    }
    return ERR_OK;
}

ErrCode BgContinuousTaskMgr::GetAllContinuousTasksInner(int32_t uid,
    std::vector<std::shared_ptr<ContinuousTaskInfo>> &list, bool includeSuspended)
{
    if (continuousTaskInfosMap_.empty()) {
        return ERR_OK;
    }
    BGTASK_LOGW("GetAllContinuousTasksInner, includeSuspended: %{public}d", includeSuspended);
    for (const auto &record : continuousTaskInfosMap_) {
        if (!record.second) {
            continue;
        }
        if (record.second->uid_ != uid) {
            continue;
        }
        if (!includeSuspended && record.second->suspendState_) {
            continue;
        }
        std::string wantAgentBundleName {"NULL"};
        std::string wantAgentAbilityName {"NULL"};
        if (record.second->wantAgentInfo_ != nullptr) {
            wantAgentBundleName = record.second->wantAgentInfo_->bundleName_;
            wantAgentAbilityName = record.second->wantAgentInfo_->abilityName_;
        }
        auto info = std::make_shared<ContinuousTaskInfo>(record.second->abilityName_, record.second->uid_,
            record.second->pid_, record.second->isFromWebview_, record.second->bgModeIds_, record.second->bgSubModeIds_,
            record.second->notificationId_, record.second->continuousTaskId_, record.second->abilityId_,
            wantAgentBundleName, wantAgentAbilityName);
        list.push_back(info);
    }
    return ERR_OK;
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
        return;
    }
    auto record = continuousTaskInfosMap_.at(key);
    SetReason(key, FREEZE_CANCEL);
    StopBackgroundRunningByTask(record);
}

void BgContinuousTaskMgr::SuspendContinuousTask(int32_t uid, int32_t pid, int32_t reason, const std::string &key)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }
    auto self = shared_from_this();
    auto task = [self, uid, pid, reason, key]() {
        if (self) {
            if (self->IsExistCallback(uid, CONTINUOUS_TASK_SUSPEND)) {
                self->HandleSuspendContinuousTask(uid, pid, reason, key);
            } else {
                self->HandleStopContinuousTask(uid, pid, 0, key);
            }
        }
    };
    handler_->PostTask(task);
}

bool BgContinuousTaskMgr::IsExistCallback(int32_t uid, uint32_t type)
{
    for (auto iter = bgTaskSubscribers_.begin(); iter != bgTaskSubscribers_.end(); ++iter) {
        if ((*iter)->isHap_ && (*iter)->uid_ == uid && (((*iter)->flag_ & type) > 0)) {
            BGTASK_LOGD("flag: %{public}u", (*iter)->flag_);
            return true;
        }
    }
    return false;
}

void BgContinuousTaskMgr::HandleSuspendContinuousTask(int32_t uid, int32_t pid, int32_t mode, const std::string &key)
{
    if (continuousTaskInfosMap_.find(key) == continuousTaskInfosMap_.end()) {
        BGTASK_LOGW("suspend TaskInfo failure, no matched task: %{public}s", key.c_str());
        return;
    }
    auto iter = continuousTaskInfosMap_.begin();
    while (iter != continuousTaskInfosMap_.end()) {
        if (iter->second->GetUid() != uid) {
            ++iter;
            continue;
        }
        BGTASK_LOGW("SuspendContinuousTask mode: %{public}d, key %{public}s", mode, key.c_str());
        iter->second->suspendState_ = true;
        uint32_t reasonValue = ContinuousTaskSuspendReason::GetSuspendReasonValue(mode);
        if (reasonValue == 0) {
            iter->second->suspendReason_ = -1;
        } else {
            iter->second->suspendReason_ = static_cast<int32_t>(reasonValue);
        }
        OnContinuousTaskChanged(iter->second, ContinuousTaskEventTriggerType::TASK_SUSPEND);
        RefreshTaskRecord();
        break;
    }
    // 暂停状态取消长时任务通知
    NotificationTools::GetInstance()->CancelNotification(continuousTaskInfosMap_[key]->GetNotificationLabel(),
        continuousTaskInfosMap_[key]->GetNotificationId());
    // 对SA来说，暂停状态等同于取消
    HandleAppContinuousTaskStop(uid);
}

void BgContinuousTaskMgr::ActiveContinuousTask(int32_t uid, int32_t pid, const std::string &key)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }
    auto self = shared_from_this();
    auto task = [self, uid, pid, key]() {
        if (self) {
            self->HandleActiveContinuousTask(uid, pid, key);
        }
    };
    handler_->PostTask(task);
}

void BgContinuousTaskMgr::HandleActiveContinuousTask(int32_t uid, int32_t pid, const std::string &key)
{
    auto findTask = [uid](const auto &target) {
        return uid == target.second->uid_ && target.second->suspendState_;
    };
    auto findTaskIter = find_if(continuousTaskInfosMap_.begin(), continuousTaskInfosMap_.end(), findTask);
    if (findTaskIter == continuousTaskInfosMap_.end()) {
        return;
    }
    std::string notificationLabel = "default";
    int32_t notificationId = ILLEGAL_NOTIFICATION_ID;
    std::vector<int32_t> notificationOldIds {};
    auto iter = continuousTaskInfosMap_.begin();
    while (iter != continuousTaskInfosMap_.end()) {
        if (iter->second->GetUid() != uid || !iter->second->suspendState_) {
            ++iter;
            continue;
        }
        BGTASK_LOGI("ActiveContinuousTask uid: %{public}d, pid: %{public}d", uid, pid);
        iter->second->suspendState_ = false;
        OnContinuousTaskChanged(iter->second, ContinuousTaskEventTriggerType::TASK_ACTIVE);
        if (iter->second->notificationId_ != -1) {
            if (notificationId == ILLEGAL_NOTIFICATION_ID ||
                CommonUtils::CheckExistNotification(notificationOldIds, iter->second->notificationId_)) {
                notificationOldIds.push_back(iter->second->notificationId_);
                SendContinuousTaskNotification(iter->second);
                notificationLabel = iter->second->notificationLabel_;
                notificationId = iter->second->notificationId_;
            } else {
                iter->second->notificationLabel_ = notificationLabel;
                iter->second->notificationId_ = notificationId;
            }
        }
        RefreshTaskRecord();
        break;
    }
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
    HandleAppContinuousTaskStop(uid);
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
        iter->second->reason_ = FREEZE_CANCEL;
        OnContinuousTaskChanged(iter->second, ContinuousTaskEventTriggerType::TASK_CANCEL);
        NotificationTools::GetInstance()->CancelNotification(iter->second->GetNotificationLabel(),
            iter->second->GetNotificationId());
        iter = continuousTaskInfosMap_.erase(iter);
        RefreshTaskRecord();
    }
    HandleAppContinuousTaskStop(uid);
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

ErrCode BgContinuousTaskMgr::GetContinuousTaskApps(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list,
    int32_t uid)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }

    ErrCode result = ERR_OK;

    handler_->PostSyncTask([this, &list, uid, &result]() {
        result = this->GetContinuousTaskAppsInner(list, uid);
        }, AppExecFwk::EventQueue::Priority::HIGH);

    return result;
}

ErrCode BgContinuousTaskMgr::GetContinuousTaskAppsInner(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list,
    int32_t uid)
{
    if (continuousTaskInfosMap_.empty()) {
        return ERR_OK;
    }

    for (auto record : continuousTaskInfosMap_) {
        if (uid != -1 && uid != record.second->uid_) {
            continue;
        }
        if (record.second->suspendState_) {
            continue;
        }
        auto appInfo = std::make_shared<ContinuousTaskCallbackInfo>(record.second->bgModeId_, record.second->uid_,
            record.second->pid_, record.second->abilityName_, record.second->isFromWebview_, record.second->isBatchApi_,
            record.second->bgModeIds_, record.second->abilityId_, record.second->fullTokenId_);
        list.push_back(appInfo);
    }
    return ERR_OK;
}

ErrCode BgContinuousTaskMgr::AVSessionNotifyUpdateNotification(int32_t uid, int32_t pid, bool isPublish)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }

    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != AVSESSION_SA_UID) {
        BGTASK_LOGE("continuous task param uid %{public}d is invalid", callingUid);
        return ERR_BGTASK_CHECK_TASK_PARAM;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask([this, uid, pid, isPublish, &result]() {
        result = this->AVSessionNotifyUpdateNotificationInner(uid, pid, isPublish);
        }, AppExecFwk::EventQueue::Priority::HIGH);

    return result;
}

ErrCode BgContinuousTaskMgr::AVSessionNotifyUpdateNotificationInner(int32_t uid, int32_t pid, bool isPublish)
{
    BGTASK_LOGD("AVSessionNotifyUpdateNotification start, uid: %{public}d, isPublish: %{public}d", uid, isPublish);
    avSessionNotification_[uid] = isPublish;
    auto findUid = [uid](const auto &target) {
        return uid == target.second->GetUid();
    };
    auto findUidIter = find_if(continuousTaskInfosMap_.begin(), continuousTaskInfosMap_.end(), findUid);
    if (findUidIter == continuousTaskInfosMap_.end()) {
        BGTASK_LOGD("continuous task is not exist: %{public}d", uid);
        return ERR_BGTASK_OBJECT_NOT_EXIST;
    }

    ErrCode result = ERR_OK;
    auto record = findUidIter->second;

    // 子类型包含avsession，不发通知
    if (!isPublish &&
        CommonUtils::CheckExistMode(record->bgSubModeIds_, ContinuousTaskSubmode::SUBMODE_AVSESSION_AUDIO_PLAYBACK)) {
        return ERR_OK;
    }

    // 只有播音类型长时任务，并且没有AVSession通知
    if (!isPublish && record->bgModeIds_.size() == 1 && record->bgModeIds_[0] == BackgroundMode::AUDIO_PLAYBACK) {
        result = SendContinuousTaskNotification(record);
        return result;
    }
    std::map<std::string, std::pair<std::string, std::string>> newPromptInfos;
    if (!CommonUtils::CheckExistMode(record->bgModeIds_, BackgroundMode::DATA_TRANSFER)) {
        std::string mainAbilityLabel = GetMainAbilityLabel(record->bundleName_, record->userId_);
        std::string notificationText = GetNotificationText(record);
        if (notificationText.empty()) {
            result = NotificationTools::GetInstance()->CancelNotification(
                record->GetNotificationLabel(), record->GetNotificationId());
                record->notificationId_ = -1;
        } else {
            newPromptInfos.emplace(record->notificationLabel_, std::make_pair(mainAbilityLabel, notificationText));
            NotificationTools::GetInstance()->RefreshContinuousNotifications(newPromptInfos, bgTaskUid_);
        }
    }
    return result;
}

void BgContinuousTaskMgr::SuspendContinuousAudioTask(int32_t uid)
{
    if (!isSysReady_.load()) {
        return;
    }
    auto self = shared_from_this();
    auto task = [self, uid]() {
        if (self) {
            self->HandleSuspendContinuousAudioTask(uid);
        }
    };
    handler_->PostTask(task);
}

void BgContinuousTaskMgr::HandleSuspendContinuousAudioTask(int32_t uid)
{
    auto iter = continuousTaskInfosMap_.begin();
    while (iter != continuousTaskInfosMap_.end()) {
        if (iter->second->GetUid() == uid &&
            CommonUtils::CheckExistMode(iter->second->bgModeIds_, BackgroundMode::AUDIO_PLAYBACK)) {
            NotificationTools::GetInstance()->CancelNotification(iter->second->GetNotificationLabel(),
                iter->second->GetNotificationId());
            if (!IsExistCallback(uid, CONTINUOUS_TASK_SUSPEND)) {
                iter++;
                continue;
            }
            if (iter->second->GetSuspendAudioTaskTimes() == 0) {
                iter->second->suspendState_ = true;
                iter->second->suspendAudioTaskTimes_ = 1;
                iter->second->suspendReason_ =
                    static_cast<int32_t>(ContinuousTaskSuspendReason::SYSTEM_SUSPEND_AUDIO_PLAYBACK_NOT_RUNNING);
                OnContinuousTaskChanged(iter->second, ContinuousTaskEventTriggerType::TASK_SUSPEND);
                RefreshTaskRecord();
                iter++;
            } else {
                OnContinuousTaskChanged(iter->second, ContinuousTaskEventTriggerType::TASK_CANCEL);
                iter = continuousTaskInfosMap_.erase(iter);
                RefreshTaskRecord();
            }
        } else {
            iter++;
        }
    }
    HandleAppContinuousTaskStop(uid);
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
    } else if (dumpOption[1] == DUMP_PARAM_GET) {
        BgContinuousTaskDumper::GetInstance()->DumpGetTask(dumpOption, dumpInfo);
    } else if (dumpOption[1] == DUMP_INNER_TASK) {
        BgContinuousTaskDumper::GetInstance()->DebugContinuousTask(dumpOption, dumpInfo);
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
        stream << "\t\tsuspendState: " << (iter->second->suspendState_ ? "true" : "false") << "\n";
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
        stream << "\t\tisCombinedTaskNotification: " <<
            (iter->second->isCombinedTaskNotification_ ? "true" : "false") << "\n";
        stream << "\t\tcombinedNotificationTaskId: " << iter->second->combinedNotificationTaskId_ << "\n";
        stream << "\t\tisByRequestObject: " << (iter->second->isByRequestObject_ ? "true" : "false") << "\n";
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
            return;
        }
        std::string taskKey = dumpOption[2];
        auto iter = continuousTaskInfosMap_.find(taskKey);
        if (iter == continuousTaskInfosMap_.end()) {
            return;
        }
        StopBackgroundRunningByTask(iter->second);
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

bool BgContinuousTaskMgr::StopContinuousTaskByUser(const std::string &mapKey)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return false;
    }
    bool result = true;
    handler_->PostSyncTask([this, mapKey, &result]() {
        SetReason(mapKey, REMOVE_NOTIFICATION_CANCEL);
        result = StopContinuousTaskByUserInner(mapKey);
    });
    return result;
}

bool BgContinuousTaskMgr::StopContinuousTaskByUserInner(const std::string &key)
{
    auto removeTask = continuousTaskInfosMap_.find(key);
    if (removeTask == continuousTaskInfosMap_.end()) {
        return false;
    }
    if (StopBackgroundRunningByTask(removeTask->second) == ERR_OK) {
        return true;
    }
    return true;
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
}

void BgContinuousTaskMgr::OnAbilityStateChanged(int32_t uid, const std::string &abilityName, int32_t abilityId)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }
    if (abilityName.empty()) {
        BGTASK_LOGE("abilityName is empty!");
        return;
    }
    StopBackgroundRunningByContext(uid, abilityName, abilityId);
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
        case ContinuousTaskEventTriggerType::TASK_SUSPEND:
            NotifySubscribersTaskSuspend(continuousTaskCallbackInfo);
            break;
        case ContinuousTaskEventTriggerType::TASK_ACTIVE:
            NotifySubscribersTaskActive(continuousTaskCallbackInfo);
            break;
        default:
            BGTASK_LOGE("unknow ContinuousTaskEventTriggerType: %{public}d", changeEventType);
            break;
    }
}

void BgContinuousTaskMgr::NotifySubscribersTaskSuspend(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    const ContinuousTaskCallbackInfo& taskCallbackInfoRef = *continuousTaskCallbackInfo;
    for (auto iter = bgTaskSubscribers_.begin(); iter != bgTaskSubscribers_.end(); ++iter) {
        if (!(*iter)->isHap_ && (*iter)->subscriber_) {
            // 对SA来说，长时任务暂停状态等同于取消长时任务，保持原有逻辑
            BGTASK_LOGD("continuous task suspend callback trigger");
            (*iter)->subscriber_->OnContinuousTaskStop(taskCallbackInfoRef);
        } else if ((*iter)->isHap_ &&
            (*iter)->uid_ == continuousTaskCallbackInfo->GetCreatorUid() && (*iter)->subscriber_) {
            // 回调通知应用长时任务暂停
            BGTASK_LOGI("uid %{public}d is hap and uid is same, need notify suspend, suspendReason: %{public}d"
                "suspendState: %{public}d", (*iter)->uid_, taskCallbackInfoRef.GetSuspendReason(),
                taskCallbackInfoRef.GetSuspendState());
            (*iter)->subscriber_->OnContinuousTaskSuspend(taskCallbackInfoRef);
        }
    }
}

void BgContinuousTaskMgr::NotifySubscribersTaskActive(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    const ContinuousTaskCallbackInfo& taskCallbackInfoRef = *continuousTaskCallbackInfo;
    for (auto iter = bgTaskSubscribers_.begin(); iter != bgTaskSubscribers_.end(); ++iter) {
        BGTASK_LOGD("continuous task active callback trigger");
        if (!(*iter)->isHap_ && (*iter)->subscriber_) {
            // 对SA来说，长时任务激活状态等同于注册长时任务，保持原有逻辑
            (*iter)->subscriber_->OnContinuousTaskStart(taskCallbackInfoRef);
        } else if ((*iter)->isHap_ &&
            (*iter)->uid_ == continuousTaskCallbackInfo->GetCreatorUid() && (*iter)->subscriber_) {
            // 回调通知应用长时任务激活
            BGTASK_LOGI("uid %{public}d is hap and uid is same, need notify active", (*iter)->uid_);
            (*iter)->subscriber_->OnContinuousTaskActive(taskCallbackInfoRef);
        }
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
        default:
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
    continuousTaskCallbackInfo->SetContinuousTaskId(continuousTaskInfo->continuousTaskId_);
    continuousTaskCallbackInfo->SetCancelReason(continuousTaskInfo->reason_);
    continuousTaskCallbackInfo->SetSuspendState(continuousTaskInfo->suspendState_);
    continuousTaskCallbackInfo->SetSuspendReason(continuousTaskInfo->suspendReason_);
    continuousTaskCallbackInfo->SetByRequestObject(continuousTaskInfo->isByRequestObject_);
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
    BgTaskHiTraceChain traceChain(__func__);
    std::string mainAbilityLabel {""};
    AppExecFwk::BundleResourceInfo bundleResourceInfo;
    if (BundleManagerHelper::GetInstance()->GetBundleResourceInfo(bundleName,
        AppExecFwk::ResourceFlag::GET_RESOURCE_INFO_ALL, bundleResourceInfo)) {
        mainAbilityLabel = bundleResourceInfo.label;
    }
    return mainAbilityLabel;
}

void BgContinuousTaskMgr::OnConfigurationChanged(const AppExecFwk::Configuration &configuration)
{
    BgTaskHiTraceChain traceChain(__func__);
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
    auto iter = avSessionNotification_.find(record->uid_);
    bool isPublish = (iter != avSessionNotification_.end()) ? iter->second : false;
    BGTASK_LOGD("AVSession Notification isPublish: %{public}d", isPublish);
    std::string notificationText {""};
    for (auto mode : record->bgModeIds_) {
        if ((mode == BackgroundMode::AUDIO_PLAYBACK && isPublish) || ((mode == BackgroundMode::VOIP ||
            mode == BackgroundMode::AUDIO_RECORDING) && record->IsSystem())) {
            continue;
        }
        if (mode == BackgroundMode::BLUETOOTH_INTERACTION &&
            CommonUtils::CheckExistMode(record->bgSubModeIds_, BackgroundSubMode::CAR_KEY)) {
            uint32_t index = BackgroundSubMode::CAR_KEY - 1;
            if (index < continuousTaskSubText_.size()) {
                notificationText += continuousTaskSubText_.at(index);
                notificationText += "\n";
            }
            continue;
        }
        uint32_t index = GetBgModeNameIndex(mode, record->isNewApi_);
        if (index < continuousTaskText_.size()) {
            notificationText += continuousTaskText_.at(index);
            notificationText += "\n";
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

ErrCode BgContinuousTaskMgr::IsModeSupported(const sptr<ContinuousTaskParam> &taskParam)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }

    if (!BundleManagerHelper::GetInstance()->CheckPermission(BGMODE_PERMISSION)) {
        BGTASK_LOGE("background mode permission is not passed");
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (BundleManagerHelper::GetInstance()->IsSystemApp(fullTokenId) &&
        CommonUtils::CheckExistMode(taskParam->bgModeIds_, BackgroundMode::TASK_KEEPING)) {
        BGTASK_LOGE("not support system app");
        return ERR_BGTASK_CONTINUOUS_SYSTEM_APP_NOT_SUPPORT_ACL;
    }
    ErrCode result = ERR_OK;
    uint64_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    std::string bundleName = BundleManagerHelper::GetInstance()->GetClientBundleName(callingUid);
    handler_->PostSyncTask([this, callingTokenId, taskParam, bundleName, &result]() {
        result = this->CheckTaskkeepingPermission(taskParam, callingTokenId, bundleName);
        }, AppExecFwk::EventQueue::Priority::HIGH);
    return result;
}

ErrCode BgContinuousTaskMgr::CheckTaskkeepingPermission(
    const sptr<ContinuousTaskParam> &taskParam, uint64_t callingTokenId, const std::string &bundleName)
{
    if (DelayedSingleton<BgtaskConfig>::GetInstance()->IsTaskKeepingExemptedQuatoApp(bundleName) ||
        SUPPORT_TASK_KEEPING) {
        return ERR_OK;
    }
    if (CommonUtils::CheckExistMode(taskParam->bgModeIds_, BackgroundMode::TASK_KEEPING) &&
        !BundleManagerHelper::GetInstance()->CheckACLPermission(BGMODE_PERMISSION_SYSTEM, callingTokenId)) {
        BGTASK_LOGW("app have no acl permission");
        return ERR_BGTASK_CONTINUOUS_APP_NOT_HAVE_BGMODE_PERMISSION_SYSTEM;
    }
    BGTASK_LOGI("app have acl permission");
    return ERR_OK;
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

ErrCode BgContinuousTaskMgr::DebugContinuousTaskInner(const sptr<ContinuousTaskParamForInner> &taskParam)
{
    if (!isSysReady_.load()) {
        return ERR_BGTASK_SYS_NOT_READY;
    }
    if (!taskParam) {
        return ERR_BGTASK_CHECK_TASK_PARAM;
    }
    if (taskParam->isStart_) {
        return StartBackgroundRunningForInner(taskParam);
    }
    return StopBackgroundRunningForInner(taskParam);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
