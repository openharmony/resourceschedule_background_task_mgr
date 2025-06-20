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
#include "data_storage_helper.h"

#include <fcntl.h>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/statfs.h>

#include "common_utils.h"
#include "continuous_task_log.h"
#include "config_policy_utils.h"
#include "directory_ex.h"
#include "hisysevent.h"
#include <file_ex.h>

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr char TASK_RECORD_FILE_PATH[] = "/data/service/el1/public/background_task_mgr/running_task";
static const std::string RESOURCE_RECORD_FILE_PATH = "/data/service/el1/public/background_task_mgr/resource_record";
static const std::string APP_RESOURCE_RECORD = "appResourceRecord";
static const std::string PROCESS_RESOURCE_RECORD = "processResourceRecord";
const std::string PARAM = "param";
const std::string FAST_FROZEN = "fast_frozen";
const std::string ENABLE = "enable";
const std::string DOZE_TIME = "doze_time";
}

DataStorageHelper::DataStorageHelper() {}

DataStorageHelper::~DataStorageHelper() {}

ErrCode DataStorageHelper::RefreshTaskRecord(const std::unordered_map<std::string,
    std::shared_ptr<ContinuousTaskRecord>> &allRecord)
{
    nlohmann::json root;
    for (const auto &iter : allRecord) {
        auto record = iter.second;
        std::string data = record->ParseToJsonStr();
        nlohmann::json recordJson = nlohmann::json::parse(data, nullptr, false);;
        if (!recordJson.is_discarded()) {
            root[iter.first] = recordJson;
        }
    }
    if (access(TASK_RECORD_FILE_PATH, F_OK) == ERR_OK) {
        BGTASK_LOGD("the file: %{private}s already exists.", TASK_RECORD_FILE_PATH);
    }
    FILE *file = fopen(TASK_RECORD_FILE_PATH, "w+");
    if (file == nullptr) {
        BGTASK_LOGE("Fail to open file: %{private}s, errno: %{public}s", TASK_RECORD_FILE_PATH, strerror(errno));
        return ERR_BGTASK_CREATE_FILE_ERR;
    }
    int closeResult = fclose(file);
    if (closeResult < 0) {
        BGTASK_LOGE("Fail to close file: %{private}s, errno: %{public}s", TASK_RECORD_FILE_PATH, strerror(errno));
        return ERR_BGTASK_CREATE_FILE_ERR;;
    }
    return SaveJsonValueToFile(root.dump(CommonUtils::jsonFormat_), TASK_RECORD_FILE_PATH);
}

ErrCode DataStorageHelper::RestoreTaskRecord(std::unordered_map<std::string,
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

ErrCode DataStorageHelper::RefreshResourceRecord(const ResourceRecordMap &appRecord,
    const ResourceRecordMap &processRecord)
{
    std::string record {""};
    ConvertMapToString(appRecord, processRecord, record);
    if (access(RESOURCE_RECORD_FILE_PATH.c_str(), F_OK) == ERR_OK) {
        BGTASK_LOGD("the file: %{private}s already exists.", RESOURCE_RECORD_FILE_PATH.c_str());
    }
    FILE *file = fopen(RESOURCE_RECORD_FILE_PATH.c_str(), "w+");
    if (file == nullptr) {
        BGTASK_LOGE("Fail to open file: %{private}s, errno: %{public}s",
            RESOURCE_RECORD_FILE_PATH.c_str(), strerror(errno));
        return ERR_BGTASK_CREATE_FILE_ERR;
    }
    int closeResult = fclose(file);
    if (closeResult < 0) {
        BGTASK_LOGE("Fail to close file: %{private}s, errno: %{public}s",
            RESOURCE_RECORD_FILE_PATH.c_str(), strerror(errno));
        return ERR_BGTASK_CREATE_FILE_ERR;;
    }
    return SaveJsonValueToFile(record, RESOURCE_RECORD_FILE_PATH);
}

ErrCode DataStorageHelper::RestoreResourceRecord(ResourceRecordMap &appRecord,
    ResourceRecordMap &processRecord)
{
    nlohmann::json root;
    if (ParseJsonValueFromFile(root, RESOURCE_RECORD_FILE_PATH) != ERR_OK) {
        BGTASK_LOGD("can not read string form file: %{private}s", RESOURCE_RECORD_FILE_PATH.c_str());
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    DivideJsonToMap(root, appRecord, processRecord);
    return ERR_OK;
}

int32_t DataStorageHelper::SaveJsonValueToFile(const std::string &value, const std::string &filePath)
{
    std::ofstream fout;
    std::string realPath;
    if (!ConvertFullPath(filePath, realPath)) {
        BGTASK_LOGE("SaveJsonValueToFile Get real file path: %{private}s failed", filePath.c_str());
        return ERR_BGTASK_GET_ACTUAL_FILE_ERR;
    }
    fout.open(realPath, std::ios::out);
    if (!fout.is_open()) {
        BGTASK_LOGE("Open file: %{private}s failed.", filePath.c_str());
        return ERR_BGTASK_OPEN_FILE_ERR;
    }
    fout << value.c_str() << std::endl;
    fout.close();

    // 数据持久化完成，进行打点
    ReportUserDataSizeEvent();
    return ERR_OK;
}

bool DataStorageHelper::ParseFastSuspendDozeTime(const std::string &FilePath, int &time)
{
    nlohmann::json jsonObj;
    std::string absolutePath = GetConfigFileAbsolutePath(FilePath);
    if (ParseJsonValueFromFile(jsonObj, absolutePath) != ERR_OK) {
        return false;
    }
    if (jsonObj.is_null() || !jsonObj.is_object() || !CommonUtils::CheckJsonValue(jsonObj, {PARAM})) {
        BGTASK_LOGW("check jsonObj value failed.");
        return false;
    }
    nlohmann::json param = jsonObj[PARAM];
    if (param.is_null() || !param.is_object() || !CommonUtils::CheckJsonValue(param, {FAST_FROZEN})) {
        BGTASK_LOGW("check param value failed.");
        return false;
    }
    nlohmann::json fastFrozen = param[FAST_FROZEN];
    if (fastFrozen.is_null() || !fastFrozen.is_object() ||
        !CommonUtils::CheckJsonValue(fastFrozen, {ENABLE, DOZE_TIME})) {
        BGTASK_LOGW("check fastFrozen value failed.");
        return false;
    }
    if (!fastFrozen[ENABLE].is_boolean() || !fastFrozen.at(ENABLE).get<bool>()) {
        BGTASK_LOGW("fast suspend switch is Disable");
        return false;
    }
    if (!fastFrozen[DOZE_TIME].is_number_integer()) {
        BGTASK_LOGW("fast suspend num is invalid");
        return false;
    }
    time = fastFrozen.at(DOZE_TIME).get<int>();
    return true;
}

int32_t DataStorageHelper::ParseJsonValueFromFile(nlohmann::json &value, const std::string &filePath)
{
    std::string realPath;
    if (!ConvertFullPath(filePath, realPath)) {
        BGTASK_LOGE("Get real path failed");
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    std::string data;
    LoadStringFromFile(realPath.c_str(), data);
    value = nlohmann::json::parse(data, nullptr, false);
    if (value.is_discarded()) {
        BGTASK_LOGE("failed due to data is discarded");
        return ERR_BGTASK_DATA_STORAGE_ERR;
    }
    return ERR_OK;
}

std::string DataStorageHelper::GetConfigFileAbsolutePath(const std::string &relativePath)
{
    if (relativePath.empty()) {
        BGTASK_LOGE("relativePath is empty");
        return "";
    }
    char buf[PATH_MAX];
    char *tmpPath = GetOneCfgFile(relativePath.c_str(), buf, PATH_MAX);
    char absolutePath[PATH_MAX] = {0};
    if (!tmpPath || strlen(tmpPath) > PATH_MAX || !realpath(tmpPath, absolutePath)) {
        BGTASK_LOGE("get file fail.");
        return "";
    }
    return std::string(absolutePath);
}

bool DataStorageHelper::ConvertFullPath(const std::string& partialPath, std::string& fullPath)
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

void DataStorageHelper::ConvertMapToString(const ResourceRecordMap &appRecord,
    const ResourceRecordMap &processRecord, std::string &recordString)
{
    nlohmann::json root;
    nlohmann::json appValue;
    ConvertMapToJson(appRecord, appValue);
    root[APP_RESOURCE_RECORD] = appValue;
    nlohmann::json processValue;
    ConvertMapToJson(processRecord, processValue);
    root[PROCESS_RESOURCE_RECORD] = processValue;
    recordString = root.dump(CommonUtils::jsonFormat_);
}

void DataStorageHelper::ConvertMapToJson(const ResourceRecordMap &appRecord, nlohmann::json &root)
{
    for (const auto &iter : appRecord) {
        nlohmann::json value;
        iter.second->ParseToJson(value);
        root[std::to_string(iter.first)] = value;
    }
}

void DataStorageHelper::DivideJsonToMap(nlohmann::json &root,
    ResourceRecordMap &appRecord, ResourceRecordMap &processRecord)
{
    if (root.contains(APP_RESOURCE_RECORD) && root.at(APP_RESOURCE_RECORD).is_object()) {
        nlohmann::json appRecordJson = root.at(APP_RESOURCE_RECORD);
        ConvertJsonToMap(appRecordJson, appRecord);
    } else {
        BGTASK_LOGE("can not read appRecord, json init fail");
    }
    if (root.contains(PROCESS_RESOURCE_RECORD) && root.at(PROCESS_RESOURCE_RECORD).is_object()) {
        nlohmann::json processrecordJson = root.at(PROCESS_RESOURCE_RECORD);
        ConvertJsonToMap(processrecordJson, processRecord);
    } else {
        BGTASK_LOGE("can not read processRecord, json init fail");
    }
}

void DataStorageHelper::ConvertJsonToMap(const nlohmann::json &value, ResourceRecordMap &recordMap)
{
    for (auto iter = value.begin(); iter != value.end(); iter++) {
        std::shared_ptr<ResourceApplicationRecord> recordPtr = std::make_shared<ResourceApplicationRecord>();
        recordPtr->ParseFromJson(iter.value());
        recordMap.emplace(static_cast<uint32_t>(std::atoi(iter.key().c_str())), recordPtr);
    }
}

uint64_t GetRemainPartitionSize(const std::string& partitionName)
{
    struct statfs stat;
    if (statfs(partitionName.c_str(), &stat) != 0) {
        return -1;
    }
    uint64_t blockSize = stat.f_bsize;
    uint64_t freeSize = stat.f_bfree * blockSize;
    constexpr double units = 1024.0;
    return freeSize / (units * units);
}

std::vector<uint64_t> GetFileOrFolderSize(const std::vector<std::string>& paths)
{
    std::vector<uint64_t> folderSize;
    for (auto path : paths) {
        folderSize.emplace_back(OHOS::GetFolderSize(path));
    }
    return folderSize;
}

void DataStorageHelper::ReportUserDataSizeEvent()
{
    std::vector<std::string> paths = {
        "/data/service/el1/public/background_task_mgr/"
    };
    uint64_t remainPartitionSize = GetRemainPartitionSize("/data");
    std::vector<uint64_t> folderSize = GetFileOrFolderSize(paths);
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::FILEMANAGEMENT, "USER_DATA_SIZE",
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "COMPONENT_NAME", "background_task_mgr",
        "PARTITION_NAME", "/data",
        "REMAIN_PARTITION_SIZE", remainPartitionSize,
        "FILE_OR_FOLDER_PATH", paths,
        "FILE_OR_FOLDER_SIZE", folderSize);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS

