/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "bgtask_config.h"
#include "data_storage_helper.h"
#include "bgtaskmgr_log_wrapper.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
const std::string SUSPEND_MANAGER_CONFIG_FILE = "etc/efficiency_manager/suspend_manager_config.json";
const std::string CONFIG_JSON_INDEX_TOP = "params";
const std::string CONFIG_JSON_INDEX_SUSPEND_SECOND = "param";
const std::string TRANSIENT_ERR_DELAYED_FROZEN_LIST = "transient_err_delayed_frozen_list";
const std::string CONTINUOUS_TASK_KEEPING_EXEMPTED_LIST = "continuous_task_keeping_exemption_list";
const std::string MALICIOUS_APP_BLOCKLIST = "malicious_app_blocklist";
const std::string TRANSIENT_EXEMPTED_QUOTA = "transient_exempted_quota";
const std::string TRANSIENT_ERR_DELAYED_FROZEN_TIME = "transient_err_delayed_frozen_time";

const std::string BACKGROUND_TASK_CONFIG_FILE = "etc/backgroundtask/config.json";
const std::string CPU_EFFICIENCY_RESOURCE_ALLOW_APPLY_BUNDLE_INFOS = "cpu_efficiency_resource_allow_apply_bundle_infos";
const std::string ALLOW_APPLY_BUNDLE_INFO_BUNDLE_NAME = "bundleName";
const std::string ALLOW_APPLY_BUNDLE_INFO_APP_SIGNATURES = "appSignatures";
const std::string ALLOW_APPLY_BUNDLE_INFO_CPU_LEVEL = "cpuLevel";
}

void BgtaskConfig::Init()
{
    if (isInit_) {
        BGTASK_LOGE("already init config!");
        return;
    }

    LoadConfigFile();
    LoadBgTaskConfigFile();
    isInit_ = true;
}

void BgtaskConfig::LoadConfigFile()
{
    nlohmann::json jsonObj;
    std::string absolutePath = DelayedSingleton<DataStorageHelper>::GetInstance()->
        GetConfigFileAbsolutePath(SUSPEND_MANAGER_CONFIG_FILE);
    if (DelayedSingleton<DataStorageHelper>::GetInstance()->ParseJsonValueFromFile(jsonObj, absolutePath) != 0) {
        BGTASK_LOGE("LoadConfigFile failed");
        return;
    }
    ParseTransientTaskExemptedQuatoList(jsonObj);
    ParseTransientTaskExemptedQuato(jsonObj);
}

void BgtaskConfig::ParseTransientTaskExemptedQuatoList(const nlohmann::json &jsonObj)
{
    nlohmann::json appArray;
    if (jsonObj.is_null() || jsonObj.empty()) {
        BGTASK_LOGE("jsonObj null");
        return;
    }
    if (!jsonObj.contains(TRANSIENT_ERR_DELAYED_FROZEN_LIST) ||
        !jsonObj[TRANSIENT_ERR_DELAYED_FROZEN_LIST].is_array()) {
        BGTASK_LOGE("no key %{public}s", TRANSIENT_ERR_DELAYED_FROZEN_LIST.c_str());
        return;
    }
    appArray = jsonObj[TRANSIENT_ERR_DELAYED_FROZEN_LIST];
    std::lock_guard<std::mutex> lock(configMutex_);
    for (const auto &app : appArray) {
        transientTaskExemptedQuatoList_.insert(app);
    }
    for (const auto &app : transientTaskExemptedQuatoList_) {
        BGTASK_LOGI("ParseTransientTaskExemptedQuatoList: %{public}s.", app.c_str());
    }
}

bool BgtaskConfig::AddExemptedQuatoData(const std::string &configData, int32_t sourceType)
{
    const nlohmann::json &jsonObj = nlohmann::json::parse(configData, nullptr, false);
    if (jsonObj.is_discarded()) {
        BGTASK_LOGE("jsonObj parse fail");
        return false;
    }
    if (jsonObj.is_null() || jsonObj.empty()) {
        BGTASK_LOGE("jsonObj null");
        return false;
    }
    if (sourceType == ConfigDataSourceType::CONFIG_CLOUD && !SetCloudConfigParam(jsonObj)) {
        return false;
    } else if (sourceType == ConfigDataSourceType::CONFIG_SUSPEND_MANAGER) {
        nlohmann::json appArray;
        if (!jsonObj.contains(TRANSIENT_ERR_DELAYED_FROZEN_LIST) ||
            !jsonObj[TRANSIENT_ERR_DELAYED_FROZEN_LIST].is_array()) {
            BGTASK_LOGE("no key %{public}s", TRANSIENT_ERR_DELAYED_FROZEN_LIST.c_str());
            return false;
        }
        appArray = jsonObj[TRANSIENT_ERR_DELAYED_FROZEN_LIST];
        std::lock_guard<std::mutex> lock(configMutex_);
        transientTaskExemptedQuatoList_.clear();
        for (const auto &app : appArray) {
            transientTaskExemptedQuatoList_.insert(app);
        }
        for (const auto &appName : transientTaskExemptedQuatoList_) {
            BGTASK_LOGI("transientTaskExemptedQuatoList appName: %{public}s", appName.c_str());
        }

        if (!jsonObj.contains(TRANSIENT_ERR_DELAYED_FROZEN_TIME) ||
            !jsonObj[TRANSIENT_ERR_DELAYED_FROZEN_TIME].is_number_integer()) {
            BGTASK_LOGE("no key %{public}s", TRANSIENT_ERR_DELAYED_FROZEN_TIME.c_str());
            return false;
        }
        transientTaskExemptedQuato_ = jsonObj[TRANSIENT_ERR_DELAYED_FROZEN_TIME].get<int>();
        BGTASK_LOGI("suspend config transientTaskExemptedQuato: %{public}d", transientTaskExemptedQuato_);
    }
    return true;
}

bool BgtaskConfig::SetCloudConfigParam(const nlohmann::json &jsonObj)
{
    if (!jsonObj.contains(CONFIG_JSON_INDEX_TOP) || !jsonObj[CONFIG_JSON_INDEX_TOP].is_object()) {
        BGTASK_LOGE("no key %{public}s", CONFIG_JSON_INDEX_TOP.c_str());
        return false;
    }
    nlohmann::json params = jsonObj[CONFIG_JSON_INDEX_TOP];
    std::lock_guard<std::mutex> lock(configMutex_);
    SetTransientTaskParam(params);
    SetContinuousTaskParam(params);
    return true;
}

void BgtaskConfig::SetTransientTaskParam(const nlohmann::json &jsonObj)
{
    if (jsonObj.contains(TRANSIENT_ERR_DELAYED_FROZEN_LIST) &&
        jsonObj[TRANSIENT_ERR_DELAYED_FROZEN_LIST].is_array()) {
        nlohmann::json appArray = jsonObj[TRANSIENT_ERR_DELAYED_FROZEN_LIST];
        transientTaskCloudExemptedQuatoList_.clear();
        for (const auto &app : appArray) {
            transientTaskCloudExemptedQuatoList_.insert(app);
        }
        for (const auto &appName : transientTaskCloudExemptedQuatoList_) {
            BGTASK_LOGI("transientTaskCloudExemptedQuatoList appName: %{public}s", appName.c_str());
        }
    } else {
        BGTASK_LOGW("no key %{public}s", TRANSIENT_ERR_DELAYED_FROZEN_LIST.c_str());
    }
    if (jsonObj.contains(CONFIG_JSON_INDEX_SUSPEND_SECOND) &&
        jsonObj[CONFIG_JSON_INDEX_SUSPEND_SECOND].is_object()) {
        nlohmann::json param = jsonObj[CONFIG_JSON_INDEX_SUSPEND_SECOND];
        if (param.contains(TRANSIENT_EXEMPTED_QUOTA) &&
            !param[TRANSIENT_EXEMPTED_QUOTA].is_number_integer()) {
            transientTaskExemptedQuato_ = param[TRANSIENT_EXEMPTED_QUOTA].get<int>();
            BGTASK_LOGI("cloud config transientTaskExemptedQuato: %{public}d", transientTaskExemptedQuato_);
        } else {
            BGTASK_LOGE("no key %{public}s", TRANSIENT_EXEMPTED_QUOTA.c_str());
        }
    } else {
        BGTASK_LOGW("no key %{public}s", CONFIG_JSON_INDEX_SUSPEND_SECOND.c_str());
    }
}

