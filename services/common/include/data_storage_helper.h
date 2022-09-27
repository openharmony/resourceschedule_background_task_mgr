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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_DATA_STORAGE_HELPER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_DATA_STORAGE_HELPER_H

#include "singleton.h"
#include "nlohmann/json.hpp"

#include "bg_efficiency_resources_mgr.h"
#include "bg_continuous_task_mgr.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class DataStorageHelper : public DelayedSingleton<BgContinuousTaskMgr> {
using ResourceRecordMap = std::unordered_map<int32_t, std::shared_ptr<ResourceApplicationRecord>>;
public:
    ErrCode RefreshTaskRecord(const std::unordered_map<std::string, std::shared_ptr<ContinuousTaskRecord>> &allRecord);
    ErrCode RestoreTaskRecord(std::unordered_map<std::string, std::shared_ptr<ContinuousTaskRecord>> &allRecord);
    ErrCode RefreshResourceRecord(const ResourceRecordMap &appRecord, const ResourceRecordMap &processRecord);
    ErrCode RestoreResourceRecord(ResourceRecordMap &appRecord, ResourceRecordMap &processRecord);

private:
    int32_t SaveJsonValueToFile(const std::string &value, const std::string &filePath);
    int32_t ParseJsonValueFromFile(nlohmann::json &value, const std::string &filePath);
    bool CreateNodeFile(const std::string &filePath);
    bool ConvertFullPath(const std::string &partialPath, std::string &fullPath);
    void ConvertMapToString(const ResourceRecordMap &appRecord,
        const ResourceRecordMap &processRecord, std::string &recordString);
    void ConvertMapToJson(const ResourceRecordMap &appRecord, nlohmann::json &root);
    void DivideJsonToMap(nlohmann::json &root,
        ResourceRecordMap &appRecord, ResourceRecordMap &processRecord);
    void ConvertJsonToMap(const nlohmann::json &value, ResourceRecordMap &recordMap);
    ErrCode ConvertStringToJson(const std::string &recordString,
        nlohmann::json &appRecord, nlohmann::json &processRecord);
    DECLARE_DELAYED_SINGLETON(DataStorageHelper);
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_DATA_STORAGE_HELPER_H