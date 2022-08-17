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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_MGR_STUB_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_MGR_STUB_H

#include <map>

#include <iremote_stub.h>
#include <nocopyable.h>

#include "ibackground_task_mgr.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundTaskMgrStub : public IRemoteStub<IBackgroundTaskMgr> {
public:
    BackgroundTaskMgrStub() = default;
    ~BackgroundTaskMgrStub() override = default;
    DISALLOW_COPY_AND_MOVE(BackgroundTaskMgrStub);

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
    static const std::map<uint32_t,
        std::function<ErrCode(BackgroundTaskMgrStub *, MessageParcel &, MessageParcel &)>> interfaces_;
    ErrCode HandleRequestSuspendDelay(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleCancelSuspendDelay(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleGetRemainingDelayTime(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleStartBackgroundRunning(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleStopBackgroundRunning(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleSubscribeBackgroundTask(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleUnsubscribeBackgroundTask(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleGetTransientTaskApps(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleGetContinuousTaskApps(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleReportStateChangeEvent(MessageParcel& data, MessageParcel& reply);
    ErrCode HandleReportTaskRequiredStateChanged(MessageParcel& data, MessageParcel& reply);
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_MGR_STUB_H