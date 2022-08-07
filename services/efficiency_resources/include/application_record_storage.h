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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_APPLICATION_RECORD_STORAGE_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_APPLICATION_RECORD_STORAGE_H

#include "resources_application_record.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class ApplicationRecordStorage {
public:
    int32_t RefreshApplicationRecord(const std::unordered_map<std::string, std::shared_ptr<ResourceApplicationRecord>> &allRecord);
    int32_t RestoreApplicationRecord(std::unordered_map<std::string, std::shared_ptr<ResourceApplicationRecord>> &allRecord);

private:
    bool CreateNodeFile(const std::string &filePath);
    bool ConvertFullPath(const std::string &partialPath, std::string &fullPath);
};
}
}
#endif  //FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_APPLICATION_RECORD_STORAGE_H