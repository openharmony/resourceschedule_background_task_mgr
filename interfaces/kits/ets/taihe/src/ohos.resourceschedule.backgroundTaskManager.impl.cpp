/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "ohos.resourceschedule.backgroundTaskManager.proj.hpp"
#include "ohos.resourceschedule.backgroundTaskManager.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "background_task_manager.h"
#include "singleton.h"
#include "bgtaskmgr_inner_errors.h"
#include "background_task_mgr_helper.h"
#include "callback_instance.h"
#include "ani_base_context.h"
#include "bgtaskmgr_log_wrapper.h"
#include "ability.h"
#include "ani_common_want_agent.h"
#include "common.h"
#include "ani_backgroundtask_subscriber.h"
#include "background_mode.h"
#include "background_sub_mode.h"
#include "background_task_submode.h"
#include "background_task_mode.h"
#include "background_task_state_info.h"

using namespace taihe;
using namespace OHOS;
using namespace ohos::resourceschedule::backgroundTaskManager;
using namespace OHOS::BackgroundTaskMgr;
using TaskSubModeType = ::ohos::resourceschedule::backgroundTaskManager::BackgroundTaskSubmode;
using TaskModeType = ::ohos::resourceschedule::backgroundTaskManager::BackgroundTaskMode;
using JsBackgroundTaskSubscriberType = ::ohos::resourceschedule::backgroundTaskManager::BackgroundTaskSubscriber;

namespace {
// To be implemented.
std::map<int32_t, std::shared_ptr<Callback>> callbackInstances_;
std::mutex callbackLock_;
static constexpr uint32_t BG_MODE_ID_BEGIN = 1;
static constexpr uint32_t BG_MODE_ID_END = 9;
static constexpr uint32_t CONTINUOUS_TASK_CANCEL = 1 << 0;
static constexpr uint32_t CONTINUOUS_TASK_SUSPEND = 1 << 1;
static constexpr uint32_t CONTINUOUS_TASK_ACTIVE = 1 << 2;
static constexpr uint32_t SUBSCRIBER_BACKGROUND_TASK_STATE = 1 << 3;
static constexpr int32_t MAX_SPECIAL_TASK_NUMS = 1;
static std::shared_ptr<BackgroundTaskMgr::AniBackgroundTaskSubscriber> backgroundTaskSubscriber_ = nullptr;
std::mutex backgroundTaskSubscriberMutex_;

struct TransientTaskCallbackInfo {
    int32_t requestId = 0;
    int32_t delayTime = 0;
};

struct AllTransientTasksCallbackInfo {
    int32_t remainingQuota = 0; // out
    std::vector<std::shared_ptr<BackgroundTaskMgr::DelaySuspendInfo>> list {}; // out
};

struct ContinuousTaskCallbackInfo {
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext {nullptr};
    uint32_t bgMode {0};
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent {nullptr};
    std::vector<uint32_t> bgModes {};
    std::vector<uint32_t> bgSubModes {};
    bool isBatchApi {false};
    int32_t notificationId {-1}; // out
    int32_t continuousTaskId {-1}; // out
    uint32_t authResult {0}; // out
    bool isCallback = false;
    int32_t errCode = 0;
    std::vector<std::shared_ptr<BackgroundTaskMgr::ContinuousTaskInfo>> list; // out
};

struct AllEfficiencyResourcesCallbackInfo {
    std::vector<std::shared_ptr<EfficiencyResourceInfo>> efficiencyResourceInfoList;    // out
};

static std::vector<std::string> g_backgroundModes = {
    "dataTransfer",
    "audioPlayback",
    "audioRecording",
    "location",
    "bluetoothInteraction",
    "multiDeviceConnection",
    "wifiInteraction",
    "voip",
    "taskKeeping"
};

ani_status GetAbilityContext(ani_env *env, const ani_object &value,
    std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext)
{
    ani_boolean stageMode = false;
    ani_status status = AbilityRuntime::IsStageContext(env, value, stageMode);
    BGTASK_LOGD("is stage mode: %{public}s", stageMode ? "true" : "false");

    if (status != ANI_OK || !stageMode) {
        return ANI_ERROR;
    }
    BGTASK_LOGD("Getting context with stage model");
    auto context = AbilityRuntime::GetStageModeContext(env, value);
    if (!context) {
        BGTASK_LOGE("get context failed");
        return ANI_ERROR;
    }
    abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
    if (!abilityContext) {
        BGTASK_LOGE("get Stage model ability context failed");
        return ANI_ERROR;
    }
    return ANI_OK;
}

ani_status GetWantAgent(ani_env *env, const ani_object &value,
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> &wantAgent)
{
    AbilityRuntime::WantAgent::WantAgent *wantAgentPtr = nullptr;
    AppExecFwk::UnwrapWantAgent(env, value, reinterpret_cast<void **>(&wantAgentPtr));
    if (wantAgentPtr == nullptr) {
        BGTASK_LOGE("wantAgentPtr is nullptr");
        return ANI_ERROR;
    }
    wantAgent = std::make_shared<AbilityRuntime::WantAgent::WantAgent>(*wantAgentPtr);
    return ANI_OK;
}

bool CheckParam(ani_env *env, ContinuousTaskCallbackInfo *asyncCallbackInfo, uintptr_t context)
{
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("asyncCallbackInfo is nullptr");
        set_business_error(
            Common::FindErrCode(ERR_BGTASK_CHECK_TASK_PARAM), Common::FindErrMsg(ERR_BGTASK_CHECK_TASK_PARAM));
        return false;
    }
    if (GetAbilityContext(env, reinterpret_cast<ani_object>(context), asyncCallbackInfo->abilityContext) != ANI_OK) {
        BGTASK_LOGE("get ability failed");
        asyncCallbackInfo->errCode = ERR_CONTEXT_NULL_OR_TYPE_ERR;
        set_business_error(
            Common::FindErrCode(asyncCallbackInfo->errCode), Common::FindErrMsg(asyncCallbackInfo->errCode));
        return false;
    }

    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    if (info == nullptr) {
        BGTASK_LOGE("ability info is nullptr");
        asyncCallbackInfo->errCode = ERR_ABILITY_INFO_EMPTY;
        set_business_error(
            Common::FindErrCode(asyncCallbackInfo->errCode), Common::FindErrMsg(asyncCallbackInfo->errCode));
        return false;
    }

    sptr<IRemoteObject> token = asyncCallbackInfo->abilityContext->GetToken();
    if (!token) {
        BGTASK_LOGE("get ability token info failed");
        asyncCallbackInfo->errCode = ERR_GET_TOKEN_ERR;
        set_business_error(
            Common::FindErrCode(asyncCallbackInfo->errCode), Common::FindErrMsg(asyncCallbackInfo->errCode));
        return false;
    }
    return true;
}

ani_status GetModes(ani_env *env, const array_view<string> &bgModes, ContinuousTaskCallbackInfo *asyncCallbackInfo)
{
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("asyncCallbackInfo is nullptr");
        set_business_error(
            Common::FindErrCode(ERR_BGTASK_CHECK_TASK_PARAM), Common::FindErrMsg(ERR_BGTASK_CHECK_TASK_PARAM));
        return ANI_ERROR;
    }
    if (bgModes.size() == 0) {
        BGTASK_LOGE("get bgModes arraylen is 0");
        asyncCallbackInfo->errCode = ERR_BGMODE_NULL_OR_TYPE_ERR;
        set_business_error(
            Common::FindErrCode(asyncCallbackInfo->errCode), Common::FindErrMsg(asyncCallbackInfo->errCode));
        return ANI_ERROR;
    }
    std::vector<string> bgModesVector(bgModes.begin(), bgModes.end());
    for (const auto &iter : bgModesVector) {
        auto it = std::find(g_backgroundModes.begin(), g_backgroundModes.end(), iter);
        if (it != g_backgroundModes.end()) {
            auto index = std::distance(g_backgroundModes.begin(), it);
            auto modeIter = std::find(asyncCallbackInfo->bgModes.begin(), asyncCallbackInfo->bgModes.end(), index + 1);
            if (modeIter == asyncCallbackInfo->bgModes.end()) {
                asyncCallbackInfo->bgModes.push_back(index + 1);
            }
        } else {
            BGTASK_LOGE("mode string is invalid");
            asyncCallbackInfo->errCode = ERR_BGMODE_NULL_OR_TYPE_ERR;
            set_business_error(
                Common::FindErrCode(asyncCallbackInfo->errCode), Common::FindErrMsg(asyncCallbackInfo->errCode));
            return ANI_ERROR;
        }
    }
    return ANI_OK;
}

bool CheckBackgroundMode(ani_env *env, ContinuousTaskCallbackInfo *asyncCallbackInfo)
{
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("asyncCallbackInfo is nullptr");
        set_business_error(
            Common::FindErrCode(ERR_BGTASK_CHECK_TASK_PARAM), Common::FindErrMsg(ERR_BGTASK_CHECK_TASK_PARAM));
        return false;
    }
    if (!asyncCallbackInfo->isBatchApi) {
        if (asyncCallbackInfo->bgMode < BG_MODE_ID_BEGIN || asyncCallbackInfo->bgMode > BG_MODE_ID_END) {
            BGTASK_LOGE("request background mode id: %{public}u out of range", asyncCallbackInfo->bgMode);
            asyncCallbackInfo->errCode = ERR_BGMODE_RANGE_ERR;
            set_business_error(
                Common::FindErrCode(asyncCallbackInfo->errCode), Common::FindErrMsg(asyncCallbackInfo->errCode));
            return false;
        }
    } else {
        for (unsigned int i = 0; i < asyncCallbackInfo->bgModes.size(); i++) {
            if (asyncCallbackInfo->bgModes[i] < BG_MODE_ID_BEGIN || asyncCallbackInfo->bgModes[i] > BG_MODE_ID_END) {
                BGTASK_LOGE("request background mode id: %{public}u out of range", asyncCallbackInfo->bgModes[i]);
                asyncCallbackInfo->errCode = ERR_BGMODE_RANGE_ERR;
                set_business_error(
                    Common::FindErrCode(asyncCallbackInfo->errCode), Common::FindErrMsg(asyncCallbackInfo->errCode));
                return false;
            }
        }
    }
    return true;
}

class ContinuousTaskRequestImpl {
public:
    ContinuousTaskRequestImpl()
    {}

    ~ContinuousTaskRequestImpl()
    {}

    ::taihe::array_view<TaskModeType> getBackgroundTaskModes()
    {
        return taiheBackgroundTaskModes_;
    }

    void setBackgroundTaskModes(::taihe::array_view<TaskModeType> backgroundTaskModes)
    {
        if (backgroundTaskModes.size() == 0) {
            return;
        }
        taiheBackgroundTaskModes_ = backgroundTaskModes;
        std::vector<uint32_t> bgModesVector(backgroundTaskModes.begin(), backgroundTaskModes.end());
        backgroundTaskModes_ = bgModesVector;
    }

    ::taihe::array_view<TaskSubModeType> getBackgroundTaskSubmodes()
    {
        return taiheBackgroundTaskSubmodes_;
    }

    void setBackgroundTaskSubmodes(::taihe::array_view<TaskSubModeType> backgroundTaskSubmodes)
    {
        if (backgroundTaskSubmodes.size() == 0) {
            return;
        }
        taiheBackgroundTaskSubmodes_ = backgroundTaskSubmodes;
        std::vector<uint32_t> bgSubModesVector(backgroundTaskSubmodes.begin(), backgroundTaskSubmodes.end());
        backgroundTaskSubmodes_ = bgSubModesVector;
    }

    uintptr_t getWantAgent()
    {
        return taiheWantAgent_;
    }

    void setWantAgent(uintptr_t wantAgent)
    {
        taiheWantAgent_ = wantAgent;
        auto env = taihe::get_env();
        AbilityRuntime::WantAgent::WantAgent *wantAgentPtr = nullptr;
        AppExecFwk::UnwrapWantAgent(env, reinterpret_cast<ani_object>(wantAgent),
            reinterpret_cast<void **>(&wantAgentPtr));
        if (wantAgentPtr == nullptr) {
            BGTASK_LOGE("wantAgentPtr is nullptr");
            return;
        }
        wantAgent_ = std::make_shared<AbilityRuntime::WantAgent::WantAgent>(*wantAgentPtr);
    }

    optional<bool> getCombinedTaskNotification()
    {
        return optional<bool>(std::in_place, combinedTaskNotification_);
    }

    void setCombinedTaskNotification(optional<bool> combinedTaskNotification)
    {
        if (combinedTaskNotification.has_value()) {
            combinedTaskNotification_ = static_cast<bool>(combinedTaskNotification.value());
        }
    }

    optional<int32_t> getContinuousTaskId()
    {
        return optional<int32_t>(std::in_place, continuousTaskId_);
    }

    void setContinuousTaskId(optional<int32_t> continuousTaskId)
    {
        if (continuousTaskId.has_value()) {
            continuousTaskId_ = static_cast<int32_t>(continuousTaskId.value());
        }
    }

    bool isModeSupported()
    {
        if (backgroundTaskModes_.size() == 0) {
            set_business_error(Common::FindErrCode(ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_IS_EMPTY),
                Common::FindErrMsg(ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_IS_EMPTY));
            return false;
        }
        int32_t specialModeSize = std::count(backgroundTaskModes_.begin(), backgroundTaskModes_.end(),
            OHOS::BackgroundTaskMgr::BackgroundTaskMode::MODE_SPECIAL_SCENARIO_PROCESSING);
        if (specialModeSize > MAX_SPECIAL_TASK_NUMS) {
            set_business_error(Common::FindErrCode(ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_ONLY_ALLOW_ONE_APPLICATION),
                Common::FindErrMsg(ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_ONLY_ALLOW_ONE_APPLICATION));
            return false;
        }
        if (specialModeSize == MAX_SPECIAL_TASK_NUMS && backgroundTaskModes_.size() > MAX_SPECIAL_TASK_NUMS) {
            set_business_error(Common::FindErrCode(ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_CONFLICTS_WITH_OTHER_TASK),
                Common::FindErrMsg(ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_CONFLICTS_WITH_OTHER_TASK));
            return false;
        }
        ContinuousTaskParam taskParam = ContinuousTaskParam(true, -1,
            nullptr, "", nullptr, "", true, backgroundTaskModes_, -1);
        int32_t errCode = OHOS::BackgroundTaskMgr::BackgroundTaskMgrHelper::IsModeSupported(taskParam);
        if (errCode) {
            BGTASK_LOGE("isModeSupported failed errCode: %{public}d", Common::FindErrCode(errCode));
            set_business_error(Common::FindErrCode(errCode), Common::FindErrMsg(errCode));
            return false;
        }
        return true;
    }

    void requestAuthFromUser(uintptr_t context, ::taihe::callback_view<void(
        ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult data)> callback)
    {
        auto env = taihe::get_env();
        std::unique_ptr<ContinuousTaskCallbackInfo> asyncCallbackInfo = std::make_unique<ContinuousTaskCallbackInfo>();
        if (!CheckModesInner(asyncCallbackInfo.get())) {
            BGTASK_LOGE("check mode or subMode failed");
            set_business_error(Common::FindErrCode(asyncCallbackInfo->errCode),
                Common::FindErrMsg(asyncCallbackInfo->errCode));
            return;
        }
        if (!CheckParam(env, asyncCallbackInfo.get(), context)) {
            BGTASK_LOGE("check param failed");
            return;
        }
        std::shared_ptr<OHOS::BackgroundTaskMgr::AuthCallbackType> taiheCallback =
            std::make_shared<OHOS::BackgroundTaskMgr::AuthCallbackType>(callback);
        std::shared_ptr<Callback> callbackPtr = std::make_shared<Callback>();
        callbackPtr->Init();
        callbackPtr->SetAuthCallbackInfo(taiheCallback);
        ContinuousTaskParam taskParam = ContinuousTaskParam(true, asyncCallbackInfo->bgMode,
            nullptr, "", nullptr, "", true, asyncCallbackInfo->bgModes, -1);
        taskParam.isByRequestObject_ = true;
        taskParam.bgSubModeIds_ = asyncCallbackInfo->bgSubModes;
        const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
        taskParam.appIndex_ = info->appIndex;
        int32_t notificationId = 0;
        asyncCallbackInfo->errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->
            RequestAuthFromUser(taskParam, *callbackPtr, notificationId);
        if (asyncCallbackInfo->errCode) {
            set_business_error(Common::FindErrCode(asyncCallbackInfo->errCode),
                Common::FindErrMsg(asyncCallbackInfo->errCode));
        } else {
            std::lock_guard<std::mutex> lock(callbackLock_);
            callbackInstances_[notificationId] = callbackPtr;
        }
    }

    ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult checkSpecialScenarioAuthSync(uintptr_t context)
    {
        auto env = taihe::get_env();
        std::unique_ptr<ContinuousTaskCallbackInfo> asyncCallbackInfo = std::make_unique<ContinuousTaskCallbackInfo>();
        if (!CheckParam(env, asyncCallbackInfo.get(), context)) {
            BGTASK_LOGE("check param failed");
            return ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::NOT_DETERMINED;
        }
        const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
        asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::CheckSpecialScenarioAuth(info->appIndex,
            asyncCallbackInfo->authResult);
        if (asyncCallbackInfo->errCode) {
            BGTASK_LOGE("checkSpecialScenarioAuth failed errCode: %{public}d",
                Common::FindErrCode(asyncCallbackInfo->errCode));
            set_business_error(Common::FindErrCode(asyncCallbackInfo->errCode),
                Common::FindErrMsg(asyncCallbackInfo->errCode));
            return ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::NOT_DETERMINED;
        }
        ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t authResultRet;
        switch (asyncCallbackInfo->authResult) {
            case OHOS::BackgroundTaskMgr::UserAuthResult::NOT_SUPPORTED:
                authResultRet = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::NOT_SUPPORTED;
                break;
            case OHOS::BackgroundTaskMgr::UserAuthResult::NOT_DETERMINED:
                authResultRet = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::NOT_DETERMINED;
                break;
            case OHOS::BackgroundTaskMgr::UserAuthResult::DENIED:
                authResultRet = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::DENIED;
                break;
            case OHOS::BackgroundTaskMgr::UserAuthResult::GRANTED_ONCE:
                authResultRet = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::GRANTED_ONCE;
                break;
            case OHOS::BackgroundTaskMgr::UserAuthResult::GRANTED_ALWAYS:
                authResultRet = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::GRANTED_ALWAYS;
                break;
            default:
                authResultRet = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::NOT_DETERMINED;
                break;
        }
        return authResultRet;
    }

    int64_t GetInner()
    {
        return reinterpret_cast<int64_t>(this);
    }

    bool CheckModesInner(ContinuousTaskCallbackInfo *asyncCallbackInfo)
    {
        if (backgroundTaskModes_.size() == 0 || backgroundTaskSubmodes_.size() == 0) {
            asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_IS_EMPTY;
            return false;
        }
        if (backgroundTaskModes_.size() != backgroundTaskSubmodes_.size()) {
            asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_LENGTH_MISMATCH;
            return false;
        }
        int32_t specialModeSize = std::count(backgroundTaskModes_.begin(), backgroundTaskModes_.end(),
            OHOS::BackgroundTaskMgr::BackgroundTaskMode::MODE_SPECIAL_SCENARIO_PROCESSING);
        if (specialModeSize > MAX_SPECIAL_TASK_NUMS) {
            asyncCallbackInfo->errCode = ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_ONLY_ALLOW_ONE_APPLICATION;
            return false;
        }
        if (specialModeSize == MAX_SPECIAL_TASK_NUMS && backgroundTaskModes_.size() > MAX_SPECIAL_TASK_NUMS) {
            asyncCallbackInfo->errCode = ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_CONFLICTS_WITH_OTHER_TASK;
            return false;
        }
        for (uint32_t index = 0; index < backgroundTaskSubmodes_.size(); index++) {
            uint32_t subMode = backgroundTaskSubmodes_[index];
            if (subMode >= OHOS::BackgroundTaskMgr::BackgroundTaskSubmode::END ||
                subMode < OHOS::BackgroundTaskMgr::BackgroundTaskSubmode::SUBMODE_CAR_KEY_NORMAL_NOTIFICATION) {
                asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_CROSS_BORDER;
                return false;
            }
            if (subMode == OHOS::BackgroundTaskMgr::BackgroundTaskSubmode::SUBMODE_NORMAL_NOTIFICATION) {
                if (!OHOS::BackgroundTaskMgr::BackgroundTaskMode::IsModeTypeMatching(backgroundTaskModes_[index])) {
                    asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_TYPE_MISMATCH;
                    return false;
                }
            } else {
                if (OHOS::BackgroundTaskMgr::BackgroundTaskMode::GetSubModeTypeMatching(subMode) !=
                    backgroundTaskModes_[index]) {
                    asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_TYPE_MISMATCH;
                    return false;
                }
            }
        }
        asyncCallbackInfo->bgModes = backgroundTaskModes_;
        asyncCallbackInfo->bgMode = backgroundTaskModes_[0];
        asyncCallbackInfo->bgSubModes = backgroundTaskSubmodes_;
        return true;
    }

public:
    int32_t continuousTaskId_ = -1;
    bool combinedTaskNotification_ = false;
    uintptr_t taiheWantAgent_ {0};
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent_ {nullptr};
    std::vector<uint32_t> backgroundTaskSubmodes_ {};
    std::vector<uint32_t> backgroundTaskModes_ {};
    ::taihe::array_view<TaskSubModeType> taiheBackgroundTaskSubmodes_;
    ::taihe::array_view<TaskModeType> taiheBackgroundTaskModes_;
};

void CancelSuspendDelay(int32_t requestId)
{
    ErrCode errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->CancelSuspendDelay(requestId);
    if (errCode != ERR_OK) {
        BGTASK_LOGE("CancelSuspendDelay failed errCode: %{public}d", Common::FindErrCode(errCode));
        set_business_error(Common::FindErrCode(errCode), Common::FindErrMsg(errCode));
    }
    std::lock_guard<std::mutex> lock(callbackLock_);
    auto findCallback = callbackInstances_.find(requestId);
    if (findCallback != callbackInstances_.end()) {
        callbackInstances_.erase(findCallback);
    }
}

int32_t GetRemainingDelayTimeSync(int32_t requestId)
{
    TransientTaskCallbackInfo callbackInfo;
    callbackInfo.requestId = requestId;
    ErrCode errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        GetRemainingDelayTime(callbackInfo.requestId, callbackInfo.delayTime);
    if (errCode) {
        BGTASK_LOGE("GetRemainingDelayTime failed errCode: %{public}d", Common::FindErrCode(errCode));
        set_business_error(Common::FindErrCode(errCode), Common::FindErrMsg(errCode));
    }
    return callbackInfo.delayTime;
}

::ohos::resourceschedule::backgroundTaskManager::DelaySuspendInfo RequestSuspendDelay(
    string_view reason, callback_view<void(UndefinedType const&)> callback)
{
    std::string tmp(reason);
    std::u16string reasonU16(OHOS::Str8ToStr16(tmp));
    std::shared_ptr<BackgroundTaskMgr::DelaySuspendInfo> delaySuspendInfo {nullptr};
    std::shared_ptr<Callback> callbackPtr = std::make_shared<Callback>();
    callbackPtr->Init();
    callbackPtr->SetCallbackInfo(callback);

    ErrCode errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        RequestSuspendDelay(reasonU16, *callbackPtr, delaySuspendInfo);
    if (errCode) {
        BGTASK_LOGE("DelaySuspendInfo failed errCode: %{public}d", Common::FindErrCode(errCode));
        set_business_error(Common::FindErrCode(errCode), Common::FindErrMsg(errCode));
    }

    ::ohos::resourceschedule::backgroundTaskManager::DelaySuspendInfo resultInfo;
    if (delaySuspendInfo) {
        resultInfo.requestId = delaySuspendInfo->GetRequestId();
        resultInfo.actualDelayTime = delaySuspendInfo->GetActualDelayTime();
        std::lock_guard<std::mutex> lock(callbackLock_);
        callbackInstances_[delaySuspendInfo->GetRequestId()] = callbackPtr;
    }
    return resultInfo;
}

void ApplyEfficiencyResources(EfficiencyResourcesRequest const& request)
{
    EfficiencyResourceInfo resourceInfo{
        static_cast<uint32_t>(request.resourceTypes),
        request.isApply,
        static_cast<uint32_t>(request.timeOut),
        std::string(request.reason),
        request.isPersist.has_value() ? request.isPersist.value() : false,
        request.isProcess.has_value() ? request.isProcess.value() : false
    };
    if (request.cpuLevel.has_value()) {
        resourceInfo.SetCpuLevel(request.cpuLevel.value().get_value());
    }
    ErrCode errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->ApplyEfficiencyResources(resourceInfo);
    if (errCode) {
        BGTASK_LOGE("ApplyEfficiencyResources failed errCode: %{public}d", Common::FindErrCode(errCode));
        set_business_error(Common::FindErrCode(errCode), Common::FindErrMsg(errCode));
    }
}

void ResetAllEfficiencyResources()
{
    ErrCode errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->ResetAllEfficiencyResources();
    if (errCode) {
        BGTASK_LOGE("ResetAllEfficiencyResources failed errCode: %{public}d", Common::FindErrCode(errCode));
        set_business_error(Common::FindErrCode(errCode), Common::FindErrMsg(errCode));
    }
}

void SetBackgroundTaskState(::ohos::resourceschedule::backgroundTaskManager::BackgroundTaskStateInfo const& stateInfo)
{
    if (!stateInfo.authResult.has_value()) {
        set_business_error(Common::FindErrCode(ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_PARAM_INVALID),
            Common::FindErrMsg(ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_PARAM_INVALID));
        return;
    }
    int32_t authValue =  static_cast<int32_t>(stateInfo.authResult.value());
    int32_t userIdValue =  static_cast<int32_t>(stateInfo.userId);
    int32_t appIndexValue =  static_cast<int32_t>(stateInfo.appIndex);
    std::string bundleNameValue = static_cast<std::string>(stateInfo.bundleName);
    if (userIdValue < 0 || appIndexValue < 0 || bundleNameValue == "") {
        set_business_error(Common::FindErrCode(ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_PARAM_INVALID),
            Common::FindErrMsg(ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_PARAM_INVALID));
        return;
    }
    std::shared_ptr<OHOS::BackgroundTaskMgr::BackgroundTaskStateInfo> taskState =
        std::make_shared<OHOS::BackgroundTaskMgr::BackgroundTaskStateInfo>(userIdValue, bundleNameValue, appIndexValue,
        authValue
    );
    ErrCode errCode = BackgroundTaskMgrHelper::SetBackgroundTaskState(taskState);
    if (errCode) {
        set_business_error(Common::FindErrCode(errCode), Common::FindErrMsg(errCode));
    }
}

::ohos::resourceschedule::backgroundTaskManager::UserAuthResult GetBackgroundTaskState(
    ::ohos::resourceschedule::backgroundTaskManager::BackgroundTaskStateInfo const& stateInfo)
{
    int32_t userId =  static_cast<int32_t>(stateInfo.userId);
    int32_t appIndex =  static_cast<int32_t>(stateInfo.appIndex);
    std::string bundleName = static_cast<std::string>(stateInfo.bundleName);
    if (userId < 0 || appIndex < 0 || bundleName == "") {
        set_business_error(Common::FindErrCode(ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_PARAM_INVALID),
            Common::FindErrMsg(ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_PARAM_INVALID));
        return ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::NOT_DETERMINED;
    }
    std::shared_ptr<OHOS::BackgroundTaskMgr::BackgroundTaskStateInfo> taskState =
        std::make_shared<OHOS::BackgroundTaskMgr::BackgroundTaskStateInfo>(userId, bundleName, appIndex);
    ErrCode errCode = BackgroundTaskMgrHelper::GetBackgroundTaskState(taskState);
    if (errCode) {
        set_business_error(Common::FindErrCode(errCode), Common::FindErrMsg(errCode));
        return ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::NOT_DETERMINED;
    }
    ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t authResult;
    switch (taskState->GetUserAuthResult()) {
        case OHOS::BackgroundTaskMgr::UserAuthResult::NOT_SUPPORTED:
            authResult = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::NOT_SUPPORTED;
            break;
        case OHOS::BackgroundTaskMgr::UserAuthResult::NOT_DETERMINED:
            authResult = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::NOT_DETERMINED;
            break;
        case OHOS::BackgroundTaskMgr::UserAuthResult::DENIED:
            authResult = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::DENIED;
            break;
        case OHOS::BackgroundTaskMgr::UserAuthResult::GRANTED_ONCE:
            authResult = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::GRANTED_ONCE;
            break;
        case OHOS::BackgroundTaskMgr::UserAuthResult::GRANTED_ALWAYS:
            authResult = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::GRANTED_ALWAYS;
            break;
        default:
            authResult = ::ohos::resourceschedule::backgroundTaskManager::UserAuthResult::key_t::NOT_DETERMINED;
            break;
    }
    return authResult;
}

void StopBackgroundRunningSync(uintptr_t context)
{
    auto env = taihe::get_env();
    std::unique_ptr<ContinuousTaskCallbackInfo> asyncCallbackInfo = std::make_unique<ContinuousTaskCallbackInfo>();
    if (!CheckParam(env, asyncCallbackInfo.get(), context)) {
        BGTASK_LOGE("check param failed");
        return;
    }
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    sptr<IRemoteObject> token = asyncCallbackInfo->abilityContext->GetToken();
    int32_t abilityId = asyncCallbackInfo->abilityContext->GetAbilityRecordId();

    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestStopBackgroundRunning(info->name, token, abilityId);
    if (asyncCallbackInfo->errCode) {
        BGTASK_LOGE("StopBackgroundRunning fail errCode: %{public}d", Common::FindErrCode(asyncCallbackInfo->errCode));
        set_business_error(
            Common::FindErrCode(asyncCallbackInfo->errCode), Common::FindErrMsg(asyncCallbackInfo->errCode));
    }
}

void StartBackgroundRunningSync(uintptr_t context,
    ::ohos::resourceschedule::backgroundTaskManager::BackgroundMode bgMode, uintptr_t wantAgent)
{
    auto env = taihe::get_env();
    std::unique_ptr<ContinuousTaskCallbackInfo> asyncCallbackInfo = std::make_unique<ContinuousTaskCallbackInfo>();
    if (!CheckParam(env, asyncCallbackInfo.get(), context)) {
        BGTASK_LOGE("check param failed");
        return;
    }
    if (GetWantAgent(env, reinterpret_cast<ani_object>(wantAgent), asyncCallbackInfo->wantAgent) != ANI_OK) {
        BGTASK_LOGE("Get ability failed");
        asyncCallbackInfo->errCode = ERR_WANTAGENT_NULL_OR_TYPE_ERR;
        set_business_error(
            Common::FindErrCode(asyncCallbackInfo->errCode), Common::FindErrMsg(asyncCallbackInfo->errCode));
        return;
    }
    sptr<IRemoteObject> token = asyncCallbackInfo->abilityContext->GetToken();
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    int32_t abilityId = asyncCallbackInfo->abilityContext->GetAbilityRecordId();

    asyncCallbackInfo->isBatchApi = false;
    asyncCallbackInfo->bgMode = static_cast<uint32_t>(bgMode);
    if (!CheckBackgroundMode(env, asyncCallbackInfo.get())) {
        BGTASK_LOGE("check backgroundMode failed");
        return;
    }
    ContinuousTaskParam taskParam = ContinuousTaskParam(true, asyncCallbackInfo->bgMode,
        asyncCallbackInfo->wantAgent, info->name, token, "", false, asyncCallbackInfo->bgModes, abilityId);
    taskParam.appIndex_ = info->appIndex;
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestStartBackgroundRunning(taskParam);
    asyncCallbackInfo->notificationId = taskParam.notificationId_;
    asyncCallbackInfo->continuousTaskId = taskParam.continuousTaskId_;
    BGTASK_LOGI("notification %{public}d, continuousTaskId %{public}d", taskParam.notificationId_,
        taskParam.continuousTaskId_);
    if (asyncCallbackInfo->errCode) {
        BGTASK_LOGE("StartBackgroundRunning fail errCode: %{public}d", Common::FindErrCode(asyncCallbackInfo->errCode));
        set_business_error(
            Common::FindErrCode(asyncCallbackInfo->errCode), Common::FindErrMsg(asyncCallbackInfo->errCode));
    }
}

static ani_enum_item GetSlotType(ani_env *env)
{
    if (env == nullptr) {
        BGTASK_LOGE("env is nullptr");
        return nullptr;
    }
    ani_enum enumType;
    if (ANI_OK != env->FindEnum("@ohos.notificationManager.notificationManager.SlotType", &enumType)) {
        BGTASK_LOGE("get slotType failed");
        return nullptr;
    }
    
    ani_enum_item enumItem;
    if (ANI_OK != env->Enum_GetEnumItemByName(enumType, "LIVE_VIEW", &enumItem)) {
        BGTASK_LOGE("get slotType item failed");
        return nullptr;
    }
    return enumItem;
}

static ani_enum_item GetContentType(ani_env *env)
{
    if (env == nullptr) {
        BGTASK_LOGE("env is nullptr");
        return nullptr;
    }
    ani_enum enumType;
    if (ANI_OK != env->FindEnum("@ohos.notificationManager.notificationManager.ContentType", &enumType)) {
        BGTASK_LOGE("get contentType failed");
        return nullptr;
    }
    
    ani_enum_item enumItem;
    if (ANI_OK != env->Enum_GetEnumItemByName(enumType, "NOTIFICATION_CONTENT_MULTILINE", &enumItem)) {
        BGTASK_LOGE("get contentType item failed");
        return nullptr;
    }
    return enumItem;
}

::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskNotification StartBackgroundRunningSync2(
    uintptr_t context, ::taihe::array_view<::taihe::string> bgModes, uintptr_t wantAgent)
{
    auto env = taihe::get_env();
    std::unique_ptr<ContinuousTaskCallbackInfo> asyncCallbackInfo = std::make_unique<ContinuousTaskCallbackInfo>();
    ::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskNotification notification;
    if (!CheckParam(env, asyncCallbackInfo.get(), context)) {
        BGTASK_LOGE("check param failed");
        return notification;
    }
    if (GetWantAgent(env, reinterpret_cast<ani_object>(wantAgent), asyncCallbackInfo->wantAgent) != ANI_OK) {
        BGTASK_LOGE("Get ability failed");
        asyncCallbackInfo->errCode = ERR_WANTAGENT_NULL_OR_TYPE_ERR;
        set_business_error(
            Common::FindErrCode(asyncCallbackInfo->errCode), Common::FindErrMsg(asyncCallbackInfo->errCode));
        return notification;
    }
    if (GetModes(env, bgModes, asyncCallbackInfo.get()) != ANI_OK) {
        BGTASK_LOGE("Get modes failed");
        return notification;
    }
    sptr<IRemoteObject> token = asyncCallbackInfo->abilityContext->GetToken();
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    int32_t abilityId = asyncCallbackInfo->abilityContext->GetAbilityRecordId();
    asyncCallbackInfo->isBatchApi = true;
    if (!CheckBackgroundMode(env, asyncCallbackInfo.get())) {
        BGTASK_LOGE("check backgroundMode failed");
        return notification;
    }

    ContinuousTaskParam taskParam = ContinuousTaskParam(true, asyncCallbackInfo->bgMode,
        asyncCallbackInfo->wantAgent, info->name, token, "", true, asyncCallbackInfo->bgModes, abilityId);
    taskParam.appIndex_ = info->appIndex;
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestStartBackgroundRunning(taskParam);
    asyncCallbackInfo->notificationId = taskParam.notificationId_;
    asyncCallbackInfo->continuousTaskId = taskParam.continuousTaskId_;
    if (asyncCallbackInfo->errCode) {
        BGTASK_LOGE("StartBackgroundRunning fail errCode: %{public}d", Common::FindErrCode(asyncCallbackInfo->errCode));
        set_business_error(
            Common::FindErrCode(asyncCallbackInfo->errCode), Common::FindErrMsg(asyncCallbackInfo->errCode));
        return notification;
    }
    if (!GetSlotType(env) || !GetContentType(env)) {
        return notification;
    }
    notification.slotType = (uintptr_t)GetSlotType(env);
    notification.contentType = (uintptr_t)GetContentType(env);
    notification.notificationId = taskParam.notificationId_;
    notification.continuousTaskId = optional<int32_t>(std::in_place, taskParam.continuousTaskId_);
    return notification;
}

::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskNotification UpdateBackgroundRunningSync(
    uintptr_t context, ::taihe::array_view<::taihe::string> bgModes)
{
    auto env = taihe::get_env();
    std::unique_ptr<ContinuousTaskCallbackInfo> asyncCallbackInfo = std::make_unique<ContinuousTaskCallbackInfo>();
    ::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskNotification notification;
    if (!CheckParam(env, asyncCallbackInfo.get(), context)) {
        BGTASK_LOGE("check param failed");
        return notification;
    }
    if (GetModes(env, bgModes, asyncCallbackInfo.get()) != ANI_OK) {
        BGTASK_LOGE("Get modes failed");
        return notification;
    }
    sptr<IRemoteObject> token = asyncCallbackInfo->abilityContext->GetToken();
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    int32_t abilityId = asyncCallbackInfo->abilityContext->GetAbilityRecordId();
    asyncCallbackInfo->isBatchApi = true;
    if (!CheckBackgroundMode(env, asyncCallbackInfo.get())) {
        BGTASK_LOGE("check backgroundMode failed");
        return notification;
    }

    ContinuousTaskParam taskParam = ContinuousTaskParam(true, asyncCallbackInfo->bgMode,
        nullptr, info->name, token, "", true, asyncCallbackInfo->bgModes, abilityId);
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestUpdateBackgroundRunning(taskParam);
    asyncCallbackInfo->notificationId = taskParam.notificationId_;
    asyncCallbackInfo->continuousTaskId = taskParam.continuousTaskId_;
    if (asyncCallbackInfo->errCode) {
        BGTASK_LOGE("StartBackgroundRunning fail errCode: %{public}d", Common::FindErrCode(asyncCallbackInfo->errCode));
        set_business_error(
            Common::FindErrCode(asyncCallbackInfo->errCode), Common::FindErrMsg(asyncCallbackInfo->errCode));
        return notification;
    }
    if (!GetSlotType(env) || !GetContentType(env)) {
        return notification;
    }
    notification.slotType = (uintptr_t)GetSlotType(env);
    notification.contentType = (uintptr_t)GetContentType(env);
    notification.notificationId = taskParam.notificationId_;
    notification.continuousTaskId = optional<int32_t>(std::in_place, taskParam.continuousTaskId_);
    return notification;
}

::ohos::resourceschedule::backgroundTaskManager::TransientTaskInfo GetTransientTaskInfoSync()
{
    ::ohos::resourceschedule::backgroundTaskManager::TransientTaskInfo resultInfo{
        .remainingQuota = 0,
        .transientTasks = {},
    };
    std::unique_ptr<AllTransientTasksCallbackInfo> asyncCallbackInfo =
        std::make_unique<AllTransientTasksCallbackInfo>();
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("input params error");
        return resultInfo;
    }
    ErrCode errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetAllTransientTasks(
        asyncCallbackInfo->remainingQuota, asyncCallbackInfo->list);
    if (errCode) {
        BGTASK_LOGE("transientTaskInfo failed errCode: %{public}d", Common::FindErrCode(errCode));
        set_business_error(Common::FindErrCode(errCode), Common::FindErrMsg(errCode));
    }
    std::vector<::ohos::resourceschedule::backgroundTaskManager::DelaySuspendInfo> aniInfoList;
    for (const auto& info : asyncCallbackInfo->list) {
        ::ohos::resourceschedule::backgroundTaskManager::DelaySuspendInfo aniInfo{
            .requestId = info->GetRequestId(),
            .actualDelayTime = info->GetActualDelayTime(),
        };
        aniInfoList.push_back(aniInfo);
    }
    auto aniTransientTasks = array<::ohos::resourceschedule::backgroundTaskManager::DelaySuspendInfo>{copy_data_t{},
        aniInfoList.data(), aniInfoList.size()};
    resultInfo.remainingQuota = asyncCallbackInfo->remainingQuota;
    resultInfo.transientTasks = aniTransientTasks;
    return resultInfo;
}

::array<taihe::string> GetAniBackgroundModes(const std::vector<uint32_t> &modes)
{
    if (modes.empty()) {
        return taihe::array<taihe::string>(nullptr, 0);
    }
    std::vector<taihe::string> aniModes;
    for (auto &iter : modes) {
        if (iter < BackgroundTaskMgr::BackgroundMode::END) {
            std::string modeStr = BackgroundTaskMgr::BackgroundMode::GetBackgroundModeStr(iter);
            aniModes.push_back(taihe::string(modeStr));
        }
    }
    array<taihe::string> modesArr(aniModes);
    return modesArr;
}

::array<taihe::string> GetAniBackgroundSubModes(const std::vector<uint32_t> &subModes)
{
    if (subModes.empty()) {
        return taihe::array<taihe::string>(nullptr, 0);
    }
    std::vector<taihe::string> aniModes;
    for (auto &iter : subModes) {
        if (iter < BackgroundTaskMgr::BackgroundSubMode::END) {
            std::string subModeStr = BackgroundTaskMgr::BackgroundSubMode::GetBackgroundSubModeStr(iter);
            aniModes.push_back(taihe::string(subModeStr));
        }
    }
    array<taihe::string> subModesArr(aniModes);
    return subModesArr;
}

array<::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskInfo> GetAllContinuousTasksSync(uintptr_t context)
{
    auto env = taihe::get_env();
    std::unique_ptr<ContinuousTaskCallbackInfo> asyncCallbackInfo = std::make_unique<ContinuousTaskCallbackInfo>();
    if (!CheckParam(env, asyncCallbackInfo.get(), context)) {
        BGTASK_LOGE("check param failed");
        return {};
    }
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestGetAllContinuousTasks(asyncCallbackInfo->list);
    if (asyncCallbackInfo->errCode) {
        BGTASK_LOGE("GetAllContinuousTasks fail errCode: %{public}d", Common::FindErrCode(asyncCallbackInfo->errCode));
        set_business_error(
            Common::FindErrCode(asyncCallbackInfo->errCode), Common::FindErrMsg(asyncCallbackInfo->errCode));
        return {};
    }
    std::vector<::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskInfo> aniInfoList;
    for (const auto& info : asyncCallbackInfo->list) {
        ::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskInfo aniInfo{
            .abilityName = info->GetAbilityName(),
            .uid = info->GetUid(),
            .pid = info->GetPid(),
            .isFromWebView = info->IsFromWebView(),
            .backgroundModes = GetAniBackgroundModes(info->GetBackgroundModes()),
            .backgroundSubModes = GetAniBackgroundSubModes(info->GetBackgroundSubModes()),
            .notificationId = info->GetNotificationId(),
            .continuousTaskId = info->GetContinuousTaskId(),
            .abilityId = info->GetAbilityId(),
            .wantAgentBundleName = info->GetWantAgentBundleName(),
            .wantAgentAbilityName = info->GetWantAgentAbilityName(),
            .suspendState = false,
            .bundleName = optional<::taihe::string>(std::in_place, info->GetBundleName()),
            .appIndex = optional<int32_t>(std::in_place, info->GetAppIndex())
        };
        aniInfoList.push_back(aniInfo);
    }
    return array<::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskInfo>{copy_data_t{},
        aniInfoList.data(), aniInfoList.size()};
}

array<::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskInfo> GetAllContinuousTasksSync2(
    uintptr_t context, bool includeSuspended)
{
    auto env = taihe::get_env();
    std::unique_ptr<ContinuousTaskCallbackInfo> asyncCallbackInfo = std::make_unique<ContinuousTaskCallbackInfo>();
    if (!CheckParam(env, asyncCallbackInfo.get(), context)) {
        BGTASK_LOGE("check param failed");
        return {};
    }
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestGetAllContinuousTasks(
        asyncCallbackInfo->list, includeSuspended);
    if (asyncCallbackInfo->errCode) {
        BGTASK_LOGE("GetAllContinuousTasks fail errCode: %{public}d", Common::FindErrCode(asyncCallbackInfo->errCode));
        set_business_error(
            Common::FindErrCode(asyncCallbackInfo->errCode), Common::FindErrMsg(asyncCallbackInfo->errCode));
        return {};
    }
    std::vector<::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskInfo> aniInfoList;
    for (const auto& info : asyncCallbackInfo->list) {
        ::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskInfo aniInfo{
            .abilityName = info->GetAbilityName(),
            .uid = info->GetUid(),
            .pid = info->GetPid(),
            .isFromWebView = info->IsFromWebView(),
            .backgroundModes = GetAniBackgroundModes(info->GetBackgroundModes()),
            .backgroundSubModes = GetAniBackgroundSubModes(info->GetBackgroundSubModes()),
            .notificationId = info->GetNotificationId(),
            .continuousTaskId = info->GetContinuousTaskId(),
            .abilityId = info->GetAbilityId(),
            .wantAgentBundleName = info->GetWantAgentBundleName(),
            .wantAgentAbilityName = info->GetWantAgentAbilityName(),
            .suspendState = info->GetSuspendState(),
            .bundleName = optional<::taihe::string>(std::in_place, info->GetBundleName()),
            .appIndex = optional<int32_t>(std::in_place, info->GetAppIndex())
        };
        aniInfoList.push_back(aniInfo);
    }
    return array<::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskInfo>{copy_data_t{},
        aniInfoList.data(), aniInfoList.size()};
}

array<::ohos::resourceschedule::backgroundTaskManager::EfficiencyResourcesInfo> GetAllEfficiencyResourcesSync()
{
    std::unique_ptr<AllEfficiencyResourcesCallbackInfo> asyncCallbackInfo =
        std::make_unique<AllEfficiencyResourcesCallbackInfo>();
    if (asyncCallbackInfo.get() == nullptr) {
        BGTASK_LOGE("asyncCallbackInfo is nullptr");
        set_business_error(
            Common::FindErrCode(ERR_BGTASK_CHECK_TASK_PARAM), Common::FindErrMsg(ERR_BGTASK_CHECK_TASK_PARAM));
        return {};
    }
    ErrCode errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        GetAllEfficiencyResources(asyncCallbackInfo->efficiencyResourceInfoList);
    if (errCode) {
        BGTASK_LOGE("GetAllEfficiencyResourcesSync failed errCode: %{public}d", Common::FindErrCode(errCode));
        set_business_error(Common::FindErrCode(errCode), Common::FindErrMsg(errCode));
        return {};
    }
    std::vector<::ohos::resourceschedule::backgroundTaskManager::EfficiencyResourcesInfo> aniInfoList;
    for (const auto& info : asyncCallbackInfo->efficiencyResourceInfoList) {
        ::ohos::resourceschedule::backgroundTaskManager::EfficiencyResourcesInfo aniInfo{
            .resourceTypes = info->GetResourceNumber(),
            .timeout = info->GetTimeOut(),
            .isPersistent = info->IsPersist(),
            .isForProcess = info->IsProcess(),
            .reason = info->GetReason(),
            .uid = info->GetUid(),
            .pid = info->GetPid(),
        };
        if (info->GetCpuLevel() != OHOS::BackgroundTaskMgr::EfficiencyResourcesCpuLevel::DEFAULT) {
            aniInfo.cpuLevel.emplace(
                ::ohos::resourceschedule::backgroundTaskManager::EfficiencyResourcesCpuLevel::from_value(
                    info->GetCpuLevel()));
        }
        aniInfoList.push_back(aniInfo);
    }
    return array<::ohos::resourceschedule::backgroundTaskManager::EfficiencyResourcesInfo>{copy_data_t{},
        aniInfoList.data(), aniInfoList.size()};
}

bool SubscribeBackgroundTask(ani_env *env, uint32_t flag = 0)
{
    if (backgroundTaskSubscriber_ == nullptr) {
        backgroundTaskSubscriber_ = std::make_shared<BackgroundTaskMgr::AniBackgroundTaskSubscriber>();
        if (backgroundTaskSubscriber_ == nullptr) {
            BGTASK_LOGE("ret is nullptr");
            set_business_error(Common::FindErrCode(ERR_BGTASK_SERVICE_INNER_ERROR),
                Common::FindErrMsg(ERR_BGTASK_SERVICE_INNER_ERROR));
            return false;
        }
    }
    backgroundTaskSubscriber_->SetFlag(flag, true);
    ErrCode errCode = BackgroundTaskMgrHelper::SubscribeBackgroundTask(*backgroundTaskSubscriber_);
    if (errCode != ERR_OK) {
        BGTASK_LOGE("SubscribeBackgroundTask failed: %{public}d, msg: %{public}s",
            errCode, (Common::FindErrMsg(errCode)).c_str());
        set_business_error(Common::FindErrCode(errCode), Common::FindErrMsg(errCode));
        return false;
    }
    return true;
}

void UnSubscribeBackgroundTask(ani_env *env, uint32_t flag = 0)
{
    if (!backgroundTaskSubscriber_->IsEmpty()) {
        return;
    }
    backgroundTaskSubscriber_->SetFlag(flag, false);
    ErrCode errCode = BackgroundTaskMgrHelper::UnsubscribeBackgroundTask(*backgroundTaskSubscriber_);
    if (errCode != ERR_OK) {
        BGTASK_LOGE("UnsubscribeBackgroundTask failed.");
        set_business_error(Common::FindErrCode(errCode), Common::FindErrMsg(errCode));
        return;
    }
    backgroundTaskSubscriber_->UnSubscriberBgtaskSaStatusChange();
    backgroundTaskSubscriber_ = nullptr;
}

void OnContinuousTaskCancel(callback_view<void(ContinuousTaskCancelInfo const& data)> callback)
{
    auto env = taihe::get_env();
    std::shared_ptr<taihe::callback<void(const ContinuousTaskCancelInfo&)>> taiheCallback =
        std::make_shared<taihe::callback<void(const ContinuousTaskCancelInfo&)>>(callback);
    if (!taiheCallback) {
        set_business_error(Common::FindErrCode(ERR_BGTASK_INVALID_PARAM), Common::FindErrMsg(ERR_BGTASK_INVALID_PARAM));
        BGTASK_LOGE("taiheCallback is invalid");
        return;
    }
    std::lock_guard<std::mutex> lock(backgroundTaskSubscriberMutex_);
    if (!SubscribeBackgroundTask(env, CONTINUOUS_TASK_CANCEL)) {
        return;
    }
    backgroundTaskSubscriber_->AddCancelObserverObject("continuousTaskCancel", taiheCallback);
    backgroundTaskSubscriber_->SubscriberBgtaskSaStatusChange();
}

void OffContinuousTaskCancel(optional_view<callback<void(ContinuousTaskCancelInfo const& data)>> callback)
{
    auto env = taihe::get_env();
    std::lock_guard<std::mutex> lock(backgroundTaskSubscriberMutex_);
    if (!backgroundTaskSubscriber_) {
        set_business_error(Common::FindErrCode(ERR_BGTASK_INVALID_PARAM), Common::FindErrMsg(ERR_BGTASK_INVALID_PARAM));
        BGTASK_LOGE("backgroundTaskSubscriber_ is null, return");
        return;
    }
    if (!callback.has_value()) {
        backgroundTaskSubscriber_->RemoveJsObserverObjects("continuousTaskCancel");
    } else {
        std::shared_ptr<taihe::callback<void(const ContinuousTaskCancelInfo&)>> taiheCallback =
            std::make_shared<taihe::callback<void(const ContinuousTaskCancelInfo&)>>(callback.value());
        backgroundTaskSubscriber_->RemoveCancelObserverObject("continuousTaskCancel", taiheCallback);
    }
    UnSubscribeBackgroundTask(env, CONTINUOUS_TASK_CANCEL);
}

void OnContinuousTaskSuspend(::taihe::callback_view<void(ContinuousTaskSuspendInfo const& data)> callback)
{
    auto env = taihe::get_env();
    std::shared_ptr<taihe::callback<void(const ContinuousTaskSuspendInfo&)>> taiheCallback =
        std::make_shared<taihe::callback<void(const ContinuousTaskSuspendInfo&)>>(callback);
    if (!taiheCallback) {
        set_business_error(Common::FindErrCode(ERR_BGTASK_INVALID_PARAM), Common::FindErrMsg(ERR_BGTASK_INVALID_PARAM));
        BGTASK_LOGE("taiheCallback is invalid");
        return;
    }
    std::lock_guard<std::mutex> lock(backgroundTaskSubscriberMutex_);
    if (!SubscribeBackgroundTask(env, CONTINUOUS_TASK_SUSPEND)) {
        return;
    }
    backgroundTaskSubscriber_->AddSuspendObserverObject("continuousTaskSuspend", taiheCallback);
    backgroundTaskSubscriber_->SubscriberBgtaskSaStatusChange();
}

void OffContinuousTaskSuspend(optional_view<callback<void(ContinuousTaskSuspendInfo const& data)>> callback)
{
    auto env = taihe::get_env();
    std::lock_guard<std::mutex> lock(backgroundTaskSubscriberMutex_);
    if (!backgroundTaskSubscriber_) {
        set_business_error(Common::FindErrCode(ERR_BGTASK_INVALID_PARAM), Common::FindErrMsg(ERR_BGTASK_INVALID_PARAM));
        BGTASK_LOGE("backgroundTaskSubscriber_ is null, return");
        return;
    }
    if (!callback.has_value()) {
        backgroundTaskSubscriber_->RemoveJsObserverObjects("continuousTaskSuspend");
    } else {
        std::shared_ptr<taihe::callback<void(const ContinuousTaskSuspendInfo&)>> taiheCallback =
            std::make_shared<taihe::callback<void(const ContinuousTaskSuspendInfo&)>>(callback.value());
        backgroundTaskSubscriber_->RemoveSuspendObserverObject("continuousTaskSuspend", taiheCallback);
    }
    UnSubscribeBackgroundTask(env, CONTINUOUS_TASK_SUSPEND);
}

void OnContinuousTaskActive(::taihe::callback_view<void(ContinuousTaskActiveInfo const& data)> callback)
{
    auto env = taihe::get_env();
    std::shared_ptr<taihe::callback<void(const ContinuousTaskActiveInfo&)>> taiheCallback =
        std::make_shared<taihe::callback<void(const ContinuousTaskActiveInfo&)>>(callback);
    if (!taiheCallback) {
        set_business_error(Common::FindErrCode(ERR_BGTASK_INVALID_PARAM), Common::FindErrMsg(ERR_BGTASK_INVALID_PARAM));
        BGTASK_LOGE("taiheCallback is invalid.");
        return;
    }
    std::lock_guard<std::mutex> lock(backgroundTaskSubscriberMutex_);
    if (!SubscribeBackgroundTask(env, CONTINUOUS_TASK_ACTIVE)) {
        return;
    }
    backgroundTaskSubscriber_->AddActiveObserverObject("continuousTaskActive", taiheCallback);
    backgroundTaskSubscriber_->SubscriberBgtaskSaStatusChange();
}

void OffContinuousTaskActive(optional_view<callback<void(ContinuousTaskActiveInfo const& data)>> callback)
{
    auto env = taihe::get_env();
    std::lock_guard<std::mutex> lock(backgroundTaskSubscriberMutex_);
    if (!backgroundTaskSubscriber_) {
        set_business_error(Common::FindErrCode(ERR_BGTASK_INVALID_PARAM), Common::FindErrMsg(ERR_BGTASK_INVALID_PARAM));
        BGTASK_LOGE("backgroundTaskSubscriber_ is null, return");
        return;
    }
    if (!callback.has_value()) {
        backgroundTaskSubscriber_->RemoveJsObserverObjects("continuousTaskActive");
    } else {
        std::shared_ptr<taihe::callback<void(const ContinuousTaskActiveInfo&)>> taiheCallback =
            std::make_shared<taihe::callback<void(const ContinuousTaskActiveInfo&)>>(callback.value());
        backgroundTaskSubscriber_->RemoveActiveObserverObject("continuousTaskActive", taiheCallback);
    }
    UnSubscribeBackgroundTask(env, CONTINUOUS_TASK_ACTIVE);
}

void StopBackgroundRunningSync2(uintptr_t context, int32_t continuousTaskId)
{
    auto env = taihe::get_env();
    std::unique_ptr<ContinuousTaskCallbackInfo> asyncCallbackInfo = std::make_unique<ContinuousTaskCallbackInfo>();
    if (continuousTaskId < 0) {
        BGTASK_LOGE("check continuousTaskId failed");
        set_business_error(Common::FindErrCode(ERR_BGTASK_CONTINUOUS_TASKID_INVALID),
            Common::FindErrMsg(ERR_BGTASK_CONTINUOUS_TASKID_INVALID));
        return;
    }
    if (!CheckParam(env, asyncCallbackInfo.get(), context)) {
        BGTASK_LOGE("check param failed");
        return;
    }
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    sptr<IRemoteObject> token = asyncCallbackInfo->abilityContext->GetToken();
    int32_t abilityId = asyncCallbackInfo->abilityContext->GetAbilityRecordId();
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestStopBackgroundRunning(info->name, token, abilityId,
        continuousTaskId);
    if (asyncCallbackInfo->errCode) {
        BGTASK_LOGE("StopBackgroundRunningSync2 fail errCode: %{public}d",
            Common::FindErrCode(asyncCallbackInfo->errCode));
        set_business_error(
            Common::FindErrCode(asyncCallbackInfo->errCode), Common::FindErrMsg(asyncCallbackInfo->errCode));
    }
}

array<::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskInfo> ObtainAllContinuousTasksSync()
{
    std::unique_ptr<ContinuousTaskCallbackInfo> asyncCallbackInfo = std::make_unique<ContinuousTaskCallbackInfo>();
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::GetAllContinuousTasksBySystem(asyncCallbackInfo->list);
    if (asyncCallbackInfo->errCode) {
        set_business_error(Common::FindErrCode(asyncCallbackInfo->errCode),
            Common::FindErrMsg(asyncCallbackInfo->errCode));
        return {};
    }
    std::vector<::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskInfo> aniInfoList;
    for (const auto& info : asyncCallbackInfo->list) {
        ::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskInfo aniInfo{
            .abilityName = info->GetAbilityName(),
            .uid = info->GetUid(),
            .pid = info->GetPid(),
            .isFromWebView = info->IsFromWebView(),
            .backgroundModes = GetAniBackgroundModes(info->GetBackgroundModes()),
            .backgroundSubModes = GetAniBackgroundSubModes(info->GetBackgroundSubModes()),
            .notificationId = info->GetNotificationId(),
            .continuousTaskId = info->GetContinuousTaskId(),
            .abilityId = info->GetAbilityId(),
            .wantAgentBundleName = info->GetWantAgentBundleName(),
            .wantAgentAbilityName = info->GetWantAgentAbilityName(),
            .suspendState = false,
            .bundleName = optional<::taihe::string>(std::in_place, info->GetBundleName()),
            .appIndex = optional<int32_t>(std::in_place, info->GetAppIndex())
        };
        aniInfoList.push_back(aniInfo);
    }
    return array<::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskInfo>{copy_data_t{},
        aniInfoList.data(), aniInfoList.size()};
}

::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskRequest CreateContinuousTaskRequest()
{
    return taihe::make_holder<ContinuousTaskRequestImpl,
        ::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskRequest>();
}

bool CheckModeAndSubMode(ani_env *env, ohos::resourceschedule::backgroundTaskManager::ContinuousTaskRequest request,
    ContinuousTaskCallbackInfo *asyncCallbackInfo)
{
    auto requetImpl = reinterpret_cast<ContinuousTaskRequestImpl*>(request->GetInner());
    std::vector<uint32_t> modes = requetImpl->backgroundTaskModes_;
    std::vector<uint32_t> subModes = requetImpl->backgroundTaskSubmodes_;
    if (modes.size() == 0 || subModes.size() == 0) {
        asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_IS_EMPTY;
        return false;
    }
    if (modes.size() != subModes.size()) {
        asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_LENGTH_MISMATCH;
        return false;
    }
    int32_t specialModeSize = std::count(modes.begin(), modes.end(),
        OHOS::BackgroundTaskMgr::BackgroundTaskMode::MODE_SPECIAL_SCENARIO_PROCESSING);
    if (specialModeSize > MAX_SPECIAL_TASK_NUMS) {
        asyncCallbackInfo->errCode = ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_ONLY_ALLOW_ONE_APPLICATION;
        return false;
    }
    if (specialModeSize == MAX_SPECIAL_TASK_NUMS && modes.size() > MAX_SPECIAL_TASK_NUMS) {
        asyncCallbackInfo->errCode = ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_CONFLICTS_WITH_OTHER_TASK;
        return false;
    }
    for (uint32_t index = 0; index < subModes.size(); index++) {
        uint32_t subMode = subModes[index];
        if (subMode >= OHOS::BackgroundTaskMgr::BackgroundTaskSubmode::END ||
            subMode < OHOS::BackgroundTaskMgr::BackgroundTaskSubmode::SUBMODE_CAR_KEY_NORMAL_NOTIFICATION) {
            asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_CROSS_BORDER;
            return false;
        }
        if (subMode == OHOS::BackgroundTaskMgr::BackgroundTaskSubmode::SUBMODE_NORMAL_NOTIFICATION) {
            if (!OHOS::BackgroundTaskMgr::BackgroundTaskMode::IsModeTypeMatching(modes[index])) {
                asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_TYPE_MISMATCH;
                return false;
            }
        } else {
            if (OHOS::BackgroundTaskMgr::BackgroundTaskMode::GetSubModeTypeMatching(subMode) != modes[index]) {
                asyncCallbackInfo->errCode = ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_TYPE_MISMATCH;
                return false;
            }
        }
    }
    asyncCallbackInfo->bgModes = modes;
    asyncCallbackInfo->bgMode = modes[0];
    asyncCallbackInfo->bgSubModes = subModes;
    return true;
}

::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskNotification StartBackgroundRunningSync3(
    uintptr_t context, ::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskRequest request)
{
    auto env = taihe::get_env();
    std::unique_ptr<ContinuousTaskCallbackInfo> asyncCallbackInfo = std::make_unique<ContinuousTaskCallbackInfo>();
    ::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskNotification notification;
    if (!CheckParam(env, asyncCallbackInfo.get(), context)) {
        BGTASK_LOGE("check param failed");
        return notification;
    }
    if (!CheckModeAndSubMode(env, request, asyncCallbackInfo.get())) {
        BGTASK_LOGE("check mode or subMode failed");
        set_business_error(Common::FindErrCode(asyncCallbackInfo->errCode),
            Common::FindErrMsg(asyncCallbackInfo->errCode));
        return notification;
    }
    sptr<IRemoteObject> token = asyncCallbackInfo->abilityContext->GetToken();
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    int32_t abilityId = asyncCallbackInfo->abilityContext->GetAbilityRecordId();
    auto requetImpl = reinterpret_cast<ContinuousTaskRequestImpl*>(request->GetInner());
    ContinuousTaskParam taskParam = ContinuousTaskParam(true, asyncCallbackInfo->bgMode,
        requetImpl->wantAgent_, info->name, token, "", true, asyncCallbackInfo->bgModes, abilityId);
    taskParam.appIndex_ = info->appIndex;
    taskParam.bgSubModeIds_ = asyncCallbackInfo->bgSubModes;
    taskParam.isByRequestObject_ = true;
    taskParam.isCombinedTaskNotification_ = requetImpl->combinedTaskNotification_;
    taskParam.combinedNotificationTaskId_ = requetImpl->continuousTaskId_;
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestStartBackgroundRunning(taskParam);
    asyncCallbackInfo->notificationId = taskParam.notificationId_;
    asyncCallbackInfo->continuousTaskId = taskParam.continuousTaskId_;
    if (asyncCallbackInfo->errCode) {
        set_business_error(
            Common::FindErrCode(asyncCallbackInfo->errCode), Common::FindErrMsg(asyncCallbackInfo->errCode));
        return notification;
    }
    if (!GetSlotType(env) || !GetContentType(env)) {
        return notification;
    }
    notification.slotType = (uintptr_t)GetSlotType(env);
    notification.contentType = (uintptr_t)GetContentType(env);
    notification.notificationId = taskParam.notificationId_;
    notification.continuousTaskId = optional<int32_t>(std::in_place, taskParam.continuousTaskId_);
    return notification;
}

::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskNotification UpdateBackgroundRunningSync2(
    uintptr_t context, ohos::resourceschedule::backgroundTaskManager::ContinuousTaskRequest request)
{
    ::ohos::resourceschedule::backgroundTaskManager::ContinuousTaskNotification notification;
    auto requetImpl = reinterpret_cast<ContinuousTaskRequestImpl*>(request->GetInner());
    if (requetImpl->continuousTaskId_ < 0) {
        set_business_error(Common::FindErrCode(ERR_BGTASK_CONTINUOUS_TASKID_INVALID),
            Common::FindErrMsg(ERR_BGTASK_CONTINUOUS_TASKID_INVALID));
        return notification;
    }
    auto env = taihe::get_env();
    std::unique_ptr<ContinuousTaskCallbackInfo> asyncCallbackInfo = std::make_unique<ContinuousTaskCallbackInfo>();
    if (!CheckParam(env, asyncCallbackInfo.get(), context)) {
        return notification;
    }
    if (!CheckModeAndSubMode(env, request, asyncCallbackInfo.get())) {
        set_business_error(Common::FindErrCode(asyncCallbackInfo->errCode),
            Common::FindErrMsg(asyncCallbackInfo->errCode));
        return notification;
    }
    sptr<IRemoteObject> token = asyncCallbackInfo->abilityContext->GetToken();
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    int32_t abilityId = asyncCallbackInfo->abilityContext->GetAbilityRecordId();
    ContinuousTaskParam taskParam = ContinuousTaskParam(true, asyncCallbackInfo->bgMode,
        requetImpl->wantAgent_, info->name, token, "", true, asyncCallbackInfo->bgModes, abilityId);
    taskParam.bgSubModeIds_ = asyncCallbackInfo->bgSubModes;
    taskParam.isCombinedTaskNotification_ = requetImpl->combinedTaskNotification_;
    taskParam.updateTaskId_ = requetImpl->continuousTaskId_;
    taskParam.isByRequestObject_ = true;
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestUpdateBackgroundRunning(taskParam);
    if (asyncCallbackInfo->errCode) {
        set_business_error(Common::FindErrCode(asyncCallbackInfo->errCode),
            Common::FindErrMsg(asyncCallbackInfo->errCode));
        return notification;
    }
    if (!GetSlotType(env) || !GetContentType(env)) {
        return notification;
    }
    notification.slotType = (uintptr_t)GetSlotType(env);
    notification.contentType = (uintptr_t)GetContentType(env);
    notification.notificationId = taskParam.notificationId_;
    notification.continuousTaskId = optional<int32_t>(std::in_place, taskParam.continuousTaskId_);
    return notification;
}

void SubscribeContinuousTaskState(JsBackgroundTaskSubscriberType subscriber)
{
    auto env = taihe::get_env();
    std::shared_ptr<JsBackgroundTaskSubscriberType> taiheSubscriber =
        std::make_shared<JsBackgroundTaskSubscriberType>(subscriber);
    if (!taiheSubscriber) {
        set_business_error(Common::FindErrCode(ERR_BGTASK_INVALID_PARAM),
            Common::FindErrMsg(ERR_BGTASK_INVALID_PARAM));
        BGTASK_LOGE("taiheSubscriber is invalid");
        return;
    }
    std::lock_guard<std::mutex> lock(backgroundTaskSubscriberMutex_);
    if (!SubscribeBackgroundTask(env, SUBSCRIBER_BACKGROUND_TASK_STATE)) {
        BGTASK_LOGE("SubscribeBackgroundTask fail.");
        return;
    }
    backgroundTaskSubscriber_->AddJsSubscribeObserverObject("subscribeContinuousTaskState", taiheSubscriber);
    backgroundTaskSubscriber_->SubscriberBgtaskSaStatusChange();
}

void UnsubscribeContinuousTaskState(JsBackgroundTaskSubscriberType subscriber)
{
    auto env = taihe::get_env();
    std::shared_ptr<JsBackgroundTaskSubscriberType> taiheSubscriber =
        std::make_shared<JsBackgroundTaskSubscriberType>(subscriber);
    std::lock_guard<std::mutex> lock(backgroundTaskSubscriberMutex_);
    if (!backgroundTaskSubscriber_ || !taiheSubscriber) {
        set_business_error(Common::FindErrCode(ERR_BGTASK_INVALID_PARAM),
            Common::FindErrMsg(ERR_BGTASK_INVALID_PARAM));
        BGTASK_LOGE("backgroundTaskSubscriber_ or taiheSubscriber is null, return");
        return;
    }
    backgroundTaskSubscriber_->RemoveJsSubscribeObserverObject("subscribeContinuousTaskState", taiheSubscriber);
    UnSubscribeBackgroundTask(env, SUBSCRIBER_BACKGROUND_TASK_STATE);
}
} // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CancelSuspendDelay(CancelSuspendDelay);
TH_EXPORT_CPP_API_GetRemainingDelayTimeSync(GetRemainingDelayTimeSync);
TH_EXPORT_CPP_API_RequestSuspendDelay(RequestSuspendDelay);
TH_EXPORT_CPP_API_StartBackgroundRunningSync(StartBackgroundRunningSync);
TH_EXPORT_CPP_API_StopBackgroundRunningSync(StopBackgroundRunningSync);
TH_EXPORT_CPP_API_ApplyEfficiencyResources(ApplyEfficiencyResources);
TH_EXPORT_CPP_API_ResetAllEfficiencyResources(ResetAllEfficiencyResources);
TH_EXPORT_CPP_API_StartBackgroundRunningSync2(StartBackgroundRunningSync2);
TH_EXPORT_CPP_API_UpdateBackgroundRunningSync(UpdateBackgroundRunningSync);
TH_EXPORT_CPP_API_GetTransientTaskInfoSync(GetTransientTaskInfoSync);
TH_EXPORT_CPP_API_GetAllContinuousTasksSync(GetAllContinuousTasksSync);
TH_EXPORT_CPP_API_GetAllContinuousTasksSync2(GetAllContinuousTasksSync2);
TH_EXPORT_CPP_API_GetAllEfficiencyResourcesSync(GetAllEfficiencyResourcesSync);
TH_EXPORT_CPP_API_OnContinuousTaskCancel(OnContinuousTaskCancel);
TH_EXPORT_CPP_API_OffContinuousTaskCancel(OffContinuousTaskCancel);
TH_EXPORT_CPP_API_OnContinuousTaskSuspend(OnContinuousTaskSuspend);
TH_EXPORT_CPP_API_OffContinuousTaskSuspend(OffContinuousTaskSuspend);
TH_EXPORT_CPP_API_OnContinuousTaskActive(OnContinuousTaskActive);
TH_EXPORT_CPP_API_OffContinuousTaskActive(OffContinuousTaskActive);
TH_EXPORT_CPP_API_StopBackgroundRunningSync2(StopBackgroundRunningSync2);
TH_EXPORT_CPP_API_SetBackgroundTaskState(SetBackgroundTaskState);
TH_EXPORT_CPP_API_GetBackgroundTaskState(GetBackgroundTaskState);
TH_EXPORT_CPP_API_ObtainAllContinuousTasksSync(ObtainAllContinuousTasksSync);
TH_EXPORT_CPP_API_CreateContinuousTaskRequest(CreateContinuousTaskRequest);
TH_EXPORT_CPP_API_StartBackgroundRunningSync3(StartBackgroundRunningSync3);
TH_EXPORT_CPP_API_UpdateBackgroundRunningSync2(UpdateBackgroundRunningSync2);
TH_EXPORT_CPP_API_SubscribeContinuousTaskState(SubscribeContinuousTaskState);
TH_EXPORT_CPP_API_UnsubscribeContinuousTaskState(UnsubscribeContinuousTaskState);
// NOLINTEND