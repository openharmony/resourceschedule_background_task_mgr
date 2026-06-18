/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "banner_notification_event_observer.h"

#include "bundle_constants.h"
#include "common_event_support.h"
#include "common_utils.h"
#include "bg_continuous_task_mgr.h"
#include "bgtaskmgr_inner_errors.h"
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
const std::string TASK_ON_BANNER_NOTIFICATION_ACTION_BUTTON_CLICK = "OnBannerNotificationActionButtonClick";
}

BannerNotificationEventObserver::BannerNotificationEventObserver(
    const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo) {}

__attribute__((no_sanitize("cfi"))) bool BannerNotificationEventObserver::Subscribe()
{
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(shared_from_this())) {
        BGTASK_LOGE("SubscribeCommonEvent occur exception.");
        return false;
    }
    return true;
}

__attribute__((no_sanitize("cfi"))) bool BannerNotificationEventObserver::Unsubscribe()
{
    if (!EventFwk::CommonEventManager::UnSubscribeCommonEvent(shared_from_this())) {
        BGTASK_LOGE("UnsubscribeCommonEvent occur exception.");
        return false;
    }
    return true;
}

void BannerNotificationEventObserver::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler)
{
    handler_ = handler;
}

void BannerNotificationEventObserver::SetBgContinuousTaskMgr(
    const std::shared_ptr<BgContinuousTaskMgr> &bgContinuousTaskMgr)
{
    bgContinuousTaskMgr_ = bgContinuousTaskMgr;
}

void BannerNotificationEventObserver::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
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
    std::string action = eventData.GetWant().GetAction();
    // 横幅通知授权按钮点击事件
    if (action != BGTASK_BANNER_NOTIFICATION_ACTION_NAME) {
        BGTASK_LOGD("action type is invaild, action: %{public}s", action.c_str());
        return;
    }
    OnBannerNotificationActionButtonClick(handler, bgContinuousTaskMgr, eventData);
}

void BannerNotificationEventObserver::OnBannerNotificationActionButtonClick(
    const std::shared_ptr<AppExecFwk::EventHandler> &handler,
    const std::shared_ptr<BgContinuousTaskMgr> &bgContinuousTaskMgr,
    const EventFwk::CommonEventData &eventData)
{
    AAFwk::Want want = eventData.GetWant();
    int32_t buttonType = want.GetIntParam(BGTASK_BANNER_NOTIFICATION_ACTION_PARAM_BTN, -1);
    int32_t uid = want.GetIntParam(BGTASK_BANNER_NOTIFICATION_ACTION_PARAM_UID, -1);
    std::string label = want.GetStringParam(BGTASK_BANNER_NOTIFICATION_ACTION_LABEL);
    if (buttonType == -1 || uid == -1 || label == "") {
        BGTASK_LOGE("OnBannerNotificationActionButtonClick get param fail.");
        return;
    }
    BGTASK_LOGI("banner notification onclick, buttonType: %{public}d, uid: %{public}d, label: %{public}s",
        buttonType, uid, label.c_str());
    auto task = [bgContinuousTaskMgr, buttonType, uid, label]() {
        bgContinuousTaskMgr->OnBannerNotificationActionButtonClick(buttonType, uid, label);
    };
    handler->PostTask(task, TASK_ON_BANNER_NOTIFICATION_ACTION_BUTTON_CLICK);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS