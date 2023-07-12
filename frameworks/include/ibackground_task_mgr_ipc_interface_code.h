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

/* SAID: 1903 */
namespace OHOS {
namespace BackgroundTaskMgr {
    enum class BackgroundTaskMgrStubInterfaceCode {
        REQUEST_SUSPEND_DELAY = MIN_TRANSACTION_ID,
        CANCEL_SUSPEND_DELAY,
        GET_REMAINING_DELAY_TIME,
        START_BACKGROUND_RUNNING,
        STOP_BACKGROUND_RUNNING,
        SUBSCRIBE_BACKGROUND_TASK,
        UNSUBSCRIBE_BACKGROUND_TASK,
        GET_TRANSIENT_TASK_APPS,
        GET_CONTINUOUS_TASK_APPS,
        APPLY_EFFICIENCY_RESOURCES,
        RESET_ALL_EFFICIENCY_RESOURCES,
        GET_EFFICIENCY_RESOURCES_INFOS,
        STOP_CONTINUOUS_TASK,
        REQUEST_BACKGROUND_RUNNING_FOR_INNER,
    };
}  // namespace BackgroundTaskMgr
}  // namespace OHOS