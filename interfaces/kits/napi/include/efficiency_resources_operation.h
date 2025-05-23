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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_NAPI_INCLUDE_EFFICIENCY_RES_OPERATION_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_NAPI_INCLUDE_EFFICIENCY_RES_OPERATION_H

#include <string>

#include "common.h"

namespace OHOS {
namespace BackgroundTaskMgr {
napi_value ApplyEfficiencyResources(napi_env env, napi_callback_info info);

napi_value ResetAllEfficiencyResources(napi_env env, napi_callback_info info);

void GetAllEfficiencyResourcesAsyncWork(napi_env env, void *data);
void GetAllEfficiencyResourcesAsyncWork(napi_env env, napi_status status, void *data);
napi_value GetAllEfficiencyResources(napi_env env, napi_callback_info info);
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_NAPI_INCLUDE_EFFICIENCY_RES_OPERATION_H