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
#include "ibackground_task_mgr_ipc_interface_code.h"

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
        BGTASK_LOGE("RequestSuspendDelay callback is null");
        return ERR_CALLBACK_NULL_OR_TYPE_ERR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        BGTASK_LOGE("RequestSuspendDelay write descriptor failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!data.WriteString16(reason)) {
        BGTASK_LOGE("RequestSuspendDelay write reason failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        BGTASK_LOGE("RequestSuspendDelay write callback failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::REQUEST_SUSPEND_DELAY), option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("RequestSuspendDelay transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("RequestSuspendDelay fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    delayInfo = DelaySuspendInfo::Unmarshalling(reply);
    if (delayInfo == nullptr) {
        BGTASK_LOGE("RequestSuspendDelay read result failed");
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
        BGTASK_LOGE("RequestSuspendDelay write descriptor failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!data.WriteInt32(requestId)) {
        BGTASK_LOGE("RequestSuspendDelay write requestId failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::CANCEL_SUSPEND_DELAY), option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("RequestSuspendDelay fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("RequestSuspendDelay fail: read result failed.");
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

    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::GET_REMAINING_DELAY_TIME), option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("GetRemainingDelayTime fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("GetRemainingDelayTime fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!reply.ReadInt32(delayTime)) {
        BGTASK_LOGE("GetRemainingDelayTime fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode BackgroundTaskMgrProxy::RequestBackgroundRunningForInner(const sptr<ContinuousTaskParamForInner> &taskParam)
{
    if (taskParam == nullptr) {
        return ERR_BGTASK_INVALID_PARAM;
    }

    MessageParcel dataInfo;
    if (!dataInfo.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    if (!dataInfo.WriteParcelable(taskParam)) {
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};

    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::REQUEST_BACKGROUND_RUNNING_FOR_INNER), option, dataInfo, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("RequestBackgroundRunningForInner fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("RequestBackgroundRunningForInner fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode BackgroundTaskMgrProxy::StartBackgroundRunning(const sptr<ContinuousTaskParam> &taskParam)
{
    if (taskParam == nullptr) {
        return ERR_BGTASK_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    if (!data.WriteParcelable(taskParam)) {
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};

    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::START_BACKGROUND_RUNNING), option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("StartBackgroundRunning fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("StartBackgroundRunning fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    int32_t notificationId = -1;
    int32_t continuousTaskId = -1;
    if (!reply.ReadInt32(notificationId)) {
        BGTASK_LOGE("StartBackgroundRunning fail: read notificationId failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!reply.ReadInt32(continuousTaskId)) {
        BGTASK_LOGE("StartBackgroundRunning fail: read continuousTaskId failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    BGTASK_LOGI("BackgroundTaskMgrProxy read notificationId %{public}d, continuousTaskId %{public}d",
        notificationId, continuousTaskId);
    taskParam->notificationId_ = notificationId;
    taskParam->continuousTaskId_ = continuousTaskId;
    return result;
}

ErrCode BackgroundTaskMgrProxy::UpdateBackgroundRunning(const sptr<ContinuousTaskParam> &taskParam)
{
    if (taskParam == nullptr) {
        return ERR_BGTASK_INVALID_PARAM;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    if (!data.WriteParcelable(taskParam)) {
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};

    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::UPDATE_BACKGROUND_RUNNING), option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("UpdateBackgroundRunning fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("UpdateBackgroundRunning fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    int32_t notificationId = -1;
    if (!reply.ReadInt32(notificationId)) {
        BGTASK_LOGE("UpdateBackgroundRunning fail: read notificationId failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    BGTASK_LOGI("read notificationId %{public}d", notificationId);
    taskParam->notificationId_ = notificationId;
    return result;
}

ErrCode BackgroundTaskMgrProxy::StopBackgroundRunning(const std::string &abilityName,
    const sptr<IRemoteObject> &abilityToken, int32_t abilityId)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    std::u16string u16AbilityName = Str8ToStr16(abilityName);
    if (!data.WriteString16(u16AbilityName)) {
        BGTASK_LOGE("StopBackgroundRunning parcel ability Name failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    if (!data.WriteRemoteObject(abilityToken)) {
        BGTASK_LOGE("StopBackgroundRunning parcel ability token failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    if (!data.WriteInt32(abilityId)) {
        BGTASK_LOGE("StopBackgroundRunning parcel abilityId failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};

    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::STOP_BACKGROUND_RUNNING), option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("StopBackgroundRunning transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("StopBackgroundRunning read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode BackgroundTaskMgrProxy::SubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber)
{
    if (subscriber == nullptr) {
        BGTASK_LOGE("SubscribeBackgroundTask subscriber is null");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        BGTASK_LOGE("SubscribeBackgroundTask write descriptor failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!data.WriteRemoteObject(subscriber->AsObject())) {
        BGTASK_LOGE("SubscribeBackgroundTask write subscriber failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::SUBSCRIBE_BACKGROUND_TASK), option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("SubscribeBackgroundTask fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("SubscribeBackgroundTask fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode BackgroundTaskMgrProxy::UnsubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber)
{
    if (subscriber == nullptr) {
        BGTASK_LOGE("UnsubscribeBackgroundTask subscriber is null");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        BGTASK_LOGE("UnsubscribeBackgroundTask write descriptor failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!data.WriteRemoteObject(subscriber->AsObject())) {
        BGTASK_LOGE("UnsubscribeBackgroundTask write subscriber failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::UNSUBSCRIBE_BACKGROUND_TASK), option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("UnsubscribeBackgroundTask fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("UnsubscribeBackgroundTask fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode BackgroundTaskMgrProxy::GetTransientTaskApps(std::vector<std::shared_ptr<TransientTaskAppInfo>> &list)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        BGTASK_LOGE("GetTransientTaskApps write descriptor failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::GET_TRANSIENT_TASK_APPS), option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("GetTransientTaskApps fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("GetTransientTaskApps fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    int32_t infoSize = reply.ReadInt32();
    for (int32_t i = 0; i < infoSize; i++) {
        auto info = TransientTaskAppInfo::Unmarshalling(reply);
        if (!info) {
            BGTASK_LOGE("GetTransientTaskApps Read Parcelable infos failed.");
            return ERR_BGTASK_PARCELABLE_FAILED;
        }
        list.emplace_back(info);
    }

    return result;
}

ErrCode BackgroundTaskMgrProxy::PauseTransientTaskTimeForInner(int32_t uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        BGTASK_LOGE("PauseTransientTaskTimeForInner write descriptor failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!data.WriteInt32(uid)) {
        BGTASK_LOGE("PauseTransientTaskTimeForInner parcel uid failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::PAUSE_TRANSIENT_TASK_TIME_FOR_INNER), option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("PauseTransientTaskTimeForInner fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("PauseTransientTaskTimeForInner fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode BackgroundTaskMgrProxy::StartTransientTaskTimeForInner(int32_t uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        BGTASK_LOGE("StartTransientTaskTimeForInner write descriptor failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!data.WriteInt32(uid)) {
        BGTASK_LOGE("StartTransientTaskTimeForInner parcel uid failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::START_TRANSIENT_TASK_TIME_FOR_INNER), option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("StartTransientTaskTimeForInner fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("StartTransientTaskTimeForInner fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode BackgroundTaskMgrProxy::GetContinuousTaskApps(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        BGTASK_LOGE("GetContinuousTaskApps write descriptor failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::GET_CONTINUOUS_TASK_APPS), option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("GetContinuousTaskApps fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("GetContinuousTaskApps fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    if (result != ERR_OK) {
        BGTASK_LOGE("GetContinuousTaskApps failed.");
        return result;
    }

    int32_t infoSize = reply.ReadInt32();
    for (int32_t i = 0; i < infoSize; i++) {
        auto info = ContinuousTaskCallbackInfo::Unmarshalling(reply);
        if (!info) {
            BGTASK_LOGE("GetContinuousTaskApps Read Parcelable infos failed.");
            return ERR_BGTASK_PARCELABLE_FAILED;
        }
        list.emplace_back(info);
    }

    return result;
}

ErrCode BackgroundTaskMgrProxy::StopContinuousTask(int32_t uid, int32_t pid, uint32_t taskType, const std::string &key)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    if (!data.WriteInt32(uid)) {
        BGTASK_LOGE("StopContinuousTask parcel uid failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    if (!data.WriteInt32(pid)) {
        BGTASK_LOGE("StopContinuousTask parcel pid failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    if (!data.WriteUint32(taskType)) {
        BGTASK_LOGE("StopContinuousTask parcel taskType failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    if (!data.WriteString(key)) {
        BGTASK_LOGE("StopContinuousTask parcel key failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};

    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::STOP_CONTINUOUS_TASK), option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("StopContinuousTask transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("StopContinuousTask read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode BackgroundTaskMgrProxy::InnerTransact(uint32_t code, MessageOption &flags,
    MessageParcel &data, MessageParcel &reply)
{
    auto remote = Remote();
    if (remote == nullptr) {
        BGTASK_LOGE("InnerTransact get Remote fail code %{public}d", code);
        return ERR_DEAD_OBJECT;
    }
    int32_t err = remote->SendRequest(code, data, reply, flags);
    switch (err) {
        case NO_ERROR: {
            return ERR_OK;
        }
        case DEAD_OBJECT: {
            BGTASK_LOGE("[InnerTransact] fail: ipcErr=%{public}d code %{public}u", err, code);
            return ERR_DEAD_OBJECT;
        }
        default: {
            BGTASK_LOGE("[InnerTransact] fail: ipcErr=%{public}d code %{public}u", err, code);
            return ERR_BGTASK_TRANSACT_FAILED;
        }
    }
}

ErrCode BackgroundTaskMgrProxy::ApplyEfficiencyResources(const sptr<EfficiencyResourceInfo> &resourceInfo)
{
    if (resourceInfo == nullptr) {
        return ERR_BGTASK_INVALID_PARAM;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        BGTASK_LOGE("ApplyEfficiencyResources write descriptor failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    if (!data.WriteParcelable(resourceInfo)) {
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    BGTASK_LOGD("start send data in apply res function from bgtask proxy");
    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::APPLY_EFFICIENCY_RESOURCES), option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("ApplyEfficiencyResources fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("ApplyEfficiencyResources fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode BackgroundTaskMgrProxy::ResetAllEfficiencyResources()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        BGTASK_LOGE("ResetAllEfficiencyResources write descriptor failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    BGTASK_LOGD("start send data in reset all res function from bgtask proxy");
    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::RESET_ALL_EFFICIENCY_RESOURCES), option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("ResetAllEfficiencyResources fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("ResetAllEfficiencyResources fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return result;
}

ErrCode BackgroundTaskMgrProxy::GetEfficiencyResourcesInfos(std::vector<std::shared_ptr<
    ResourceCallbackInfo>> &appList, std::vector<std::shared_ptr<ResourceCallbackInfo>> &procList)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        BGTASK_LOGE("GetEfficiencyResourcesInfos write descriptor failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::GET_EFFICIENCY_RESOURCES_INFOS), option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("GetEfficiencyResourcesInfos fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("GetEfficiencyResourcesInfos fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }

    if (result != ERR_OK) {
        return result;
    }

    int32_t infoSize = reply.ReadInt32();
    for (int32_t i = 0; i < infoSize; i++) {
        auto info = ResourceCallbackInfo::Unmarshalling(reply);
        if (!info) {
            BGTASK_LOGE("GetContinuousTaskApps Read Parcelable infos failed.");
            return ERR_BGTASK_PARCELABLE_FAILED;
        }
        appList.emplace_back(info);
    }

    infoSize = reply.ReadInt32();
    for (int32_t i = 0; i < infoSize; i++) {
        auto info = ResourceCallbackInfo::Unmarshalling(reply);
        if (!info) {
            BGTASK_LOGE("GetContinuousTaskApps Read Parcelable infos failed.");
            return ERR_BGTASK_PARCELABLE_FAILED;
        }
        procList.emplace_back(info);
    }

    return result;
}

ErrCode BackgroundTaskMgrProxy::SetBgTaskConfig(const std::string &configData, int32_t sourceType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    if (!data.WriteInterfaceToken(BackgroundTaskMgrProxy::GetDescriptor())) {
        BGTASK_LOGE("SetBgTaskConfig write descriptor failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!data.WriteString(configData)) {
        BGTASK_LOGE("SetBgTaskConfig write configData failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    if (!data.WriteInt32(sourceType)) {
        BGTASK_LOGE("SetBgTaskConfig write sourceType failed");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    ErrCode result = InnerTransact(static_cast<uint32_t>(
        BackgroundTaskMgrStubInterfaceCode::SET_BGTASK_CONFIG), option, data, reply);
    if (result != ERR_OK) {
        BGTASK_LOGE("SetBgTaskConfig fail: transact ErrCode=%{public}d", result);
        return ERR_BGTASK_TRANSACT_FAILED;
    }
    if (!reply.ReadInt32(result)) {
        BGTASK_LOGE("SetBgTaskConfig fail: read result failed.");
        return ERR_BGTASK_PARCELABLE_FAILED;
    }
    return result;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS