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

#include "background_task_mgr_service.h"
#include "background_mode.h"
#include "bgtask_config.h"
#include "bgtask_hitrace_chain.h"
#include "bundle_manager_helper.h"
#include <functional>
#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "bgtaskmgr_inner_errors.h"
#include "bundle_constants.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "common_utils.h"
#include "file_ex.h"
#include "ipc_skeleton.h"
#include "string_ex.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"

#include "bgtaskmgr_log_wrapper.h"
#include <parameters.h>

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr int32_t NO_DUMP_PARAM_NUMS = 0;
static constexpr int32_t RESOURCE_SCHEDULE_SERVICE_UID = 1096;
static constexpr uint32_t CHECK_TIMEOUT = 10;
static constexpr char BGMODE_PERMISSION[] = "ohos.permission.KEEP_BACKGROUND_RUNNING";
static constexpr char SET_BACKGROUND_TASK_STATE_PERMISSION[] = "ohos.permission.SET_BACKGROUND_TASK_STATE";
static constexpr char GET_BACKGROUND_TASK_INFO_PERMISSION[] = "ohos.permission.GET_BACKGROUND_TASK_INFO";
const int32_t ENG_MODE = OHOS::system::GetIntParameter("const.debuggable", 0);
const std::string BGTASK_SERVICE_NAME = "BgtaskMgrService";
static constexpr char EXTENSION_BACKUP[] = "backup";
static constexpr char EXTENSION_RESTORE[] = "restore";
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(
    DelayedSingleton<BackgroundTaskMgrService>::GetInstance().get());
}

BackgroundTaskMgrService::BackgroundTaskMgrService()
    : SystemAbility(BACKGROUND_TASK_MANAGER_SERVICE_ID, true) {}

BackgroundTaskMgrService::~BackgroundTaskMgrService() {}

void BackgroundTaskMgrService::OnStart()
{
    BgTaskHiTraceChain traceChain(__func__);
    BGTASK_LOGI("BackgroundTaskMgrService service onStart.");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        BGTASK_LOGW("Service has already started.");
        return;
    }
    Init();
    AddSystemAbilityListener(APP_MGR_SERVICE_ID);
    AddSystemAbilityListener(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    AddSystemAbilityListener(SA_ID_VOIP_CALL_MANAGER);
    AddSystemAbilityListener(SA_ID_HEALTH_SPORT);
    AddSystemAbilityListener(SUSPEND_MANAGER_SYSTEM_ABILITY_ID);
}

void BackgroundTaskMgrService::SetReady(uint32_t flag)
{
    BgTaskHiTraceChain traceChain(__func__);
    BGTASK_LOGI("BackgroundTaskMgrService service SetReady.");
    {
        std::lock_guard<std::mutex> lock(readyMutex_);
        if (dependsReady_ == ServiceReadyState::ALL_READY) {
            return;
        }
        dependsReady_ |= flag;
        if (dependsReady_ != ServiceReadyState::ALL_READY) {
            return;
        }
    }
    DelayedSingleton<BgtaskConfig>::GetInstance()->Init();
    BGTASK_LOGI("BackgroundTaskMgrService service Publish.");
    if (!Publish(DelayedSingleton<BackgroundTaskMgrService>::GetInstance().get())) {
        BGTASK_LOGE("Service start failed!");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    BGTASK_LOGI("background task manager service start succeed!");
}

int32_t BackgroundTaskMgrService::OnExtension(const std::string& extension, MessageParcel& data, MessageParcel& reply)
{
    BgTaskHiTraceChain traceChain(__func__);
    BGTASK_LOGI("extension is %{public}s", extension.c_str());
    if (extension == EXTENSION_BACKUP) {
        // 业务备份
        return BgContinuousTaskMgr::GetInstance()->OnBackup(data, reply);
    } else if (extension == EXTENSION_RESTORE) {
        // 业务恢复
        return BgContinuousTaskMgr::GetInstance()->OnRestore(data, reply);
    }
    return 0;
}

void BackgroundTaskMgrService::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    BgTaskHiTraceChain traceChain(__func__);
    DelayedSingleton<BgEfficiencyResourcesMgr>::GetInstance()->OnAddSystemAbility(systemAbilityId, deviceId);
}

void BackgroundTaskMgrService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    BgTaskHiTraceChain traceChain(__func__);
    DelayedSingleton<BgEfficiencyResourcesMgr>::GetInstance()->OnRemoveSystemAbility(systemAbilityId, deviceId);
    BgContinuousTaskMgr::GetInstance()->OnRemoveSystemAbility(systemAbilityId, deviceId);
    DelayedSingleton<BgTransientTaskMgr>::GetInstance()->OnRemoveSystemAbility(systemAbilityId, deviceId);
}

void BackgroundTaskMgrService::Init()
{
    BgTaskHiTraceChain traceChain(__func__);
    runner_ = AppExecFwk::EventRunner::Create(BGTASK_SERVICE_NAME);
    DelayedSingleton<BgTransientTaskMgr>::GetInstance()->Init(runner_);
    DelayedSingleton<BgEfficiencyResourcesMgr>::GetInstance()->Init(runner_);
    BgContinuousTaskMgr::GetInstance()->Init(runner_);
}

void BackgroundTaskMgrService::OnStop()
{
    BgTaskHiTraceChain traceChain(__func__);
    BgContinuousTaskMgr::GetInstance()->Clear();
    DelayedSingleton<BgEfficiencyResourcesMgr>::GetInstance()->Clear();
    state_ = ServiceRunningState::STATE_NOT_START;
    BGTASK_LOGI("background task manager stop");
}

bool BackgroundTaskMgrService::CheckCallingToken()
{
    Security::AccessToken::AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenFlag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenFlag == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE ||
        tokenFlag == Security::AccessToken::ATokenTypeEnum::TOKEN_SHELL) {
        return true;
    }
    return false;
}

bool BackgroundTaskMgrService::CheckHapCalling(bool &isHap)
{
    Security::AccessToken::AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenFlag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenFlag == Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        isHap = true;
        return BundleManagerHelper::GetInstance()->CheckPermission(BGMODE_PERMISSION);
    }
    return false;
}

bool BackgroundTaskMgrService::CheckCallingProcess()
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    // only rss is allowed to call
    if (callingUid != RESOURCE_SCHEDULE_SERVICE_UID) {
        BGTASK_LOGW("uid %{public}d pid %{public}d not allowed to call", callingUid, callingPid);
        return false;
    }
    return true;
}

ErrCode BackgroundTaskMgrService::RequestSuspendDelay(const std::string& reason,
    const sptr<IExpiredCallback>& callback, DelaySuspendInfo &delayInfo)
{
    if (CheckAtomicService()) {
        pid_t callingPid = IPCSkeleton::GetCallingPid();
        pid_t callingUid = IPCSkeleton::GetCallingUid();
        BGTASK_LOGE("uid %{public}d pid %{public}d Check atomisc service fail, apply transienttask not allowed",
            callingUid, callingPid);
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    std::u16string reasonu16 = Str8ToStr16(reason);
    std::shared_ptr<DelaySuspendInfo> delayInfoPtr = std::make_shared<DelaySuspendInfo>(delayInfo);
    ErrCode result = DelayedSingleton<BgTransientTaskMgr>::GetInstance()->RequestSuspendDelay(
        reasonu16, callback, delayInfoPtr);
    if (result == ERR_OK) {
        delayInfo = *delayInfoPtr;
    }
    return result;
}

ErrCode BackgroundTaskMgrService::CancelSuspendDelay(int32_t requestId)
{
    return DelayedSingleton<BgTransientTaskMgr>::GetInstance()->CancelSuspendDelay(requestId);
}

ErrCode BackgroundTaskMgrService::GetRemainingDelayTime(int32_t requestId, int32_t &delayTime)
{
    return DelayedSingleton<BgTransientTaskMgr>::GetInstance()->GetRemainingDelayTime(requestId, delayTime);
}

ErrCode BackgroundTaskMgrService::GetAllTransientTasks(int32_t &remainingQuota,
    std::vector<std::shared_ptr<DelaySuspendInfo>> &list)
{
    return DelayedSingleton<BgTransientTaskMgr>::GetInstance()->GetAllTransientTasks(remainingQuota, list);
}

void BackgroundTaskMgrService::ForceCancelSuspendDelay(int32_t requestId)
{
    DelayedSingleton<BgTransientTaskMgr>::GetInstance()->ForceCancelSuspendDelay(requestId);
}

ErrCode BackgroundTaskMgrService::StartBackgroundRunning(const ContinuousTaskParam &taskParam,
    int32_t& notificationId, int32_t& continuousTaskId)
{
    if (CheckAtomicService()) {
        if (taskParam.isByRequestObject_) {
            return ERR_BGTASK_PERMISSION_DENIED;
        }
        for (const auto mode : taskParam.bgModeIds_) {
            if (mode != BackgroundMode::AUDIO_PLAYBACK && mode != BackgroundMode::MULTI_DEVICE_CONNECTION &&
                mode != BackgroundMode::LOCATION) {
                pid_t callingPid = IPCSkeleton::GetCallingPid();
                pid_t callingUid = IPCSkeleton::GetCallingUid();
                BGTASK_LOGE("uid %{public}d pid %{public}d Check atomisc service fail,"
                    " apply continuoustask not allowed", callingUid, callingPid);
                return ERR_BGTASK_PERMISSION_DENIED;
            }
        }
    }
    auto paramPtr = sptr<ContinuousTaskParam>(new ContinuousTaskParam(taskParam));
    ErrCode result = BgContinuousTaskMgr::GetInstance()->StartBackgroundRunning(paramPtr);
    if (result == ERR_OK) {
        notificationId = paramPtr->notificationId_;
        continuousTaskId = paramPtr->continuousTaskId_;
    }
    return result;
}

ErrCode BackgroundTaskMgrService::UpdateBackgroundRunning(const ContinuousTaskParam &taskParam,
    int32_t& notificationId, int32_t& continuousTaskId)
{
    if (CheckAtomicService()) {
        if (taskParam.isByRequestObject_) {
            return ERR_BGTASK_PERMISSION_DENIED;
        }
        for (const auto mode : taskParam.bgModeIds_) {
            if (mode != BackgroundMode::AUDIO_PLAYBACK && mode != BackgroundMode::MULTI_DEVICE_CONNECTION &&
                mode != BackgroundMode::LOCATION) {
                pid_t callingPid = IPCSkeleton::GetCallingPid();
                pid_t callingUid = IPCSkeleton::GetCallingUid();
                BGTASK_LOGE("uid %{public}d pid %{public}d Check atomisc service fail,"
                    " update continuoustask not allowed", callingUid, callingPid);
                return ERR_BGTASK_PERMISSION_DENIED;
            }
        }
    }
    auto paramPtr = sptr<ContinuousTaskParam>(new ContinuousTaskParam(taskParam));
    ErrCode result = BgContinuousTaskMgr::GetInstance()->UpdateBackgroundRunning(paramPtr);
    if (result == ERR_OK) {
        notificationId = paramPtr->notificationId_;
        continuousTaskId = paramPtr->continuousTaskId_;
    }
    return result;
}

ErrCode BackgroundTaskMgrService::RequestBackgroundRunningForInner(const ContinuousTaskParamForInner &taskParam)
{
    BgTaskHiTraceChain traceChain(__func__);
    auto paramPtr = sptr<ContinuousTaskParamForInner>(new ContinuousTaskParamForInner(taskParam));
    return BgContinuousTaskMgr::GetInstance()->RequestBackgroundRunningForInner(paramPtr);
}

ErrCode BackgroundTaskMgrService::RequestGetContinuousTasksByUidForInner(int32_t uid,
    std::vector<ContinuousTaskInfo> &list)
{
    BgTaskHiTraceChain traceChain(__func__);
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    if (!CheckCallingToken() && uid != callingUid) {
        BGTASK_LOGW("RequestGetContinuousTasksByUidForInner not allowed");
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    std::vector<std::shared_ptr<ContinuousTaskInfo>> tasksList;
    ErrCode result = BgContinuousTaskMgr::GetInstance()->RequestGetContinuousTasksByUidForInner(uid, tasksList);
    if (result == ERR_OK) {
        for (const auto& ptr : tasksList) {
            if (ptr != nullptr) {
                list.push_back(*ptr);
            }
        }
    }
    return result;
}

ErrCode BackgroundTaskMgrService::StopBackgroundRunning(const std::string &abilityName,
    const sptr<IRemoteObject> &abilityToken, int32_t abilityId, int32_t continuousTaskId)
{
    return BgContinuousTaskMgr::GetInstance()->StopBackgroundRunning(abilityName, abilityId, continuousTaskId);
}

ErrCode BackgroundTaskMgrService::GetAllContinuousTasks(std::vector<ContinuousTaskInfo> &list)
{
    std::vector<std::shared_ptr<ContinuousTaskInfo>> tasksList;
    ErrCode result = BgContinuousTaskMgr::GetInstance()->GetAllContinuousTasks(tasksList);
    if (result == ERR_OK) {
        for (const auto& ptr : tasksList) {
            if (ptr != nullptr) {
                list.push_back(*ptr);
            }
        }
    }
    return result;
}

ErrCode BackgroundTaskMgrService::GetAllContinuousTasks(
    std::vector<std::shared_ptr<ContinuousTaskInfo>> &list, bool includeSuspended)
{
    return BgContinuousTaskMgr::GetInstance()->GetAllContinuousTasks(list, includeSuspended);
}

ErrCode BackgroundTaskMgrService::GetTransientTaskApps(std::vector<TransientTaskAppInfo> &list)
{
    BgTaskHiTraceChain traceChain(__func__);
    if (!CheckCallingToken()) {
        BGTASK_LOGW("GetTransientTaskApps not allowed");
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    std::vector<std::shared_ptr<TransientTaskAppInfo>> resultList;
    ErrCode result = DelayedSingleton<BgTransientTaskMgr>::GetInstance()->GetTransientTaskApps(resultList);
    if (result == ERR_OK) {
        for (const auto& ptr : resultList) {
            if (ptr != nullptr) {
                list.push_back(*ptr);
            }
        }
    }

    return result;
}

ErrCode BackgroundTaskMgrService::PauseTransientTaskTimeForInner(int32_t uid)
{
    BgTaskHiTraceChain traceChain(__func__);
    return DelayedSingleton<BgTransientTaskMgr>::GetInstance()->PauseTransientTaskTimeForInner(uid);
}

ErrCode BackgroundTaskMgrService::StartTransientTaskTimeForInner(int32_t uid)
{
    BgTaskHiTraceChain traceChain(__func__);
    return DelayedSingleton<BgTransientTaskMgr>::GetInstance()->StartTransientTaskTimeForInner(uid);
}

ErrCode BackgroundTaskMgrService::GetContinuousTaskApps(std::vector<ContinuousTaskCallbackInfo> &list)
{
    BgTaskHiTraceChain traceChain(__func__);
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> resultList;
    if (!CheckCallingToken()) {
        BGTASK_LOGW("uid %{public}d pid %{public}d GetContinuousTaskApps not allowed", callingUid, callingPid);
        ErrCode state = BgContinuousTaskMgr::GetInstance()->GetContinuousTaskApps(resultList, callingUid);
        if (state != ERR_OK) {
            return state;
        }
        for (const auto& ptr : resultList) {
            if (ptr != nullptr) {
                list.push_back(*ptr);
            }
        }
        return ERR_OK;
    }
    int32_t timeoutId = HiviewDFX::XCollie::GetInstance().SetTimer("GetContinuousTaskAppsTimeout", CHECK_TIMEOUT,
        [](void *) {
            BGTASK_LOGE("BackgroundTaskMgrService::GetContinuousTaskApps Timeout:%{public}d", CHECK_TIMEOUT);
        }, nullptr, HiviewDFX::XCOLLIE_FLAG_DEFAULT);
    ErrCode result = BgContinuousTaskMgr::GetInstance()->GetContinuousTaskApps(resultList);
    HiviewDFX::XCollie::GetInstance().CancelTimer(timeoutId);
    if (result == ERR_OK) {
        for (const auto& ptr : resultList) {
            if (ptr != nullptr) {
                list.push_back(*ptr);
            }
        }
    }

    return result;
}

ErrCode BackgroundTaskMgrService::SubscribeBackgroundTask(
    const sptr<IBackgroundTaskSubscriber>& subscriber, uint32_t flag)
{
    BgTaskHiTraceChain traceChain(__func__);
    bool isHap = false;
    if (!CheckCallingToken() && !CheckHapCalling(isHap)) {
        BGTASK_LOGW("SubscribeBackgroundTask not allowed");
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    // 系统应用通过系统API接口注册
    if (isHap && flag == SUBSCRIBER_BACKGROUND_TASK_STATE) {
        if (CheckAtomicService()) {
            return ERR_BGTASK_PERMISSION_DENIED;
        }
        uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
        if (!BundleManagerHelper::GetInstance()->IsSystemApp(fullTokenId)) {
            return ERR_BGTASK_NOT_SYSTEM_APP;
        }
        if (!BundleManagerHelper::GetInstance()->CheckPermission(GET_BACKGROUND_TASK_INFO_PERMISSION)) {
            return ERR_BGTASK_PERMISSION_DENIED;
        }
    }
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    BGTASK_LOGI("uid %{public}d pid %{public}d isHap %{public}d flag %{public}u subscribe",
        callingUid, callingPid, isHap, flag);
    auto subscriberInfo = std::make_shared<SubscriberInfo>(subscriber, callingUid, callingPid, isHap, flag);
    if (BgContinuousTaskMgr::GetInstance()->AddSubscriber(subscriberInfo) != ERR_OK) {
        BGTASK_LOGE("continuous task subscribe background task failed");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    if (!isHap) {
        if (DelayedSingleton<BgTransientTaskMgr>::GetInstance()->SubscribeBackgroundTask(subscriber) != ERR_OK
            || DelayedSingleton<BgEfficiencyResourcesMgr>::GetInstance()->AddSubscriber(subscriber) != ERR_OK) {
            BGTASK_LOGE("transient task or efficiency resource subscribe background task failed");
            return ERR_BGTASK_SYS_NOT_READY;
        }
    }
    BGTASK_LOGW("subscribe background task success");
    return ERR_OK;
}

ErrCode BackgroundTaskMgrService::UnsubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber,
    uint32_t flag)
{
    BgTaskHiTraceChain traceChain(__func__);
    bool isHap = false;
    if (!CheckCallingToken() && !CheckHapCalling(isHap)) {
        BGTASK_LOGW("UnsubscribeBackgroundTask not allowed");
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    if (isHap && flag == SUBSCRIBER_BACKGROUND_TASK_STATE) {
        if (CheckAtomicService()) {
            return ERR_BGTASK_PERMISSION_DENIED;
        }
        uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
        if (!BundleManagerHelper::GetInstance()->IsSystemApp(fullTokenId)) {
            return ERR_BGTASK_NOT_SYSTEM_APP;
        }
        if (!BundleManagerHelper::GetInstance()->CheckPermission(GET_BACKGROUND_TASK_INFO_PERMISSION)) {
            return ERR_BGTASK_PERMISSION_DENIED;
        }
    }
    if (BgContinuousTaskMgr::GetInstance()->RemoveSubscriber(subscriber, flag) != ERR_OK) {
        BGTASK_LOGE("continuous task unsubscribe background task failed");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    if (!isHap) {
        if (DelayedSingleton<BgTransientTaskMgr>::GetInstance()->UnsubscribeBackgroundTask(subscriber) != ERR_OK
            || DelayedSingleton<BgEfficiencyResourcesMgr>::GetInstance()->RemoveSubscriber(subscriber) != ERR_OK) {
            BGTASK_LOGE("transient task or efficiency resource unsubscribe background task failed");
            return ERR_BGTASK_SYS_NOT_READY;
        }
    }
    BGTASK_LOGW("unsubscribe background task success");
    return ERR_OK;
}

void BackgroundTaskMgrService::HandleRequestExpired(const int32_t requestId)
{
    DelayedSingleton<BgTransientTaskMgr>::GetInstance()->HandleRequestExpired(requestId);
}

void BackgroundTaskMgrService::HandleExpiredCallbackDeath(const wptr<IRemoteObject>& remote)
{
    DelayedSingleton<BgTransientTaskMgr>::GetInstance()->HandleExpiredCallbackDeath(remote);
    DelayedSingleton<BgContinuousTaskMgr>::GetInstance()->HandleAuthExpiredCallbackDeath(remote);
}

void BackgroundTaskMgrService::HandleSubscriberDeath(const wptr<IRemoteObject>& remote)
{
    DelayedSingleton<BgTransientTaskMgr>::GetInstance()->HandleSubscriberDeath(remote);
}

ErrCode BackgroundTaskMgrService::ApplyEfficiencyResources(const EfficiencyResourceInfo &resourceInfo)
{
    auto resourcePtr = sptr<EfficiencyResourceInfo>(new EfficiencyResourceInfo(resourceInfo));
    return DelayedSingleton<BgEfficiencyResourcesMgr>::GetInstance()->ApplyEfficiencyResources(resourcePtr);
}

ErrCode BackgroundTaskMgrService::ResetAllEfficiencyResources()
{
    return DelayedSingleton<BgEfficiencyResourcesMgr>::GetInstance()->ResetAllEfficiencyResources();
}

ErrCode BackgroundTaskMgrService::GetAllEfficiencyResources(std::vector<EfficiencyResourceInfo> &resourceInfoList)
{
    BgTaskHiTraceChain traceChain(__func__);
    std::vector<std::shared_ptr<EfficiencyResourceInfo>> list {};
    ErrCode result = DelayedSingleton<BgEfficiencyResourcesMgr>::GetInstance()->GetAllEfficiencyResources(list);
    if (result == ERR_OK) {
        for (const auto& ptr : list) {
            if (ptr != nullptr) {
                resourceInfoList.push_back(*ptr);
            }
        }
    }
    return result;
}

ErrCode BackgroundTaskMgrService::GetEfficiencyResourcesInfos(
    std::vector<ResourceCallbackInfo> &appList,
    std::vector<ResourceCallbackInfo> &procList)
{
    if (!CheckCallingToken()) {
        BGTASK_LOGW("GetEfficiencyResourcesInfos not allowed");
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    std::vector<std::shared_ptr<ResourceCallbackInfo>> resultAppList;
    std::vector<std::shared_ptr<ResourceCallbackInfo>> resultProcList;
    ErrCode result = DelayedSingleton<BgEfficiencyResourcesMgr>::GetInstance()->
        GetEfficiencyResourcesInfos(resultAppList, resultProcList);
    if (result == ERR_OK) {
        for (const auto& ptr : resultAppList) {
            if (ptr != nullptr) {
                appList.push_back(*ptr);
            }
        }
        for (const auto& ptr : resultProcList) {
            if (ptr != nullptr) {
                procList.push_back(*ptr);
            }
        }
    }
    return result;
}

ErrCode BackgroundTaskMgrService::StopContinuousTask(int32_t uid, int32_t pid, uint32_t taskType,
    const std::string &key)
{
    BgTaskHiTraceChain traceChain(__func__);
    if (!CheckCallingToken() || !CheckCallingProcess()) {
        BGTASK_LOGW("StopContinuousTask not allowed");
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    BgContinuousTaskMgr::GetInstance()->StopContinuousTask(uid, pid, taskType, key);
    return ERR_OK;
}

ErrCode BackgroundTaskMgrService::SuspendContinuousTask(
    int32_t uid, int32_t pid, int32_t reason, const std::string &key)
{
    if (!CheckCallingToken() || !CheckCallingProcess()) {
        BGTASK_LOGW("SuspendContinuousTask not allowed");
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    BgContinuousTaskMgr::GetInstance()->SuspendContinuousTask(uid, pid, reason, key);
    return ERR_OK;
}

ErrCode BackgroundTaskMgrService::ActiveContinuousTask(int32_t uid, int32_t pid, const std::string &key)
{
    if (!CheckCallingToken() || !CheckCallingProcess()) {
        BGTASK_LOGW("ActiveContinuousTask not allowed");
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    BgContinuousTaskMgr::GetInstance()->ActiveContinuousTask(uid, pid, key);
    return ERR_OK;
}

ErrCode BackgroundTaskMgrService::AVSessionNotifyUpdateNotification(int32_t uid, int32_t pid, bool isPublish)
{
    if (!CheckCallingToken()) {
        BGTASK_LOGW("AVSessionNotifyUpdateNotification not allowed");
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    return BgContinuousTaskMgr::GetInstance()->AVSessionNotifyUpdateNotification(uid, pid, isPublish);
}

ErrCode BackgroundTaskMgrService::SetBgTaskConfig(const std::string &configData, int32_t sourceType)
{
    BgTaskHiTraceChain traceChain(__func__);
    if (!CheckCallingToken()) {
        BGTASK_LOGW("SetBgTaskConfig not allowed");
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    return DelayedSingleton<BgTransientTaskMgr>::GetInstance()->SetBgTaskConfig(configData, sourceType);
}

ErrCode BackgroundTaskMgrService::SuspendContinuousAudioTask(int32_t uid)
{
    if (!CheckCallingToken() || !CheckCallingProcess()) {
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    BgContinuousTaskMgr::GetInstance()->SuspendContinuousAudioTask(uid);
    return ERR_OK;
}

ErrCode BackgroundTaskMgrService::IsModeSupported(const ContinuousTaskParam &taskParam)
{
    if (CheckAtomicService()) {
        pid_t callingPid = IPCSkeleton::GetCallingPid();
        pid_t callingUid = IPCSkeleton::GetCallingUid();
        BGTASK_LOGE("uid %{public}d pid %{public}d Check atomisc service fail, IsModeSupported not allowed",
            callingUid, callingPid);
        return ERR_BGTASK_PERMISSION_DENIED;
    }

    auto paramPtr = sptr<ContinuousTaskParam>(new ContinuousTaskParam(taskParam));
    return BgContinuousTaskMgr::GetInstance()->IsModeSupported(paramPtr);
}

ErrCode BackgroundTaskMgrService::SetSupportedTaskKeepingProcesses(const std::set<std::string> &processSet)
{
    if (!CheckCallingToken() || !CheckCallingProcess()) {
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    DelayedSingleton<BgtaskConfig>::GetInstance()->SetSupportedTaskKeepingProcesses(processSet);
    return ERR_OK;
}

ErrCode BackgroundTaskMgrService::SetMaliciousAppConfig(const std::set<std::string> &maliciousAppSet)
{
    if (!CheckCallingToken() || !CheckCallingProcess()) {
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    DelayedSingleton<BgtaskConfig>::GetInstance()->SetMaliciousAppConfig(maliciousAppSet);
    return ERR_OK;
}

ErrCode BackgroundTaskMgrService::RequestAuthFromUser(const ContinuousTaskParam &taskParam,
    const sptr<IExpiredCallback> &callback, int32_t &notificationId)
{
    if (CheckAtomicService()) {
        pid_t callingPid = IPCSkeleton::GetCallingPid();
        pid_t callingUid = IPCSkeleton::GetCallingUid();
        BGTASK_LOGE("uid %{public}d pid %{public}d Check atomisc service fail, IsModeSupported not allowed",
            callingUid, callingPid);
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    Security::AccessToken::AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenFlag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenFlag != Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    auto paramPtr = sptr<ContinuousTaskParam>(new ContinuousTaskParam(taskParam));
    return BgContinuousTaskMgr::GetInstance()->RequestAuthFromUser(paramPtr, callback, notificationId);
}

ErrCode BackgroundTaskMgrService::CheckSpecialScenarioAuth(int32_t appIndex, uint32_t &authResult)
{
    if (CheckAtomicService()) {
        pid_t callingPid = IPCSkeleton::GetCallingPid();
        pid_t callingUid = IPCSkeleton::GetCallingUid();
        BGTASK_LOGE("uid %{public}d pid %{public}d Check atomisc service fail, IsModeSupported not allowed",
            callingUid, callingPid);
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    Security::AccessToken::AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenFlag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenFlag != Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    return BgContinuousTaskMgr::GetInstance()->CheckSpecialScenarioAuth(appIndex, authResult);
}

ErrCode BackgroundTaskMgrService::CheckTaskAuthResult(const std::string &bundleName, int32_t userId, int32_t appIndex)
{
    if (!CheckCallingToken() || !CheckCallingProcess()) {
        BGTASK_LOGW("CheckTaskAuthResult not allowed");
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    return BgContinuousTaskMgr::GetInstance()->CheckTaskAuthResult(bundleName, userId, appIndex);
}

ErrCode BackgroundTaskMgrService::EnableContinuousTaskRequest(int32_t uid, bool isEnable)
{
    if (!CheckCallingToken() || !CheckCallingProcess()) {
        BGTASK_LOGW("EnableContinuousTaskRequest not allowed");
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    return BgContinuousTaskMgr::GetInstance()->EnableContinuousTaskRequest(uid, isEnable);
}

ErrCode BackgroundTaskMgrService::SetBackgroundTaskState(const BackgroundTaskStateInfo &taskParam)
{
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    if (CheckAtomicService()) {
        pid_t callingPid = IPCSkeleton::GetCallingPid();
        BGTASK_LOGE("uid %{public}d pid %{public}d Check atomisc service fail, SetBackgroundTaskState not allowed",
            callingUid, callingPid);
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    Security::AccessToken::AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenFlag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenFlag != Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        BGTASK_LOGE("uid %{public}d is not hap, SetBackgroundTaskState not allowed", callingUid);
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    if (!BundleManagerHelper::GetInstance()->CheckPermission(SET_BACKGROUND_TASK_STATE_PERMISSION)) {
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!BundleManagerHelper::GetInstance()->IsSystemApp(fullTokenId)) {
        BGTASK_LOGE("uid %{public}d is not system hap, SetBackgroundTaskState not allowed", callingUid);
        return ERR_BGTASK_NOT_SYSTEM_APP;
    }
    std::shared_ptr<BackgroundTaskStateInfo> taskState = std::make_shared<BackgroundTaskStateInfo>(
        taskParam.GetUserId(), taskParam.GetBundleName(), taskParam.GetAppIndex(), taskParam.GetUserAuthResult());
    return BgContinuousTaskMgr::GetInstance()->SetBackgroundTaskState(taskState);
}

ErrCode BackgroundTaskMgrService::GetBackgroundTaskState(const BackgroundTaskStateInfo &taskParam,
    uint32_t &authResult)
{
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    if (CheckAtomicService()) {
        pid_t callingPid = IPCSkeleton::GetCallingPid();
        BGTASK_LOGE("uid %{public}d pid %{public}d Check atomisc service fail, GetBackgroundTaskState not allowed",
            callingUid, callingPid);
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    Security::AccessToken::AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenFlag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenFlag != Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        BGTASK_LOGE("uid %{public}d is not hap, GetBackgroundTaskState not allowed", callingUid);
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    if (!BundleManagerHelper::GetInstance()->CheckPermission(SET_BACKGROUND_TASK_STATE_PERMISSION)) {
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!BundleManagerHelper::GetInstance()->IsSystemApp(fullTokenId)) {
        BGTASK_LOGE("uid %{public}d is not system hap, GetBackgroundTaskState not allowed", callingUid);
        return ERR_BGTASK_NOT_SYSTEM_APP;
    }
    std::shared_ptr<BackgroundTaskStateInfo> taskState = std::make_shared<BackgroundTaskStateInfo>(
        taskParam.GetUserId(), taskParam.GetBundleName(), taskParam.GetAppIndex());
    return BgContinuousTaskMgr::GetInstance()->GetBackgroundTaskState(taskState, authResult);
}

ErrCode BackgroundTaskMgrService::GetAllContinuousTasksBySystem(
    std::vector<std::shared_ptr<ContinuousTaskInfo>> &list)
{
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    if (CheckAtomicService()) {
        pid_t callingPid = IPCSkeleton::GetCallingPid();
        BGTASK_LOGE("uid %{public}d pid %{public}d Check atomisc service fail, GetAllContinuousTasksBySystem"
            " not allowed", callingUid, callingPid);
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    Security::AccessToken::AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenFlag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenFlag != Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        BGTASK_LOGE("uid %{public}d is not hap, GetAllContinuousTasksBySystem not allowed", callingUid);
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    if (!BundleManagerHelper::GetInstance()->CheckPermission(GET_BACKGROUND_TASK_INFO_PERMISSION)) {
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!BundleManagerHelper::GetInstance()->IsSystemApp(fullTokenId)) {
        BGTASK_LOGE("uid %{public}d is not system hap, GetAllContinuousTasksBySystem not allowed", callingUid);
        return ERR_BGTASK_NOT_SYSTEM_APP;
    }
    return BgContinuousTaskMgr::GetInstance()->GetAllContinuousTasksBySystem(list);
}

ErrCode BackgroundTaskMgrService::SetSpecialExemptedProcess(const std::set<std::string> &bundlNameSet)
{
    if (!CheckCallingToken() || !CheckCallingProcess()) {
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    DelayedSingleton<BgtaskConfig>::GetInstance()->SetSpecialExemptedProcess(bundlNameSet);
    return ERR_OK;
}

bool BackgroundTaskMgrService::CheckAtomicService()
{
    uint64_t tokenId = IPCSkeleton::GetCallingFullTokenID();
    return Security::AccessToken::AccessTokenKit::IsAtomicServiceByFullTokenID(tokenId);
}

bool BackgroundTaskMgrService::AllowDump()
{
    if (ENG_MODE == 0) {
        BGTASK_LOGE("Not eng mode");
        return false;
    }
    Security::AccessToken::AccessTokenID tokenId = IPCSkeleton::GetFirstTokenID();
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId, "ohos.permission.DUMP");
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        BGTASK_LOGE("CheckPermission failed");
        return false;
    }
    return true;
}

int32_t BackgroundTaskMgrService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    if (!AllowDump()) {
        return ERR_BGTASK_PERMISSION_DENIED;
    }
    std::vector<std::string> argsInStr;
    std::transform(args.begin(), args.end(), std::back_inserter(argsInStr),
        [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });
    std::string result;

    int32_t ret = ERR_OK;

    if (argsInStr.size() == NO_DUMP_PARAM_NUMS) {
        DumpUsage(result);
    } else {
        std::vector<std::string> infos;
        if (argsInStr[0] == "-h") {
            DumpUsage(result);
        } else if (argsInStr[0] == "-T") {
            ret = DelayedSingleton<BgTransientTaskMgr>::GetInstance()->ShellDump(argsInStr, infos);
        } else if (argsInStr[0] == "-C") {
            ret = BgContinuousTaskMgr::GetInstance()->ShellDump(argsInStr, infos);
        } else if (argsInStr[0] == "-E") {
            ret = DelayedSingleton<BgEfficiencyResourcesMgr>::GetInstance()->ShellDump(argsInStr, infos);
        } else {
            infos.emplace_back("Error params.\n");
            ret = ERR_BGTASK_INVALID_PARAM;
        }
        for (auto info : infos) {
            result.append(info);
        }
    }

    if (!SaveStringToFd(fd, result)) {
        BGTASK_LOGE("BackgroundTaskMgrService dump save string to fd failed!");
        ret = ERR_BGTASK_METHOD_CALLED_FAILED;
    }
    return ret;
}

void BackgroundTaskMgrService::DumpUsage(std::string &result)
{
    std::string dumpHelpMsg =
    "usage: bgtask dump [<options>]\n"
    "options list:\n"
    "    -h                                   help menu\n"
    "    -T                                   transient task commands:\n"
    "        BATTARY_LOW                          battary low mode\n"
    "        BATTARY_OKAY                         battary okay mode\n"
    "        DUMP_CANCEL                          cancel dump mode\n"
    "        All                                  list all request\n"
    "    -C                                   continuous task commands:\n"
    "        --all                                list all running continuous task infos\n"
    "        --cancel_all                         cancel all running continuous task\n"
    "        --cancel {continuous task key}       cancel one task by specifying task key\n"
    "    -E                                   efficiency resources commands;\n"
    "        --all                                list all efficiency resource aplications\n"
    "        --reset_all                          reset all efficiency resource aplications\n"
    "        --resetapp {uid} {resources}          reset one application of uid by specifying \n"
    "        --resetproc {pid} {resources}         reset one application of pid by specifying \n";

    result.append(dumpHelpMsg);
}  // namespace
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
