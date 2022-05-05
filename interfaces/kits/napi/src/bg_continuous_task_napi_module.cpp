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

#include "bg_continuous_task_napi_module.h"

#include "ability.h"
#include "want_agent.h"
#include "napi_base_context.h"

#include "background_mode.h"
#include "background_task_mgr_helper.h"
#include "bgtaskmgr_inner_errors.h"
#include "common.h"
#include "continuous_task_log.h"
#include "continuous_task_param.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr uint32_t MAX_START_BG_RUNNING_PARAMS = 4;
static constexpr uint32_t MAX_STOP_BG_RUNNING_PARAMS = 2;
static constexpr uint32_t CALLBACK_RESULT_PARAMS_NUM = 2;
static constexpr uint32_t BG_MODE_ID_BEGIN = 1;
static constexpr uint32_t BG_MODE_ID_END = 9;
}

struct AsyncCallbackInfo : public AsyncWorkData {
    explicit AsyncCallbackInfo(napi_env env) : AsyncWorkData(env) {}
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext {nullptr};
    uint32_t bgMode {0};
    AbilityRuntime::WantAgent::WantAgent *wantAgent {nullptr};
};

napi_value WrapVoidToJS(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

napi_value WrapUndefinedToJS(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value GetCallbackErrorValue(napi_env env, int32_t errCode)
{
    napi_value jsObject = nullptr;
    napi_value jsValue = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &jsValue));
    NAPI_CALL(env, napi_create_object(env, &jsObject));
    NAPI_CALL(env, napi_set_named_property(env, jsObject, "code", jsValue));
    return jsObject;
}

napi_value GetAbilityContext(const napi_env &env, const napi_value &value,
    std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext)
{
    bool stageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, value, stageMode);
    BGTASK_LOGI("is stage mode: %{public}s", stageMode ? "true" : "false");

    if (status != napi_ok || !stageMode) {
        BGTASK_LOGI("Getting context with FA model");
        auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
        if (!ability) {
            BGTASK_LOGE("Failed to get native ability instance");
            return nullptr;
        }
        abilityContext = ability->GetAbilityContext();
        if (!abilityContext) {
            BGTASK_LOGE("get FA model ability context failed");
            return nullptr;
        }
        return WrapVoidToJS(env);
    } else {
        BGTASK_LOGI("Getting context with stage model");
        auto context = AbilityRuntime::GetStageModeContext(env, value);
        if (!context) {
            BGTASK_LOGE("get context failed");
            return nullptr;
        }
        abilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
        if (!abilityContext) {
            BGTASK_LOGE("get Stage model ability context failed");
            return nullptr;
        }
        return WrapVoidToJS(env);
    }
}

void StartBackgroundRunningExecuteCB(napi_env env, void *data)
{
    BGTASK_LOGI("begin");
    AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("asyncCallbackInfo is nullptr");
        return;
    }
    if (asyncCallbackInfo->errCode != ERR_OK) {
        BGTASK_LOGE("input params parse failed");
        return;
    }
    if (asyncCallbackInfo->abilityContext == nullptr) {
        asyncCallbackInfo->errCode = ERR_BGTASK_INVALID_PARAM;
        BGTASK_LOGE("abilityContext is null");
        return;
    }
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    if (info == nullptr) {
        BGTASK_LOGE("ability info is null");
        asyncCallbackInfo->errCode = ERR_BGTASK_INVALID_PARAM;
        return;
    }

    if (asyncCallbackInfo->wantAgent == nullptr) {
        BGTASK_LOGE("wantAgent param is nullptr");
        asyncCallbackInfo->errCode = ERR_BGTASK_INVALID_PARAM;
        return;
    }

    sptr<IRemoteObject> token = asyncCallbackInfo->abilityContext->GetToken();
    if (!token) {
        BGTASK_LOGE("get ability token info failed");
        asyncCallbackInfo->errCode = ERR_BGTASK_INVALID_PARAM;
        return;
    }

    if (asyncCallbackInfo->bgMode < BG_MODE_ID_BEGIN || asyncCallbackInfo->bgMode > BG_MODE_ID_END) {
        BGTASK_LOGE("request background mode id: %{public}u out of range", asyncCallbackInfo->bgMode);
        asyncCallbackInfo->errCode = ERR_BGTASK_INVALID_PARAM;
        return;
    }

    ContinuousTaskParam taskParam = ContinuousTaskParam(true, asyncCallbackInfo->bgMode,
        std::make_shared<AbilityRuntime::WantAgent::WantAgent>(*asyncCallbackInfo->wantAgent), info->name, token);
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestStartBackgroundRunning(taskParam);

    BGTASK_LOGI("end");
}

void CallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    BGTASK_LOGI("begin");
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    std::unique_ptr<AsyncCallbackInfo> callbackPtr {asyncCallbackInfo};
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_value result[CALLBACK_RESULT_PARAMS_NUM] = {0};
    napi_value callResult = 0;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    if (asyncCallbackInfo->errCode == ERR_OK) {
        result[0] = WrapUndefinedToJS(env);
        napi_create_int32(env, 0, &result[1]);
    } else {
        result[1] = WrapUndefinedToJS(env);
        result[0] = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
    }

    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, asyncCallbackInfo->callback, &callback));
    NAPI_CALL_RETURN_VOID(env,
        napi_call_function(env, undefined, callback, CALLBACK_RESULT_PARAMS_NUM, result, &callResult));

    BGTASK_LOGI("end");
}

void PromiseCompletedCB(napi_env env, napi_status status, void *data)
{
    BGTASK_LOGI("begin");
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    std::unique_ptr<AsyncCallbackInfo> callbackPtr {asyncCallbackInfo};
    napi_value result = 0;
    if (asyncCallbackInfo->errCode == ERR_OK) {
        napi_create_int32(env, 0, &result);
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, asyncCallbackInfo->deferred, result));
    } else {
        result = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
        NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, asyncCallbackInfo->deferred, result));
    }

    BGTASK_LOGI("end");
}

