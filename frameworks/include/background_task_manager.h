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
#include "efficiency_resource_info.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundTaskManager {
public:
    BackgroundTaskManager();

    virtual ~BackgroundTaskManager();

    /**
     * @brief Cancel delay suspend of background task.
     *
     * @param requestId Id of the requested background task.
     * @return ERR_OK if success, else fail.
     */
    ErrCode CancelSuspendDelay(int32_t requestId);

    /**
     * @brief Request delay suspend for background task.
     *
     * @param reason Reason of requesting delay suspend.
     * @param callback Called back to notify the application.
     * @param delayInfo Info of background task which request delay suspend.
     * @return ERR_OK if success, else fail.
     */
    ErrCode RequestSuspendDelay(const std::u16string &reason,
        const ExpiredCallback &callback, std::shared_ptr<DelaySuspendInfo> &delayInfo);

    /**
     * @brief Get the time remaining before the background tasks enter the suspended state.
     *
     * @param requestId Id of the requested background task.
     * @param delayTime Remaining time.
     * @return ERR_OK if success, else fail.
     */
    ErrCode GetRemainingDelayTime(int32_t requestId, int32_t &delayTime);

    /**
     * @brief Request service to keep running background.
     *
     * @param taskParam Request params.
     * @return ERR_OK if success, else fail.
     */
    ErrCode RequestStartBackgroundRunning(const ContinuousTaskParam &taskParam);

    /**
     * @brief Request service to stop running background.
     *
     * @param abilityName Ability name of the requester ability.
     * @param abilityToken Ability token to mark an unique running ability instance.
     * @return ERR_OK if success, else fail.
     */
    ErrCode RequestStopBackgroundRunning(const std::string &abilityName, const sptr<IRemoteObject> &abilityToken);

    /**
     * @brief Reset proxy for background task.
     */
    void ResetBackgroundTaskManagerProxy();

    /**
     * @brief Subscribes background task event.
     *
     * @param subscriber Subscriber token.
     * @return ERR_OK if success, else fail.
     */
    ErrCode SubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber);

    /**
     * @brief Unsubscribes background task event.
     *
     * @param subscriber Subscriber token.
     * @return ERR_OK if success, else fail.
     */
    ErrCode UnsubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber);

    /**
     * @brief Get transient task applications.
     * @param list transient task apps.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetTransientTaskApps(std::vector<std::shared_ptr<TransientTaskAppInfo>> &list);

    /**
     * @brief Get all continuous task running infos
     * @param list continuous task infos.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetContinuousTaskApps(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list);

    /**
     * @brief Apply or unapply efficiency resources.
     * 
     * @param resourceInfo Request params.
     * @param isSuccess succeed to apply efficiency recources.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ApplyEfficiencyResources(const EfficiencyResourceInfo &resourceInfo, bool &isSuccess);

    /**
     * @brief Reset all efficiency resources.
     * 
     * @return ERR_OK if success, else fail.
     */
    ErrCode ResetAllEfficiencyResources();


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
