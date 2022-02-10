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

#include "bgtaskmgr_shell_command.h"

#include <getopt.h>
#include <iostream>

#include "iservice_registry.h"
#include "singleton.h"
#include "system_ability_definition.h"

#include "bgtaskmgr_inner_errors.h"
#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr int32_t MIN_CONTINUOUS_TASK_DUMP_PARAMS_NUM = 4;
static const struct option OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"transient", no_argument, nullptr, 'T'},
    {"continuous", no_argument, nullptr, 'C'},
};

static const std::string HELP_MSG = "usage: bgtask <command> [<options>]\n"
                                    "These are common commands list:\n"
                                    "  help                         list available commands\n"
                                    "  dump                         dump the info of bgtaskmgr\n";
static const std::string DUMP_HELP_MSG =
    "usage: bgtask dump [<options>]\n"
    "options list:\n"
    "  -h                                       help menu\n"
    "  -T                                       transient task commands:\n"
    "           BATTARY_LOW                     battary low mode\n"
    "           BATTARY_OKAY                    battary okay mode\n"
    "           SCREEN_ON                       sreen on mode\n"
    "           SCREEN_OFF                      sreen off mode\n"
    "           DUMP_CANCEL                     cancel dump mode\n"
    "           All                             list all request\n"
    "  -C,      --all                           list all running continuous task infos\n"
    "           --cancel_all                    cancel all running continuous task\n"
    "           --cancel <continuous task key>  cancel one task by specifying task key\n";
}  // namespace

BgtaskmgrShellCommand::BgtaskmgrShellCommand(int argc, char *argv[]) : ShellCommand(argc, argv, "bgtask")
{}

ErrCode BgtaskmgrShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", std::bind(&BgtaskmgrShellCommand::RunAsHelpCommand, this)},
        {"dump", std::bind(&BgtaskmgrShellCommand::RunAsDumpCommand, this)},
    };

    return ERR_OK;
}

ErrCode BgtaskmgrShellCommand::CreateMessageMap()
{
    messageMap_ = {};

    return ERR_OK;
}

ErrCode BgtaskmgrShellCommand::init()
{
    ErrCode result = OHOS::ERR_OK;

    if (!btm_) {
        btm_ = DelayedSingleton<BackgroundTaskManager>::GetInstance();
    }

    if (!btm_) {
        result = OHOS::ERR_INVALID_VALUE;
        BGTASK_LOGE("BgtaskmgrShellCommand init failed");
    }

    return result;
}

ErrCode BgtaskmgrShellCommand::RunAsHelpCommand()
{
    resultReceiver_.append(HELP_MSG);
    return ERR_OK;
}

ErrCode BgtaskmgrShellCommand::RunAsDumpCommand()
{
    int ind = 0;
    int option = getopt_long(argc_, argv_, "hTC", OPTIONS, &ind);

    ErrCode ret = ERR_OK;
    if (btm_ == nullptr) {
        return ERR_INVALID_VALUE;
    }
    std::vector<std::string> infos;

    switch (option) {
        case 'h':
            resultReceiver_.append(DUMP_HELP_MSG);
            break;
        case 'T':
            if (btm_ != nullptr) {
                ret = btm_->ShellDump(argList_, infos);
            }
            break;
        case 'C':
            if (argc_ < MIN_CONTINUOUS_TASK_DUMP_PARAMS_NUM || btm_ == nullptr) {
                resultReceiver_.append(DUMP_HELP_MSG);
                ret = ERR_BGTASK_SERVICE_NOT_CONNECTED;
                break;
            }
            ret = btm_->ShellDump(argList_, infos);
            break;
        default:
            resultReceiver_.append("Please input correct command!\n");
            resultReceiver_.append(DUMP_HELP_MSG);
            break;
    }
    
    for (auto info : infos) {
        resultReceiver_.append(info);
    }

    return ret;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS