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
std::map<int32_t, std::shared_ptr<ExpiredCallback>> callbackInstances_;
static const uint32_t REQUEST_SUSPEND_DELAY_PARAMS = 2;

struct CallbackReceiveDataWorker {
    napi_env env = nullptr;
    napi_ref ref = nullptr;
    std::shared_ptr<ExpiredCallback> callback = nullptr;
};

CallbackInstance::CallbackInstance() {}

CallbackInstance::~CallbackInstance()
{
    if (expiredCallbackInfo_.ref == nullptr) {
        return;
    }
    DeleteNapiRef();
}

void UvQueueWorkDeleteRef(uv_work_t *work, int32_t status)
{
    if (work == nullptr) {
        return;
    }
    CallbackReceiveDataWorker *dataWorkerData = static_cast<CallbackReceiveDataWorker *>(work->data);
    if (dataWorkerData == nullptr) {
        delete work;
        work = nullptr;
        return;
    }
    napi_delete_reference(dataWorkerData->env, dataWorkerData->ref);
    delete dataWorkerData;
    dataWorkerData = nullptr;
    delete work;
}

void CallbackInstance::DeleteNapiRef()
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(expiredCallbackInfo_.env, &loop);
    if (loop == nullptr) {
        return;
    }
    CallbackReceiveDataWorker *dataWorker = new (std::nothrow) CallbackReceiveDataWorker();
    if (dataWorker == nullptr) {
        return;
    }

    dataWorker->env = expiredCallbackInfo_.env;
    dataWorker->ref = expiredCallbackInfo_.ref;
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        BGTASK_LOGE("DeleteNapiRef new work failed");
        delete dataWorker;
        dataWorker = nullptr;
        return;
    }
    work->data = (void *)dataWorker;

    int32_t ret = uv_queue_work(loop, work, [](uv_work_t *work) {}, UvQueueWorkDeleteRef);
    if (ret != 0) {
        delete dataWorker;
        dataWorker = nullptr;
        delete work;
        work = nullptr;
    }
}

void UvQueueWorkOnExpired(uv_work_t *work, int32_t status)
{
    BGTASK_LOGD("OnExpired uv_work_t start");

    if (work == nullptr) {
        BGTASK_LOGE("UvQueueWorkOnExpired work is null");
        return;
    }

    CallbackReceiveDataWorker *dataWorkerData = static_cast<CallbackReceiveDataWorker *>(work->data);
    if (dataWorkerData == nullptr) {
        BGTASK_LOGE("UvQueueWorkOnExpired dataWorkerData is null");
        delete work;
        work = nullptr;
        return;
    }

    Common::SetCallback(dataWorkerData->env, dataWorkerData->ref, Common::NapiGetNull(dataWorkerData->env));

    auto findCallback = std::find_if(callbackInstances_.begin(), callbackInstances_.end(),
        [&](const auto& callbackInstance) { return callbackInstance.second == dataWorkerData->callback; }
    );
    if (findCallback != callbackInstances_.end()) {
        callbackInstances_.erase(findCallback);
    }

    delete dataWorkerData;
    dataWorkerData = nullptr;
    delete work;
}

void CallbackInstance::OnExpired()
{
    auto findCallback = std::find_if(callbackInstances_.begin(), callbackInstances_.end(),
        [&](const auto& callbackInstance) { return callbackInstance.second.get() == this; }
    );
    if (findCallback == callbackInstances_.end()) {
        BGTASK_LOGI("expired callback is not found");
        return;
    }

    if (expiredCallbackInfo_.ref == nullptr) {
        BGTASK_LOGE("expired callback unset");
        callbackInstances_.erase(findCallback);
        return;
    }

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(expiredCallbackInfo_.env, &loop);
    if (loop == nullptr) {
        BGTASK_LOGE("loop instance is nullptr");
        callbackInstances_.erase(findCallback);
        return;
    }

    CallbackReceiveDataWorker *dataWorker = new (std::nothrow) CallbackReceiveDataWorker();
    if (dataWorker == nullptr) {
        BGTASK_LOGE("new dataWorker failed");
        callbackInstances_.erase(findCallback);
        return;
    }

    dataWorker->env = expiredCallbackInfo_.env;
    dataWorker->ref = expiredCallbackInfo_.ref;
    dataWorker->callback = shared_from_this();

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        BGTASK_LOGW("OnExpired new work failed");
        delete dataWorker;
        dataWorker = nullptr;
        callbackInstances_.erase(findCallback);
        return;
    }

    work->data = (void *)dataWorker;

    int32_t ret = uv_queue_work(loop, work, [](uv_work_t *work) {}, UvQueueWorkOnExpired);
    if (ret != 0) {
        delete dataWorker;
        dataWorker = nullptr;
        delete work;
        work = nullptr;
        callbackInstances_.erase(findCallback);
    }
}

void CallbackInstance::SetCallbackInfo(const napi_env &env, const napi_ref &ref)
{
    expiredCallbackInfo_.env = env;
    expiredCallbackInfo_.ref = ref;
}

napi_value GetExpiredCallback(
    const napi_env &env, const napi_value &value, std::shared_ptr<CallbackInstance> &callback)
{
    napi_ref result = nullptr;
    callback = std::make_shared<CallbackInstance>();
    callback->Init();

    napi_create_reference(env, value, 1, &result);
    callback->SetCallbackInfo(env, result);

    return Common::NapiGetNull(env);
}

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info,
    std::u16string &reason, std::shared_ptr<CallbackInstance> &callback, bool isThrow)
{
    size_t argc = REQUEST_SUSPEND_DELAY_PARAMS;
    napi_value argv[REQUEST_SUSPEND_DELAY_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != REQUEST_SUSPEND_DELAY_PARAMS) {
        Common::HandleParamErr(env, ERR_PARAM_NUMBER_ERR, isThrow);
        return nullptr;
    }

    // argv[0] : reason
    if (Common::GetU16StringValue(env, argv[0], reason) == nullptr) {
        BGTASK_LOGE("ParseParameters failed, reason is nullptr.");
        Common::HandleParamErr(env, ERR_REASON_NULL_OR_TYPE_ERR, isThrow);
        return nullptr;
    }

    // arg[1] : callback
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
    if (valuetype != napi_function) {
        Common::HandleParamErr(env, ERR_CALLBACK_NULL_OR_TYPE_ERR, isThrow);
        return nullptr;
    }

    if (GetExpiredCallback(env, argv[1], callback) == nullptr) {
        BGTASK_LOGE("ExpiredCallback parse failed");
        Common::HandleParamErr(env, ERR_CALLBACK_NULL_OR_TYPE_ERR, isThrow);
        return nullptr;
    }
    return Common::NapiGetNull(env);
}

napi_value RequestSuspendDelay(napi_env env, napi_callback_info info, bool isThrow)
{
    std::shared_ptr<CallbackInstance> callback = nullptr;
    std::u16string reason;
    if (ParseParameters(env, info, reason, callback, isThrow) == nullptr) {
        return Common::NapiGetNull(env);
    }

    std::shared_ptr<DelaySuspendInfo> delaySuspendInfo {nullptr};
    ErrCode errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        RequestSuspendDelay(reason, *callback, delaySuspendInfo);
    Common::HandleErrCode(env, errCode, isThrow);
    if (!delaySuspendInfo) {
        return Common::NapiGetNull(env);
    }
    callbackInstances_[delaySuspendInfo->GetRequestId()] = callback;

    napi_value result = nullptr;
    napi_create_object(env, &result);
    if (!Common::SetDelaySuspendInfo(env, delaySuspendInfo, result)) {
        BGTASK_LOGW("Set DelaySuspendInfo object failed");
    }
    return result;
}

napi_value RequestSuspendDelay(napi_env env, napi_callback_info info)
{
    return RequestSuspendDelay(env, info, false);
}

napi_value RequestSuspendDelayThrow(napi_env env, napi_callback_info info)
{
    return RequestSuspendDelay(env, info, true);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS