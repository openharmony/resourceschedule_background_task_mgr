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

#include "ability_manager_client.h"
#include "bgtaskmgr_inner_errors.h"
#include "bundle_constants.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "file_ex.h"
#include "ipc_skeleton.h"
#include "string_ex.h"

#include "bgtaskmgr_log_wrapper.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr int32_t NO_DUMP_PARAM_NUMS = 0;
static constexpr int32_t MIN_DUMP_PARAM_NUMS = 1;
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(
    DelayedSingleton<BackgroundTaskMgrService>::GetInstance().get());
}

BackgroundTaskMgrService::BackgroundTaskMgrService()
    : SystemAbility(BACKGROUND_TASK_MANAGER_SERVICE_ID, true) {}

BackgroundTaskMgrService::~BackgroundTaskMgrService() {}

void BackgroundTaskMgrService::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        BGTASK_LOGW("Service has already started.");
        return;
    }
    Init();
    if (!Publish(DelayedSingleton<BackgroundTaskMgrService>::GetInstance().get())) {
        BGTASK_LOGE("Service start failed!");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    BGTASK_LOGI("background task manager service start succeed!");
}

void BackgroundTaskMgrService::Init()
{
    DelayedSingleton<BgTransientTaskMgr>::GetInstance()->Init();
    BgContinuousTaskMgr::GetInstance()->Init();
}

void BackgroundTaskMgrService::OnStop()
{
    BgContinuousTaskMgr::GetInstance()->Clear();
    state_ = ServiceRunningState::STATE_NOT_START;
    BGTASK_LOGI("background task manager stop");
}

ErrCode BackgroundTaskMgrService::RequestSuspendDelay(const std::u16string& reason,
    const sptr<IExpiredCallback>& callback, std::shared_ptr<DelaySuspendInfo> &delayInfo)
{
    return DelayedSingleton<BgTransientTaskMgr>::GetInstance()->RequestSuspendDelay(reason, callback, delayInfo);
}

ErrCode BackgroundTaskMgrService::CancelSuspendDelay(int32_t requestId)
{
    return DelayedSingleton<BgTransientTaskMgr>::GetInstance()->CancelSuspendDelay(requestId);
}

ErrCode BackgroundTaskMgrService::GetRemainingDelayTime(int32_t requestId, int32_t &delayTime)
{
    return DelayedSingleton<BgTransientTaskMgr>::GetInstance()->GetRemainingDelayTime(requestId, delayTime);
}

void BackgroundTaskMgrService::ForceCancelSuspendDelay(int32_t requestId)
{
    DelayedSingleton<BgTransientTaskMgr>::GetInstance()->ForceCancelSuspendDelay(requestId);
}

ErrCode BackgroundTaskMgrService::StartBackgroundRunning(const sptr<ContinuousTaskParam> &taskParam)
{
    return BgContinuousTaskMgr::GetInstance()->StartBackgroundRunning(taskParam);
}

ErrCode BackgroundTaskMgrService::StopBackgroundRunning(const std::string &abilityName,
    const sptr<IRemoteObject> &abilityToken)
{
    return BgContinuousTaskMgr::GetInstance()->StopBackgroundRunning(abilityName);
}

ErrCode BackgroundTaskMgrService::GetTransientTaskApps(std::vector<std::shared_ptr<TransientTaskAppInfo>> &list)
{
    return DelayedSingleton<BgTransientTaskMgr>::GetInstance()->GetTransientTaskApps(list);
}

ErrCode BackgroundTaskMgrService::GetContinuousTaskApps(std::vector<std::shared_ptr<ContinuousTaskCallbackInfo>> &list)
{
    return BgContinuousTaskMgr::GetInstance()->GetContinuousTaskApps(list);
}

ErrCode BackgroundTaskMgrService::SubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber)
{
    if (DelayedSingleton<BgTransientTaskMgr>::GetInstance()->SubscribeBackgroundTask(subscriber) == ERR_OK
        && BgContinuousTaskMgr::GetInstance()->AddSubscriber(subscriber) == ERR_OK) {
        return ERR_OK;
    }
    return ERR_BGTASK_SYS_NOT_READY;
}

ErrCode BackgroundTaskMgrService::UnsubscribeBackgroundTask(const sptr<IBackgroundTaskSubscriber>& subscriber)
{
    if (DelayedSingleton<BgTransientTaskMgr>::GetInstance()->UnsubscribeBackgroundTask(subscriber) == ERR_OK
        && BgContinuousTaskMgr::GetInstance()->RemoveSubscriber(subscriber) == ERR_OK) {
        return ERR_OK;
    }
    return ERR_BGTASK_SYS_NOT_READY;
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

ErrCode BackgroundTaskMgrService::ReportStateChangeEvent(const EventType type, const std::string &infos)
{
    return BgContinuousTaskMgr::GetInstance()->ReportStateChangeEvent(type, infos);
}

int32_t BackgroundTaskMgrService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    std::vector<std::string> argsInStr;
    std::transform(args.begin(), args.end(), std::back_inserter(argsInStr),
        [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });

    std::string result;

    int32_t ret = ERR_OK;

    if (argsInStr.size() == NO_DUMP_PARAM_NUMS) {
        DumpUsage(result);
    } else if (argsInStr.size() >= MIN_DUMP_PARAM_NUMS) {
        std::vector<std::string> infos;
        if (argsInStr[0] == "-h") {
            DumpUsage(result);
        } else if (argsInStr[0] == "-T") {
            ret = DelayedSingleton<BgTransientTaskMgr>::GetInstance()->ShellDump(argsInStr, infos);
        } else if (argsInStr[0] == "-C") {
            ret = BgContinuousTaskMgr::GetInstance()->ShellDump(argsInStr, infos);
        } else {
            infos.emplace_back("Error params.\n");
            ret = ERR_BGTASK_INVALID_PARAM;
        }
        for (auto info : infos) {
            result.append(info);
        }
    }

    if (!SaveStringToFd(fd, result)) {
        BGTASK_LOGE("BackgroundTaskMgrService dump save string to fd failed!");
        ret = ERR_BGTASK_METHOD_CALLED_FAILED;
    }
    return ret;
}

void BackgroundTaskMgrService::DumpUsage(std::string &result)
{
    std::string dumpHelpMsg =
    "usage: bgtask dump [<options>]\n"
    "options list:\n"
    "    -h                                   help menu\n"
    "    -T                                   transient task commands:\n"
    "        BATTARY_LOW                          battary low mode\n"
    "        BATTARY_OKAY                         battary okay mode\n"
    "        DUMP_CANCEL                          cancel dump mode\n"
    "        All                                  list all request\n"
    "    -C                                   continuous task commands:\n"
    "        --all                                list all running continuous task infos\n"
    "        --cancel_all                         cancel all running continuous task\n"
    "        --cancel {continuous task key}       cancel one task by specifying task key\n"
    "        --detection --all                    list all collected info for task detection\n";

    result.append(dumpHelpMsg);
}  // namespace
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
