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

#include "background_task_mgr_ffi.h"
#include "singleton.h"
#include "background_task_manager.h"
#include "transient_task_log.h"
#include "bgtaskmgr_inner_errors.h"
#include "background_task_mgr_helper.h"
#include "string_ex.h"
#include "cj_lambda.h"
#include "cj_fn_invoker.h"
#include "callback_ffi.h"
#include "background_task_mgr_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
std::map<int32_t, std::shared_ptr<CallbackFFI>> callbackInstances_;
std::mutex callbackLock_;

extern "C" {
    int32_t CJ_RequestSuspendDelay(void (*callback)(), const char* reason, RetDelaySuspendInfo* ret)
    {
        LOGI("CJ_RequestSuspendDelay start");
        std::string tmp(reason);
        std::u16string reasonu16(Str8ToStr16(tmp));
        std::shared_ptr<DelaySuspendInfo> delaySuspendInfo {nullptr};
        std::shared_ptr<CallbackFFI> callbackffi = std::make_shared<CallbackFFI>();
        callbackffi->Init();
        callbackffi->SetCallbackInfo(callback);

        ErrCode errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->
        RequestSuspendDelay(reasonu16, *callbackffi, delaySuspendInfo);
        if (errCode != SUCCESS_CODE) {
            return errCode;
        }
        ret->requestId = delaySuspendInfo->GetRequestId();
        ret->actualDelayTime = delaySuspendInfo->GetActualDelayTime();
        
        {
            std::lock_guard<std::mutex> lock(callbackLock_);
            callbackInstances_[delaySuspendInfo->GetRequestId()] = callbackffi;
        }
        return errCode;
    }

    int32_t CJ_GetRemainingDelayTime(int32_t requestId, int32_t& delayTime)
    {
        return DelayedSingleton<BackgroundTaskManager>::GetInstance()->
            GetRemainingDelayTime(requestId, delayTime);
    }

    int32_t CJ_CancelSuspendDelay(int32_t requestId)
    {
        auto errCode = DelayedSingleton<BackgroundTaskManager>::GetInstance()->CancelSuspendDelay(requestId);
        std::lock_guard<std::mutex> lock(callbackLock_);
        auto findCallback = callbackInstances_.find(requestId);
        if (findCallback != callbackInstances_.end()) {
            LOGI("CJ_CancelSuspendDelay erase");
            callbackInstances_.erase(findCallback);
            LOGI("CJ_CancelSuspendDelay erase ok");
        }
        return errCode;
    }

    int32_t CJ_StopBackgroundRunning(OHOS::AbilityRuntime::AbilityContext* context)
    {
        const std::shared_ptr<AppExecFwk::AbilityInfo> info = context->GetAbilityInfo();
        sptr<IRemoteObject> token = context->GetToken();
        return BackgroundTaskMgrHelper::RequestStopBackgroundRunning(info->name, token);
    }

    int32_t CJ_ApplyEfficiencyResources(RetEfficiencyResourcesRequest request)
    {
        EfficiencyResourceInfo params{request.resourceTypes, request.isApply, request.timeOut, request.reason,
            request.isPersist, request.isProcess};
        return DelayedSingleton<BackgroundTaskManager>::GetInstance()->ApplyEfficiencyResources(params);
    }

    CallbackFFI::CallbackFFI() {}

    CallbackFFI::~CallbackFFI()
    {
        LOGI("~CallbackFFI");
    }

    void CallbackFFI::OnExpired()
    {
        LOGI("OnExpired start");
        std::lock_guard<std::mutex> lock(callbackLock_);
        auto findCallback = std::find_if(callbackInstances_.begin(), callbackInstances_.end(),
            [&](const auto& callbackInstance) { return callbackInstance.second.get() == this; }
        );
        if (findCallback == callbackInstances_.end()) {
            LOGI("expired callback not found");
            return;
        }
        LOGI("call CJ callback");
        findCallback->second->ffiCallback_();
        LOGI("OnExpired end");
        callbackInstances_.erase(findCallback);
        return;
    }

    void CallbackFFI::OnExpiredAuth(int32_t authResult) {}

    void CallbackFFI::SetCallbackInfo(void (*callback)())
    {
        ffiCallback_ = CJLambda::Create(callback);
    }
}

}
}