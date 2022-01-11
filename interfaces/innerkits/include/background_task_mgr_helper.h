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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_INTERFACES_INNERKITS_CORE_INCLUDE_BACKGROUND_TASK_MGR_HELPER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_INTERFACES_INNERKITS_CORE_INCLUDE_BACKGROUND_TASK_MGR_HELPER_H

#include "background_task_subscriber.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundTaskMgrHelper {
public:
    /**
     * Subscribes background task event.
     *
     * @param subscriber subscriber token.
     * @return Returns ERR_OK on success, others on failure.
     */
    static ErrCode SubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber);
    
    /**
     * Unsubscribes background task event.
     *
     * @param subscriber subscriber token.
     * @return Returns ERR_OK on success, others on failure.
     */
    static ErrCode UnsubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber);
};

} // namespace BackgroundTaskMgr
} // namespace OHOS
#endif