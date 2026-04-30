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

#include "dialog_event_observer.h"

#include "bundle_constants.h"
#include "common_event_support.h"
#include "common_utils.h"
#include "bg_continuous_task_mgr.h"
#include "bgtaskmgr_inner_errors.h"
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
const std::string TASK_ON_PERMISSION_DIALOG_BUTTON_CLICK = "OnPermissionDialogButtonClick";
}

DialogEventObserver::DialogEventObserver(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo) {}

__attribute__((no_sanitize("cfi"))) bool DialogEventObserver::Subscribe()
{
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(shared_from_this())) {
        BGTASK_LOGE("SubscribeCommonEvent occur exception.");
        return false;
    }
    return true;
}

__attribute__((no_sanitize("cfi"))) bool DialogEventObserver::Unsubscribe()
{
    if (!EventFwk::CommonEventManager::UnSubscribeCommonEvent(shared_from_this())) {
        BGTASK_LOGE("UnsubscribeCommonEvent occur exception.");
        return false;
    }
    return true;
}

void DialogEventObserver::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler)
{
    handler_ = handler;
}

void DialogEventObserver::SetBgContinuousTaskMgr(const std::shared_ptr<BgContinuousTaskMgr> &bgContinuousTaskMgr)
{
    bgContinuousTaskMgr_ = bgContinuousTaskMgr;
}

void DialogEventObserver::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
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
    // 授权弹窗点击事件
    if (action != BGTASK_AUTH_DIALOG_EVENT_NAME) {
        BGTASK_LOGD("action type is invaild, action: %{public}s", action.c_str());
        return;
    }
    OnPermissionDialogButtonClick(handler, bgContinuousTaskMgr, eventData);
}

void DialogEventObserver::OnPermissionDialogButtonClick(
    const std::shared_ptr<AppExecFwk::EventHandler> &handler,
    const std::shared_ptr<BgContinuousTaskMgr> &bgContinuousTaskMgr,
    const EventFwk::CommonEventData &eventData)
{
    int32_t authResult = eventData.GetCode();
    AAFwk::Want want = eventData.GetWant();
    std::string bundleName = "";
    if (want.HasParameter("bundleName")) {
        bundleName = want.GetStringParam("bundleName");
    }
    int32_t bundleUid = -1;
    if (want.HasParameter("bundleUid")) {
        bundleUid = std::atoi(want.GetStringParam("bundleUid").c_str());
    }
    int32_t appIndex = -1;
    if (want.HasParameter("appIndex")) {
        appIndex = std::atoi(want.GetStringParam("appIndex").c_str());
    }
    if (appIndex == -1 || bundleUid == -1 || bundleName == "" || authResult < 0) {
        BGTASK_LOGE("OnPermissionDialogButtonClick get param fail.");
        return;
    }
    BGTASK_LOGI("dialog click, authresult: %{public}d, uid: %{public}d, bundleName: %{public}s, appIndex: %{public}d",
        authResult, bundleUid, bundleName.c_str(), appIndex);
    auto task = [bgContinuousTaskMgr, authResult, bundleUid, bundleName, appIndex]() {
        bgContinuousTaskMgr->OnPermissionDialogButtonClickInner(authResult, bundleUid, bundleName, appIndex);
    };
    handler->PostTask(task, TASK_ON_PERMISSION_DIALOG_BUTTON_CLICK);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS