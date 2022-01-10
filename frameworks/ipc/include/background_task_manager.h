/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_FRAMEWORKS_IPC_INCLUDE_BACKGROUND_TASK_MGR_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_FRAMEWORKS_IPC_INCLUDE_BACKGROUND_TASK_MGR_H

#include "background_task_subscriber.h"
#include "expired_callback.h"
#include "ibackground_task_mgr.h"
#include "iremote_object.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundTaskManager {
public:
    BackgroundTaskManager();

    virtual ~BackgroundTaskManager();

    ErrCode CancelSuspendDelay(int32_t requestId);

    ErrCode RequestSuspendDelay(const std::u16string &reason,
        const ExpiredCallback &callback, std::shared_ptr<DelaySuspendInfo> &delayInfo);
    
    ErrCode GetRemainingDelayTime(int32_t requestId, int32_t &delayTime);

    void ResetBackgroundTaskManagerProxy();

    ErrCode SubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber);

    ErrCode UnsubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber);
    
    ErrCode ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);

private:
    bool GetBackgroundTaskManagerProxy();

private:
    class BgTaskMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        BgTaskMgrDeathRecipient(BackgroundTaskManager &backgroundTaskManager);

        ~BgTaskMgrDeathRecipient();

        virtual void OnRemoteDied(const wptr<IRemoteObject> &object) override;

    private:
        BackgroundTaskManager &backgroundTaskManager_;
    };

private:
    std::mutex mutex_;
    sptr<BackgroundTaskMgr::IBackgroundTaskMgr> backgroundTaskMgrProxy_;
    sptr<BgTaskMgrDeathRecipient> recipient_;
};

} // namespace BackgroundTaskMgr
} // namespace OHOS
#endif