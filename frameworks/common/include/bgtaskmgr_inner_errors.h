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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_FRAMEWORKS_COMMON_INCLUDE_BGTASKMGR_INNER_ERRORS_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_FRAMEWORKS_COMMON_INCLUDE_BGTASKMGR_INNER_ERRORS_H

#include "errors.h"

namespace OHOS {
namespace BackgroundTaskMgr {
/**
 * ErrCode layout
 *
 * +-----+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * | Bit |31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10|09|08|07|06|05|04|03|02|01|00|
 * +-----+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |Field|Reserved|        Subsystem      |  Module      |                              Code             |
 * +-----+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 */

// BackgroundTaskMgr's module const defined.
enum : int {
    BGTASK_MODULE_COMMON = 0x00,
};

// Offset of background task manager sub-system's errcode base.
constexpr ErrCode BGTASK_COMMON_ERR_OFFSET = ErrCodeOffset(SUBSYS_IAWARE, BGTASK_MODULE_COMMON);
//Bgtask Common Error Code Defined.
enum : int {
    ERR_BGTASK_SERVICE_NOT_READY = BGTASK_COMMON_ERR_OFFSET + 1,
    ERR_BGTASK_SERVICE_NOT_CONNECTED,
    ERR_BGTASK_REMOTE_DEAD,
    ERR_BGTASK_PARCELABLE_FAILED,
    ERR_BGTASK_TRANSACT_FAILED,
    ERR_BGTASK_INVALID_PARAM,
};

} // namespace BackgroundTaskMgr
} // namespace OHOS
#endif