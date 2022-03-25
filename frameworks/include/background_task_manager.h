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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_MANAGER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_MANAGER_H

#include "background_task_subscriber.h"
#include "expired_callback.h"
#include "ibackground_task_mgr.h"
#include "iremote_object.h"
#include "want_agent.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundTaskManager {
public:
    BackgroundTaskManager();

    virtual ~BackgroundTaskManager();

    /**
     * @brief cancel delay suspend of background task.
     *
     * @param requestId Id of the requested background task.
     * @return ERR_OK if success, else fail.
     */
    ErrCode CancelSuspendDelay(int32_t requestId);

    /**
     * @brief reuqest delay suspend for background task.
     *
     * @param reason Reason of requesting delay suspend.
     * @param callback Called back to notify the application.
     * @param delayInfo Info of background task which request delay suspend.
     * @return ERR_OK if success, else fail.
     */
    ErrCode RequestSuspendDelay(const std::u16string &reason,
        const ExpiredCallback &callback, std::shared_ptr<DelaySuspendInfo> &delayInfo);

    /**
     * @brief get the time remaining before the background tasks enter the suspended state.
     *
     * @param requestId Id of the requested background task.
     * @param delayTime Remaining Time.
     * @return ERR_OK if success, else fail.
     */
    ErrCode GetRemainingDelayTime(int32_t requestId, int32_t &delayTime);

    /**
     * @brief request service to keep running background.
     *
     * @param taskParam request params.
     * @return ERR_OK if success, else fail.
     */
    ErrCode RequestStartBackgroundRunning(const ContinuousTaskParam &taskParam);

    /**
     * @brief request service to stop running background.
     *
     * @param abilityName ability name of the requester ability.
     * @param abilityToken ability token to mark an unique running ability instance.
     * @return ERR_OK if success, else fail.
     */
    ErrCode RequestStopBackgroundRunning(const std::string &abilityName, const sptr<IRemoteObject> &abilityToken);

    /**
     * @brief reset proxy for background task.
     */
    void ResetBackgroundTaskManagerProxy();

    /**
     * @brief Subscribes background task event.
     *
     * @param subscriber subscriber token.
     * @return ERR_OK if success, else fail.
     */
    ErrCode SubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber);

    /**
     * @brief Unsubscribes background task event.
     *
     * @param subscriber subscriber token.
     * @return ERR_OK if success, else fail.
     */
    ErrCode UnsubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber);

    /**
     * @brief Dump info of continous tasks or transient tasks.
     *
     * @param dumpOption select continous tasks or transient tasks.
     * @param dumpInfo info of continous tasks or transient tasks.
     * @return ERR_OK if success, else fail.
     */
    ErrCode ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);

private:
    bool GetBackgroundTaskManagerProxy();

private:
    class BgTaskMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit BgTaskMgrDeathRecipient(BackgroundTaskManager &backgroundTaskManager);

        ~BgTaskMgrDeathRecipient();

        void OnRemoteDied(const wptr<IRemoteObject> &object) override;

    private:
        BackgroundTaskManager &backgroundTaskManager_;
    };

private:
    std::mutex mutex_;
    sptr<BackgroundTaskMgr::IBackgroundTaskMgr> backgroundTaskMgrProxy_;
    sptr<BgTaskMgrDeathRecipient> recipient_;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_MANAGER_H