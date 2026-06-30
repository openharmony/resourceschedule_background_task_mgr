/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "app_state_observer_plugin_adapter.h"
#include "bgtask_plugin_mgr.h"
#include "res_type.h"
#include "res_data.h"
#include "res_sched_json_util.h"
#include "process_data.h"
#include "bgtaskmgr_log_wrapper.h"
#include "bg_continuous_task_mgr.h"
#include "bg_transient_task_mgr.h"
#include "bg_efficiency_resources_mgr.h"
#include "singleton.h"

namespace OHOS {
namespace BackgroundTaskMgr {
using namespace AppExecFwk;
using namespace OHOS::ResourceSchedule;
namespace {
const bool SELF_REGISTER = AppStateObserverPluginAdapter::SelfRegister();
}

bool AppStateObserverPluginAdapter::SelfRegister()
{
    auto obj = AppStateObserverPluginAdapter::GetInstance();
    if (obj == nullptr) {
        BGTASK_LOGE("AppStateObserverPluginAdapter null.");
        return false;
    }
    obj->InitCbMap(obj->cbMap_);
    obj->InitAbilityCbMap(obj->cbMap_);
    for (const auto &info : obj->cbMap_) {
        BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(info.first, obj);
    }
    return true;
}

bool AppStateObserverPluginAdapter::UnmarshallingProcessData(const nlohmann::json& payload,
    AppExecFwk::ProcessData& processData)
{
    if (!ResCommonUtil::ParseStringParameterFromJson("bundleName", processData.bundleName, payload) ||
        !ResCommonUtil::ParseStringJsonPayloadToNumber("pid", processData.pid, payload) ||
        !ResCommonUtil::ParseStringJsonPayloadToNumber("uid", processData.uid, payload)) {
        return false;
    }
    int32_t processType = 0;
    int32_t state = 0;
    int32_t extensionType = 0;
    int32_t preloadMode = 0;
    if (!ResCommonUtil::ParseStringJsonPayloadToNumber("processType", processType, payload) ||
        !ResCommonUtil::ParseStringJsonPayloadToNumber("state", state, payload) ||
        !ResCommonUtil::ParseStringJsonPayloadToNumber("extensionType", extensionType, payload) ||
        !ResCommonUtil::ParseStringJsonPayloadToNumber("preloadMode", preloadMode, payload)) {
        return false;
    }
    processData.processType = static_cast<AppExecFwk::ProcessType>(processType);
    processData.state = static_cast<AppExecFwk::AppProcessState>(state);
    processData.extensionType = static_cast<AppExecFwk::ExtensionAbilityType>(extensionType);
    processData.preloadMode = preloadMode;
    return true;
}

void AppStateObserverPluginAdapter::OnProcessCreated(const nlohmann::json& payload)
{
    AppExecFwk::ProcessData processData;
    if (!UnmarshallingProcessData(payload, processData)) {
        return;
    }
    BGTASK_LOGD("OnProcessCreated, bundleName: %{public}s, uid: %{public}d, pid: %{public}d",
        processData.bundleName.c_str(), processData.uid, processData.pid);
    appStateObserver_->OnProcessCreated(processData);
}

void AppStateObserverPluginAdapter::OnProcessDied(const nlohmann::json& payload)
{
    AppExecFwk::ProcessData processData;
    if (!UnmarshallingProcessData(payload, processData)) {
        return;
    }
    BGTASK_LOGD("OnProcessDied, bundleName: %{public}s, uid: %{public}d, pid: %{public}d",
        processData.bundleName.c_str(), processData.uid, processData.pid);
    if (decisionMaker_ != nullptr) {
        decisionMaker_->OnProcessDied(processData);
    }
    appStateObserver_->OnProcessDied(processData);
}

void AppStateObserverPluginAdapter::OnProcessStateChanged(const nlohmann::json& payload)
{
    AppExecFwk::ProcessData processData;
    if (!UnmarshallingProcessData(payload, processData)) {
        return;
    }
    BGTASK_LOGD("OnProcessStateChanged, bundleName: %{public}s, uid: %{public}d, pid: %{public}d",
        processData.bundleName.c_str(), processData.uid, processData.pid);
    if (decisionMaker_ == nullptr) {
        BGTASK_LOGI("decisionMaker_ is nullptr");
        return;
    }
    decisionMaker_->OnProcessStateChanged(processData);
}

bool AppStateObserverPluginAdapter::UnmarshallingAppStateData(const nlohmann::json& payload,
    AppExecFwk::AppStateData& appStateData)
{
    if (!ResCommonUtil::ParseStringParameterFromJson("bundleName", appStateData.bundleName, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("pid", appStateData.pid, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("uid", appStateData.uid, payload)) {
        return false;
    }
    int32_t state = 0;
    int32_t extensionType = 0;
    int32_t preloadMode = 0;
    if (!ResCommonUtil::ParseIntParameterFromJson("state", state, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("extensionType", extensionType, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("preloadMode", preloadMode, payload)) {
        return false;
    }
    appStateData.state = static_cast<uint32_t>(state);
    appStateData.extensionType = static_cast<AppExecFwk::ExtensionAbilityType>(extensionType);
    appStateData.preloadMode = preloadMode;
    return true;
}

void AppStateObserverPluginAdapter::OnAppStateChanged(const nlohmann::json& payload)
{
    AppExecFwk::AppStateData appStateData;
    if (!UnmarshallingAppStateData(payload, appStateData)) {
        return;
    }
    BGTASK_LOGD("OnAppStateChanged, bundleName: %{public}s, uid: %{public}d, pid: %{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.pid);
    appStateObserver_->OnAppStateChanged(appStateData);
}

void AppStateObserverPluginAdapter::OnAppStopped(const nlohmann::json &payload)
{
    AppExecFwk::AppStateData appStateData;
    if (!UnmarshallingAppStateData(payload, appStateData)) {
        return;
    }
    BGTASK_LOGD("OnAppStopped, bundleName: %{public}s, uid: %{public}d, pid: %{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.pid);
    appStateObserver_->OnAppStopped(appStateData);
}

void AppStateObserverPluginAdapter::OnAppCacheStateChanged(const nlohmann::json &payload)
{
    AppExecFwk::AppStateData appStateData;
    if (!UnmarshallingAppStateData(payload, appStateData)) {
        return;
    }
    BGTASK_LOGD("OnAppCacheStateChanged, bundleName: %{public}s, uid: %{public}d, pid: %{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.pid);
    appStateObserver_->OnAppCacheStateChanged(appStateData);
}

bool AppStateObserverPluginAdapter::UnmarshallingAbilityStateData(const nlohmann::json &payload,
    AppExecFwk::AbilityStateData &data)
{
    if (!ResCommonUtil::ParseIntParameterFromJson("pid", data.pid, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("uid", data.uid, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("recordId", data.abilityRecordId, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("abilityType", data.abilityType, payload) ||
        !ResCommonUtil::ParseStringParameterFromJson("abilityName", data.abilityName, payload)) {
        return false;
    }
    bool verified = false;
    if (ResCommonUtil::ParseIntParameterFromJson("extensionState", data.abilityState, payload) ||
        ResCommonUtil::ParseIntParameterFromJson("uiExtensionState", data.abilityState, payload)) {
        verified = ResCommonUtil::ParseIntParameterFromJson("extensionAbilityType",
            data.extensionAbilityType, payload);
    } else {
        verified = ResCommonUtil::ParseIntParameterFromJson("abilityState", data.abilityState, payload) &&
            ResCommonUtil::ParseIntParameterFromJson("extType", data.extensionAbilityType, payload);
    }
    return verified;
}

void AppStateObserverPluginAdapter::OnAbilityStateChanged(const nlohmann::json &payload)
{
    AppExecFwk::AbilityStateData abilityStateData;
    if (!UnmarshallingAbilityStateData(payload, abilityStateData)) {
        BGTASK_LOGE("Unmarshalling fail");
        return;
    }

    BGTASK_LOGD("uid:%{public}d pid:%{public}d name:%{public}s abilityState:%{public}d", abilityStateData.uid,
        abilityStateData.pid, abilityStateData.abilityName.c_str(), abilityStateData.abilityState);
    appStateObserver_->OnAbilityStateChanged(abilityStateData);
}

void AppStateObserverPluginAdapter::InitCbMap(std::unordered_map<uint32_t,
    std::unordered_map<int32_t, std::function<void(const int32_t, const nlohmann::json&)>>>& cbMap)
{
    cbMap[ResType::RES_TYPE_PROCESS_STATE_CHANGE] = {
        { ResType::ProcessStatus::PROCESS_DIED, [this](const int32_t stateType, const nlohmann::json& payload) {
                this->OnProcessDied(payload);
            } },
        { ResType::ProcessStatus::PROCESS_FOREGROUND, [this](const int32_t stateType, const nlohmann::json& payload) {
                this->OnProcessStateChanged(payload);
            } },
        { ResType::ProcessStatus::PROCESS_BACKGROUND, [this](const int32_t stateType, const nlohmann::json& payload) {
                this->OnProcessStateChanged(payload);
            } },
        { ResType::ProcessStatus::PROCESS_CREATED, [this](const int32_t stateType, const nlohmann::json& payload) {
                this->OnProcessCreated(payload);
            } },
    };

    cbMap[ResType::RES_TYPE_ON_APP_STATE_CHANGED] = {
        { RES_VALUE_FOR_ALL, [this](const int32_t stateType, const nlohmann::json& payload) {
                this->OnAppStateChanged(payload);
            } },
    };
    cbMap[ResType::RES_TYPE_APP_STOPPED] = {
        { RES_VALUE_FOR_ALL, [this](const int32_t stateType, const nlohmann::json& payload) {
                this->OnAppStopped(payload);
            } },
    };
    const int RES_TYPE_EXT_ON_APP_CACHED_STATE_CHANGED = 10008;
    cbMap[RES_TYPE_EXT_ON_APP_CACHED_STATE_CHANGED] = {
        { RES_VALUE_FOR_ALL, [this](const int32_t stateType, const nlohmann::json& payload) {
                this->OnAppCacheStateChanged(payload);
            } },
    };
}

void AppStateObserverPluginAdapter::InitAbilityCbMap(CallBackMap &cbMap)
{
    cbMap[ResType::RES_TYPE_ABILITY_STATE_CHANGE] = {
        {static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_CREATE),
            [this](const int32_t value, const nlohmann::json &payload) { this->OnAbilityStateChanged(payload); }},
        {static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_TERMINATED),
            [this](const int32_t value, const nlohmann::json &payload) { this->OnAbilityStateChanged(payload); }},
    };
}

void AppStateObserverPluginAdapter::Init()
{
    appStateObserver_ = std::make_shared<BackgroundTaskMgr::AppStateObserver>();
    decisionMaker_ = DelayedSingleton<BackgroundTaskMgr::BgTransientTaskMgr>::GetInstance()->GetDecisionMaker();
    if (!appStateObserver_ || !decisionMaker_) {
        BGTASK_LOGE("appStateObserver_ or decisionMaker_ is null");
        return;
    }
    auto handler = DelayedSingleton<BackgroundTaskMgr::BgContinuousTaskMgr>::GetInstance()->GetHandler();
    if (handler == nullptr) {
        BGTASK_LOGE("GetHandler failed, handler is null");
        return;
    }
    appStateObserver_->SetEventHandler(handler);
    BGTASK_LOGI("AppStateObserverPluginAdapter init succeed");
}

void AppStateObserverPluginAdapter::Uninit()
{
    BGTASK_LOGI("AppStateObserverPluginAdapter uninit");
}

std::string AppStateObserverPluginAdapter::GetPluginName() const
{
    return "AppStateObserverPluginAdapter";
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS