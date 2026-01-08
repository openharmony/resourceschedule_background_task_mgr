/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "transient_task_api.h"
#include "singleton.h"
#include "background_task_manager.h"
#include "transient_task_log.h"
#include "bgtaskmgr_inner_errors.h"
#include "background_task_mgr_helper.h"
#include "string_ex.h"
#include "callback.h"

namespace OHOS {
namespace BackgroundTaskMgr {
std::map<int32_t, std::shared_ptr<Callback>> callbackInstances_;
std::recursive_mutex callbackLock_;
const int32_t INNER_ERROR_SHIFT = 100;

extern "C" {
int32_t OH_BackgroundTaskManager_RequestSuspendDelay(const char* reason,
    TransientTask_Callback callback, TransientTask_DelaySuspendInfo *info)
    {
        if (!callback || !info) {
            LOGI("OH_BackgroundTaskManager_RequestSuspendDelay param is null");
            return ERR_TRANSIENT_TASK_INVALID_PARAM;
        }
        LOGI("OH_BackgroundTaskManager_RequestSuspendDelay start");
        std::string tmp(reason);
        std::u16string reasonu16(Str8ToStr16(tmp));
        std::shared_ptr<DelaySuspendInfo> delaySuspendInfo {nullptr};
        std::shared_ptr<Callback> expiredCallback = std::make_shared<Callback>();
        expiredCallback->Init();
        expiredCallback->SetCallbackInfo(callback);

        ErrCode errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        RequestSuspendDelay(reasonu16, *expiredCallback, delaySuspendInfo);
        if (errCode != 0) {
            return errCode / INNER_ERROR_SHIFT;
        }
        info->requestId = delaySuspendInfo->GetRequestId();
        info->actualDelayTime = delaySuspendInfo->GetActualDelayTime();
        std::lock_guard<std::recursive_mutex> lock(callbackLock_);
        callbackInstances_[delaySuspendInfo->GetRequestId()] = expiredCallback;
        return ERR_TRANSIENT_TASK_OK;
    }

int32_t OH_BackgroundTaskManager_GetRemainingDelayTime(int32_t requestId, int32_t *delayTime)
{
    auto errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        GetRemainingDelayTime(requestId, *delayTime);
    return errCode / INNER_ERROR_SHIFT;
}

int32_t OH_BackgroundTaskManager_CancelSuspendDelay(int32_t requestId)
{
    auto errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->CancelSuspendDelay(requestId);
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    auto findCallback = callbackInstances_.find(requestId);
    if (findCallback != callbackInstances_.end()) {
        LOGI("CancelSuspendDelay erase");
        callbackInstances_.erase(findCallback);
        LOGI("CancelSuspendDelay erase ok");
    }

    return errCode / INNER_ERROR_SHIFT;
}

int32_t OH_BackgroundTaskManager_GetTransientTaskInfo(TransientTask_TransientTaskInfo *info)
{
    int32_t remainingQuotaValue = 0;
    std::vector<std::shared_ptr<DelaySuspendInfo>> listValue;
    auto errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        GetAllTransientTasks(remainingQuotaValue, listValue);
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    if (errCode != 0) {
        return errCode / INNER_ERROR_SHIFT;
    }
    info->remainingQuota = remainingQuotaValue;
    int index = 0;
    for (const auto &transientTasksRecord : listValue) {
        info->transientTasks[index].requestId = transientTasksRecord->GetRequestId();
        info->transientTasks[index].actualDelayTime = transientTasksRecord->GetActualDelayTime();
        index++;
    }
    return ERR_TRANSIENT_TASK_OK;
}

Callback::Callback() {}

Callback::~Callback()
{
    LOGI("~Callback");
}

void Callback::OnExpired()
{
    LOGI("OnExpired start");
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    auto findCallback = std::find_if(callbackInstances_.begin(), callbackInstances_.end(),
        [&](const auto& callbackInstance) { return callbackInstance.second.get() == this; }
    );
    if (findCallback == callbackInstances_.end()) {
        LOGI("expired callback not found");
        return;
    }
    auto callback = findCallback->second;
    LOGI("call native callback");
    callback->ffiCallback_();
    callbackInstances_.erase(findCallback);
    LOGI("OnExpired end");
}

void Callback::OnExpiredAuth(int32_t authResult) {}

void Callback::SetCallbackInfo(void (*callback)())
{
    ffiCallback_ = callback;
}
}
}
}