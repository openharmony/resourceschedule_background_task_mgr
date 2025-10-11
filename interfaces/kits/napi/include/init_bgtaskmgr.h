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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_NAPI_INCLUDE_INIT_BGTASKMGR_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_NAPI_INCLUDE_INIT_BGTASKMGR_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace BackgroundTaskMgr {
#ifdef __cplusplus
extern "C" {
#endif

__attribute__((constructor)) void RegisterModule(void);
void SetNamedPropertyByInteger(napi_env env, napi_value dstObj, int32_t objName, const char *propName);
void SetNamedPropertyByString(napi_env env, napi_value dstObj, const char *objName, const char *propName);
napi_value BackgroundTaskMgrInit(napi_env env, napi_value exports);
napi_value BackgroundModeInit(napi_env env, napi_value exports);
napi_value ContinuousTaskCancelReasonInit(napi_env env, napi_value exports);
napi_value ContinuousTaskSuspendReasonInit(napi_env env, napi_value exports);
napi_value BackgroundTaskModeInit(napi_env env, napi_value exports);
napi_value BackgroundTaskSubModeInit(napi_env env, napi_value exports);
napi_value ContinuousTaskRequestInit(napi_env env, napi_value exports);
napi_value UserAuthResultInit(napi_env env, napi_value exports);
napi_value ContinuousTaskRequestConstructor(napi_env env, napi_callback_info exports);
static napi_value InitApi(napi_env env, napi_value exports);

#ifdef __cplusplus
}
#endif

/*
 * Module define
 */
napi_module g_apiModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitApi,
    .nm_modname = "resourceschedule.backgroundTaskManager",
    .nm_priv = nullptr,
    .reserved = {nullptr}
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_NAPI_INCLUDE_INIT_BGTASKMGR_H