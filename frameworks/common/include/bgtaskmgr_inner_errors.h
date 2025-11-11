/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include <string_ex.h>

namespace OHOS {
namespace BackgroundTaskMgr {
/**
 * ErrCode layout
 *
 * +--+--+--+--+--+--+--+--+---+---+
 * |09|08|07|06|05|04|03|02| 01| 00|
 * +--+--+--+--+--+--+--+--+---+---+
 * | Syscap |      Code    |Subcode|
 * +--+--+--+--+--+--+--+--+---+---+
 */
const int OFFSET = 100;
const int THRESHOLD = 1000;
// Bgtask Common Error Code Defined.
enum : int32_t {
    // errcode for common
    ERR_BGTASK_PERMISSION_DENIED = 201,
    ERR_BGTASK_NOT_SYSTEM_APP = 202,
    ERR_BGTASK_INVALID_PARAM = 401,
    // errcode for Continuous Task
    ERR_BGTASK_NO_MEMORY = 980000101,
    ERR_BGTASK_PARCELABLE_FAILED = 980000201,
    ERR_BGTASK_TRANSACT_FAILED = 980000301,
    ERR_BGTASK_SYS_NOT_READY = 980000401,
    ERR_BGTASK_SERVICE_NOT_CONNECTED,
    ERR_BGTASK_OBJECT_EXISTS = 980000501,
    ERR_BGTASK_OBJECT_NOT_EXIST,
    ERR_BGTASK_KEEPING_TASK_VERIFY_ERR,
    ERR_BGTASK_INVALID_BGMODE,
    ERR_BGTASK_INVALID_UID,
    ERR_BGTASK_CONTINUOUS_REQUEST_NULL_OR_TYPE,
    ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_IS_EMPTY,
    ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_LENGTH_MISMATCH,
    ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_CROSS_BORDER,
    ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_TYPE_MISMATCH,
    ERR_BGTASK_CONTINUOUS_NOT_UPDATE_BY_OLD_INTERFACE,
    ERR_BGTASK_CONTINUOUS_TASKID_INVALID,
    ERR_BGTASK_CONTINUOUS_DATA_TRANSFER_NOT_MERGE_NOTIFICATION,
    ERR_BGTASK_CONTINUOUS_SPECIAL_SCENARIO_PROCESSING_NOT_MERGE_NOTIFICATION,
    ERR_BGTASK_CONTINUOUS_NOT_MERGE_NOTIFICATION_NOT_EXIST,
    ERR_BGTASK_CONTINUOUS_NOT_MERGE_COMBINED_FALSE,
    ERR_BGTASK_CONTINUOUS_NOT_UPDATE_BECAUSE_MERGE,
    ERR_BGTASK_CONTINUOUS_NOT_MERGE_CURRENTTASK_COMBINED_FALSE,
    ERR_BGTASK_CONTINUOUS_UPDATE_FAIL_SAME_MODE_AND_MERGED,
    ERR_BGTASK_CONTINUOUS_SYSTEM_APP_NOT_SUPPORT_ACL,
    ERR_BGTASK_CONTINUOUS_APP_NOT_HAVE_BGMODE_PERMISSION_SYSTEM,
    ERR_BGTASK_CONTINUOUS_NOT_APPLY_MAX_TASK,
    ERR_BGTASK_CONTINUOUS_DATA_TRANSFER_NOT_UPDATE,
    ERR_BGTASK_CONTINUOUS_NOT_APPLY_ONBACKGROUND,
    ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_ONLY_ALLOW_ONE_APPLICATION,
    ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_CONFLICTS_WITH_OTHER_TASK,
    ERR_BGTASK_GET_APP_INDEX_FAIL,
    ERR_BGTASK_APP_DETECTED_MALICIOUS_BEHAVIOR,
    ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_EMPTY,
    ERR_BGTASK_CONTINUOUS_CALLBACK_NULL_OR_TYPE_ERR,
    ERR_BGTASK_CONTINUOUS_CALLBACK_EXISTS,
    ERR_BGTASK_MALICIOUS_CONTINUOUSTASK,
    ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_NOTSUPPORT_DEVICE,
    ERR_BGTASK_NOTIFICATION_VERIFY_FAILED = 980000601,
    ERR_BGTASK_NOTIFICATION_ERR,
    ERR_BGTASK_CHECK_TASK_PARAM,
    ERR_BGTASK_CONTINUOUS_UPDATE_NOTIFICATION_FAIL,
    ERR_BGTASK_CREATE_FILE_ERR = 980000701,
    ERR_BGTASK_GET_ACTUAL_FILE_ERR,
    ERR_BGTASK_OPEN_FILE_ERR,
    // errcode for Transient Task
    ERR_BGTASK_INVALID_PID_OR_UID = 990000101,
    ERR_BGTASK_INVALID_BUNDLE_NAME,
    ERR_BGTASK_INVALID_REQUEST_ID,
    ERR_BGTASK_INVALID_CALLBACK = 990000201,
    ERR_BGTASK_CALLBACK_EXISTS,
    ERR_BGTASK_CALLBACK_NOT_EXIST,
    ERR_BGTASK_NOT_IN_PRESET_TIME,
    ERR_BGTASK_EXCEEDS_THRESHOLD,
    ERR_BGTASK_TIME_INSUFFICIENT,
    ERR_BGTASK_NOREQUEST_TASK,
    ERR_BGTASK_FOREGROUND,
    ERR_BGTASK_TRANSIENT_PARCELABLE_FAILED = 990000301,
    ERR_BGTASK_TRANSIENT_SYS_NOT_READY = 990000401,
    ERR_BGTASK_TRANSIENT_SERVICE_NOT_CONNECTED,
    // errcode for Efficiency Resource
    ERR_BGTASK_RESOURCES_EXCEEDS_MAX = 1870000101,
    ERR_BGTASK_RESOURCES_INVALID_PID_OR_UID,
    // other inner errcode
    ERR_BGTASK_METHOD_CALLED_FAILED,
    ERR_BGTASK_DATA_STORAGE_ERR,
    ERR_BGTASK_SERVICE_INNER_ERROR,
    ERR_BGTASK_INVALID_PROCESS_NAME,
    ERR_BGTASK_RESOURCES_PARCELABLE_FAILED = 1870000201,
    ERR_BGTASK_RESOURCES_SYS_NOT_READY = 1870000401,
    ERR_BGTASK_RESOURCES_SERVICE_NOT_CONNECTED,
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
    ERR_ABILITY_INFO_EMPTY,
    ERR_GET_TOKEN_ERR,
    ERR_BGMODE_RANGE_ERR,
    ERR_APP_NAME_EMPTY,
    ERR_RESOURCE_TYPES_INVALID,
    ERR_ISAPPLY_NULL_OR_TYPE_ERR,
    ERR_TIMEOUT_INVALID,
    ERR_ISPERSIST_NULL_OR_TYPE_ERR,
    ERR_ISPROCESS_NULL_OR_TYPE_ERR,
};

const std::map<int32_t, std::string> SA_ERRCODE_MSG_MAP = {
    {ERR_BGTASK_PERMISSION_DENIED, "Permission denied."},
    {ERR_BGTASK_NOT_SYSTEM_APP,
        "System API verification failed. Only system application can apply."},
    {ERR_BGTASK_NO_MEMORY, "Memory operation failed. Failed to allocate the memory."},
    {ERR_BGTASK_SYS_NOT_READY, "System service operation failed. The system service is not ready."},
    {ERR_BGTASK_SERVICE_NOT_CONNECTED, "System service operation failed. The system service is not connected."},
    {ERR_BGTASK_PARCELABLE_FAILED,
        "Failed to write data into parcel. Possible reasons: 1. Invalid parameters; 2. Failed to apply for memory."},
    {ERR_BGTASK_TRANSACT_FAILED, "Internal transaction failed."},
    {ERR_BGTASK_OBJECT_EXISTS,
        "Continuous Task verification failed. The application has applied for a continuous task."},
    {ERR_BGTASK_OBJECT_NOT_EXIST,
        "Continuous Task verification failed. The application has not applied for a continuous task."},
    {ERR_BGTASK_KEEPING_TASK_VERIFY_ERR,
        "Continuous Task verification failed. TASK_KEEPING background mode only supported in particular device."},
    {ERR_BGTASK_INVALID_BGMODE, "Continuous Task verification failed. The bgMode is invalid."},
    {ERR_BGTASK_INVALID_UID, "Continuous Task verification failed. The uid is invalid."},
    {ERR_BGTASK_NOTIFICATION_VERIFY_FAILED, "Notification verification failed for a continuous task."
        " The title or text of the notification cannot be empty."},
    {ERR_BGTASK_NOTIFICATION_ERR, "Notification verification failed. Failed to send or cancel the notification."},
    {ERR_BGTASK_CREATE_FILE_ERR, "Continuous task storage failed. Failed to create the storage task file."},
    {ERR_BGTASK_GET_ACTUAL_FILE_ERR, "Task storage failed. Failed to get the actual storage task file."},
    {ERR_BGTASK_OPEN_FILE_ERR, "Task storage failed. Failed to open the file."},
    {ERR_BGTASK_INVALID_PID_OR_UID,
        "Caller information verification failed for a transient task. Invalid pid or uid."},
    {ERR_BGTASK_INVALID_BUNDLE_NAME,
        "Caller information verification failed for a transient task. The bundleName cannot be found."},
    {ERR_BGTASK_INVALID_REQUEST_ID,
        "Caller information verification failed for a transient task. Invalid requestId."},
    {ERR_BGTASK_INVALID_CALLBACK,
        "Transient task verification failed. The callback cannot be empty."},
    {ERR_BGTASK_CALLBACK_EXISTS, "Transient task verification failed. The callback already exists."},
    {ERR_BGTASK_CALLBACK_NOT_EXIST, "Transient task verification failed. The callback does not exist."},
    {ERR_BGTASK_NOT_IN_PRESET_TIME,
        "Transient task verification failed. Request is not allow after the preset time of entering background."},
    {ERR_BGTASK_EXCEEDS_THRESHOLD, "Transient task verification failed. The number of request exceeds the threshold."},
    {ERR_BGTASK_TIME_INSUFFICIENT,
        "Transient task verification failed. The remaining time to run transient task is insufficient."},
    {ERR_BGTASK_RESOURCES_EXCEEDS_MAX, "Caller information verification failed for an energy"
        " resource request. The number of resources applied exceeds maximun."},
    {ERR_BGTASK_RESOURCES_INVALID_PID_OR_UID,
        "Caller information verification failed for an energy resource. Invalid pid or uid."},
    {ERR_BGTASK_RESOURCES_PARCELABLE_FAILED,
        "Failed to write data into parcel. Possible reasons: 1. Invalid parameters; 2. Failed to apply for memory."},
    {ERR_BGTASK_RESOURCES_SYS_NOT_READY, "System service operation failed. The system service is not ready."},
    {ERR_BGTASK_RESOURCES_SERVICE_NOT_CONNECTED,
        "System service operation failed. The system service is not connected."},
    {ERR_BGTASK_SERVICE_INNER_ERROR, "Service inner error."},
    {ERR_BGTASK_NOREQUEST_TASK, "Transient task verification failed. application no request transient task."},
    {ERR_BGTASK_FOREGROUND, "Transient task verification failed. application is foreground."},
    {ERR_BGTASK_TRANSIENT_PARCELABLE_FAILED,
        "Failed to write data into parcel. Possible reasons: 1. Invalid parameters; 2. Failed to apply for memory."},
    {ERR_BGTASK_TRANSIENT_SYS_NOT_READY, "System service operation failed. The system service is not ready."},
    {ERR_BGTASK_TRANSIENT_SERVICE_NOT_CONNECTED,
        "System service operation failed. The system service is not connected."},
    {ERR_BGTASK_INVALID_PROCESS_NAME, "Transient task verification failed. caller process name invaild."},
    {ERR_BGTASK_CONTINUOUS_REQUEST_NULL_OR_TYPE,
        "Continuous Task verification failed. "
        "The continuousRequestInfo cannot be null and its type must be continuousRequestInfo object."},
    {ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_IS_EMPTY,
        "Continuous Task verification failed. The backgroundTaskModes or backgroundTaskSubmodes cannot be empty."},
    {ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_LENGTH_MISMATCH,
        "Continuous Task verification failed. "
        "The length of the backgroundTaskModes does not match the length of the backgroundTaskSubmodes."},
    {ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_CROSS_BORDER,
        "Continuous Task verification failed. The backgroundTaskModes or backgroundTaskSubmodes are out of scope."},
    {ERR_BGTASK_CONTINUOUS_MODE_OR_SUBMODE_TYPE_MISMATCH,
        "Continuous Task verification failed. "
        "The sequence of backgroundTaskModes does not match the backgroundTaskSubmodes."},
    {ERR_BGTASK_CONTINUOUS_NOT_UPDATE_BY_OLD_INTERFACE,
        "Continuous Task verification failed. "
        "This task is requested through a new interface, and update operations are not permitted via this interface."},
    {ERR_BGTASK_CONTINUOUS_TASKID_INVALID,
        "Continuous Task verification failed. The continuous task Id is invalid."},
    {ERR_BGTASK_CONTINUOUS_DATA_TRANSFER_NOT_MERGE_NOTIFICATION,
        "Continuous Task verification failed. "
        "This background task mode: DATA_TRANSFER type do not support merged notification."},
    {ERR_BGTASK_CONTINUOUS_NOT_MERGE_NOTIFICATION_NOT_EXIST,
        "Continuous Task verification failed. This continuous task notification does not exist and cannot be merged."},
    {ERR_BGTASK_CONTINUOUS_NOT_MERGE_COMBINED_FALSE,
        "Continuous Task verification failed. Target continuous task do not support merged."},
    {ERR_BGTASK_CONTINUOUS_NOT_UPDATE_BECAUSE_MERGE,
        "Continuous Task verification failed. The continuous task not update because of merged."},
    {ERR_BGTASK_CONTINUOUS_NOT_MERGE_CURRENTTASK_COMBINED_FALSE,
        "Continuous Task verification failed. Current continuous task do not support merged."},
    {ERR_BGTASK_CONTINUOUS_UPDATE_FAIL_SAME_MODE_AND_MERGED,
        "Continuous Task verification failed. Current continuous task support merged but the modes are inconsistent."},
    {ERR_BGTASK_CONTINUOUS_UPDATE_NOTIFICATION_FAIL,
        "Notification verification failed. Current continuous task refresh notification fail."},
    {ERR_BGTASK_CONTINUOUS_SYSTEM_APP_NOT_SUPPORT_ACL,
        "Continuous Task verification failed. System app not support taskkeeping ACL permission."},
    {ERR_BGTASK_CONTINUOUS_APP_NOT_HAVE_BGMODE_PERMISSION_SYSTEM,
        "Continuous Task verification failed. App has no ACL permission of Taskkeeping."},
    {ERR_BGTASK_CONTINUOUS_NOT_APPLY_MAX_TASK,
        "Continuous Task verification failed. "
        "The current ability to apply for continuous tasks has exceeded the maximum limit."},
    {ERR_BGTASK_CONTINUOUS_DATA_TRANSFER_NOT_UPDATE,
        "Continuous Task verification failed. The background task mode: DATA_TRANSFER type do not support update."},
    {ERR_BGTASK_CONTINUOUS_NOT_APPLY_ONBACKGROUND,
        "Continuous Task verification failed. The continuous task are not allowed to be applied on background."},
    {ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_ONLY_ALLOW_ONE_APPLICATION,
        "Continuous Task verification failed. Special scenario processing only allowed one application."},
    {ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_CONFLICTS_WITH_OTHER_TASK,
        "Continuous Task verification failed. Special scenario processing cannot coexist with other continuous task."},
    {ERR_BGTASK_GET_APP_INDEX_FAIL, "Continuous Task verification failed. Unable to obtain the app index."},
    {ERR_BGTASK_APP_DETECTED_MALICIOUS_BEHAVIOR, "Continuous Task verification failed. "
        "The current app has detected malicious behavior and is prohibited from applying."},
    {ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_EMPTY, "Continuous Task verification failed. "
        "The background task mode: MODE_SPECIAL_SCENARIO_PROCESSINGSFER type is empty."},
    {ERR_BGTASK_CONTINUOUS_CALLBACK_NULL_OR_TYPE_ERR, "Continuous Task verification failed. "
        "The callback cannot be null and its type must be function."},
    {ERR_BGTASK_CONTINUOUS_CALLBACK_EXISTS, "Continuous Task verification failed. The callback already exists."},
    {ERR_BGTASK_CONTINUOUS_SPECIAL_SCENARIO_PROCESSING_NOT_MERGE_NOTIFICATION, "Continuous Task verification failed. "
        "This background task mode: MODE_SPECIAL_SCENARIO_PROCESSINGSFER type do not support merged notification."},
    {ERR_BGTASK_MALICIOUS_CONTINUOUSTASK, "Malicious long-running tasks are not allowed to continue applying."},
    {ERR_BGTASK_SPECIAL_SCENARIO_PROCESSING_NOTSUPPORT_DEVICE, "Continuous Task verification failed. "
        "The background task mode: MODE_SPECIAL_SCENARIO_PROCESSINGSFER type do not support this device."},
};

const std::map<int32_t, std::string> PARAM_ERRCODE_MSG_MAP = {
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
    {ERR_APP_NAME_EMPTY, "The app name of abilityInfo in context cannot be null."},
    {ERR_RESOURCE_TYPES_INVALID, "The resourcesType cannot be null and must be integer greater than 0."},
    {ERR_ISAPPLY_NULL_OR_TYPE_ERR, "The isApply cannot be null and its type must be boolean."},
    {ERR_TIMEOUT_INVALID, "The timeOut cannot be null and must be integer greater than 0."},
    {ERR_ISPERSIST_NULL_OR_TYPE_ERR, "The isPersist cannot be null and must be boolean."},
    {ERR_ISPROCESS_NULL_OR_TYPE_ERR, "The isProcess cannot be null and must be boolean."},
    {ERR_BGTASK_INVALID_PARAM, "The input param is invalid."}
};

}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_FRAMEWORKS_COMMON_INCLUDE_BGTASKMGR_INNER_ERRORS_H