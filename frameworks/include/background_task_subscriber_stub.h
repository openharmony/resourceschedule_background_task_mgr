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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_SUBSCRIBER_STUB_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_SUBSCRIBER_STUB_H

#include <iremote_stub.h>

#include "bgtaskmgr_inner_errors.h"
#include "ibackground_task_subscriber.h"
#include "resource_callback_info.h"
namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundTaskSubscriberStub : public IRemoteStub<IBackgroundTaskSubscriber> {
public:
    BackgroundTaskSubscriberStub();
    ~BackgroundTaskSubscriberStub() override;
    DISALLOW_COPY_AND_MOVE(BackgroundTaskSubscriberStub);

    /**
     * @brief Request service code and service data.
     *
     * @param code Service request code.
     * @param data MessageParcel object.
     * @param reply Local service response.
     * @param option Point out async or sync.
     * @return ERR_OK if success, else fail.
     */
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

private:
    ErrCode HandleOnConnected();
    ErrCode HandleOnDisconnected();
    ErrCode HandleOnTransientTaskStart(MessageParcel& data);
    ErrCode HandleOnTransientTaskEnd(MessageParcel& data);
    ErrCode HandleOnTransientTaskErr(MessageParcel& data);
    ErrCode HandleOnAppTransientTaskStart(MessageParcel& data);
    ErrCode HandleOnAppTransientTaskEnd(MessageParcel& data);
    ErrCode HandleOnContinuousTaskStart(MessageParcel &data);
    ErrCode HandleOnContinuousTaskUpdate(MessageParcel &data);
    ErrCode HandleOnContinuousTaskCancel(MessageParcel &data);
    ErrCode HandleOnAppContinuousTaskStop(MessageParcel &data);
    ErrCode HandleOnAppEfficiencyResourcesApply(MessageParcel &data);
    ErrCode HandleOnAppEfficiencyResourcesReset(MessageParcel &data);
    ErrCode HandleOnProcEfficiencyResourcesApply(MessageParcel &data);
    ErrCode HandleOnProcEfficiencyResourcesReset(MessageParcel &data);
    ErrCode HandleTransientTask(uint32_t code, MessageParcel &data);
    ErrCode OnRemoteRequestInner(uint32_t code,
        MessageParcel& data, MessageParcel& reply, MessageOption& option);
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_SUBSCRIBER_STUB_H