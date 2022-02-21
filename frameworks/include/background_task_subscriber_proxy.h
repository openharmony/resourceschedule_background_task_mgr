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

    void OnConnected() override;
    void OnDisconnected() override;
    void OnTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info) override;
    void OnTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) override;
    void OnContinuousTaskStart(
        const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) override;
    void OnContinuousTaskStop(
        const std::shared_ptr<ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo) override;

private:
    static inline BrokerDelegator<BackgroundTaskSubscriberProxy> delegator_;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_SUBSCRIBER_PROXY_H