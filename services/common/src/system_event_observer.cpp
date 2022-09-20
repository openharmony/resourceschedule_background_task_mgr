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

#include "system_event_observer.h"

#include "bundle_constants.h"
#include "common_event_support.h"

#include "bg_continuous_task_mgr.h"
#include "bgtaskmgr_inner_errors.h"
#include "continuous_task_log.h"
#include "bg_efficiency_resources_mgr.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
const std::string TASK_ON_BUNDLEINFO_CHANGED = "OnBundleInfoChanged";
const std::string TASK_ON_OS_ACCOUNT_CHANGED = "OnOsAccountChanged";
}

SystemEventObserver::SystemEventObserver(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo) {}

bool SystemEventObserver::Subscribe()
{
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(shared_from_this())) {
        BGTASK_LOGI("SubscribeCommonEvent occur exception.");
        return false;
    }
    return true;
}

bool SystemEventObserver::Unsubscribe()
{
    if (!EventFwk::CommonEventManager::UnSubscribeCommonEvent(shared_from_this())) {
        BGTASK_LOGI("UnsubscribeCommonEvent occur exception.");
        return false;
    }
    return true;
}

void SystemEventObserver::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler)
{
    handler_ = handler;
}

void SystemEventObserver::SetBgContinuousTaskMgr(const std::shared_ptr<BgContinuousTaskMgr> &bgContinuousTaskMgr)
{
    bgContinuousTaskMgr_ = bgContinuousTaskMgr;
}

void SystemEventObserver::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    OnReceiveEventContinuousTask(eventData);
    OnReceiveEventEfficiencyRes(eventData);
}

void SystemEventObserver::OnReceiveEventContinuousTask(const EventFwk::CommonEventData &eventData)
{
    auto handler = handler_.lock();
    if (!handler) {
        BGTASK_LOGE("handler is null");
        return;
    }
    auto bgContinuousTaskMgr = bgContinuousTaskMgr_.lock();
    if (!bgContinuousTaskMgr) {
        BGTASK_LOGE("bgContinuousTaskMgr is null");
        return;
    }
    AAFwk::Want want = eventData.GetWant();
    std::string action = want.GetAction();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED
        || action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_ADDED
        || action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED) {
        int32_t userId = eventData.GetCode();
        BGTASK_LOGI("user of id :%{public}d state is changed, action is : %{public}s", userId, action.c_str());
        auto task = [=]() { bgContinuousTaskMgr->OnAccountsStateChanged(userId); };
        handler->PostTask(task, TASK_ON_OS_ACCOUNT_CHANGED);
        return;
    }
    std::string bundleName = want.GetElement().GetBundleName();
    int32_t uid = want.GetIntParam(AppExecFwk::Constants::UID, -1);
    BGTASK_LOGI("get common event action = %{public}s, bundleName = %{public}s, uid = %{public}d",
        action.c_str(), bundleName.c_str(), uid);
    auto task = [=]() { bgContinuousTaskMgr->OnBundleInfoChanged(action, bundleName, uid); };
    handler->PostTask(task, TASK_ON_BUNDLEINFO_CHANGED);
}

void SystemEventObserver::OnReceiveEventEfficiencyRes(const EventFwk::CommonEventData &eventData)
{
    AAFwk::Want want = eventData.GetWant();
    std::string action = want.GetAction();
    std::string bundleName = want.GetElement().GetBundleName();
    int32_t uid = want.GetIntParam(AppExecFwk::Constants::UID, -1);
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        DelayedSingleton<BgEfficiencyResourcesMgr>::GetInstance()->RemoveAppRecord(uid, bundleName, true);
    }
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS