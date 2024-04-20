/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef BACKGROUOND_TASK_MANAGER_FFI_H
#define BACKGROUOND_TASK_MANAGER_FFI_H

#include "ffi_remote_data.h"
#include "cj_common_ffi.h"
#include "napi_base_context.h"
#include "ability.h"
#include <cstdint>
#include <memory>
#include <string>

extern "C" {
    struct RetDelaySuspendInfo {
        int32_t requestId;
        int32_t actualDelayTime;
    };

    struct RetEfficiencyResourcesRequest {
        uint32_t resourceTypes;
        bool isApply;
        uint32_t timeOut;
        char* reason;
        bool isPersist;
        bool isProcess;
    };

    FFI_EXPORT int32_t CJ_RequestSuspendDelay(void (*callback)(), const char* reason, RetDelaySuspendInfo* ret);
    FFI_EXPORT int32_t CJ_GetRemainingDelayTime(int32_t requestId, int32_t& delayTime);
    FFI_EXPORT int32_t CJ_CancelSuspendDelay(int32_t requestId);
    FFI_EXPORT int32_t CJ_StopBackgroundRunning(OHOS::AbilityRuntime::AbilityContext* context);
    FFI_EXPORT int32_t CJ_ApplyEfficiencyResources(RetEfficiencyResourcesRequest request);
}

#endif