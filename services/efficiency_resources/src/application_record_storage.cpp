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

#include "application_record_storage.h"

#include <fcntl.h>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>

#include "errors.h"
#include "json/json.h"

#include "bgtaskmgr_inner_errors.h"
#include "bundle_manager_helper.h"
#include "efficiency_resource_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr char RESOURCE_APPLICATION_FILE_PATH[] = "/data/service/el1/public/background_task_mgr/resource_application";
static constexpr int32_t MAX_BUFFER = 512;
}

int32_t ApplicationRecordStorage::RefreshApplicationRecord(const std::unordered_map<std::string,
    std::shared_ptr<ResourceApplicationRecord>> &allRecord)
{
    Json::Value root;
    for (auto &iter : allRecord) {
        auto record = iter.second;
        std::string data = record->ParseToJsonStr();
        JSONCPP_STRING errs;
        Json::Value recordJson;
        Json::CharReaderBuilder readerBuilder;
        const std::unique_ptr<Json::CharReader> jsonReader(readerBuilder.newCharReader());
        bool res = jsonReader->parse(data.c_str(), data.c_str() + data.length(), &recordJson, &errs);
        if (res && errs.empty()) {
            root[iter.first] = recordJson;
        }
    }
    Json::StreamWriterBuilder writerBuilder;
    std::ostringstream os;
    std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
    jsonWriter->write(root, &os);
    std::string result = os.str();
    if (!CreateNodeFile(RESOURCE_APPLICATION_FILE_PATH)) {
        BGTASK_LOGE("Create file failed.");
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    std::ofstream fout;
    std::string realPath;
    if (!ConvertFullPath(RESOURCE_APPLICATION_FILE_PATH, realPath)) {
        BGTASK_LOGE("Get real path failed");
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    fout.open(realPath, std::ios::out);
    if (!fout.is_open()) {
        BGTASK_LOGE("Open file failed.");
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    fout << result.c_str() << std::endl;
    fout.close();
    return ERR_OK;
}

int32_t ApplicationRecordStorage::RestoreApplicationRecord(std::unordered_map<std::string,
    std::shared_ptr<ResourceApplicationRecord>> &allRecord)
{
    std::ifstream fin;
    std::string realPath;
    if (!ConvertFullPath(RESOURCE_APPLICATION_FILE_PATH, realPath)) {
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
    JSONCPP_STRING errs;
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    const std::unique_ptr<Json::CharReader> jsonReader(readerBuilder.newCharReader());
    bool res = jsonReader->parse(data.c_str(), data.c_str() + data.length(), &root, &errs);
    fin.close();
    if (!res || !errs.empty()) {
        BGTASK_LOGE("parse json file failed!");
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    for (auto it : root.getMemberNames()) {
        Json::Value recordJson = root[it];
        std::shared_ptr<ResourceApplicationRecord> record = std::make_shared<ResourceApplicationRecord>();
        if (record->ParseFromJson(recordJson)) {
            allRecord.emplace(it, record);
        }
    }
    return ERR_OK;
}

bool ApplicationRecordStorage::CreateNodeFile(const std::string &filePath)
{
    if (access(filePath.c_str(), F_OK) == ERR_OK) {
        BGTASK_LOGD("the file: %{public}s already exists.", RESOURCE_APPLICATION_FILE_PATH);
        return true;
    }
    int32_t fd = open(filePath.c_str(), O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < ERR_OK) {
        BGTASK_LOGE("Fail to open file: %{public}s", RESOURCE_APPLICATION_FILE_PATH);
        return false;
    }
    close(fd);
    return true;
}

bool ApplicationRecordStorage::ConvertFullPath(const std::string& partialPath, std::string& fullPath)
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
