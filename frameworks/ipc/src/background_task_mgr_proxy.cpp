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

#include "background_task_mgr_proxy.h"

#include <message_parcel.h>
#include <string_ex.h>

#include "bgtaskmgr_log_wrapper.h"

using namespace std;

namespace OHOS {
namespace BackgroundTaskMgr {
BackgroundTaskMgrProxy::BackgroundTaskMgrProxy(const sptr<IRemoteObject>& impl)
    :IRemoteProxy<IBackgroundTaskMgr>(impl) {}
BackgroundTaskMgrProxy::~BackgroundTaskMgrProxy() {}
    
ErrCode BackgroundTaskMgrProxy::RequestSuspendDelay(const std::u16string& reason, 
        const sptr<IExpiredCallback>& callback, std::shared_ptr<DelaySuspendInfo> &delayInfo)
{
    if (callback == nullptr) {
        BGTASK_LOGE(" BackgroundTaskMgrProxy::%{public}s callback is null", __func__);
        return ERR_BGTASK_INVALID_PARAM;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        BGTASK_LOGE(" BackgroundTaskMgrProxy::%{public}s, write descriptor failed", __func__);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!data.WriteString16(reason)) {
        BGTASK_LOGE(" BackgroundTaskMgrProxy::%{public}s, write reason failed", __func__);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        BGTASK_LOGE(" BackgroundTaskMgrProxy::%{public}s, write callback failed", __func__);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(REQUEST_SUSPEND_DELAY, option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGI("fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGI("fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    delayInfo = DelaySuspendInfo::Unmarshalling(reply);
    if (delayInfo == nullptr) {
        BGTASK_LOGE("BackgroundTaskMgrProxy::%{public}s, read result failed", __func__);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode BackgroundTaskMgrProxy::CancelSuspendDelay(int32_t requestId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        BGTASK_LOGE(" BackgroundTaskMgrProxy::%{public}s, write descriptor failed", __func__);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!data.WriteInt32(requestId)) {
        BGTASK_LOGE(" BackgroundTaskMgrProxy::%{public}s, write requestId failed", __func__);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(CANCEL_SUSPEND_DELAY, option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGI("fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGI("fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode BackgroundTaskMgrProxy::GetRemainingDelayTime(int32_t requestId, int32_t &delayTime)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        BGTASK_LOGE(" BackgroundTaskMgrProxy::%{public}s, write descriptor failed", __func__);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!data.WriteInt32(requestId)) {
        BGTASK_LOGE(" BackgroundTaskMgrProxy::%{public}s, write requestId failed", __func__);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(GET_REMAINING_DELAY_TIME, option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGI("fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGI("fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!reply.ReadInt32(delayTime)) {
        BGTASK_LOGI("fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode BackgroundTaskMgrProxy::SubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber)
{
    if (subscriber == nullptr) {
        BGTASK_LOGE(" BackgroundTaskMgrProxy::%{public}s subscriber is null", __func__);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        BGTASK_LOGE(" BackgroundTaskMgrProxy::%{public}s, write descriptor failed", __func__);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!data.WriteRemoteObject(subscriber->AsObject())) {
        BGTASK_LOGE(" BackgroundTaskMgrProxy::%{public}s, write subscriber failed", __func__);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(SUBSCRIBE_BACKGROUND_TASK, option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGI("fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGI("fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode BackgroundTaskMgrProxy::UnsubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber)
{
    if (subscriber == nullptr) {
        BGTASK_LOGE(" BackgroundTaskMgrProxy::%{public}s subscriber is null", __func__);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        BGTASK_LOGE(" BackgroundTaskMgrProxy::%{public}s, write descriptor failed", __func__);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!data.WriteRemoteObject(subscriber->AsObject())) {
        BGTASK_LOGE(" BackgroundTaskMgrProxy::%{public}s, write subscriber failed", __func__);
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(UNSUBSCRIBE_BACKGROUND_TASK, option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGI("fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGI("fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode BackgroundTaskMgrProxy::ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        BGTASK_LOGE("[ShellDump] fail: write interface token failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    if (!data.WriteStringVector(dumpOption)) {
        BGTASK_LOGE("[ShellDump] fail: write option failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(SHELL_DUMP, option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGI("fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGI("fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!reply.ReadStringVector(&dumpInfo)) {
        BGTASK_LOGI("[ShellDump] fail: read dumpInfo failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode BackgroundTaskMgrProxy::InnerTransact(uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply)
{
    auto remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGI("[InnerTransact] fail: get Remote fail code %{public}d", code);
        return ERR_DEAD_OBJECT;
    }
    int err = remote->SendRequest(code, data, reply, flags);
    switch (err) {
        case NO_ERROR: {
            return ERR_OK;
        }
        case DEAD_OBJECT: {
            BGTASK_LOGI("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return ERR_DEAD_OBJECT;
        }
        default: {
            BGTASK_LOGI("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return ERR_BGTASK_TRANSACT_FAILED;
        }
    }
}
} // namespace BackgroundTaskMgr
} // namespace OHOS