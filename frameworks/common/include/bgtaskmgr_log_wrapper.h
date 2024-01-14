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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_COMMON_INCLUDE_BGTASKMGR_LOG_WRAPPER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_COMMON_INCLUDE_BGTASKMGR_LOG_WRAPPER_H

#include <string>

#include "hilog/log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
#ifndef BGTASK_MGR_LOG_DOMAIN
#define BGTASK_MGR_LOG_DOMAIN 0xD001711
#endif

#ifndef BGTASK_MGR_LOG_TAG
#define BGTASK_MGR_LOG_TAG "BACKGROUND_TASK"
#endif

enum class BgTaskMgrLogLevel : uint8_t { DEBUG = 0, INFO, WARN, ERROR, FATAL };

static constexpr OHOS::HiviewDFX::HiLogLabel BGTASK_MGR_LABEL = {LOG_CORE, BGTASK_MGR_LOG_DOMAIN,
    BGTASK_MGR_LOG_TAG};

class BgTaskMgrLogWrapper {
public:
    BgTaskMgrLogWrapper() = delete;
    ~BgTaskMgrLogWrapper() = delete;

    static bool JudgeLevel(const BgTaskMgrLogLevel &level);

    static void SetLogLevel(const BgTaskMgrLogLevel &level)
    {
        level_ = level;
    }

    static const BgTaskMgrLogLevel &GetLogLevel()
    {
        return level_;
    }

    static std::string GetBriefFileName(const char *str);

private:
    static BgTaskMgrLogLevel level_;
};

#define BGTASK_PRINT_LOG(LEVEL, Level, fmt, ...)                  \
    if (BackgroundTaskMgr::BgTaskMgrLogWrapper::JudgeLevel(BackgroundTaskMgr::BgTaskMgrLogLevel::LEVEL))     \
    OHOS::HiviewDFX::HiLog::Level(BackgroundTaskMgr::BGTASK_MGR_LABEL,               \
        fmt,        \
        ##__VA_ARGS__)

#define BGTASK_LOGD(fmt, ...) BGTASK_PRINT_LOG(DEBUG, Debug, fmt, ##__VA_ARGS__)
#define BGTASK_LOGI(fmt, ...) BGTASK_PRINT_LOG(INFO, Info, fmt, ##__VA_ARGS__)
#define BGTASK_LOGW(fmt, ...) BGTASK_PRINT_LOG(WARN, Warn, fmt, ##__VA_ARGS__)
#define BGTASK_LOGE(fmt, ...) BGTASK_PRINT_LOG(ERROR, Error, fmt, ##__VA_ARGS__)
#define BGTASK_LOGF(fmt, ...) BGTASK_PRINT_LOG(FATAL, Fatal, fmt, ##__VA_ARGS__)
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_COMMON_INCLUDE_BGTASKMGR_LOG_WRAPPER_H