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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_COMMON_INCLUDE_BGTASKMGR_INNER_ERRORS_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_COMMON_INCLUDE_BGTASKMGR_INNER_ERRORS_H

#include "errors.h"
#include <map>

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
enum : int32_t {
    BGTASK_MODULE_COMMON = 0x00,
};

// Offset of background task manager sub-system's errcode base.
constexpr ErrCode BGTASK_COMMON_ERR_OFFSET = ErrCodeOffset(SUBSYS_IAWARE, BGTASK_MODULE_COMMON);
// Bgtask Common Error Code Defined.
enum : int32_t { 
    // errcode for common
    ERR_BGTASK_PERMISSION_DENIED = 201,
    ERR_BGTASK_INVALID_PARAM = 401,
    // errcode for Continuous Task
    ERR_BGTASK_SERVICE_FAILED = 9800003,
    ERR_BGTASK_IPC_FAILED,
    ERR_BGTASK_CONTINUOUS_VERIFY_FAILED,
    ERR_BGTASK_NOTIFICATION_FAILED,
    ERR_BGTASK_INNER_ERROR,
    // errcode for Transient Task
    ERR_BGTASK_CALLER_VERIFY_FAILED = 9900001,
    ERR_BGTASK_TRANSIENT_VERIFY_FAILED,
    // errcode for Efficiency Resource
    ERR_BGTASK_EFFICIENCY_CALLER_VERIFY_FAILED = 10000001,

    ERR_BGTASK_SERVICE_NOT_READY = BGTASK_COMMON_ERR_OFFSET + 1,
    ERR_BGTASK_SERVICE_NOT_CONNECTED,
    ERR_BGTASK_REMOTE_DEAD,
    ERR_BGTASK_PARCELABLE_FAILED,
    ERR_BGTASK_TRANSACT_FAILED,
    ERR_BGTASK_NOT_ALLOWED,
    ERR_BGTASK_OBJECT_EXISTS,
    ERR_BGTASK_METHOD_CALLED_FAILED,
    ERR_BGTASK_NON_SYSTEM_APP,
    ERR_BGTASK_SYS_NOT_READY,
    ERR_BGTASK_TASK_ERR,
    ERR_BGTASK_NOTIFICATION_ERR,
    ERR_BGTASK_INVALID_BGMODE,
    ERR_BGTASK_NO_MEMORY,
    ERR_BGTASK_DATA_STORAGE_ERR,
};

enum ParamErr: int32_t {
    ERR_PARAM_NUMBER_ERR = 9800201,
    ERR_REASON_TYPE_ERR,
    ERR_CALLBACK_TYPE_ERR,
    ERR_REQUEST_ID_TYPE_ERR,
    ERR_CONTEXT_TYPE_ERR,
    ERR_BGMODE_TYPE_ERR,
    ERR_WANTAGENT_TYPE_ERR,
    ERR_BGMODE_RANGE_ERR,
    ERR_RESOURCE_TYPE_ERR,
    ERR_ISAPPLY_TYPE_ERR,
    ERR_TIMEOUT_TYPE_ERR,
    ERR_TIMEOUT_RANGE_ERR,
    ERR_RESOURCE_TYPES_INVALID,
    ERR_CALLER_INFO_VERIFICATION_ERR,
};

enum SAErr: int32_t {
    ERR_APPLY_CONTINUOUS_TASK_REPEAT = 9800201,
    ERR_CANCLE_CONTINUOUS_TASK_REPEAT,
    ERR_WIFI_VOIP_VERIFY_ERR,
    ERR_KEEPING_TASK_VERIFY_ERR,
    ERR_INVALID_BGMODE,
    ERR_INVALID_REQUEST_ID,
    ERR_INVALID_BUNDLE_NAME,
    ERR_INVALID_PID_OR_UID,
    ERR_CALLBACK_EMPTY_ERR,
    ERR_CALLBACK_ALREADY_EXIST,
    ERR_CALLBACK_NOT_EXIST,
    ERR_BGTASK_EXCEEDS_ERR,
    ERR_TIME_INSUFFICIENT_ERR,

    ERR_GET_APP_INFO_FAILED,
    ERR_APPLY_SYSTEM_RESOURCE_REPEAT,
};

static std::map<int32_t, std::string> saErrCodeMsgMap = {
    {ERR_APPLY_CONTINUOUS_TASK_REPEAT, "Parcel operation failed. Failed to read the parcel."},
    {ERR_CANCLE_CONTINUOUS_TASK_REPEAT, "Parcel operation failed. Failed to write the parcel."},
    {ERR_WIFI_VOIP_VERIFY_ERR, "System service operation failed. Failed to get system ability manager."},
    {ERR_KEEPING_TASK_VERIFY_ERR, "System service operation failed. Failed to get system ability."},
    {ERR_INVALID_BGMODE, "System service operation failed. The service is not ready."},
    {ERR_INVALID_REQUEST_ID, "IPC communication failed. Failed to access the system service."},
    {ERR_INVALID_PID_OR_UID, "Check workInfo failed. Current bundleUid and input uid do not match."},
    {ERR_CALLBACK_EMPTY_ERR, "StartWork failed. The work has been already added."},
    {ERR_CALLBACK_ALREADY_EXIST, "StartWork failed. Each uid can add up to 10 works."},
    {ERR_CALLBACK_NOT_EXIST, "The workId do not exist."},
    {ERR_BGTASK_EXCEEDS_ERR, "Check workInfo failed. Current bundleUid and input uid do not match."},
    {ERR_TIME_INSUFFICIENT_ERR, "StartWork failed. The work has been already added."},
    {ERR_GET_APP_INFO_FAILED, "StartWork failed. Each uid can add up to 10 works."},
    {ERR_APPLY_SYSTEM_RESOURCE_REPEAT, "The workId do not exist."},
};

static std::map<int32_t, std::string> paramErrCodeMsgMap = {
    {ERR_PARAM_NUMBER_ERR, "The number of arguments is wrong."},
    {ERR_REASON_TYPE_ERR, "The type of workInfo must be {key: value} object."},
    {ERR_CALLBACK_TYPE_ERR, "The bundleName and abilityName cannot be empty."},
    {ERR_REQUEST_ID_TYPE_ERR, "The workId must be greater than 0."},
    {ERR_CONTEXT_TYPE_ERR, "The workinfo condition cannot be empty."},
    {ERR_BGMODE_TYPE_ERR, "The value of networkType ranges from NETWORK_TYPE_ANY to NETWORK_TYPE_ETHERNET."},
    {ERR_WANTAGENT_TYPE_ERR, "The value of chargerType ranges from CHARGING_PLUGGED_ANY to CHARGING_UNPLUGGED."},
    {ERR_ISAPPLY_TYPE_ERR, "The value of batteryLevel ranges from 0 to 100."},
    {ERR_TIMEOUT_RANGE_ERR, "The value of batteryStatus ranges from BATTERY_STATUS_LOW to BATTERY_STATUS_LOW_OR_OKAY."},
    {ERR_RESOURCE_TYPES_INVALID, "The value of storageRequest ranges from STORAGE_LEVEL_LOW to STORAGE_LEVEL_LOW_OR_OKAY."},
    {ERR_CALLER_INFO_VERIFICATION_ERR, "The number of repeatCount must be greater than or equal to 0."},

};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_COMMON_INCLUDE_BGTASKMGR_INNER_ERRORS_H