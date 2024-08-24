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

#include "hilog/log.h"

#ifdef LOG_DOMAIN
#undef LOG_DOMAIN
#endif
#define LOG_DOMAIN 0xD001711

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "BACKGROUND_TASK"

namespace OHOS {
namespace BackgroundTaskMgr {

#ifndef FORMAT_LOG
#define FORMAT_LOG(fmt, ...) "[%{public}s:%{public}d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__
#endif

#define BACKGROUND_TASK_PRINT_LOGD(fmt, ...) HILOG_DEBUG(LOG_CORE, fmt, ##__VA_ARGS__)
#define BACKGROUND_TASK_PRINT_LOGI(fmt, ...) HILOG_INFO(LOG_CORE, fmt, ##__VA_ARGS__)
#define BACKGROUND_TASK_PRINT_LOGW(fmt, ...) HILOG_WARN(LOG_CORE, fmt, ##__VA_ARGS__)
#define BACKGROUND_TASK_PRINT_LOGE(fmt, ...) HILOG_ERROR(LOG_CORE, fmt, ##__VA_ARGS__)
#define BACKGROUND_TASK_PRINT_LOGF(fmt, ...) HILOG_FATAL(LOG_CORE, fmt, ##__VA_ARGS__)

#define BGTASK_LOGD(...) BACKGROUND_TASK_PRINT_LOGD(FORMAT_LOG(__VA_ARGS__))
#define BGTASK_LOGI(...) BACKGROUND_TASK_PRINT_LOGI(FORMAT_LOG(__VA_ARGS__))
#define BGTASK_LOGW(...) BACKGROUND_TASK_PRINT_LOGW(FORMAT_LOG(__VA_ARGS__))
#define BGTASK_LOGE(...) BACKGROUND_TASK_PRINT_LOGE(FORMAT_LOG(__VA_ARGS__))
#define BGTASK_LOGF(...) BACKGROUND_TASK_PRINT_LOGF(FORMAT_LOG(__VA_ARGS__))
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_COMMON_INCLUDE_BGTASKMGR_LOG_WRAPPER_H