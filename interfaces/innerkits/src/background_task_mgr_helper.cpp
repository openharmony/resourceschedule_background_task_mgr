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

#include "background_task_mgr_helper.h"

#include "singleton.h"

#include "bgtaskmgr_log_wrapper.h"
#include "background_task_manager.h"
#include "ibackground_task_mgr.h"

namespace OHOS {
namespace BackgroundTaskMgr {
ErrCode BackgroundTaskMgrHelper::RequestStartBackgroundRunning(const ContinuousTaskParam &taskParam)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->RequestStartBackgroundRunning(taskParam);
}

ErrCode BackgroundTaskMgrHelper::RequestStopBackgroundRunning(const std::string &abilityName,
    const sptr<IRemoteObject> &abilityToken)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        RequestStopBackgroundRunning(abilityName, abilityToken);
}
ErrCode BackgroundTaskMgrHelper::SubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->SubscribeBackgroundTask(subscriber);
}

ErrCode BackgroundTaskMgrHelper::UnsubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->UnsubscribeBackgroundTask(subscriber);
}

ErrCode BackgroundTaskMgrHelper::GetTransientTaskApps(std::vector<std::shared_ptr<TransientTaskAppInfo>> &list)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetTransientTaskApps(list);
}

ErrCode BackgroundTaskMgrHelper::GetContinuousTaskApps(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetContinuousTaskApps(list);
}

ErrCode BackgroundTaskMgrHelper::GetEfficiencyResourcesInfos(std::vector<std::shared_ptr<ResourceCallbackInfo>> &appList,
    std::vector<std::shared_ptr<ResourceCallbackInfo>> &procList)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetEfficiencyResourcesInfos(appList, procList);
}

ErrCode BackgroundTaskMgrHelper::StopContinuousTask(int32_t uid, int32_t pid, uint32_t taskType)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->StopContinuousTask(uid, pid, taskType);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS