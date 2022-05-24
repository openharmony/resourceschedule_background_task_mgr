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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_SUBSCRIBER_PROXY_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_SUBSCRIBER_PROXY_H

#include <iremote_proxy.h>

#include "ibackground_task_subscriber.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundTaskSubscriberProxy : public IRemoteProxy<IBackgroundTaskSubscriber> {
public:
    BackgroundTaskSubscriberProxy() = delete;
    explicit BackgroundTaskSubscriberProxy(const sptr<IRemoteObject>& impl);
    ~BackgroundTaskSubscriberProxy() override;
    DISALLOW_COPY_AND_MOVE(BackgroundTaskSubscriberProxy);

    /**
     * @brief Called back when the subscriber is connected to Background Task Manager Service.
     */
    void OnConnected() override;

    /**
     * @brief Called back when the subscriber is disconnected from Background Task Manager Service.
     */
    void OnDisconnected() override;

    /**
     * @brief Called back when a transient task start.
     *
     * @param info Transient task app info.
     */
    void OnTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info) override;

    /**
     * @brief Called back when a transient task end.
     *
     * @param info Info of transient tasks.
     */
    void OnTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) override;

    /**
     * Called back when the app has transient task.
     *
     * @param info App info of transient task.
     **/
    void OnAppTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info) override;

    /**
     * Called back when the app does not have transient task.
     *
     * @param info App info transient task .
     **/
    void OnAppTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) override;

    /**
     * @brief Called back when a continuous task start.
     *
     * @param continuousTaskCallbackInfo Continuous task app info.
     */
    void OnContinuousTaskStart(
        const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) override;

    /**
     * @brief Called back when a continuous task stop.
     *
     * @param continuousTaskCallbackInfo Continuous task app info.
     */
    void OnContinuousTaskStop(
        const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) override;

private:
    static inline BrokerDelegator<BackgroundTaskSubscriberProxy> delegator_;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_SUBSCRIBER_PROXY_H