napi_value StartBackgroundRunningAsync(
    napi_env env, napi_value *argv, const uint32_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    BGTASK_LOGI("begin");
    if (argv == nullptr || asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("param is nullptr");
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[argCallback], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
    NAPI_CALL(env, napi_create_reference(env, argv[argCallback], 1, &asyncCallbackInfo->callback));

    NAPI_CALL(env, napi_create_async_work(env,
        nullptr,
        resourceName,
        StartBackgroundRunningExecuteCB,
        CallbackCompletedCB,
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

    BGTASK_LOGI("end");
    return WrapVoidToJS(env);
}

napi_value StartBackgroundRunningPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    BGTASK_LOGI("begin");
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("param is nullptr");
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    asyncCallbackInfo->deferred = deferred;

    NAPI_CALL(env, napi_create_async_work(env,
        nullptr,
        resourceName,
        StartBackgroundRunningExecuteCB,
        PromiseCompletedCB,
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    BGTASK_LOGI("end");
    return promise;
}

napi_value GetBackgroundMode(const napi_env &env, const napi_value &value, uint32_t &bgMode)
{
    BGTASK_LOGI("begin");

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
    napi_get_value_uint32(env, value, &bgMode);

    BGTASK_LOGI("get bgmode info: %{public}u", bgMode);
    return WrapVoidToJS(env);
}

napi_value GetWantAgent(const napi_env &env, const napi_value &value, AbilityRuntime::WantAgent::WantAgent *&wantAgent)
{
    BGTASK_LOGI("begin");
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
    napi_unwrap(env, value, (void **)&wantAgent);

    BGTASK_LOGI("end");
    return WrapVoidToJS(env);
}

napi_value StartBackgroundRunning(napi_env env, napi_callback_info info)
{
    BGTASK_LOGI("begin");
    AsyncCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("asyncCallbackInfo == nullpter");
        return WrapVoidToJS(env);
    }
    std::unique_ptr<AsyncCallbackInfo> callbackPtr {asyncCallbackInfo};

    size_t argc = MAX_START_BG_RUNNING_PARAMS;
    napi_value argv[MAX_START_BG_RUNNING_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc > MAX_START_BG_RUNNING_PARAMS) {
        BGTASK_LOGE("wrong param nums");
        return nullptr;
    }

    // argv[0] : context : AbilityContext
    if (GetAbilityContext(env, argv[0], asyncCallbackInfo->abilityContext) == nullptr) {
        BGTASK_LOGE("Get ability context failed");
        asyncCallbackInfo->errCode = ERR_BGTASK_INVALID_PARAM;
    }

    // argv[1] : bgMode : BackgroundMode
    if (GetBackgroundMode(env, argv[1], asyncCallbackInfo->bgMode) == nullptr) {
        BGTASK_LOGE("input bgmode param not number");
        asyncCallbackInfo->errCode = ERR_BGTASK_INVALID_PARAM;
    }

    // argv[2] : wantAgent: WantAgent
    if (GetWantAgent(env, argv[2], asyncCallbackInfo->wantAgent) == nullptr) {
        BGTASK_LOGE("input wantAgent param is not object");
        asyncCallbackInfo->errCode = ERR_BGTASK_INVALID_PARAM;
    }

    napi_value ret = 0;

    if (argc == MAX_START_BG_RUNNING_PARAMS) {
        ret = StartBackgroundRunningAsync(env, argv, MAX_START_BG_RUNNING_PARAMS - 1, asyncCallbackInfo);
    } else {
        ret = StartBackgroundRunningPromise(env, asyncCallbackInfo);
    }

    if (ret == nullptr) {
        BGTASK_LOGE("ret is nullpter");
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    BGTASK_LOGI("end");
    callbackPtr.release();
    return ret;
}

void StopBackgroundRunningExecuteCB(napi_env env, void *data)
{
    BGTASK_LOGI("begin");
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("asyncCallbackInfo is nullptr");
        return;
    }
    if (asyncCallbackInfo->abilityContext == nullptr) {
        asyncCallbackInfo->errCode = ERR_BGTASK_INVALID_PARAM;
        BGTASK_LOGE("ability context is null");
        return;
    }
    const std::shared_ptr<AppExecFwk::AbilityInfo> info = asyncCallbackInfo->abilityContext->GetAbilityInfo();
    if (info == nullptr) {
        BGTASK_LOGE("abilityInfo is null");
        asyncCallbackInfo->errCode = ERR_BGTASK_INVALID_PARAM;
        return;
    }

    sptr<IRemoteObject> token = asyncCallbackInfo->abilityContext->GetToken();
    if (!token) {
        BGTASK_LOGE("get ability token info failed");
        asyncCallbackInfo->errCode = ERR_BGTASK_INVALID_PARAM;
        return;
    }
    asyncCallbackInfo->errCode = BackgroundTaskMgrHelper::RequestStopBackgroundRunning(info->name, token);
}

napi_value StopBackgroundRunningAsync(napi_env env, napi_value *argv,
    const uint32_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    BGTASK_LOGI("begin");
    if (argv == nullptr || asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("param is nullptr");
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, argv[argCallback], 1, &asyncCallbackInfo->callback));
    }

    NAPI_CALL(env, napi_create_async_work(env,
        nullptr,
        resourceName,
        StopBackgroundRunningExecuteCB,
        CallbackCompletedCB,
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    BGTASK_LOGI("end");
    return WrapVoidToJS(env);
}

napi_value StopBackgroundRunningPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    BGTASK_LOGI("begin");
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("param is nullptr");
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = 0;
    napi_create_promise(env, &deferred, &promise);

    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        StopBackgroundRunningExecuteCB,
        PromiseCompletedCB,
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    BGTASK_LOGI("end");
    return promise;
}

napi_value StopBackgroundRunning(napi_env env, napi_callback_info info)
{
    BGTASK_LOGI("begin");
    AsyncCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("asyncCallbackInfo is nullpter");
        return WrapVoidToJS(env);
    }
    std::unique_ptr<AsyncCallbackInfo> callbackPtr {asyncCallbackInfo};

    size_t argc = MAX_STOP_BG_RUNNING_PARAMS;
    napi_value argv[MAX_STOP_BG_RUNNING_PARAMS] = {nullptr};

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc > MAX_STOP_BG_RUNNING_PARAMS) {
        BGTASK_LOGE("wrong param nums");
        return nullptr;
    }

    // argv[0] : context : AbilityContext
    if (GetAbilityContext(env, argv[0], asyncCallbackInfo->abilityContext) == nullptr) {
        BGTASK_LOGE("Get ability context failed");
        asyncCallbackInfo->errCode = ERR_BGTASK_INVALID_PARAM;
    }

    napi_value ret = 0;
    if (argc == MAX_STOP_BG_RUNNING_PARAMS) {
        ret = StopBackgroundRunningAsync(env, argv, MAX_STOP_BG_RUNNING_PARAMS - 1, asyncCallbackInfo);
    } else {
        ret = StopBackgroundRunningPromise(env, asyncCallbackInfo);
    }

    if (ret == nullptr) {
        BGTASK_LOGE("ret is nullpter");
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    BGTASK_LOGI("end");
    callbackPtr.release();
    return ret;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS