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

#include "request_suspend_delay.h"

#include <uv.h>

#include "singleton.h"

#include "background_task_manager.h"
#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
static const uint32_t REQUEST_SUSPEND_DELAY_PARAMS = 2;

struct CallbackReceiveDataWorker {
    napi_env env = nullptr;
    napi_ref ref = nullptr;
};

CallbackInstance::CallbackInstance() {}

CallbackInstance::~CallbackInstance()
{
    if (expiredCallbackInfo_.ref != nullptr) {
        napi_delete_reference(expiredCallbackInfo_.env, expiredCallbackInfo_.ref);
    }
}

void UvQueueWorkOnExpired(uv_work_t *work, int32_t status)
{
    BGTASK_LOGI("OnExpired uv_work_t start");

    if (work == nullptr) {
        BGTASK_LOGE("work is null");
        return;
    }

    CallbackReceiveDataWorker *dataWorkerData = (CallbackReceiveDataWorker *)work->data;
    if (dataWorkerData == nullptr) {
        BGTASK_LOGE("dataWorkerData is null");
        delete work;
        work = nullptr;
        return;
    }

    Common::SetCallback(dataWorkerData->env, dataWorkerData->ref, Common::NapiGetNull(dataWorkerData->env));

    delete dataWorkerData;
    dataWorkerData = nullptr;
    delete work;
    work = nullptr;
}

void CallbackInstance::OnExpired()
{
    BGTASK_LOGI("enter");

    if (expiredCallbackInfo_.ref == nullptr) {
        BGTASK_LOGE("expired callback unset");
        return;
    }

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(expiredCallbackInfo_.env, &loop);
    if (loop == nullptr) {
        BGTASK_LOGE("loop instance is nullptr");
        return;
    }

    CallbackReceiveDataWorker *dataWorker = new (std::nothrow) CallbackReceiveDataWorker();
    if (dataWorker == nullptr) {
        BGTASK_LOGE("new dataWorker failed");
        return;
    }

    dataWorker->env = expiredCallbackInfo_.env;
    dataWorker->ref = expiredCallbackInfo_.ref;

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        BGTASK_LOGE("new work failed");
        delete dataWorker;
        dataWorker = nullptr;
        return;
    }

    work->data = (void *)dataWorker;

    int32_t ret = uv_queue_work(loop, work, [](uv_work_t *work) {}, UvQueueWorkOnExpired);
    if (ret != 0) {
        delete dataWorker;
        dataWorker = nullptr;
        delete work;
        work = nullptr;
    }
}

void CallbackInstance::SetCallbackInfo(const napi_env &env, const napi_ref &ref)
{
    expiredCallbackInfo_.env = env;
    expiredCallbackInfo_.ref = ref;
}

napi_value GetExpiredCallback(
    const napi_env &env, const napi_value &value, CallbackInstancesInfo &callbcakInfo)
{
    napi_ref result = nullptr;

    callbcakInfo.callback = new (std::nothrow) CallbackInstance();
    if (callbcakInfo.callback == nullptr) {
        BGTASK_LOGE("callback is null");
        return nullptr;
    }

    napi_create_reference(env, value, 1, &result);
    callbcakInfo.callback->SetCallbackInfo(env, result);

    return Common::NapiGetNull(env);
}

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info,
    std::u16string &reason, CallbackInstance *&callback)
{
    size_t argc = REQUEST_SUSPEND_DELAY_PARAMS;
    napi_value argv[REQUEST_SUSPEND_DELAY_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc == REQUEST_SUSPEND_DELAY_PARAMS, "Wrong number of arguments");

    // argv[0] : reason
    if (Common::GetU16StringValue(env, argv[0], reason) == nullptr) {
        BGTASK_LOGE("ParseParameters failed, reason is nullptr ");
        return nullptr;
    }

    // arg[1] : callback
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Object expected.");

    CallbackInstancesInfo callbackInstancesInfo;
    if (GetExpiredCallback(env, argv[1], callbackInstancesInfo) == nullptr) {
        BGTASK_LOGE("CallbackInstancesInfo parse failed");
        return nullptr;
    }
    callback = callbackInstancesInfo.callback;
    return Common::NapiGetNull(env);
}

napi_value RequestSuspendDelay(napi_env env, napi_callback_info info)
{
    CallbackInstance *objectInfo = nullptr;
    std::u16string reason;
    if (ParseParameters(env, info, reason, objectInfo) == nullptr) {
        if (objectInfo) {
            delete objectInfo;
            objectInfo = nullptr;
        }
        return Common::NapiGetNull(env);
    }

    std::shared_ptr<DelaySuspendInfo> delaySuspendInfo;
    DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        RequestSuspendDelay(reason, *objectInfo, delaySuspendInfo);

    if (objectInfo) {
        delete objectInfo;
        objectInfo = nullptr;
    }

    napi_value result = nullptr;
    napi_create_object(env, &result);
    if (!Common::SetDelaySuspendInfo(env, delaySuspendInfo, result)) {
        BGTASK_LOGW("Set DelaySuspendInfo object failed");
    }
    return result;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS