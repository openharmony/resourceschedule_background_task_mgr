/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "continuous_task_request.h"
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
     * @brief Request service to get all transient tasks.
     *
     * @param remainingQuota remain quota for the day.
     * @param list transient tasks.
     * @return ERR_OK if success, else fail.
     */
    ErrCode GetAllTransientTasks(int32_t &remainingQuota, std::vector<std::shared_ptr<DelaySuspendInfo>> &list);

    /**
     * @brief Request service to keep running background.
     *
     * @param taskParam Request params.
     * @return ERR_OK if success, else fail.
     */
    ErrCode RequestStartBackgroundRunning(ContinuousTaskParam &taskParam);

    /**
     * @brief Request service to update running background.
     *
     * @param taskParam Request params.
     * @return ERR_OK if success, else fail.
     */
    ErrCode RequestUpdateBackgroundRunning(ContinuousTaskParam &taskParam);

    /**
     * @brief Request service to keep or stop running background for inner ability.
     *
     * @param taskParam Request params.
     * @return ERR_OK if success, else fail.
     */
    ErrCode RequestBackgroundRunningForInner(const ContinuousTaskParamForInner &taskParam);

    /**
     * @brief Request service to get continuous task info by uid for inner ability.
     *
     * @param uid app uid.
     * @param list continuous task info list.
     * @return ERR_OK if success, else fail.
     */
    ErrCode RequestGetContinuousTasksByUidForInner(int32_t uid,
        std::vector<std::shared_ptr<ContinuousTaskInfo>> &list);

    /**
     * @brief Request service to stop running background.
     *
     * @param abilityName Ability name of the requester ability.
     * @param abilityToken Ability token to mark an unique running ability instance.
     * @param abilityId Ability identity.
     * @param continuousTaskId continuous task id.
     * @return ERR_OK if success, else fail.
     */
    ErrCode RequestStopBackgroundRunning(const std::string &abilityName, const sptr<IRemoteObject> &abilityToken,
        int32_t abilityId, int32_t continuousTaskId = -1);

    /**
     * @brief Request service to get all continuous tasks.
     *
     * @param list continuous task info.
     * @return ERR_OK if success, else fail.
     */
    ErrCode RequestGetAllContinuousTasks(std::vector<std::shared_ptr<ContinuousTaskInfo>> &list);

    /**
     * @brief Request service to get all continuous tasks.
     *
     * @param list continuous task info.
     * @param includeSuspended whether include suspended continuous task.
     * @return ERR_OK if success, else fail.
     */
    ErrCode RequestGetAllContinuousTasks(
        std::vector<std::shared_ptr<ContinuousTaskInfo>> &list, bool includeSuspended);

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
     * @return Returns ERR_OK if success, else failure.
     */
    ErrCode GetTransientTaskApps(std::vector<std::shared_ptr<TransientTaskAppInfo>> &list);

    /**
     * @brief Pause transient task time by uid for inner ability.
     * @param uid app uid.
     * @return Returns ERR_OK if success, else failure.
     */
    ErrCode PauseTransientTaskTimeForInner(int32_t uid);

    /**
     * @brief Start transient task time by uid for inner ability.
     * @param uid app uid.
     * @return Returns ERR_OK if success, else failure.
     */
    ErrCode StartTransientTaskTimeForInner(int32_t uid);

    /**
     * @brief Get all continuous task running infos
     * @param list continuous task infos.
     * @return Returns ERR_OK if success, else failure.
     */
    ErrCode GetContinuousTaskApps(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list);

    /**
     * @brief Apply or unapply efficiency resources.
     *
     * @param resourceInfo Request params.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ApplyEfficiencyResources(const EfficiencyResourceInfo &resourceInfo);

    /**
     * @brief Reset all efficiency resources.
     *
     * @return ERR_OK if success, else fail.
     */
    ErrCode ResetAllEfficiencyResources();

    /**
     * @brief Get all efficiency resources
     *
     * @param resourceInfoList efficiency resource infos.
     * @return ERR_OK if success, else fail.
     */
    ErrCode GetAllEfficiencyResources(std::vector<std::shared_ptr<EfficiencyResourceInfo>> &resourceInfoList);

    /**
     * @brief Get all effficiency resources running infos.
     * @param appList EFficiency Resources infos of apps.
     * @param procList  EFficiency Resources infos of processes.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetEfficiencyResourcesInfos(std::vector<std::shared_ptr<ResourceCallbackInfo>> &appList,
        std::vector<std::shared_ptr<ResourceCallbackInfo>> &procList);

    /*
     * @brief Request stop continuous task.
     * @param uid app uid.
     * @param pid app pid.
     * @param taskType continuous task type.
     * @param key continuous task key.
     * @return Returns ERR_OK if success, else failure.
     */
    ErrCode StopContinuousTask(int32_t uid, int32_t pid, uint32_t taskType, const std::string &key);

    /*
     * @brief Request suspend continuous task.
     * @param uid app uid.
     * @param pid app pid.
     * @param reason continuous task suspend reason.
     * @param key continuous task key.
     * @return Returns ERR_OK if success, else failure.
     */
    ErrCode SuspendContinuousTask(int32_t uid, int32_t pid, int32_t reason, const std::string &key);

    /*
     * @brief Request active continuous task.
     * @param uid app uid.
     * @param pid app pid.
     * @param key continuous task key.
     * @return Returns ERR_OK if success, else failure.
     */
    ErrCode ActiveContinuousTask(int32_t uid, int32_t pid, const std::string &key);

    /**
     * @brief AVsession notify update notification.
     * @param uid app uid.
     * @param pid app pid.
     * @param isPublish isPublish notification.
     * @return Returns ERR_OK if success, else failure.
     */
    ErrCode AVSessionNotifyUpdateNotification(int32_t uid, int32_t pid, bool isPublish = false);

    /**
     * @brief Set background task config.
     * @param configData config param.
     * @param sourceType data source.
     * @return Returns ERR_OK if success, else failure.
     */
    ErrCode SetBgTaskConfig(const std::string &configData, int32_t sourceType);

    /**
     * @brief Request suspend continuous audio playback task.
     * @param uid app uid.
     * @return Returns ERR_OK if success, else failure.
     */
    ErrCode SuspendContinuousAudioTask(int32_t uid);

    /**
     * @brief Get is request mode supported.
     * @param isModeSupported is request mode supported.
     * @return Returns ERR_OK if success, else failure.
     */
    ErrCode IsModeSupported(ContinuousTaskParam &taskParam);

    /**
     * @brief Set supported taskKeeping processes.
     * @param processSet process set.
     * @return Returns ERR_OK if success, else failure.
     */
    ErrCode SetSupportedTaskKeepingProcesses(const std::set<std::string> &processSet);

private:
    bool GetBackgroundTaskManagerProxy();

private:
    class BgTaskMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit BgTaskMgrDeathRecipient(BackgroundTaskManager &backgroundTaskManager);

        ~BgTaskMgrDeathRecipient() override;

        void OnRemoteDied(const wptr<IRemoteObject> &object) override;

    private:
        BackgroundTaskManager &backgroundTaskManager_;
    };

private:
    std::mutex mutex_;
    sptr<BackgroundTaskMgr::IBackgroundTaskMgr> proxy_;
    sptr<BgTaskMgrDeathRecipient> recipient_;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_MANAGER_H
