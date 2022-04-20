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

#include "bgtask_common.h"

#include <parameters.h>
#include <string_ex.h>

#include "time_provider.h"

using OHOS::system::GetIntParameter;
using OHOS::system::GetParameter;

namespace OHOS {
namespace BackgroundTaskMgr {
constexpr int32_t DELAY_TIME_NORMAL_DEFAULT = 3 * MSEC_PER_MIN; // 3min
constexpr int32_t DELAY_TIME_LOW_POWER_DEFAULT = 1 * MSEC_PER_MIN; // 1min
constexpr int32_t ALLOW_REQUEST_TIME_BG_DEFAULT = 6 * MSEC_PER_SEC; // 6s
constexpr int32_t INIT_QUOTA_DEFAULT = 10 * MSEC_PER_MIN; // 10min
constexpr int64_t QUOTA_UPDATE_DEFAULT = 1 * MSEC_PER_DAY; // 1day

const std::string DELAY_TIME_NOMAL_PARAM = "persist.sys.bgtask_delaytime_normal";
const std::string INIT_QUOTA_PARAM = "persist.sys.bgtask_init_quota";
const std::string QUOTA_UPDATE_PARAM = "persist.sys.bgtask_quota_update";

const int32_t DELAY_TIME_NORMAL = GetIntParameter(DELAY_TIME_NOMAL_PARAM, DELAY_TIME_NORMAL_DEFAULT);
const int32_t DELAY_TIME_LOW_POWER = DELAY_TIME_LOW_POWER_DEFAULT;
const int32_t ALLOW_REQUEST_TIME_BG = ALLOW_REQUEST_TIME_BG_DEFAULT;
const int32_t INIT_QUOTA = GetIntParameter(INIT_QUOTA_PARAM, INIT_QUOTA_DEFAULT);
const int64_t QUOTA_UPDATE = GetIntParameter(QUOTA_UPDATE_PARAM, QUOTA_UPDATE_DEFAULT);
}  // namespace BackgroundTaskMgr
}  // namespace OHOS