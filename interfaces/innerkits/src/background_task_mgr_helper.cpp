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

#include "background_task_manager.h"
#include "ibackground_task_mgr.h"

namespace OHOS {
namespace BackgroundTaskMgr {
ErrCode BackgroundTaskMgrHelper::SubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->SubscribeBackgroundTask(subscriber);
}

ErrCode BackgroundTaskMgrHelper::UnsubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber)
{
    return DelayedSingleton<BackgroundTaskManager>::GetInstance()->UnsubscribeBackgroundTask(subscriber);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS