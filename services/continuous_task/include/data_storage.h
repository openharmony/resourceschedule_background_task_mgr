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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_DATA_STORAGE_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_DATA_STORAGE_H

#include "continuous_task_record.h"
#include "json/json.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class DataStorage {
public:
    int32_t RefreshTaskRecord(const std::unordered_map<std::string, std::shared_ptr<ContinuousTaskRecord>> &allRecord);
    int32_t RestoreTaskRecord(std::unordered_map<std::string, std::shared_ptr<ContinuousTaskRecord>> &allRecord);

    int32_t RefreshTaskDetectionInfo(const std::string &detectionInfos);
    int32_t RestoreTaskDetectionInfo(Json::Value &value);

private:
    int32_t SaveJsonValueToFile(const Json::Value &value, const std::string &filePath);
    int32_t ParseJsonValueFromFile(Json::Value &value, const std::string &filePath);
    bool CreateNodeFile(const std::string &filePath);
    bool ConvertFullPath(const std::string &partialPath, std::string &fullPath);
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_DATA_STORAGE_H