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

#include "background_task_mgr_service.h"

#include <functional>
#include <iomanip>
#include <sstream>

#include "ability_manager_client.h"
#include "bundle_constants.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "ipc_skeleton.h"

#include "bgtaskmgr_log_wrapper.h"

namespace OHOS {
namespace BackgroundTaskMgr {

namespace {
static const std::string TRANSIENT_TASK_DUMP = "-T";
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(
    DelayedSingleton<BackgroundTaskMgrService>::GetInstance().get());
}

BackgroundTaskMgrService::BackgroundTaskMgrService()
    : SystemAbility(BACKGROUND_TASK_MANAGER_SERVICE_ID, true)
{}

BackgroundTaskMgrService::~BackgroundTaskMgrService()
{}

void BackgroundTaskMgrService::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        BGTASK_LOGI("Service has already started.");
        return;
    }

    Init();

    if (!Publish(DelayedSingleton<BackgroundTaskMgrService>::GetInstance().get())) {
        BGTASK_LOGI("Service start failed!");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    BGTASK_LOGI("background task manager service start succeed!");
}

void BackgroundTaskMgrService::Init()
{
    DelayedSingleton<BgTransientTaskMgr>::GetInstance()->Init();
}

void BackgroundTaskMgrService::OnStop()
{
    state_ = ServiceRunningState::STATE_NOT_START;
    BGTASK_LOGI("background task manager stop");
}

ErrCode BackgroundTaskMgrService::RequestSuspendDelay(const std::u16string& reason, 
        const sptr<IExpiredCallback>& callback, std::shared_ptr<DelaySuspendInfo> &delayInfo)
{
    delayInfo = DelayedSingleton<BgTransientTaskMgr>::GetInstance()->RequestSuspendDelay(reason, callback);
    return ERR_OK;
}

ErrCode BackgroundTaskMgrService::CancelSuspendDelay(int32_t requestId)
{
    DelayedSingleton<BgTransientTaskMgr>::GetInstance()->CancelSuspendDelay(requestId);
    return ERR_OK;
}

ErrCode BackgroundTaskMgrService::GetRemainingDelayTime(int32_t requestId, int32_t &delayTime)
{
    delayTime = DelayedSingleton<BgTransientTaskMgr>::GetInstance()->GetRemainingDelayTime(requestId);
    return ERR_OK;
}

void BackgroundTaskMgrService::ForceCancelSuspendDelay(int32_t requestId)
{
    DelayedSingleton<BgTransientTaskMgr>::GetInstance()->ForceCancelSuspendDelay(requestId);
}

ErrCode BackgroundTaskMgrService::SubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber)
{
    DelayedSingleton<BgTransientTaskMgr>::GetInstance()->SubscribeTransientTask(subscriber);
    return ERR_OK;
}

ErrCode BackgroundTaskMgrService::UnsubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber)
{
    DelayedSingleton<BgTransientTaskMgr>::GetInstance()->UnsubscribeTransientTask(subscriber);
    return ERR_OK;
}

void BackgroundTaskMgrService::HandleRequestExpired(const int32_t requestId)
{
    DelayedSingleton<BgTransientTaskMgr>::GetInstance()->HandleRequestExpired(requestId);
}

void BackgroundTaskMgrService::HandleExpiredCallbackDeath(const wptr<IRemoteObject>& remote)
{
    DelayedSingleton<BgTransientTaskMgr>::GetInstance()->HandleExpiredCallbackDeath(remote);
}

void BackgroundTaskMgrService::HandleSubscriberDeath(const wptr<IRemoteObject>& remote)
{
    DelayedSingleton<BgTransientTaskMgr>::GetInstance()->HandleSubscriberDeath(remote);
}

ErrCode BackgroundTaskMgrService::ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo)
{
    if (dumpOption[0] == TRANSIENT_TASK_DUMP) {
        DelayedSingleton<BgTransientTaskMgr>::GetInstance()->ShellDump(dumpOption, dumpInfo);
    }
    return ERR_OK;
}

}  // namespace BackgroundTaskMgr
}  // namespace OHOS