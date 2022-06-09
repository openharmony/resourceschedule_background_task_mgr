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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_IBACKGROUND_TASK_MGR_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_IBACKGROUND_TASK_MGR_H

#include <string>

#include <ipc_types.h>
#include <iremote_broker.h>
#include <nocopyable.h>

#include "bgtaskmgr_inner_errors.h"
#include "continuous_task_param.h"
#include "delay_suspend_info.h"
#include "iexpired_callback.h"
#include "ibackground_task_subscriber.h"
#include "want_agent.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class IBackgroundTaskMgr : public IRemoteBroker {
public:
    IBackgroundTaskMgr() = default;
    ~IBackgroundTaskMgr() override = default;
    DISALLOW_COPY_AND_MOVE(IBackgroundTaskMgr);

    /**
     * @brief Reuqest delay suspend for background task.
     *
     * @param reason Reason of requesting delay suspend.
     * @param delayInfo Info of background which request delay suspend.
     * @return ERR_OK if success, else fail.
     */
    virtual ErrCode RequestSuspendDelay(const std::u16string& reason,
        const sptr<IExpiredCallback>& callback, std::shared_ptr<DelaySuspendInfo> &delayInfo) = 0;
    
    /**
     * @brief Cancel delay suspend of background task.
     *
     * @param requestId Id of the requested background task.
     * @return ERR_OK if success, else fail.
     */
    virtual ErrCode CancelSuspendDelay(int32_t requestId) = 0;

    /**
     * @brief Get the time remaining before the background tasks enter the suspended state.
     *
     * @param requestId Id of the requested background task.
     * @param delayTime Remaining time.
     * @return ERR_OK if success, else fail.
     */
    virtual ErrCode GetRemainingDelayTime(int32_t requestId, int32_t &delayTime) = 0;

    /**
     * @brief Request service to keep running background.
     *
     * @param taskParam Request params.
     * @return ERR_OK if success, else fail.
     */
    virtual ErrCode StartBackgroundRunning(const sptr<ContinuousTaskParam> &taskParam) = 0;

    /**
     * @brief Request service to stop running background.
     *
     * @param abilityName Ability name of the requester ability.
     * @param abilityToken Ability token to mark an unique running ability instance.
     * @return ERR_OK if success, else fail.
     */
    virtual ErrCode StopBackgroundRunning(const std::string &abilityName, const sptr<IRemoteObject> &abilityToken) = 0;

    /**
     * @brief Subscribes background task event.
     *
     * @param subscriber Subscriber token.
     * @return ERR_OK if success, else fail.
     */
    virtual ErrCode SubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber> &subscriber) = 0;

    /**
     * @brief Unsubscribes background task event.
     *
     * @param subscriber Subscriber token.
     * @return ERR_OK if success, else fail.
     */
    virtual ErrCode UnsubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber> &subscriber) = 0;

    /**
     * @brief Get transient task applications.
     * @param list transient task apps.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual ErrCode GetTransientTaskApps(std::vector<std::shared_ptr<TransientTaskAppInfo>> &list) = 0;

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.resourceschedule.IBackgroundTaskMgr");

protected:
    enum InterfaceId : uint32_t {
        REQUEST_SUSPEND_DELAY = MIN_TRANSACTION_ID,
        CANCEL_SUSPEND_DELAY,
        GET_REMAINING_DELAY_TIME,
        START_BACKGROUND_RUNNING,
        STOP_BACKGROUND_RUNNING,
        SUBSCRIBE_BACKGROUND_TASK,
        UNSUBSCRIBE_BACKGROUND_TASK,
        GET_TRANSIENT_TASK_APPS,
    };
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_IBACKGROUND_TASK_MGR_H