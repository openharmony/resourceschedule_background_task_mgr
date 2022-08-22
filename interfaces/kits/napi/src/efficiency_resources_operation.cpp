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

#include "efficiency_resources_operation.h"
#include "singleton.h"
#include "background_task_manager.h"
#include "efficiency_resource_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
    namespace {
        static constexpr int32_t APPLY_EFFICIENCY_RESOURCES_PARAMS = 1;
    }

    napi_value GetNamedBoolValue(const napi_env &env, napi_value &object, const char* utf8name,
        bool& result)
    {
        bool hasNamedProperty = false;
        napi_value boolValue;
        if (napi_has_named_property(env, object, utf8name, &hasNamedProperty) == napi_ok && hasNamedProperty) {
            NAPI_CALL(env, napi_get_named_property(env, object, utf8name, &boolValue));
            if (Common::GetBooolValue(env, boolValue, result) == nullptr) {
                BGTASK_LOGE("ParseParameters failed, %{public}s is nullptr.", utf8name);
                return nullptr;
            }
        }
        return Common::NapiGetNull(env);
    }

    napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, EfficiencyResourceInfo &params)
    {
        size_t argc = APPLY_EFFICIENCY_RESOURCES_PARAMS;
        napi_value argv[APPLY_EFFICIENCY_RESOURCES_PARAMS] = {nullptr};
        NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
        NAPI_ASSERT(env, argc == APPLY_EFFICIENCY_RESOURCES_PARAMS, "Wrong number of arguments");
        
        napi_value singleParam;
        int32_t resourceNumber {0};
        bool isApply {false};
        int32_t timeOut {0};
        std::string reason {""};
        bool isPersist {false};
        bool isProcess {false};
        NAPI_CALL(env, napi_get_named_property(env, argv[0], "number", &singleParam));
        if (Common::GetInt32NumberValue(env, singleParam, resourceNumber) == nullptr) {
            BGTASK_LOGE("ParseParameters failed, resourceNumber is nullptr.");
            return nullptr;
        }
        NAPI_CALL(env, napi_get_named_property(env, argv[0], "isApply", &singleParam));
        if (Common::GetBooolValue(env, singleParam, isApply) == nullptr) {
            BGTASK_LOGE("ParseParameters failed, isApply is nullptr.");
            return nullptr;
        }
        NAPI_CALL(env, napi_get_named_property(env, argv[0], "timeOut", &singleParam));
        if (Common::GetInt32NumberValue(env, singleParam, timeOut) == nullptr) {
            BGTASK_LOGE("ParseParameters failed, timeOut is nullptr.");
            return nullptr;
        }
        NAPI_CALL(env, napi_get_named_property(env, argv[0], "reason", &singleParam));
        if (Common::GetStringValue(env, singleParam, reason) == nullptr) {
            BGTASK_LOGE("ParseParameters failed, reason is nullptr.");
            return nullptr;
        }
        if (GetNamedBoolValue(env, argv[0], "isPersist", isPersist) == nullptr ||
            GetNamedBoolValue(env, argv[0], "isProcess", isProcess) == nullptr) {
            return nullptr;
        }
        if (timeOut < 0) {
            BGTASK_LOGE("ParseParameters failed, timeOut is negatibve.");
            return nullptr;
        }
        params = EfficiencyResourceInfo{resourceNumber, isApply, timeOut, reason, isPersist, isProcess};
        return Common::NapiGetNull(env);
    }

    bool CheckValidInfo(EfficiencyResourceInfo &params)
    {
        if (params.GetResourceNumber() == 0 || (params.IsApply() && !params.IsPersist()
            && params.GetTimeOut() == 0)) {
            return false;
        }
        return true;
    }

    napi_value ApplyEfficiencyResources(napi_env env, napi_callback_info info)
    {
        EfficiencyResourceInfo params;
        bool isSuccess = false;
        if (ParseParameters(env, info, params) == nullptr) {
            return Common::NapiGetboolean(env, isSuccess);
        }
        if (!CheckValidInfo(params)) {
            BGTASK_LOGD("params make no sense, unnecessary to execute");
            return Common::NapiGetboolean(env, false);
        }
        DelayedSingleton<BackgroundTaskManager>::GetInstance()->ApplyEfficiencyResources(params, isSuccess);
        return Common::NapiGetboolean(env, isSuccess);
    }

    napi_value ResetAllEfficiencyResources(napi_env env, napi_callback_info info)
    {
        DelayedSingleton<BackgroundTaskManager>::GetInstance()->ResetAllEfficiencyResources();
        return Common::NapiGetNull(env);
    }
}
}