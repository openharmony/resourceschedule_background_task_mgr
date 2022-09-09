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
#include "efficiency_resource_info.h"
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
    napi_value boolValue = nullptr;
    if (napi_has_named_property(env, object, utf8name, &hasNamedProperty) == napi_ok && hasNamedProperty) {
        NAPI_CALL(env, napi_get_named_property(env, object, utf8name, &boolValue));
        if (!Common::GetBooleanValue(env, boolValue, result)) {
            BGTASK_LOGE("ParseParameters failed, %{public}s is nullptr", utf8name);
            return nullptr;
        }
        BGTASK_LOGD("GetNamedBoolValue: %{public}s is %{public}d", utf8name, result);
    }
    return Common::NapiGetNull(env);
}

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, EfficiencyResourceInfo &params)
{
    size_t argc = APPLY_EFFICIENCY_RESOURCES_PARAMS;
    napi_value argv[APPLY_EFFICIENCY_RESOURCES_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc == APPLY_EFFICIENCY_RESOURCES_PARAMS, "Wrong number of arguments");
    
    napi_value singleParam = nullptr;
    int32_t resourceNumber {0};
    bool isApply {false};
    int32_t timeOut {0};
    std::string reason {""};
    bool isPersist {false};
    bool isProcess {false};
    NAPI_CALL(env, napi_get_named_property(env, argv[0], "resourceTypes", &singleParam));
    if (!Common::GetInt32NumberValue(env, singleParam, resourceNumber)) {
        BGTASK_LOGE("ParseParameters failed, resourceNumber is nullptr");
        return nullptr;
    }

    NAPI_CALL(env, napi_get_named_property(env, argv[0], "isApply", &singleParam));
    if (!Common::GetBooleanValue(env, singleParam, isApply)) {
        BGTASK_LOGE("ParseParameters failed, isApply is nullptr");
        return nullptr;
    }

    NAPI_CALL(env, napi_get_named_property(env, argv[0], "timeOut", &singleParam));
    if (!Common::GetInt32NumberValue(env, singleParam, timeOut)) {
        BGTASK_LOGE("ParseParameters failed, timeOut is nullptr");
        return nullptr;
    }
    NAPI_CALL(env, napi_get_named_property(env, argv[0], "reason", &singleParam));
    if (!Common::GetStringValue(env, singleParam, reason)) {
        BGTASK_LOGE("ParseParameters failed, reason is nullptr");
        return nullptr;
    }
    if (!GetNamedBoolValue(env, argv[0], "isPersist", isPersist) ||
        !GetNamedBoolValue(env, argv[0], "isProcess", isProcess)) {
        return nullptr;
    }
    if (timeOut < 0) {
        BGTASK_LOGE("ParseParameters failed, timeOut is negative");
        return nullptr;
    }
    params = EfficiencyResourceInfo {resourceNumber, isApply, timeOut, reason, isPersist, isProcess};
    return Common::NapiGetNull(env);
}

bool CheckValidInfo(const EfficiencyResourceInfo &params)
{
    if (params.GetResourceNumber() == 0 || (params.IsApply() && !params.IsPersist()
        && params.GetTimeOut() == 0)) {
        return false;
    }
    return true;
}

napi_value ApplyEfficiencyResources(napi_env env, napi_callback_info info)
{
    BGTASK_LOGD("start ApplyEfficiencyResources");
    EfficiencyResourceInfo params;
    napi_value result = nullptr;
    if (ParseParameters(env, info, params) == nullptr || !CheckValidInfo(params)) {
        NAPI_CALL(env, napi_get_boolean(env, false, &result));
        return result;
    }
    ErrCode suc = DelayedSingleton<BackgroundTaskManager>::GetInstance()->ApplyEfficiencyResources(params);
    NAPI_CALL(env, napi_get_boolean(env, suc == ERR_OK, &result));
    return result;
}

napi_value ResetAllEfficiencyResources(napi_env env, napi_callback_info info)
{
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->ResetAllEfficiencyResources();
    return Common::NapiGetNull(env);
}
}
}