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

#include "data_storage.h"

#include <fcntl.h>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <sys/stat.h>

#include "errors.h"

#include "bgtaskmgr_inner_errors.h"
#include "bundle_manager_helper.h"
#include "common_utils.h"
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr char TASK_RECORD_FILE_PATH[] = "/data/service/el1/public/background_task_mgr/running_task";
static constexpr int32_t MAX_BUFFER = 512;
}

int32_t DataStorage::RefreshTaskRecord(const std::unordered_map<std::string,
    std::shared_ptr<ContinuousTaskRecord>> &allRecord)
{
    nlohmann::json root;
    for (auto &iter : allRecord) {
        auto record = iter.second;
        std::string data = record->ParseToJsonStr();
        nlohmann::json recordJson = nlohmann::json::parse(data, nullptr, false);;
        if (!recordJson.is_null() && !recordJson.is_discarded()) {
            root[iter.first] = recordJson;
        }
    }
    return SaveJsonValueToFile(root.dump(CommonUtils::JSON_FORMAT), TASK_RECORD_FILE_PATH);
}

int32_t DataStorage::RestoreTaskRecord(std::unordered_map<std::string,
    std::shared_ptr<ContinuousTaskRecord>> &allRecord)
{
    nlohmann::json root;
    if (ParseJsonValueFromFile(root, TASK_RECORD_FILE_PATH) != ERR_OK) {
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    for (auto iter = root.begin(); iter != root.end(); iter++) {
        nlohmann::json recordJson = iter.value();
        std::shared_ptr<ContinuousTaskRecord> record = std::make_shared<ContinuousTaskRecord>();
        if (record->ParseFromJson(recordJson)) {
            allRecord.emplace(iter.key(), record);
        }
    }
    return ERR_OK;
}

int32_t DataStorage::SaveJsonValueToFile(const std::string &value, const std::string &filePath)
{
    if (!CreateNodeFile(filePath)) {
        BGTASK_LOGE("Create file failed.");
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    std::ofstream fout;
    std::string realPath;
    if (!ConvertFullPath(filePath, realPath)) {
        BGTASK_LOGE("SaveJsonValueToFile Get real file path: %{public}s failed", filePath.c_str());
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    fout.open(realPath, std::ios::out);
    if (!fout.is_open()) {
        BGTASK_LOGE("Open file: %{public}s failed.", filePath.c_str());
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    fout << value.c_str() << std::endl;
    fout.close();
    return ERR_OK;
}

int32_t DataStorage::ParseJsonValueFromFile(nlohmann::json &value, const std::string &filePath)
{
    std::ifstream fin;
    std::string realPath;
    if (!ConvertFullPath(filePath, realPath)) {
        BGTASK_LOGE("Get real path failed");
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    fin.open(realPath, std::ios::in);
    if (!fin.is_open()) {
        BGTASK_LOGE("cannot open file %{public}s", realPath.c_str());
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    char buffer[MAX_BUFFER];
    std::ostringstream os;
    while (!fin.eof()) {
        fin.getline(buffer, MAX_BUFFER);
        os << buffer;
    }
    std::string data = os.str();
    value = nlohmann::json::parse(data, nullptr, false);
    if (value.is_discarded()) {
        BGTASK_LOGE("failed due to data is discarded");
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    return ERR_OK;
}

bool DataStorage::CreateNodeFile(const std::string &filePath)
{
    if (access(filePath.c_str(), F_OK) == ERR_OK) {
        BGTASK_LOGD("the file: %{public}s already exists.", filePath.c_str());
        return true;
    }
    int32_t fd = open(filePath.c_str(), O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < ERR_OK) {
        BGTASK_LOGE("Fail to open file: %{public}s", filePath.c_str());
        return false;
    }
    close(fd);
    return true;
}

bool DataStorage::ConvertFullPath(const std::string& partialPath, std::string& fullPath)
{
    if (partialPath.empty() || partialPath.length() >= PATH_MAX) {
        return false;
    }
    char tmpPath[PATH_MAX] = {0};
    if (realpath(partialPath.c_str(), tmpPath) == nullptr) {
        return false;
    }
    fullPath = tmpPath;
    return true;
}
}
}
