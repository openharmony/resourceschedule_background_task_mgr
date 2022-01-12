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

    ErrCode RequestSuspendDelay(const std::u16string& reason, 
        const sptr<IExpiredCallback>& callback, std::shared_ptr<DelaySuspendInfo> &delayInfo) override;
    ErrCode CancelSuspendDelay(int32_t requestId) override;
    ErrCode GetRemainingDelayTime(int32_t requestId, int32_t &delayTime) override;
    ErrCode SubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber) override;
    ErrCode UnsubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber) override;
    ErrCode ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo) override;

private:
    ErrCode InnerTransact(uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply);

    static inline BrokerDelegator<BackgroundTaskMgrProxy> delegator_;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_INCLUDE_BACKGROUND_TASK_MGR_PROXY_H