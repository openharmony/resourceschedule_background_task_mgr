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
        {BackgroundTaskMgrStub::SHELL_DUMP,
            std::bind(&BackgroundTaskMgrStub::HandleShellDump,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
};

ErrCode BackgroundTaskMgrStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    std::u16string descriptor = BackgroundTaskMgrStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        BGTASK_LOGE("Local descriptor not match remote.");
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

    BGTASK_LOGE("Failed to call interface %{public}u, err:%{public}d", code, result);
    return result;
}

ErrCode BackgroundTaskMgrStub::HandleRequestSuspendDelay(MessageParcel& data, MessageParcel& reply)
{
    std::u16string reason = data.ReadString16();
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        BGTASK_LOGE("Read callback fail.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    std::shared_ptr<DelaySuspendInfo> info;
    ErrCode result = RequestSuspendDelay(reason, iface_cast<IExpiredCallback>(callback), info);
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("Write result failed, ErrCode=%{public}d", result);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    if (info == nullptr || !info->Marshalling(reply)) {
        BGTASK_LOGE("Write result fail.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleCancelSuspendDelay(MessageParcel& data, MessageParcel& reply)
{
    int32_t id = data.ReadInt32();
    ErrCode result = CancelSuspendDelay(id);
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("Write result failed, ErrCode=%{public}d", result);
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
        BGTASK_LOGE("Write result failed, ErrCode=%{public}d", result);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!reply.WriteInt32(time)) {
        BGTASK_LOGE("Write result fail.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleStartBackgroundRunning(MessageParcel &data, MessageParcel &reply)
{
    sptr<ContinuousTaskParam> taskParam = data.ReadParcelable<ContinuousTaskParam>();
    if (taskParam == nullptr) {
        BGTASK_LOGE("ContinuousTaskParam ReadParcelable failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    ErrCode result = StartBackgroundRunning(taskParam);
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("write result failed, ErrCode=%{public}d", result);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleStopBackgroundRunning(MessageParcel &data, MessageParcel &reply)
{
    std::string abilityName;
    if (!data.ReadString(abilityName)) {
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    sptr<IRemoteObject> abilityToken = data.ReadRemoteObject();
    ErrCode result = StopBackgroundRunning(abilityName, abilityToken);
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("write result failed, ErrCode=%{public}d", result);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleSubscribeBackgroundTask(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> subscriber = data.ReadRemoteObject();
    if (subscriber == nullptr) {
        BGTASK_LOGE("Read callback fail.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    ErrCode result = SubscribeBackgroundTask(iface_cast<IBackgroundTaskSubscriber>(subscriber));
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("Write result failed, ErrCode=%{public}d", result);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleUnsubscribeBackgroundTask(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> subscriber = data.ReadRemoteObject();
    if (subscriber == nullptr) {
        BGTASK_LOGE("Read callback fail.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    ErrCode result = UnsubscribeBackgroundTask(iface_cast<IBackgroundTaskSubscriber>(subscriber));
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("Write result failed, ErrCode=%{public}d", result);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return ERR_OK;
}

ErrCode BackgroundTaskMgrStub::HandleShellDump(MessageParcel& data, MessageParcel& reply)
{
    std::vector<std::string> dumpOption;
    if (!data.ReadStringVector(&dumpOption)) {
        BGTASK_LOGE("Read dumpOption failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    std::vector<std::string> bgtaskmgrInfo;
    ErrCode result = ShellDump(dumpOption, bgtaskmgrInfo);
    if (!reply.WriteInt32(result)) {
        BGTASK_LOGE("Write result failed, ErrCode=%{public}d", result);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!reply.WriteStringVector(bgtaskmgrInfo)) {
        BGTASK_LOGE("Write bgtaskmgrInfo fail.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return ERR_OK;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS