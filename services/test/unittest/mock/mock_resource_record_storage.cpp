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

#include "resource_record_storage.h"

#include "errors.h"
#include "bundle_manager_helper.h"
#include "common_utils.h"

#include "bgtaskmgr_inner_errors.h"
#include "efficiency_resource_log.h"
#include "resource_application_record.h"

namespace OHOS {
namespace BackgroundTaskMgr {
ErrCode ResourceRecordStorage::RefreshResourceRecord(const ResourceRecordMap &appRecord,
    const ResourceRecordMap &processRecord)
{
    return ERR_OK;
}

ErrCode ResourceRecordStorage::RestoreResourceRecord(ResourceRecordMap &appRecord,
    ResourceRecordMap &processRecord)
{
    return ERR_OK;
}
}
}
