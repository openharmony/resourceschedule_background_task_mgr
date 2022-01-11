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

#include "time_provider.h"

namespace OHOS {
namespace BackgroundTaskMgr {
int64_t TimeProvider::GetCurrentTime(ClockType type)
{
    struct timespec ts;
    clock_gettime(type, &ts);
    return ((static_cast<int64_t>(ts.tv_sec) * MSEC_PER_SEC) + (static_cast<int64_t>(ts.tv_nsec) / NSEC_PER_MSEC));
}
} // namespace BackgroundTaskMgr
} // namespace OHOS

