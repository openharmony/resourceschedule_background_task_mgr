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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_MGR_PROXY_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_MGR_PROXY_H

#include <iremote_proxy.h>
#include <nocopyable.h>

#include <ibackground_task_mgr.h>

namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundTaskMgrProxy final : public IRemoteProxy<IBackgroundTaskMgr> {
public:
    explicit BackgroundTaskMgrProxy(const sptr<IRemoteObject>& impl);
    ~BackgroundTaskMgrProxy() override;
    DISALLOW_COPY_AND_MOVE(BackgroundTaskMgrProxy);

    /**
     * @brief Reuqest delay suspend for background task.
     *
     * @param reason Reason of requesting delay suspend.
     * @param callback Called back to notify the application.
     * @param delayInfo Info of background which request delay suspend.
     * @return ERR_OK if success, else fail.
     */
    ErrCode RequestSuspendDelay(const std::u16string& reason,
        const sptr<IExpiredCallback>& callback, std::shared_ptr<DelaySuspendInfo> &delayInfo) override;
    
    /**
     * @brief Cancel delay suspend of background task.
     *
     * @param requestId Id of the requested background task.
     * @return ERR_OK if success, else fail.
     */
    ErrCode CancelSuspendDelay(int32_t requestId) override;

    /**
     * @brief Get the time remaining before the background tasks enter the suspended state.
     *
     * @param requestId Id of the requested background task.
     * @param delayTime Remaining time.
     * @return ERR_OK if success, else fail.
     */
    ErrCode GetRemainingDelayTime(int32_t requestId, int32_t &delayTime) override;

    /**
     * @brief Request service to keep running background.
     *
     * @param taskParam Request params.
     * @return ERR_OK if success, else fail.
     */
    ErrCode StartBackgroundRunning(const sptr<ContinuousTaskParam> &taskParam) override;

    /**
     * @brief Request service to stop running background.
     *
     * @param abilityName Ability name of the requester ability.
     * @param abilityToken Ability token to mark an unique running ability instance.
     * @return ERR_OK if success, else fail.
     */
    ErrCode StopBackgroundRunning(const std::string &abilityName, const sptr<IRemoteObject> &abilityToken) override;

    /**
     * @brief Subscribes background task event.
     *
     * @param subscriber Subscriber token.
     * @return ERR_OK if success, else fail.
     */
    ErrCode SubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber) override;

    /**
     * @brief Unsubscribes background task event.
     *
     * @param subscriber Subscriber token.
     * @return ERR_OK if success, else fail.
     */
    ErrCode UnsubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber) override;

    /**
     * @brief Get transient task applications.
     * @param list transient task apps.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetTransientTaskApps(std::vector<std::shared_ptr<TransientTaskAppInfo>> &list) override;

    /**
     * @brief Dump info of continous tasks or transient tasks.
     *
     * @param dumpOption Select continous tasks or transient tasks.
     * @param dumpInfo Info of continous tasks or transient tasks.
     * @return ERR_OK if success, else fail.
     */
    ErrCode ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo) override;

private:
    ErrCode InnerTransact(uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply);

    static inline BrokerDelegator<BackgroundTaskMgrProxy> delegator_;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_MGR_PROXY_H