/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "bgtaskmgr_inner_errors.h"
#include "common.h"

namespace OHOS {
namespace BackgroundTaskMgr {
const std::map<int32_t, std::string> SA_ERRCODE_MSG_MAP = {
    {ERR_BGTASK_PERMISSION_DENIED, "Permission denied."},
    {ERR_BGTASK_NOT_SYSTEM_APP,
        "System API verification failed. Only system application can apply."},
    {ERR_BGTASK_NO_MEMORY, "Memory operation failed. Failed to allocate the memory."},
    {ERR_BGTASK_SYS_NOT_READY, "System service operation failed. The system service is not ready."},
    {ERR_BGTASK_SERVICE_NOT_CONNECTED, "System service operation failed. The system service is not connected."},
    {ERR_BGTASK_PARCELABLE_FAILED, "Parcel operation failed."},
    {ERR_BGTASK_TRANSACT_FAILED, "Internal transaction failed."},
    {ERR_BGTASK_OBJECT_EXISTS,
        "Continuous Task verification failed. The application has applied for a continuous task."},
    {ERR_BGTASK_OBJECT_NOT_EXIST,
        "Continuous Task verification failed. The application has not applied for a continuous task."},
    {ERR_BGTASK_KEEPING_TASK_VERIFY_ERR,
        "Continuous Task verification failed. TASK_KEEPING background mode only supported in particular device."},
    {ERR_BGTASK_INVALID_BGMODE, "Continuous Task verification failed. The bgMode is invalid."},
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
    {ERR_BGTASK_SERVICE_INNER_ERROR, "Service inner error."},
    {ERR_BGTASK_NOREQUEST_TASK, "Transient task verification failed. application no request transient task."},
    {ERR_BGTASK_FOREGROUND, "Transient task verification failed. application is foreground."},
    {ERR_BGTASK_INVALID_PROCESS_NAME, "Transient task verification failed. caller process name invaild."},
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

std::string Common::FindErrMsg(const int32_t errCode)
{
    if (errCode == ERR_OK) {
        return "";
    }
    auto iter = SA_ERRCODE_MSG_MAP.find(errCode);
    if (iter != SA_ERRCODE_MSG_MAP.end()) {
        std::string errMessage = "BussinessError ";
        int32_t errCodeInfo = FindErrCode(errCode);
        errMessage.append(std::to_string(errCodeInfo)).append(": ").append(iter->second);
        return errMessage;
    }
    iter = PARAM_ERRCODE_MSG_MAP.find(errCode);
    if (iter != PARAM_ERRCODE_MSG_MAP.end()) {
        std::string errMessage = "BussinessError 401: Parameter error. ";
        errMessage.append(iter->second);
        return errMessage;
    }
    return "Inner error.";
}

int32_t Common::FindErrCode(const int32_t errCodeIn)
{
    auto iter = PARAM_ERRCODE_MSG_MAP.find(errCodeIn);
    if (iter != PARAM_ERRCODE_MSG_MAP.end()) {
        return ERR_BGTASK_INVALID_PARAM;
    }
    return errCodeIn > THRESHOLD ? errCodeIn / OFFSET : errCodeIn;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS