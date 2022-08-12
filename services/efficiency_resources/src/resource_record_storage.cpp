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

#include <fcntl.h>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>

#include "errors.h"
#include "json/json.h"

#include "bgtaskmgr_inner_errors.h"
#include "bundle_manager_helper.h"
#include "efficiency_resource_log.h"
#include "limits.h"
#include "resource_application_record.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr char RESOURCE_RECORD_FILE_PATH[] = "/data/service/el1/public/background_task_mgr/resource_record";
static constexpr char APP_RESOURCE_RECORD[] = "appResourceRecord";
static constexpr char PROCESS_RESOURCE_RECORD[] = "processResourceRecord";
static constexpr int32_t MAX_BUFFER = 512;
}

ErrCode ResourceRecordStorage::RefreshResourceRecord(const ResourceRecordMap &appRecord,
    const ResourceRecordMap &processRecord)
{
    std::string record {""};
    ConvertMapToString(appRecord, processRecord, record);
    return WriteStringToFile(record, RESOURCE_RECORD_FILE_PATH);
}

ErrCode ResourceRecordStorage::RestoreResourceRecord(ResourceRecordMap &appRecord,
    ResourceRecordMap &processRecord)
{
    std::string recordString {""};
    if (ReadStringFromFile(recordString, RESOURCE_RECORD_FILE_PATH) != ERR_OK) {
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    Json::Value root;
    return ConvertStringToMap(recordString, appRecord, processRecord);
}

void ResourceRecordStorage::ConvertMapToString(const ResourceRecordMap &appRecord, 
    const ResourceRecordMap &processRecord, std::string &recordString)
{
    Json::Value root;
    Json::Value appValue;
    ConvertMapToJson(appRecord, appValue);
    root[APP_RESOURCE_RECORD] = appValue;
    Json::Value processValue;
    ConvertMapToJson(processRecord, processValue);
    root[PROCESS_RESOURCE_RECORD] = processValue;
    recordString = root.toStyledString();
}

void ResourceRecordStorage::ConvertMapToJson(const ResourceRecordMap &appRecord, Json::Value &root)
{
    for (const auto &iter : appRecord) {
        Json::Value value;
        iter.second->ParseToJson(value);
        root[std::to_string(iter.first)] = value;
    }
}

ErrCode ResourceRecordStorage::ConvertStringToMap(const std::string &recordString, 
    ResourceRecordMap &appRecord, ResourceRecordMap &processRecord)
{
    Json::Value appRecordJson;
    Json::Value processrecordJson;
    if (ConvertStringToJson(recordString, appRecordJson, processrecordJson) != ERR_OK) {
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    ConvertJsonToMap(appRecordJson, appRecord);
    ConvertJsonToMap(processrecordJson, processRecord);
    return ERR_OK;
}

void ResourceRecordStorage::ConvertJsonToMap(const Json::Value &value, ResourceRecordMap &recordMap)
{
    for (auto &it : value.getMemberNames()) {
        std::shared_ptr<ResourceApplicationRecord> recordPtr = std::make_shared<ResourceApplicationRecord>();
        recordPtr->ParseFromJson(value[it]);
        recordMap.emplace(std::stoi(it), recordPtr);
    }
}

ErrCode ResourceRecordStorage::ConvertStringToJson(const std::string &recordString, 
    Json::Value &appRecord, Json::Value &processRecord)
{
    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value root;
    std::string errs;
    bool res = reader->parse(recordString.c_str(), recordString.c_str()+recordString.size(), &root, 
        &errs);
    if (!res || !errs.empty()) {
        BGTASK_LOGE("parse json file failed!");
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }    
    appRecord = root[APP_RESOURCE_RECORD];
    processRecord = root[PROCESS_RESOURCE_RECORD];
    return ERR_OK;
}

bool ResourceRecordStorage::CreateNodeFile(const std::string &filePath)
{
    if (access(filePath.c_str(), F_OK) == ERR_OK) {
        BGTASK_LOGD("the file: %{public}s already exists.", RESOURCE_RECORD_FILE_PATH);
        return true;
    }
    int32_t fd = open(filePath.c_str(), O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < ERR_OK) {
        BGTASK_LOGE("Fail to open file: %{public}s", RESOURCE_RECORD_FILE_PATH);
        return false;
    }
    close(fd);
    return true;
}

bool ResourceRecordStorage::ConvertFullPath(const std::string& partialPath, std::string& fullPath)
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

ErrCode ResourceRecordStorage::WriteStringToFile(const std::string &result, const std::string &filePath)
{
    if (!CreateNodeFile(filePath)) {
        BGTASK_LOGE("Create file failed.");
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    std::ofstream fout;
    std::string realPath;
    if (!ConvertFullPath(filePath, realPath)) {
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

ErrCode ResourceRecordStorage::ReadStringFromFile(std::string &result, const std::string &filePath)
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
    result = os.str();
    fin.close();
    return ERR_OK;
}
}
}
