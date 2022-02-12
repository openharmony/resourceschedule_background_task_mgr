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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_UTILS_BGTASK_OBSERVER_INCLUDE_BGTASK_OBSERVER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_UTILS_BGTASK_OBSERVER_INCLUDE_BGTASK_OBSERVER_H

#include "background_task_mgr_helper.h"
#include "background_task_subscriber.h"
#include "iremote_object.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class BgTaskObserver : public OHOS::BackgroundTaskMgr::BackgroundTaskSubscriber {
public:
    virtual void OnContinuousTaskStart(const std::shared_ptr<OHOS::BackgroundTaskMgr::ContinuousTaskCallbackInfo>
        &continuousTaskCallbackInfo) override;

    virtual void OnContinuousTaskStop(const std::shared_ptr<OHOS::BackgroundTaskMgr::ContinuousTaskCallbackInfo>
        &continuousTaskCallbackInfo) override;

    virtual void OnRemoteDied(const wptr<IRemoteObject> &object) override;

    std::atomic<bool> isRemoteDied_ {false};
};
}
}
#endif