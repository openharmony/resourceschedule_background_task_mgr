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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_BGTASK_COMMON_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_BGTASK_COMMON_H

#include <cstdint>

namespace OHOS {
namespace BackgroundTaskMgr {
extern const int32_t DELAY_TIME_NORMAL;
extern const int32_t DELAY_TIME_LOW_POWER;
extern const int32_t ALLOW_REQUEST_TIME_BG;
extern const int32_t INIT_QUOTA;
extern const int64_t QUOTA_UPDATE;
extern const bool SUPPORT_TASK_KEEPING;
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_BGTASK_COMMON_H