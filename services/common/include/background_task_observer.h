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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_COMMON_INCLUDE_BGTASK_OBSERVER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_COMMON_INCLUDE_BGTASK_OBSERVER_H

#include <sys/types.h>
#include "continuous_task_callback_info.h"
#include "transient_task_app_info.h"
#include "single_instance.h"
#include "resource_callback_info.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace BackgroundTaskMgr {

class BackgroundTaskObserver {
    DECLARE_SINGLE_INSTANCE(BackgroundTaskObserver)
public:
    void OnTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info);
    void OnTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info);
    void OnTransientTaskErr(const std::shared_ptr<TransientTaskAppInfo>& info);
    void OnAppTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info);
    void OnAppTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info);
    void OnContinuousTaskStart(const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo);
    void OnContinuousTaskStop(const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo);
    void OnContinuousTaskUpdate(const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo);
    void OnAppEfficiencyResourcesApply(
        const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo);
    void OnAppEfficiencyResourcesReset(
        const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo);
    void OnProcEfficiencyResourcesApply(
        const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo);
    void OnProcEfficiencyResourcesReset(
        const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo);

private:
    void MarshallingTransientTaskAppInfo(
        const std::shared_ptr<TransientTaskAppInfo>& info, nlohmann::json& payload);
    void MarshallingContinuousTaskCallbackInfo(
        const std::shared_ptr<ContinuousTaskCallbackInfo>& continuousTaskCallbackInfo, nlohmann::json& payload);
    void MarshallingResourceInfo(
        const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo, nlohmann::json &payload);
    bool CheckTransientTaskAppInfo(const std::shared_ptr<TransientTaskAppInfo>& info);
    bool CheckContinuousTaskInfo(const std::shared_ptr<ContinuousTaskCallbackInfo>& eventData);
};
} // namespace BackgroundTaskMgr
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_COMMON_INCLUDE_BGTASK_OBSERVER_H
