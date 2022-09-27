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

// Bgtask Common Error Code Defined.
enum : int32_t { 
    // errcode for common
    ERR_BGTASK_PERMISSION_DENIED = 201,
    ERR_BGTASK_INVALID_PARAM = 401,
    // errcode for Continuous Task
    ERR_BGTASK_SYS_NOT_READY = 9800001,
    ERR_BGTASK_SERVICE_NOT_CONNECTED,
    ERR_BGTASK_PARCELABLE_FAILED,
    ERR_BGTASK_IPC_FAILED,
    ERR_BGTASK_OBJECT_EXISTS,
    ERR_BGTASK_OBJECT_NOT_EXIST,
    ERR_BGTASK_WIFI_VOIP_VERIFY_ERR,
    ERR_BGTASK_KEEPING_TASK_VERIFY_ERR,
    ERR_BGTASK_INVALID_BGMODE,
    ERR_BGTASK_NOTIFICATION_VERIFY_FAILED,
    ERR_BGTASK_NOTIFICATION_ERR,
    ERR_BGTASK_DATA_STORAGE_ERR,
    // errcode for Transient Task
    // ERR_BGTASK_CALLER_VERIFY_FAILED = 9900001,
    ERR_BGTASK_SERVICE_NOT_READY = 9900001,
    ERR_BGTASK_INVALID_PID_OR_UID,
    ERR_BGTASK_INVALID_BUNDLE_NAME,
    ERR_BGTASK_INVALID_REQUEST_ID,
    ERR_BGTASK_CALLBACK_NOT_EXIST,
    ERR_BGTASK_CALLBACK_EXISTS,
    ERR_BGTASK_NOT_IN_PRESET_TIME,
    ERR_BGTASK_EXCEEDS_THRESHOLD,
    ERR_BGTASK_TIME_INSUFFICIENT,
    // errcode for Efficiency Resource
    ERR_BGTASK_EFFICIENCY_CALLER_VERIFY_FAILED = 18700001,
    ERR_BGTASK_RESOURCES_EXIST,
    ERR_BGTASK_INNER_ERROR,
    // other inner errcode
    ERR_BGTASK_TRANSACT_FAILED,
    ERR_BGTASK_NOT_ALLOWED,
    ERR_BGTASK_METHOD_CALLED_FAILED,
    ERR_BGTASK_NO_MEMORY,
};

enum ParamErr: int32_t {
    ERR_PARAM_NUMBER_ERR = 9800401,
    ERR_REASON_NULL_OR_TYPE_ERR,
    ERR_CALLBACK_NULL_OR_TYPE_ERR,
    ERR_REQUESTID_NULL_OR_ID_TYPE_ERR,
    ERR_REQUESTID_ILLEGAL,
    ERR_CONTEXT_NULL_OR_TYPE_ERR,
    ERR_BGMODE_NULL_OR_TYPE_ERR,
    ERR_WANTAGENT_NULL_OR_TYPE_ERR,
    ERR_ABILITY_INFO_EMPTY,//
    ERR_GET_TOKEN_ERR,//
    ERR_BGMODE_RANGE_ERR,
    ERR_APP_NAME_EMPTY,//
    ERR_RESOURCE_TYPE_ERR,
    ERR_ISAPPLY_TYPE_ERR,
    ERR_TIMEOUT_TYPE_ERR,
    ERR_TIMEOUT_RANGE_ERR,
    ERR_RESOURCE_TYPES_INVALID,
    ERR_CALLER_INFO_VERIFICATION_ERR,
};

static std::map<int32_t, std::string> saErrCodeMsgMap = {
    {ERR_BGTASK_PERMISSION_DENIED, "Permission denied."},
    {ERR_BGTASK_SERVICE_NOT_READY, "Parcel operation failed. Failed to read the parcel."},
    {ERR_BGTASK_SERVICE_NOT_CONNECTED, "Parcel operation failed. Failed to write the parcel."},
    {ERR_BGTASK_PARCELABLE_FAILED, "System service operation failed. Failed to get system ability manager."},
    {ERR_BGTASK_IPC_FAILED, "System service operation failed. Failed to get system ability."},
    {ERR_BGTASK_OBJECT_EXISTS, "System service operation failed. The service is not ready."},
    {ERR_BGTASK_OBJECT_NOT_EXIST, "IPC communication failed. Failed to access the system service."},
    {ERR_BGTASK_WIFI_VOIP_VERIFY_ERR, "Check workInfo failed. Current bundleUid and input uid do not match."},
    {ERR_BGTASK_KEEPING_TASK_VERIFY_ERR, "StartWork failed. The work has been already added."},
    {ERR_BGTASK_INVALID_BGMODE, "StartWork failed. Each uid can add up to 10 works."},
    {ERR_BGTASK_NOTIFICATION_VERIFY_FAILED, "The workId do not exist."},
    {ERR_BGTASK_NOTIFICATION_ERR, "Check workInfo failed. Current bundleUid and input uid do not match."},
    {ERR_BGTASK_DATA_STORAGE_ERR, "StartWork failed. The work has been already added."},
    {ERR_BGTASK_INVALID_PID_OR_UID, "StartWork failed. Each uid can add up to 10 works."},
    {ERR_BGTASK_INVALID_BUNDLE_NAME, "The workId do not exist."},
    {ERR_BGTASK_INVALID_REQUEST_ID, "StartWork failed. The work has been already added."},
    {ERR_BGTASK_CALLBACK_NOT_EXIST, "StartWork failed. Each uid can add up to 10 works."},
    {ERR_BGTASK_CALLBACK_EXISTS, "The workId do not exist."},
    {ERR_BGTASK_NOT_IN_PRESET_TIME, "Check workInfo failed. Current bundleUid and input uid do not match."},
    {ERR_BGTASK_EXCEEDS_THRESHOLD, "StartWork failed. The work has been already added."},
    {ERR_BGTASK_TIME_INSUFFICIENT, "StartWork failed. Each uid can add up to 10 works."},
    {ERR_BGTASK_EFFICIENCY_CALLER_VERIFY_FAILED, "The workId do not exist."},
    {ERR_BGTASK_RESOURCES_EXIST, "StartWork failed. Each uid can add up to 10 works."},
    {ERR_BGTASK_INNER_ERROR, "The workId do not exist."},
};

static std::map<int32_t, std::string> paramErrCodeMsgMap = {
    {ERR_PARAM_NUMBER_ERR, "The number of arguments is wrong."},
    {ERR_REASON_NULL_OR_TYPE_ERR, "The reason cannot be null and its type must be string."},
    {ERR_CALLBACK_NULL_OR_TYPE_ERR, "The callback cannot be null and its type must be function."},
    {ERR_REQUESTID_NULL_OR_ID_TYPE_ERR, "The requestId cannot be null and its type must be integer."},
    {ERR_REQUESTID_ILLEGAL, "The requestId must be greater than 0."},
    {ERR_CONTEXT_NULL_OR_TYPE_ERR, "The context cannot be null and its type must be Context."},
    {ERR_BGMODE_NULL_OR_TYPE_ERR, "The bgMode cannot be null and its type must be BackgroundMode object."},
    {ERR_WANTAGENT_NULL_OR_TYPE_ERR, "The wantAgent cannot be null and its type must be WantAgent object."},
    {ERR_ABILITY_INFO_EMPTY, "The abilityInfo of context cannot be null."},
    {ERR_GET_TOKEN_ERR, "The token of context cannot be null."},
    {ERR_BGMODE_RANGE_ERR, "The value of bgMode ranges from BG_MODE_ID_BEGIN to BG_MODE_ID_END."},
    {ERR_APP_NAME_EMPTY, "The app name of abilityInfo cannot be null."},
    {ERR_RESOURCE_TYPE_ERR, "The value of chargerType ranges from CHARGING_PLUGGED_ANY to CHARGING_UNPLUGGED."},
    {ERR_ISAPPLY_TYPE_ERR, "The value of batteryLevel ranges from 0 to 100."},
    {ERR_TIMEOUT_RANGE_ERR, "The value of batteryStatus ranges from BATTERY_STATUS_LOW to BATTERY_STATUS_LOW_OR_OKAY."},
    {ERR_RESOURCE_TYPES_INVALID, "The value of storageRequest ranges from STORAGE_LEVEL_LOW to STORAGE_LEVEL_LOW_OR_OKAY."},
    {ERR_CALLER_INFO_VERIFICATION_ERR, "The number of repeatCount must be greater than or equal to 0."},
    {ERR_BGTASK_INVALID_PARAM, "The input param is invalid."}

};

}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_COMMON_INCLUDE_BGTASKMGR_INNER_ERRORS_H