void BgtaskConfig::SetContinuousTaskParam(const nlohmann::json &jsonObj)
{
    if (jsonObj.contains(CONTINUOUS_TASK_KEEPING_EXEMPTED_LIST) &&
        jsonObj[CONTINUOUS_TASK_KEEPING_EXEMPTED_LIST].is_array()) {
        nlohmann::json appArrayTaskKeeping = jsonObj[CONTINUOUS_TASK_KEEPING_EXEMPTED_LIST];
        taskKeepingExemptedQuatoList_.clear();
        for (const auto &app : appArrayTaskKeeping) {
            taskKeepingExemptedQuatoList_.insert(app);
        }
        for (const auto &appName : taskKeepingExemptedQuatoList_) {
            BGTASK_LOGI("taskKeepingExemptedQuatoList_ appName: %{public}s", appName.c_str());
        }
    } else {
        BGTASK_LOGW("no key %{public}s", CONTINUOUS_TASK_KEEPING_EXEMPTED_LIST.c_str());
    }
    if (jsonObj.contains(MALICIOUS_APP_BLOCKLIST) &&
        jsonObj[MALICIOUS_APP_BLOCKLIST].is_array()) {
        nlohmann::json appArrayMalicious = jsonObj[MALICIOUS_APP_BLOCKLIST];
        maliciousAppBlocklist_.clear();
        for (const auto &app : appArrayMalicious) {
            maliciousAppBlocklist_.insert(app);
        }
        for (const auto &appName : maliciousAppBlocklist_) {
            BGTASK_LOGI("maliciousAppBlocklist_ appName: %{public}s", appName.c_str());
        }
    } else {
        BGTASK_LOGW("no key %{public}s", MALICIOUS_APP_BLOCKLIST.c_str());
    }
}

void BgtaskConfig::ParseTransientTaskExemptedQuato(const nlohmann::json &jsonObj)
{
    if (jsonObj.is_null() || jsonObj.empty()) {
        BGTASK_LOGE("jsonObj null");
        return;
    }
    if (!jsonObj.contains(TRANSIENT_EXEMPTED_QUOTA) || !jsonObj[TRANSIENT_EXEMPTED_QUOTA].is_number_integer()) {
        BGTASK_LOGE("no key %{public}s", TRANSIENT_EXEMPTED_QUOTA.c_str());
        return;
    }
    std::lock_guard<std::mutex> lock(configMutex_);
    transientTaskExemptedQuato_ = jsonObj[TRANSIENT_EXEMPTED_QUOTA].get<int32_t>();
    BGTASK_LOGI("transientTaskExemptedQuato_ %{public}d", transientTaskExemptedQuato_);
}

bool BgtaskConfig::IsTransientTaskExemptedQuatoApp(const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(configMutex_);
    if (transientTaskCloudExemptedQuatoList_.size() > 0) {
        return transientTaskCloudExemptedQuatoList_.count(bundleName) > 0;
    }
    return transientTaskExemptedQuatoList_.count(bundleName) > 0;
}

bool BgtaskConfig::IsTaskKeepingExemptedQuatoApp(const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(configMutex_);
    return taskKeepingExemptedQuatoList_.count(bundleName) > 0;
}

int32_t BgtaskConfig::GetTransientTaskExemptedQuato()
{
    std::lock_guard<std::mutex> lock(configMutex_);
    return transientTaskExemptedQuato_;
}

void BgtaskConfig::SetSupportedTaskKeepingProcesses(const std::set<std::string> &processSet)
{
    std::lock_guard<std::mutex> lock(configMutex_);
    taskKeepingExemptedQuatoList_.insert(processSet.begin(), processSet.end());
    for (const auto &item : taskKeepingExemptedQuatoList_) {
        BGTASK_LOGI("taskKeeping Exemption proc: %{public}s", item.c_str());
    }
}

void BgtaskConfig::SetMaliciousAppConfig(const std::set<std::string> &maliciousAppSet)
{
    std::lock_guard<std::mutex> lock(configMutex_);
    maliciousAppBlocklist_.insert(maliciousAppSet.begin(), maliciousAppSet.end());
    for (const auto &item : maliciousAppBlocklist_) {
        BGTASK_LOGI("malicious app blocklist proc: %{public}s", item.c_str());
    }
}

bool BgtaskConfig::IsMaliciousAppConfig(const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(configMutex_);
    return maliciousAppBlocklist_.count(bundleName) > 0;
}

void BgtaskConfig::LoadBgTaskConfigFile()
{
    nlohmann::json jsonObj;
    std::string absolutePath = DelayedSingleton<DataStorageHelper>::GetInstance()->
        GetConfigFileAbsolutePath(BACKGROUND_TASK_CONFIG_FILE);
    
    if (DelayedSingleton<DataStorageHelper>::GetInstance()->ParseJsonValueFromFile(jsonObj, absolutePath) != 0) {
        BGTASK_LOGE("LoadBgTaskConfigFile failed");
        return;
    }
    ParseCpuEfficiencyResourceApplyBundleInfos(jsonObj);
}

void BgtaskConfig::ParseCpuEfficiencyResourceApplyBundleInfos(const nlohmann::json &jsonObj)
{
    if (jsonObj.is_null() || jsonObj.empty()) {
        BGTASK_LOGE("%{public}s jsonObj null", __func__);
        return;
    }
    if (!jsonObj.contains(CPU_EFFICIENCY_RESOURCE_ALLOW_APPLY_BUNDLE_INFOS) ||
        !jsonObj[CPU_EFFICIENCY_RESOURCE_ALLOW_APPLY_BUNDLE_INFOS].is_array()) {
        BGTASK_LOGE("no key %{public}s", CPU_EFFICIENCY_RESOURCE_ALLOW_APPLY_BUNDLE_INFOS.c_str());
        return;
    }

    nlohmann::json allowApplyCpuLevelBundleInfos = jsonObj[CPU_EFFICIENCY_RESOURCE_ALLOW_APPLY_BUNDLE_INFOS];
    std::lock_guard<std::mutex> lock(configMutex_);
    for (const auto &bundleInfoJsonObj : allowApplyCpuLevelBundleInfos) {
        if (!bundleInfoJsonObj.contains(ALLOW_APPLY_BUNDLE_INFO_BUNDLE_NAME) ||
            !bundleInfoJsonObj[ALLOW_APPLY_BUNDLE_INFO_BUNDLE_NAME].is_string()) {
            BGTASK_LOGE("prop %{public}s invalid", ALLOW_APPLY_BUNDLE_INFO_BUNDLE_NAME.c_str());
            continue;
        }
        if (!bundleInfoJsonObj.contains(ALLOW_APPLY_BUNDLE_INFO_CPU_LEVEL) ||
            !bundleInfoJsonObj[ALLOW_APPLY_BUNDLE_INFO_CPU_LEVEL].is_number_integer()) {
            BGTASK_LOGE("prop %{public}s invalid", ALLOW_APPLY_BUNDLE_INFO_CPU_LEVEL.c_str());
            continue;
        }
        if (!bundleInfoJsonObj.contains(ALLOW_APPLY_BUNDLE_INFO_APP_SIGNATURES) ||
            !bundleInfoJsonObj[ALLOW_APPLY_BUNDLE_INFO_APP_SIGNATURES].is_array()) {
            BGTASK_LOGE("prop %{public}s invalid", ALLOW_APPLY_BUNDLE_INFO_APP_SIGNATURES.c_str());
            continue;
        }

        std::vector<std::string> appSignatures;
        nlohmann::json signArrayJsonObj = bundleInfoJsonObj[ALLOW_APPLY_BUNDLE_INFO_APP_SIGNATURES];
        for (const auto &appSign : signArrayJsonObj) {
            if (!appSign.is_string()) {
                BGTASK_LOGE("prop %{public}s value invalid", ALLOW_APPLY_BUNDLE_INFO_APP_SIGNATURES.c_str());
                continue;
            }
            appSignatures.push_back(appSign.get<std::string>());
        }
        std::string bundleName = bundleInfoJsonObj[ALLOW_APPLY_BUNDLE_INFO_BUNDLE_NAME].get<std::string>();
        int32_t cpuLevel = bundleInfoJsonObj[ALLOW_APPLY_BUNDLE_INFO_CPU_LEVEL].get<int32_t>();
        bgTaskConfigFileInfo_.AddCpuLevelConfigInfo({bundleName, appSignatures, cpuLevel});
    }
    for (const auto &[bundleName, info] : bgTaskConfigFileInfo_.GetAllowApplyCpuBundleInfoMap()) {
        BGTASK_LOGI("%{public}s: bundleName %{public}s, cpuLevel %{public}d", __func__, bundleName.c_str(), info.cpuLevel);
    }
}

bool BgtaskConfig::CheckRequestCpuLevelBundleNameConfigured(const std::string &bundleName)
{
    return bgTaskConfigFileInfo_.CheckBundleName(bundleName);
}

bool BgtaskConfig::CheckRequestCpuLevelAppSignatures(const std::string &bundleName, const std::string &appId,
    const std::string &appIdentifier)
{
    return bgTaskConfigFileInfo_.CheckAppSignatures(bundleName, appId, appIdentifier);
}

bool BgtaskConfig::CheckRequestCpuLevel(const std::string &bundleName, int32_t cpuLevel)
{
    return bgTaskConfigFileInfo_.CheckCpuLevel(bundleName, cpuLevel);
}
}
}