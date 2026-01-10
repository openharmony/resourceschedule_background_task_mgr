/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include <unordered_map>

namespace OHOS {
namespace BackgroundTaskMgr {
std::string Common::FindErrMsg(const int32_t errCode)
{
    if (errCode == ERR_OK) {
        return "";
    }
    std::string errMsg = BusinessErrorMap::GetSaErrMap(errCode);
    if (errMsg != "") {
        std::string errMessage = "BusinessError ";
        int32_t errCodeInfo = FindErrCode(errCode);
        errMessage.append(std::to_string(errCodeInfo)).append(": ").append(errMsg);
        return errMessage;
    }
    errMsg = BusinessErrorMap::GetParamErrMap(errCode);
    if (errMsg != "") {
        std::string errMessage = "BusinessError 401: Parameter error. ";
        errMessage.append(errMsg);
        return errMessage;
    }
    return "Inner error.";
}

int32_t Common::FindErrCode(const int32_t errCodeIn)
{
    if (BusinessErrorMap::GetParamErrMap(errCodeIn) != "") {
        return ERR_BGTASK_INVALID_PARAM;
    }
    return errCodeIn > THRESHOLD ? errCodeIn / OFFSET : errCodeIn;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS