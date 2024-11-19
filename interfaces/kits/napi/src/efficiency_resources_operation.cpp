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

#include "common.h"
#include "background_task_manager.h"
#include "efficiency_resource_info.h"
#include "efficiency_resource_log.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
    static constexpr int32_t APPLY_EFFICIENCY_RESOURCES_PARAMS = 1;
}

napi_value GetNamedBoolValue(const napi_env &env, napi_value &object, const char* utf8name,
    bool& result, bool isNecessary)
{
    bool hasNamedProperty = false;
    napi_value boolValue = nullptr;
    if (napi_has_named_property(env, object, utf8name, &hasNamedProperty) != napi_ok || !hasNamedProperty) {
        if (isNecessary) {
            BGTASK_LOGE("ParseParameters failed, %{public}s not exist, is nullptr", utf8name);
            return nullptr;
        } else {
            return Common::NapiGetNull(env);
        }
    }
    BGTASK_NAPI_CALL(env, napi_get_named_property(env, object, utf8name, &boolValue));
    if (!Common::GetBooleanValue(env, boolValue, result)) {
        BGTASK_LOGE("ParseParameters failed, %{public}s is nullptr", utf8name);
        return nullptr;
    }
    BGTASK_LOGD("GetNamedBoolValue: %{public}s is %{public}d", utf8name, result);
    return Common::NapiGetNull(env);
}

napi_value GetNamedInt32Value(const napi_env &env, napi_value &object, const char* utf8name,
    int32_t& result)
{
    bool hasNamedProperty = false;
    napi_value intValue = nullptr;
    if (napi_has_named_property(env, object, utf8name, &hasNamedProperty) != napi_ok || !hasNamedProperty) {
        BGTASK_LOGE("ParseParameters failed, %{public}s not exist, is nullptr", utf8name);
        return nullptr;
    }
    BGTASK_NAPI_CALL(env, napi_get_named_property(env, object, utf8name, &intValue));
    if (!Common::GetInt32NumberValue(env, intValue, result)) {
        BGTASK_LOGE("ParseParameters failed, %{public}s is nullptr", utf8name);
        return nullptr;
    }
    if (result < 0) {
        BGTASK_LOGE("%{public}s can't be a negtive number: %{public}d", utf8name, result);
        return nullptr;
    }
    BGTASK_LOGD("GetNamedInt32Value: %{public}s is %{public}d", utf8name, result);
    return Common::NapiGetNull(env);
}

napi_value GetNamedStringValue(const napi_env &env, napi_value &object, std::string& result)
{
    bool hasNamedProperty = false;
    napi_value stringValue = nullptr;
    if (napi_has_named_property(env, object, "reason", &hasNamedProperty) != napi_ok || !hasNamedProperty) {
        BGTASK_LOGE("ParseParameters failed, reason not exist, is nullptr");
        return nullptr;
    }
    BGTASK_NAPI_CALL(env, napi_get_named_property(env, object, "reason", &stringValue));
    if (!Common::GetStringValue(env, stringValue, result)) {
        BGTASK_LOGE("ParseParameters failed, reason is nullptr");
        return nullptr;
    }
    BGTASK_LOGD("GetNamedStringValue: reason is %{public}s", result.c_str());
    return Common::NapiGetNull(env);
}

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info,
    EfficiencyResourceInfo &params, bool isThrow)
{
    size_t argc = APPLY_EFFICIENCY_RESOURCES_PARAMS;
    napi_value argv[APPLY_EFFICIENCY_RESOURCES_PARAMS] = {nullptr};
    BGTASK_NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != APPLY_EFFICIENCY_RESOURCES_PARAMS) {
        Common::HandleParamErr(env, ERR_PARAM_NUMBER_ERR, isThrow);
        return nullptr;
    }
    int32_t resourceNumber {0};
    bool isApply {false};
    int32_t timeOut {0};
    std::string reason {""};
    bool isPersist {false};
    bool isProcess {false};

    if (!GetNamedInt32Value(env, argv[0], "resourceTypes", resourceNumber)) {
        Common::HandleParamErr(env, ERR_RESOURCE_TYPES_INVALID, isThrow);
        return nullptr;
    }
    if (!GetNamedBoolValue(env, argv[0], "isApply", isApply, true)) {
        Common::HandleParamErr(env, ERR_ISAPPLY_NULL_OR_TYPE_ERR, isThrow);
        return nullptr;
    }
    if (!GetNamedInt32Value(env, argv[0], "timeOut", timeOut)) {
        Common::HandleParamErr(env, ERR_TIMEOUT_INVALID, isThrow);
        return nullptr;
    }
    if (!GetNamedStringValue(env, argv[0], reason)) {
        Common::HandleParamErr(env, ERR_REASON_NULL_OR_TYPE_ERR, isThrow);
        return nullptr;
    }
    if (!GetNamedBoolValue(env, argv[0], "isPersist", isPersist, false)) {
        Common::HandleParamErr(env, ERR_ISPERSIST_NULL_OR_TYPE_ERR, isThrow);
        return nullptr;
    }
    if (!GetNamedBoolValue(env, argv[0], "isProcess", isProcess, false)) {
        Common::HandleParamErr(env, ERR_ISPROCESS_NULL_OR_TYPE_ERR, isThrow);
        return nullptr;
    }
    params = EfficiencyResourceInfo {resourceNumber, isApply, timeOut, reason, isPersist, isProcess};
    return Common::NapiGetNull(env);
}

bool CheckValidInfo(napi_env env, const EfficiencyResourceInfo &params, bool isThrow)
{
    if (params.GetResourceNumber() == 0) {
        Common::HandleParamErr(env, ERR_RESOURCE_TYPES_INVALID, isThrow);
        return false;
    }
    if (params.IsApply() && !params.IsPersist() && params.GetTimeOut() == 0) {
        Common::HandleParamErr(env, ERR_TIMEOUT_INVALID, isThrow);
        return false;
    }
    return true;
}

napi_value ApplyEfficiencyResources(napi_env env, napi_callback_info info)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::EfficiencyResource::Napi::ApplyEfficiencyResources");
    EfficiencyResourceInfo params;
    if (ParseParameters(env, info, params, true) == nullptr || !CheckValidInfo(env, params, true)) {
        return Common::NapiGetNull(env);
    }
    ErrCode errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->ApplyEfficiencyResources(params);
    Common::HandleErrCode(env, errCode, true);
    return Common::NapiGetNull(env);
}

napi_value ResetAllEfficiencyResources(napi_env env, napi_callback_info info)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::EfficiencyResource::Napi::ResetAllEfficiencyResources");
    ErrCode errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->ResetAllEfficiencyResources();
    Common::HandleErrCode(env, errCode, true);
    return Common::NapiGetNull(env);
}
}
}