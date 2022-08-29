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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_RES_RECORD_STORAGE_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_RES_RECORD_STORAGE_H

#include <unordered_map>
#include "nlohmann/json.hpp"
#include "bgtaskmgr_inner_errors.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class ResourceApplicationRecord;

class ResourceRecordStorage {
using ResourceRecordMap = std::unordered_map<int32_t, std::shared_ptr<ResourceApplicationRecord>>;
public:
    ErrCode RefreshResourceRecord(const ResourceRecordMap &appRecord, const ResourceRecordMap &processRecord);
    ErrCode RestoreResourceRecord(ResourceRecordMap &appRecord, ResourceRecordMap &processRecord);

private:
    void ConvertMapToString(const ResourceRecordMap &appRecord,
        const ResourceRecordMap &processRecord, std::string &recordString);
    void ConvertMapToJson(const ResourceRecordMap &appRecord, nlohmann::json &root);
    ErrCode ConvertStringToMap(const std::string &recordString,
        ResourceRecordMap &appRecord, ResourceRecordMap &processRecord);
    void ConvertJsonToMap(const nlohmann::json &value, ResourceRecordMap &recordMap);
    ErrCode ConvertStringToJson(const std::string &recordString,
        nlohmann::json &appRecord, nlohmann::json &processRecord);
    bool CreateNodeFile(const std::string &filePath);
    bool ConvertFullPath(const std::string &partialPath, std::string &fullPath);
    ErrCode WriteStringToFile(const std::string &result, const std::string &filePath);
    ErrCode ReadStringFromFile(std::string &result, const std::string &filePath);
};
}
}
#endif //FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_EFFICIENCY_RESOURCES_INCLUDE_RES_RECORD_STORAGE_H