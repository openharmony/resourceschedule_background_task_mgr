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

#include "os_account_observer.h"

#include "bg_continuous_task_mgr.h"
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
const std::string TASK_ON_OS_ACCOUNT_CHANGED = "OnOsAccountChanged";
}

OsAccountObserver::OsAccountObserver(const AccountSA::OsAccountSubscribeInfo &subscribeInfo)
    : OsAccountSubscriber(subscribeInfo) {}

bool OsAccountObserver::Subscribe()
{
    BGTASK_LOGI("Subscribe called");
    int ret = OHOS::AccountSA::OsAccountManager::SubscribeOsAccount(shared_from_this());
    if (ret != 0) {
        BGTASK_LOGE("register os account failed ret %{public}d", ret);
        return false;
    }
    return true;
}

bool OsAccountObserver::Unsubscribe()
{
    BGTASK_LOGI("UnSubscribe called");
    int ret = OHOS::AccountSA::OsAccountManager::UnsubscribeOsAccount(shared_from_this());
    if (ret != 0) {
        BGTASK_LOGE("register os account failed ret %{public}d", ret);
        return false;
    }
    return true;
}

void OsAccountObserver::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler)
{
    BGTASK_LOGI("set os account observer handler");
    handler_ = handler;
}

void OsAccountObserver::SetBgContinuousTaskMgr(const std::shared_ptr<BgContinuousTaskMgr> &bgContinuousTaskMgr)
{
    BGTASK_LOGI("set os account oberver bgContinuousTaskMgr");
    bgContinuousTaskMgr_ = bgContinuousTaskMgr;
}

void OsAccountObserver::OnAccountsChanged(const int &id)
{
    BGTASK_LOGI("on accounts state changed : %{public}d actived", id);
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

    auto task = [=]() { bgContinuousTaskMgr->OnAccountsStateChanged(id); };
    handler->PostTask(task, TASK_ON_OS_ACCOUNT_CHANGED);
}
}
}