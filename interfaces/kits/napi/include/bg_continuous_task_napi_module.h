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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_NAPI_INCLUDE_BG_CONTINUOUS_TASK_NAPI_MODULE_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_NAPI_INCLUDE_BG_CONTINUOUS_TASK_NAPI_MODULE_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace BackgroundTaskMgr {
napi_value StartBackgroundRunning(napi_env env, napi_callback_info info);
napi_value StartBackgroundRunningThrow(napi_env env, napi_callback_info info);
napi_value UpdateBackgroundRunningThrow(napi_env env, napi_callback_info info);
napi_value StopBackgroundRunning(napi_env env, napi_callback_info info);
napi_value StopBackgroundRunningThrow(napi_env env, napi_callback_info info);
napi_value OnOnContinuousTaskCallback(napi_env env, napi_callback_info info);
napi_value OffOnContinuousTaskCallback(napi_env env, napi_callback_info info);
napi_value GetAllContinuousTasksThrow(napi_env env, napi_callback_info info);
napi_value StartBackgroundRunningWithTaskKeepingThrow(napi_env env, napi_callback_info info);
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_NAPI_INCLUDE_CANCEL_SUSPEND_DELAY_H