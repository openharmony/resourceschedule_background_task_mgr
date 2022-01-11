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

#include "init.h"

#include "cancel_suspend_delay.h"
#include "request_suspend_delay.h"
#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {

EXTERN_C_START

napi_value BackgroundTaskMgrInit(napi_env env, napi_value exports)
{
    BGTASK_LOGI("BackgroundTaskMgrInit");
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("requestSuspendDelay", RequestSuspendDelay),
        DECLARE_NAPI_FUNCTION("cancelSuspendDelay", CancelSuspendDelay),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    return exports;
}

/*
 * Module export function
 */
static napi_value Init(napi_env env, napi_value exports)
{
    /*
     * Propertise define
     */
    BackgroundTaskMgrInit(env, exports);
    return exports;
}

/*
 * Module register function
 */
__attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
EXTERN_C_END

} // namespace BackgroundTaskMgr
} // namespace OHOS