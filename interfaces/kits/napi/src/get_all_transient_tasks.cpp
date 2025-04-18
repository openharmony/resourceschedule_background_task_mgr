/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "get_all_transient_tasks.h"

#include "singleton.h"

#include "background_task_manager.h"
#include "hitrace_meter.h"
#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
struct AsyncCallbackInfoGetAllTransientTasks : public AsyncWorkData {
    explicit AsyncCallbackInfoGetAllTransientTasks(napi_env env) : AsyncWorkData(env) {}
    int32_t remainingQuota = 0; // out
    std::vector<std::shared_ptr<DelaySuspendInfo>> list; // out
};

void GetAllTransientTasksExecuteCB(napi_env env, void *data)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Napi::GetAllContinuousTasksExecuteCB");
    AsyncCallbackInfoGetAllTransientTasks *asyncCallbackInfo =
        static_cast<AsyncCallbackInfoGetAllTransientTasks *>(data);
    if (asyncCallbackInfo == nullptr || asyncCallbackInfo->errCode != ERR_OK) {
        BGTASK_LOGE("input params error");
        return;
    }
    asyncCallbackInfo->errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->GetAllTransientTasks(
        asyncCallbackInfo->remainingQuota, asyncCallbackInfo->list);
}

void GetAllTransientTasksPromiseCompletedCB(napi_env env, napi_status status, void *data)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Napi::GetAllTransientTasksPromiseCompletedCB");
    AsyncCallbackInfoGetAllTransientTasks *asyncCallbackInfo =
        static_cast<AsyncCallbackInfoGetAllTransientTasks *>(data);
    std::unique_ptr<AsyncCallbackInfoGetAllTransientTasks> callbackPtr {asyncCallbackInfo};
    napi_value result {nullptr};
    if (asyncCallbackInfo->errCode == ERR_OK) {
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &result));
        napi_value napiRemainingQuota = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, asyncCallbackInfo->remainingQuota, &napiRemainingQuota));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "remainingQuota", napiRemainingQuota));
        
        napi_value info {nullptr};
        NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &info));
        if (asyncCallbackInfo->list.size() > 0) {
            uint32_t count = 0;
            for (const auto &delaySuspendInfo : asyncCallbackInfo->list) {
                napi_value napiWork = Common::GetNapiDelaySuspendInfo(env, delaySuspendInfo);
                NAPI_CALL_RETURN_VOID(env, napi_set_element(env, info, count, napiWork));
                count++;
            }
        }
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "transientTasks", info));
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, asyncCallbackInfo->deferred, result));
    } else {
        std::string errMsg = Common::FindErrMsg(env, asyncCallbackInfo->errCode);
        int32_t errCodeInfo = Common::FindErrCode(env, asyncCallbackInfo->errCode);
        result = Common::GetCallbackErrorValue(env, errCodeInfo, errMsg);
        NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, asyncCallbackInfo->deferred, result));
    }
    callbackPtr.release();
}

napi_value GetAllTransientTasksPromise(napi_env env, AsyncCallbackInfoGetAllTransientTasks *asyncCallbackInfo,
    bool isThrow)
{
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("param is nullptr");
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise {nullptr};
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    asyncCallbackInfo->deferred = deferred;
    NAPI_CALL(env, napi_create_async_work(env,
        nullptr,
        resourceName,
        GetAllTransientTasksExecuteCB,
        GetAllTransientTasksPromiseCompletedCB,
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    return promise;
}

napi_value GetAllTransientTasks(napi_env env, napi_callback_info info, bool isThrow)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS,
        "BackgroundTaskManager::ContinuousTask::Napi::GetAllTransientTasks");
    if (env == nullptr) {
        BGTASK_LOGE("env param invaild.");
        return Common::NapiGetNull(env);
    }
    AsyncCallbackInfoGetAllTransientTasks *asyncCallbackInfo =
        new (std::nothrow) AsyncCallbackInfoGetAllTransientTasks(env);
    if (asyncCallbackInfo == nullptr) {
        BGTASK_LOGE("asyncCallbackInfo == nullpter");
        return Common::NapiGetNull(env);
    }
    std::unique_ptr<AsyncCallbackInfoGetAllTransientTasks> callbackPtr {asyncCallbackInfo};
    
    napi_value ret {nullptr};
    ret = GetAllTransientTasksPromise(env, asyncCallbackInfo, isThrow);
    callbackPtr.release();
    if (ret == nullptr) {
        BGTASK_LOGE("ret is nullpter");
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        return Common::NapiGetNull(env);
    }
    return ret;
}

napi_value GetAllTransientTasksThrow(napi_env env, napi_callback_info info)
{
    return GetAllTransientTasks(env, info, true);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS