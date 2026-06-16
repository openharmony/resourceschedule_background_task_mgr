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

#include "background_task_observer.h"
#include "bgtaskmgr_log_wrapper.h"
#include "res_sched_mgr.h"
#include "res_type.h"
#include "background_mode.h"

extern "C" void ReportDataInProcess(uint32_t resType, int64_t value, const nlohmann::json& payload);

namespace OHOS {
namespace BackgroundTaskMgr {
IMPLEMENT_SINGLE_INSTANCE(BackgroundTaskObserver);
using namespace OHOS::ResourceSchedule;
bool BackgroundTaskObserver::CheckTransientTaskAppInfo(const std::shared_ptr<TransientTaskAppInfo>& info)
{
    return info->GetUid() > 0 && info->GetPid() >= 0
        && info->GetPackageName().size() > 0;
}

void BackgroundTaskObserver::MarshallingTransientTaskAppInfo(
    const std::shared_ptr<TransientTaskAppInfo>& info, nlohmann::json& payload)
{
    payload["pid"] = std::to_string(info->GetPid());
    payload["uid"] = std::to_string(info->GetUid());
    payload["bundleName"] = info->GetPackageName();
}

void BackgroundTaskObserver::OnTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info)
{
    if (!info || !CheckTransientTaskAppInfo(info)) {
        BGTASK_LOGE("info is nullptr or invalid app info!");
        return;
    }

    nlohmann::json payload;
    MarshallingTransientTaskAppInfo(info, payload);
    ReportDataInProcess(ResType::RES_TYPE_TRANSIENT_TASK, ResType::TransientTaskStatus::TRANSIENT_TASK_START, payload);
}

void BackgroundTaskObserver::OnTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info)
{
    if (!info || !CheckTransientTaskAppInfo(info)) {
        BGTASK_LOGE("info is nullptr or invalid app info!");
        return;
    }

    nlohmann::json payload;
    MarshallingTransientTaskAppInfo(info, payload);
    ReportDataInProcess(ResType::RES_TYPE_TRANSIENT_TASK, ResType::TransientTaskStatus::TRANSIENT_TASK_END, payload);
}

void BackgroundTaskObserver::OnTransientTaskErr(const std::shared_ptr<TransientTaskAppInfo>& info)
{
    if (!info || !CheckTransientTaskAppInfo(info)) {
        BGTASK_LOGE("info is nullptr or invalid app info!");
        return;
    }

    nlohmann::json payload;
    MarshallingTransientTaskAppInfo(info, payload);
    ReportDataInProcess(ResType::RES_TYPE_TRANSIENT_TASK, ResType::TransientTaskStatus::TRANSIENT_TASK_ERR, payload);
}

void BackgroundTaskObserver::OnAppTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info)
{
    nlohmann::json payload;
    MarshallingTransientTaskAppInfo(info, payload);
    ReportDataInProcess(
        ResType::RES_TYPE_TRANSIENT_TASK, ResType::TransientTaskStatus::APP_TRANSIENT_TASK_START, payload);
}

void BackgroundTaskObserver::OnAppTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info)
{
    nlohmann::json payload;
    MarshallingTransientTaskAppInfo(info, payload);
    ReportDataInProcess(
        ResType::RES_TYPE_TRANSIENT_TASK, ResType::TransientTaskStatus::APP_TRANSIENT_TASK_END, payload);
}

bool BackgroundTaskObserver::CheckContinuousTaskInfo(const std::shared_ptr<ContinuousTaskCallbackInfo>& eventData)
{
    return eventData->GetCreatorUid() > 0 && eventData->GetCreatorPid() >= 0
        && eventData->GetAbilityName().size() > 0 && eventData->GetTypeId() > 0
        && eventData->GetTypeId() <= BackgroundTaskMgr::BackgroundMode::END;
}

void BackgroundTaskObserver::MarshallingContinuousTaskCallbackInfo(
    const std::shared_ptr<ContinuousTaskCallbackInfo>& continuousTaskCallbackInfo, nlohmann::json& payload)
{
    payload["pid"] = std::to_string(continuousTaskCallbackInfo->GetCreatorPid());
    payload["uid"] = std::to_string(continuousTaskCallbackInfo->GetCreatorUid());
    payload["abilityName"] = continuousTaskCallbackInfo->GetAbilityName();
    payload["isBatchApi"] =  std::to_string(continuousTaskCallbackInfo->IsBatchApi());
    payload["typeId"] = std::to_string(continuousTaskCallbackInfo->GetTypeId());
    payload["abilityId"] = std::to_string(continuousTaskCallbackInfo->GetAbilityId());
    payload["continuousTaskId"] = std::to_string(continuousTaskCallbackInfo->GetContinuousTaskId());
    payload["isFromWebview"] = continuousTaskCallbackInfo->IsFromWebview();
    payload["typeIds"] = continuousTaskCallbackInfo->GetTypeIds();
    payload["tokenId"] = continuousTaskCallbackInfo->GetTokenId();
    payload["isByRequestObject"] = continuousTaskCallbackInfo->IsByRequestObject();
    payload["bundleName"] = continuousTaskCallbackInfo->GetBundleName();
    payload["userId"] = continuousTaskCallbackInfo->GetUserId();
    payload["appIndex"] = continuousTaskCallbackInfo->GetAppIndex();
    payload["suspendState"] = continuousTaskCallbackInfo->GetSuspendState();
    payload["suspendReason"] = continuousTaskCallbackInfo->GetSuspendReason();
}

void BackgroundTaskObserver::OnContinuousTaskStart(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    if (!continuousTaskCallbackInfo || !CheckContinuousTaskInfo(continuousTaskCallbackInfo)) {
        BGTASK_LOGE("continuousTaskCallbackInfo is nullptr or invalid event data!");
        return;
    }

    nlohmann::json payload;
    MarshallingContinuousTaskCallbackInfo(continuousTaskCallbackInfo, payload);
    ReportDataInProcess(
        ResType::RES_TYPE_CONTINUOUS_TASK, ResType::ContinuousTaskStatus::CONTINUOUS_TASK_START, payload);
}

void BackgroundTaskObserver::OnContinuousTaskStop(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    if (!continuousTaskCallbackInfo || !CheckContinuousTaskInfo(continuousTaskCallbackInfo)) {
        BGTASK_LOGE("continuousTaskCallbackInfo is nullptr or invalid event data!");
        return;
    }

    nlohmann::json payload;
    MarshallingContinuousTaskCallbackInfo(continuousTaskCallbackInfo, payload);
    ReportDataInProcess(
        ResType::RES_TYPE_CONTINUOUS_TASK, ResType::ContinuousTaskStatus::CONTINUOUS_TASK_END, payload);
}

void BackgroundTaskObserver::OnContinuousTaskUpdate(
    const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    if (!continuousTaskCallbackInfo || !CheckContinuousTaskInfo(continuousTaskCallbackInfo)) {
        BGTASK_LOGE("continuousTaskCallbackInfo is nullptr or invalid event data!");
        return;
    }

    nlohmann::json payload;
    MarshallingContinuousTaskCallbackInfo(continuousTaskCallbackInfo, payload);
    ReportDataInProcess(
        ResType::RES_TYPE_CONTINUOUS_TASK, ResType::ContinuousTaskStatus::CONTINUOUS_TASK_UPDATE, payload);
}

void BackgroundTaskObserver::MarshallingResourceInfo(
    const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo, nlohmann::json &payload)
{
    if (!resourceInfo) {
        BGTASK_LOGE("resourceInfo is nullptr!");
        return;
    }
    payload["pid"] = resourceInfo->GetPid();
    payload["uid"] = resourceInfo->GetUid();
    payload["resourceNumber"] = resourceInfo->GetResourceNumber();
    payload["bundleName"] = resourceInfo->GetBundleName();
    payload["cpuLevel"] = resourceInfo->GetCpuLevel();
}

void BackgroundTaskObserver::OnAppEfficiencyResourcesApply(
    const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo)
{
    nlohmann::json payload;
    MarshallingResourceInfo(resourceInfo, payload);
    ReportDataInProcess(
        ResType::RES_TYPE_EFFICIENCY_RESOURCES_STATE_CHANGED,
        ResType::EfficiencyResourcesStatus::APP_EFFICIENCY_RESOURCES_APPLY,
        payload);
}

void BackgroundTaskObserver::OnAppEfficiencyResourcesReset(
    const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo)
{
    nlohmann::json payload;
    MarshallingResourceInfo(resourceInfo, payload);
    ReportDataInProcess(
        ResType::RES_TYPE_EFFICIENCY_RESOURCES_STATE_CHANGED,
        ResType::EfficiencyResourcesStatus::APP_EFFICIENCY_RESOURCES_RESET,
        payload);
}

void BackgroundTaskObserver::OnProcEfficiencyResourcesApply(
    const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo)
{
    nlohmann::json payload;
    MarshallingResourceInfo(resourceInfo, payload);
    ReportDataInProcess(
        ResType::RES_TYPE_EFFICIENCY_RESOURCES_STATE_CHANGED,
        ResType::EfficiencyResourcesStatus::PROC_EFFICIENCY_RESOURCES_APPLY,
        payload);
}

void BackgroundTaskObserver::OnProcEfficiencyResourcesReset(
    const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo)
{
    nlohmann::json payload;
    MarshallingResourceInfo(resourceInfo, payload);
    ReportDataInProcess(
        ResType::RES_TYPE_EFFICIENCY_RESOURCES_STATE_CHANGED,
        ResType::EfficiencyResourcesStatus::PROC_EFFICIENCY_RESOURCES_RESET,
        payload);
}
} // namespace BackgroundTaskMgr
} // namespace OHOS
