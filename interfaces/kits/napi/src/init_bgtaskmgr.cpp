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

#include "init_bgtaskmgr.h"

#include "background_mode.h"
#include "bg_continuous_task_napi_module.h"
#include "cancel_suspend_delay.h"
#include "get_remaining_delay_time.h"
#include "request_suspend_delay.h"
#include "transient_task_log.h"
#include "efficiency_resources_operation.h"
#include "resource_type.h"
#include "common.h"

namespace OHOS {
namespace BackgroundTaskMgr {
EXTERN_C_START

napi_value BackgroundTaskMgrInit(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("requestSuspendDelay", RequestSuspendDelayThrow),
        DECLARE_NAPI_FUNCTION("cancelSuspendDelay", CancelSuspendDelayThrow),
        DECLARE_NAPI_FUNCTION("getRemainingDelayTime", GetRemainingDelayTimeThrow),
        DECLARE_NAPI_FUNCTION("startBackgroundRunning", StartBackgroundRunningThrow),
        DECLARE_NAPI_FUNCTION("updateBackgroundRunning", UpdateBackgroundRunningThrow),
        DECLARE_NAPI_FUNCTION("stopBackgroundRunning", StopBackgroundRunningThrow),
        DECLARE_NAPI_FUNCTION("applyEfficiencyResources", ApplyEfficiencyResources),
        DECLARE_NAPI_FUNCTION("resetAllEfficiencyResources", ResetAllEfficiencyResources),
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

    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(BackgroundMode::DATA_TRANSFER), "DATA_TRANSFER");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(BackgroundMode::AUDIO_PLAYBACK), "AUDIO_PLAYBACK");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(BackgroundMode::AUDIO_RECORDING),
        "AUDIO_RECORDING");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(BackgroundMode::LOCATION),
        "LOCATION");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(BackgroundMode::BLUETOOTH_INTERACTION),
        "BLUETOOTH_INTERACTION");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(BackgroundMode::MULTI_DEVICE_CONNECTION),
        "MULTI_DEVICE_CONNECTION");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(BackgroundMode::WIFI_INTERACTION), "WIFI_INTERACTION");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(BackgroundMode::VOIP), "VOIP");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(BackgroundMode::TASK_KEEPING), "TASK_KEEPING");

    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(ResourceType::CPU), "CPU");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(ResourceType::COMMON_EVENT), "COMMON_EVENT");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(ResourceType::TIMER), "TIMER");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(ResourceType::WORK_SCHEDULER), "WORK_SCHEDULER");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(ResourceType::BLUETOOTH), "BLUETOOTH");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(ResourceType::GPS), "GPS");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(ResourceType::AUDIO), "AUDIO");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(ResourceType::RUNNING_LOCK), "RUNNING_LOCK");
    SetNamedPropertyByInteger(env, obj, static_cast<uint32_t>(ResourceType::SENSOR), "SENSOR");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("BackgroundMode", obj),
        DECLARE_NAPI_PROPERTY("ResourceType", obj),
    };

    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

/*
 * Module export function
 */
static napi_value InitApi(napi_env env, napi_value exports)
{
    /*
     * Properties define
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
    napi_module_register(&g_apiModule);
}
EXTERN_C_END
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
