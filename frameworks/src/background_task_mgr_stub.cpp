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

#include "background_task_mgr_stub.h"

#include <ipc_skeleton.h>
#include <string_ex.h>

#include "hitrace_meter.h"
#include "bgtaskmgr_inner_errors.h"
#include "bgtaskmgr_log_wrapper.h"
#include "delay_suspend_info.h"

using namespace std;

namespace OHOS {
namespace BackgroundTaskMgr {
const std::map<uint32_t, std::function<ErrCode(BackgroundTaskMgrStub *, MessageParcel &, MessageParcel &)>>
    BackgroundTaskMgrStub::interfaces_ = {
        {BackgroundTaskMgrStub::REQUEST_SUSPEND_DELAY,
            std::bind(&BackgroundTaskMgrStub::HandleRequestSuspendDelay,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {BackgroundTaskMgrStub::CANCEL_SUSPEND_DELAY,
            std::bind(&BackgroundTaskMgrStub::HandleCancelSuspendDelay,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {BackgroundTaskMgrStub::GET_REMAINING_DELAY_TIME,
            std::bind(&BackgroundTaskMgrStub::HandleGetRemainingDelayTime,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {BackgroundTaskMgrStub::START_BACKGROUND_RUNNING,
            std::bind(&BackgroundTaskMgrStub::HandleStartBackgroundRunning,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {BackgroundTaskMgrStub::STOP_BACKGROUND_RUNNING,
            std::bind(&BackgroundTaskMgrStub::HandleStopBackgroundRunning,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {BackgroundTaskMgrStub::SUBSCRIBE_BACKGROUND_TASK,
            std::bind(&BackgroundTaskMgrStub::HandleSubscribeBackgroundTask,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {BackgroundTaskMgrStub::UNSUBSCRIBE_BACKGROUND_TASK,
            std::bind(&BackgroundTaskMgrStub::HandleUnsubscribeBackgroundTask,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {BackgroundTaskMgrStub::GET_TRANSIENT_TASK_APPS,
            std::bind(&BackgroundTaskMgrStub::HandleGetTransientTaskApps,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {BackgroundTaskMgrStub::GET_CONTINUOUS_TASK_APPS,
            std::bind(&BackgroundTaskMgrStub::HandleGetContinuousTaskApps,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {BackgroundTaskMgrStub::APPLY_EFFICIENCY_RESOURCES,
            std::bind(&BackgroundTaskMgrStub::HandleApplyEfficiencyResources,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {BackgroundTaskMgrStub::RESET_ALL_EFFICIENCY_RESOURCES,
            std::bind(&BackgroundTaskMgrStub::HandleResetAllEfficiencyResources,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {BackgroundTaskMgrStub::GET_EFFICIENCY_RESOURCES_INFOS,
            std::bind(&BackgroundTaskMgrStub::HandleGetEfficiencyResourcesInfos,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {BackgroundTaskMgrStub::REPORT_STATE_CHANGE_EVENT,
            std::bind(&BackgroundTaskMgrStub::HandleReportStateChangeEvent,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {BackgroundTaskMgrStub::STOP_CONTINUOUS_TASK,
            std::bind(&BackgroundTaskMgrStub::HandleStopContinuousTask,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
};

ErrCode BackgroundTaskMgrStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    std::u16string descriptor = BackgroundTaskMgrStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        BGTASK_LOGE("BackgroundTaskMgrStub: Local descriptor not match remote.");
        return ERR_TRANSACTION_FAILED;
    }
    auto it = interfaces_.find(code);
    if (it == interfaces_.end()) {
        return IRemoteStub<IBackgroundTaskMgr>::OnRemoteRequest(code, data, reply, option);
    }

    auto fun = it->second;
    if (fun == nullptr) {
        return IRemoteStub<IBackgroundTaskMgr>::OnRemoteRequest(code, data, reply, option);
    }

    ErrCode result = fun(this, data, reply);
    if (SUCCEEDED(result)) {
        return ERR_OK;
    }

    BGTASK_LOGE("BackgroundTaskMgrStub: Failed to call interface %{public}u, err:%{public}d", code, result);
    return result;
}

ErrCode BackgroundTaskMgrStub::HandleRequestSuspendDelay(MessageParcel& data, MessageParcel& reply)
{
    std::u16string reason = data.ReadString16();
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        BGTASK_LOGE("HandleRequestSuspendDelay Read callback fail.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    std::shared_ptr<DelaySuspendInfo> info;
    ErrCode result = RequestSuspendDelay(reason, iface_cast<IExpiredCallback>(callback), info);
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("HandleRequestSuspendDelay Write result failed, ErrCode=%{public}d", result);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    if (info == nullptr || !info->Marshalling(reply)) {
        BGTASK_LOGE("HandleRequestSuspendDelay Write result fail.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleCancelSuspendDelay(MessageParcel& data, MessageParcel& reply)
{
    int32_t id = data.ReadInt32();
    ErrCode result = CancelSuspendDelay(id);
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("HandleCancelSuspendDelay Write result failed, ErrCode=%{public}d", result);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleGetRemainingDelayTime(MessageParcel& data, MessageParcel& reply)
{
    int32_t id = data.ReadInt32();
    int32_t time = 0;
    ErrCode result =  GetRemainingDelayTime(id, time);
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("HandleGetRemainingDelayTime Write result failed, ErrCode=%{public}d", result);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!reply.WriteInt32(time)) {
        BGTASK_LOGE("HandleGetRemainingDelayTime Write result fail.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleStartBackgroundRunning(MessageParcel &data, MessageParcel &reply)
{
    StartTrace(HITRACE_TAG_OHOS, "BackgroundTaskMgrStub::HandleStartBackgroundRunning");
    sptr<ContinuousTaskParam> taskParam = data.ReadParcelable<ContinuousTaskParam>();
    if (taskParam == nullptr) {
        BGTASK_LOGE("ContinuousTaskParam ReadParcelable failed");
        FinishTrace(HITRACE_TAG_OHOS);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    ErrCode result = StartBackgroundRunning(taskParam);
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("HandleStopBackgroundRunning write result failed, ErrCode=%{public}d", result);
        FinishTrace(HITRACE_TAG_OHOS);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    FinishTrace(HITRACE_TAG_OHOS);
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleStopBackgroundRunning(MessageParcel &data, MessageParcel &reply)
{
    StartTrace(HITRACE_TAG_OHOS, "BackgroundTaskMgrStub::HandleStopBackgroundRunning");
    std::string abilityName;
    if (!data.ReadString(abilityName)) {
        FinishTrace(HITRACE_TAG_OHOS);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    sptr<IRemoteObject> abilityToken = data.ReadRemoteObject();
    ErrCode result = StopBackgroundRunning(abilityName, abilityToken);
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("HandleStopBackgroundRunning write result failed, ErrCode=%{public}d", result);
        FinishTrace(HITRACE_TAG_OHOS);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    FinishTrace(HITRACE_TAG_OHOS);
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleSubscribeBackgroundTask(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> subscriber = data.ReadRemoteObject();
    if (subscriber == nullptr) {
        BGTASK_LOGE("HandleSubscribeBackgroundTask Read callback fail.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    ErrCode result = SubscribeBackgroundTask(iface_cast<IBackgroundTaskSubscriber>(subscriber));
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("HandleSubscribeBackgroundTask Write result failed, ErrCode=%{public}d", result);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleUnsubscribeBackgroundTask(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> subscriber = data.ReadRemoteObject();
    if (subscriber == nullptr) {
        BGTASK_LOGE("HandleUnsubscribeBackgroundTask Read callback fail.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    ErrCode result = UnsubscribeBackgroundTask(iface_cast<IBackgroundTaskSubscriber>(subscriber));
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("HandleUnsubscribeBackgroundTask Write result failed, ErrCode=%{public}d", result);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleGetTransientTaskApps(MessageParcel& data, MessageParcel& reply)
{
    std::vector<std::shared_ptr<TransientTaskAppInfo>> appinfos;
    ErrCode result = GetTransientTaskApps(appinfos);
    if (!reply.WriteInt32(result)) {
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    reply.WriteInt32(appinfos.size());
    for (auto &info : appinfos) {
        if (info == nullptr) {
            return ERR_BGTASK_INVALID_PARAM;
        }
        if (!info->Marshalling(reply)) {
            return ERR_BGTASK_PARCELABLE_FAILED;
        }
    }
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleGetContinuousTaskApps(MessageParcel& data, MessageParcel& reply)
{
    std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> appInfos;
    ErrCode result = GetContinuousTaskApps(appInfos);
    if (!reply.WriteInt32(result)) {
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    reply.WriteInt32(appInfos.size());
    for (auto &info : appInfos) {
        if (info == nullptr) {
            return ERR_BGTASK_INVALID_PARAM;
        }
        if (!info->Marshalling(reply)) {
            return ERR_BGTASK_PARCELABLE_FAILED;
        }
    }
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleApplyEfficiencyResources(MessageParcel& data, MessageParcel& reply)
{
    sptr<EfficiencyResourceInfo> resourceInfoPtr = data.ReadParcelable<EfficiencyResourceInfo>();
    if (resourceInfoPtr == nullptr) {
        BGTASK_LOGE("EfficiencyResourceInfo ReadParcelable failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    bool isSuccess {false};
    ErrCode result = ApplyEfficiencyResources(resourceInfoPtr, isSuccess);
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("HandleApplyEfficiencyResources write result failed, ErrCode=%{public}d", result);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!reply.WriteBool(isSuccess)) {
        BGTASK_LOGE("HandleApplyEfficiencyResources Write result fail.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleResetAllEfficiencyResources(MessageParcel& data, MessageParcel& reply)
{
    ErrCode result = ResetAllEfficiencyResources();
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("HandleCancelSuspendDelay Write result failed, ErrCode=%{public}d", result);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleGetEfficiencyResourcesInfos(MessageParcel& data, MessageParcel& reply)
{
    std::vector<std::shared_ptr<ResourceCallbackInfo>> appInfos;
    std::vector<std::shared_ptr<ResourceCallbackInfo>> procInfos;
    ErrCode result = GetEfficiencyResourcesInfos(appInfos, procInfos);
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("HandleGetEfficiencyResourcesInfos write result failed, ErrCode=%{public}d", result);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (WriteInfoToParcel(appInfos, reply) != ERR_OK
        || WriteInfoToParcel(procInfos, reply) != ERR_OK) {
        BGTASK_LOGE("HandleGetEfficiencyResourcesInfos write result failed");
        return ERR_BGTASK_INVALID_PARAM;
    }
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::WriteInfoToParcel(const std::vector<std::shared_ptr<ResourceCallbackInfo>>& infoMap, MessageParcel& reply)
{
    reply.WriteInt32(infoMap.size());
    for (auto &info : infoMap) {
        if (info == nullptr) {
            return ERR_BGTASK_INVALID_PARAM;
        }
        if (!info->Marshalling(reply)) {
            return ERR_BGTASK_PARCELABLE_FAILED;
        }
    }
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleReportStateChangeEvent(MessageParcel& data, MessageParcel& reply)

ErrCode BackgroundTaskMgrStub::HandleStopContinuousTask(MessageParcel& data, MessageParcel& reply)
{
    int32_t uid = data.ReadInt32();
    int32_t pid = data.ReadInt32();
    uint32_t taskType = data.ReadUint32();
    ErrCode result = StopContinuousTask(uid, pid, taskType);
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("HandleStopContinuousTask write result failed, ErrCode=%{public}d", result);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return ERR_OK;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS