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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_IBACKGROUND_TASK_MGR_IPC_INTERFACE_CODE_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_IBACKGROUND_TASK_MGR_IPC_INTERFACE_CODE_H

/* SAID: 1903 */
namespace OHOS {
namespace BackgroundTaskMgr {
    enum class IBackgroundTaskSubscriberInterfaceCode {
        ON_CONNECTED = FIRST_CALL_TRANSACTION,
        ON_DISCONNECTED,
        ON_TRANSIENT_TASK_START,
        ON_TRANSIENT_TASK_END,
        ON_APP_TRANSIENT_TASK_START,
        ON_APP_TRANSIENT_TASK_END,
        ON_CONTINUOUS_TASK_START,
        ON_CONTINUOUS_TASK_STOP,
        ON_APP_CONTINUOUS_TASK_STOP,
        ON_APP_EFFICIENCY_RESOURCES_APPLY,
        ON_APP_EFFICIENCY_RESOURCES_RESET,
        ON_PROC_EFFICIENCY_RESOURCES_APPLY,
        ON_PROC_EFFICIENCY_RESOURCES_RESET,
    };
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_IBACKGROUND_TASK_MGR_IPC_INTERFACE_CODE_H
