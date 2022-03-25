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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_TASK_MGR_HELPER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_TASK_MGR_HELPER_H

#include "background_task_subscriber.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundTaskMgrHelper {
public:
    /**
     * request service to keep running background
     *
     * @param taskParam request params.
     * @return ERR_OK if success, else fail.
     */
    static ErrCode RequestStartBackgroundRunning(const ContinuousTaskParam &taskParam);

    /**
     * request service to stop running background
     *
     * @param abilityName ability name of the requester ability
     * @param abilityToken ability token to mark an unique running ability instance
     * @return ERR_OK if success, else fail.
     */
    static ErrCode RequestStopBackgroundRunning(const std::string &abilityName,
        const sptr<IRemoteObject> &abilityToken);

    /**
     * Subscribes background task event.
     *
     * @param subscriber subscriber token.
     * @return ERR_OK if success, else fail.
     */
    static ErrCode SubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber);

    /**
     * Unsubscribes background task event.
     *
     * @param subscriber subscriber token.
     * @return ERR_OK if success, else fail.
     */
    static ErrCode UnsubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber);
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_TASK_MGR_HELPER_H