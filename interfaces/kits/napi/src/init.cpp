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

#include "background_mode.h"
#include "bg_continuous_task_napi_module.h"
#include "cancel_suspend_delay.h"
#include "get_remaining_delay_time.h"
#include "request_suspend_delay.h"
#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
EXTERN_C_START

napi_value BackgroundTaskMgrInit(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("requestSuspendDelay", RequestSuspendDelay),
        DECLARE_NAPI_FUNCTION("cancelSuspendDelay", CancelSuspendDelay),
        DECLARE_NAPI_FUNCTION("getRemainingDelayTime", GetRemainingDelayTime),
        DECLARE_NAPI_FUNCTION("startBackgroundRunning", StartBackgroundRunning),
        DECLARE_NAPI_FUNCTION("stopBackgroundRunning", StopBackgroundRunning),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    return exports;
}

void SetNamedPropertyByInteger(napi_env env, napi_value dstObj, int32_t objName, const char *propName)
{
    napi_value prop = nullptr;
    if (napi_create_int32(env, objName, &prop) == napi_ok) {
        napi_set_named_property(env, dstObj, propName, prop);
    }
}

napi_value BackgroundModeInit(napi_env env, napi_value exports)
{
    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    SetNamedPropertyByInteger(env, obj, (uint32_t)BackgroundMode::DATA_TRANSFER, "DATA_TRANSFER");
    SetNamedPropertyByInteger(env, obj, (uint32_t)BackgroundMode::AUDIO_PLAYBACK, "AUDIO_PLAYBACK");
    SetNamedPropertyByInteger(env, obj, (uint32_t)BackgroundMode::AUDIO_RECORDING, "AUDIO_RECORDING");
    SetNamedPropertyByInteger(env, obj, (uint32_t)BackgroundMode::LOCATION, "LOCATION");
    SetNamedPropertyByInteger(env, obj, (uint32_t)BackgroundMode::BLUETOOTH_INTERACTION, "BLUETOOTH_INTERACTION");
    SetNamedPropertyByInteger(env, obj, (uint32_t)BackgroundMode::MULTI_DEVICE_CONNECTION, "MULTI_DEVICE_CONNECTION");
    SetNamedPropertyByInteger(env, obj, (uint32_t)BackgroundMode::WIFI_INTERACTION, "WIFI_INTERACTION");
    SetNamedPropertyByInteger(env, obj, (uint32_t)BackgroundMode::VOIP, "VOIP");
    SetNamedPropertyByInteger(env, obj, (uint32_t)BackgroundMode::TASK_KEEPING, "TASK_KEEPING");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("BackgroundMode", obj),
    };

    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
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
    BackgroundModeInit(env, exports);
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
}  // namespace BackgroundTaskMgr
}  // namespace OHOS