/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "background_mode.h"
#include "bg_continuous_task_dumper.h"
#include "bg_continuous_task_mgr.h"
#include <charconv>
#include <sstream>
#include <system_error>

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr int32_t MAX_DUMP_INNER_PARAM_NUMS = 4;
static constexpr int32_t MAX_DUMP_PARAM_NUMS = 3;

static bool ParseDumpInt32(const std::string &text, int32_t &out)
{
    if (text.empty()) {
        return false;
    }
    int32_t value = 0;
    auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (ec != std::errc{} || ptr != text.data() + text.size()) {
        return false;
    }
    out = value;
    return true;
}
}

BgContinuousTaskDumper::BgContinuousTaskDumper() {}

BgContinuousTaskDumper::~BgContinuousTaskDumper() {}

void BgContinuousTaskDumper::DebugContinuousTask(const std::vector<std::string> &dumpOption,
    std::vector<std::string> &dumpInfo)
{
    if (dumpOption.size() != MAX_DUMP_INNER_PARAM_NUMS && dumpOption.size() != MAX_DUMP_INNER_PARAM_NUMS + 1) {
        dumpInfo.emplace_back("param invaild\n");
        return;
    }
    std::string modeStr = dumpOption[MAX_DUMP_INNER_PARAM_NUMS - 2].c_str();
    uint32_t mode = 0;
    if (modeStr == "WORKOUT") {
        mode = BackgroundMode::WORKOUT;
    } else if (modeStr == "NEARBY_DATA_TRANSFER") {
        mode = BackgroundMode::NEARBY_DATA_TRANSFER;
    }
    if (mode == 0) {
        dumpInfo.emplace_back("param invaild\n");
        return;
    }
    std::string operationType = dumpOption[MAX_DUMP_INNER_PARAM_NUMS - 1].c_str();
    if (operationType != "apply" && operationType != "reset") {
        dumpInfo.emplace_back("param invaild\n");
        return;
    }
    bool isApply = (operationType == "apply");
    int32_t uid = 1;
    if (dumpOption.size() == MAX_DUMP_INNER_PARAM_NUMS + 1) {
        std::string uidStr = dumpOption[MAX_DUMP_INNER_PARAM_NUMS].c_str();
        if (!ParseDumpInt32(uidStr, uid) || uid < 0) {
            dumpInfo.emplace_back("param invaild\n");
            return;
        }
    }
    sptr<ContinuousTaskParamForInner> taskParam = sptr<ContinuousTaskParamForInner>(
        new ContinuousTaskParamForInner(uid, mode, isApply));
    ErrCode ret = ERR_OK;
    ret = BgContinuousTaskMgr::GetInstance()->DebugContinuousTaskInner(taskParam);
    if (ret != ERR_OK) {
        dumpInfo.emplace_back("dump inner continuous task fail.\n");
    } else {
        dumpInfo.emplace_back("dump inner continuous task success.\n");
    }
}

void BgContinuousTaskDumper::DumpGetTask(const std::vector<std::string> &dumpOption,
    std::vector<std::string> &dumpInfo)
{
    if (dumpOption.size() != MAX_DUMP_PARAM_NUMS) {
        dumpInfo.emplace_back("param invaild\n");
        return;
    }
    int32_t uid = 0;
    if (!ParseDumpInt32(dumpOption[MAX_DUMP_PARAM_NUMS - 1], uid) || uid < 0) {
        dumpInfo.emplace_back("param invaild\n");
        return;
    }
    std::vector<std::shared_ptr<ContinuousTaskInfo>> list;
    ErrCode ret = BgContinuousTaskMgr::GetInstance()->RequestGetContinuousTasksByUidForInner(uid, list);
    if (ret != ERR_OK) {
        dumpInfo.emplace_back("param invaild\n");
        return;
    }
    if (list.empty()) {
        dumpInfo.emplace_back("No running continuous task\n");
        return;
    }
    std::stringstream stream;
    uint32_t index = 1;
    for (const auto &info : list) {
        stream.str("");
        stream.clear();
        stream << "No." << index;
        stream << "\t\tabilityName: " << info->GetAbilityName() << "\n";
        stream << "\t\tisFromWebview: " << (info->IsFromWebView() ? "true" : "false") << "\n";
        stream << "\t\tuid: " << info->GetUid() << "\n";
        stream << "\t\tpid: " << info->GetPid() << "\n";
        stream << "\t\tnotificationId: " << info->GetNotificationId() << "\n";
        stream << "\t\tcontinuousTaskId: " << info->GetContinuousTaskId() << "\n";
        stream << "\t\tabilityId: " << info->GetAbilityId() << "\n";
        stream << "\t\twantAgentBundleName: " << info->GetWantAgentBundleName() << "\n";
        stream << "\t\twantAgentAbilityName: " << info->GetWantAgentAbilityName() << "\n";
        stream << "\t\tbackgroundModes: " << info->ToString(info->GetBackgroundModes()) << "\n";
        stream << "\t\tbackgroundSubModes: " << info->ToString(info->GetBackgroundSubModes()) << "\n";
        stream << "\n";
        dumpInfo.emplace_back(stream.str());
        index++;
    }
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
