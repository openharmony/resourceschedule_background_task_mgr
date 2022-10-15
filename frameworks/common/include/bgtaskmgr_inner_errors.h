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
    ERR_BGTASK_INVALID_PARAM = 401,
    // errcode for Continuous Task
    ERR_BGTASK_NO_MEMORY = 980000101,
    ERR_BGTASK_PARCELABLE_FAILED = 980000201,
    ERR_BGTASK_TRANSACT_FAILED = 980000301,
    ERR_BGTASK_SYS_NOT_READY = 980000401,
    ERR_BGTASK_SERVICE_NOT_CONNECTED,
    ERR_BGTASK_OBJECT_EXISTS = 980000501,
    ERR_BGTASK_OBJECT_NOT_EXIST,
    ERR_BGTASK_WIFI_VOIP_VERIFY_ERR,
    ERR_BGTASK_KEEPING_TASK_VERIFY_ERR,
    ERR_BGTASK_INVALID_BGMODE,
    ERR_BGTASK_NOTIFICATION_VERIFY_FAILED = 980000601,
    ERR_BGTASK_NOTIFICATION_ERR,
    ERR_BGTASK_CHECK_TASK_PARAM,
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
    // errcode for Efficiency Resource
    ERR_BGTASK_RESOURCES_EXCEEDS_MAX = 1870000101,
    ERR_BGTASK_RESOURCES_INVALID_PID_OR_UID,
    // other inner errcode
    ERR_BGTASK_METHOD_CALLED_FAILED,
    ERR_BGTASK_DATA_STORAGE_ERR,
    ERR_BGTASK_SERVICE_INNER_ERROR,
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

static std::map<int32_t, std::string> saErrCodeMsgMap = {
    {ERR_BGTASK_PERMISSION_DENIED, "Permission denied."},
    {ERR_BGTASK_NO_MEMORY, "Memory operation failed. Failed to allocate the memory."},
    {ERR_BGTASK_SYS_NOT_READY, "System service operation failed. The system service is not ready."},
    {ERR_BGTASK_SERVICE_NOT_CONNECTED, "System service operation failed. The system service is not connected."},
    {ERR_BGTASK_PARCELABLE_FAILED, "Parcel operation failed."},
    {ERR_BGTASK_TRANSACT_FAILED, "IPC communication failed."},
    {ERR_BGTASK_OBJECT_EXISTS,
        "Background Task verification failed. The application has applied for a background task."},
    {ERR_BGTASK_OBJECT_NOT_EXIST,
        "Background Task verification failed. The application has not applied for a background task."},
    {ERR_BGTASK_WIFI_VOIP_VERIFY_ERR,
        "Background Task verification failed. Only system application can apply for WIFI_INTERACTION or VoIP."},
    {ERR_BGTASK_KEEPING_TASK_VERIFY_ERR,
        "Background Task verification failed. Only PC device can apply for KEEPING_TASK."},
    {ERR_BGTASK_INVALID_BGMODE, "Background Task verification failed. The bgMode is invalid."},
    {ERR_BGTASK_NOTIFICATION_VERIFY_FAILED,
        "Notification verification failed. The title or text of the notification cannot be empty."},
    {ERR_BGTASK_NOTIFICATION_ERR, "Notification verification failed. Failed to send or cancel the notification."},
    {ERR_BGTASK_CREATE_FILE_ERR, "Task storage failed. Failed to create the storage task file."},
    {ERR_BGTASK_GET_ACTUAL_FILE_ERR, "Task storage failed. Failed to get the actual storage task file."},
    {ERR_BGTASK_OPEN_FILE_ERR, "Task storage failed. Failed to open the file."},
    {ERR_BGTASK_INVALID_PID_OR_UID, "Caller information verification failed. Invalid pid or uid."},
    {ERR_BGTASK_INVALID_BUNDLE_NAME, "Caller information verification failed. The bundleName cannot be found."},
    {ERR_BGTASK_INVALID_REQUEST_ID, "Caller information verification failed. Invalid requestId."},
    {ERR_BGTASK_INVALID_CALLBACK, "Caller information verification failed. The callback cannot be empty."},
    {ERR_BGTASK_CALLBACK_EXISTS, "SuspendDelay verification failed. The callback already exists."},
    {ERR_BGTASK_CALLBACK_NOT_EXIST, "SuspendDelay verification failed. The callback does not exist."},
    {ERR_BGTASK_NOT_IN_PRESET_TIME,
        "SuspendDelay verification failed. Request is not allow after the preset time of entering background."},
    {ERR_BGTASK_EXCEEDS_THRESHOLD, "SuspendDelay verification failed. The number of request exceeds the threshold."},
    {ERR_BGTASK_TIME_INSUFFICIENT,
        "SuspendDelay verification failed. The remaining time to run transient task is insufficient."},
    {ERR_BGTASK_RESOURCES_EXCEEDS_MAX,
        "Caller information verification failed. The number of resources applied exceeds maximun."},
    {ERR_BGTASK_SERVICE_INNER_ERROR, "Service inner error."},
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