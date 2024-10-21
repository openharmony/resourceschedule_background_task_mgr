/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "parameters.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
const std::string SUSPEND_MANAGER_CONFIG_FILE = "etc/efficiency_manager/suspend_manager_config.json";
const std::string CONFIG_JSON_INDEX_TOP = "params";
const std::string CONFIG_JSON_INDEX_SUSPEND_SECOND = "param";
const std::string TRANSIENT_ERR_DELAYED_FROZEN_LIST = "transient_err_delayed_frozen_list";
const std::string TRANSIENT_EXEMPTED_QUOTA = "transient_exempted_quota";
const std::string TRANSIENT_ERR_DELAYED_FROZEN_TIME = "transient_err_delayed_frozen_time";
}

void BgtaskConfig::Init()
{
    if (isInit_) {
        BGTASK_LOGE("already init config!");
        return;
    }

    LoadConfigFile();
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

    if (!params.contains(TRANSIENT_ERR_DELAYED_FROZEN_LIST) ||
        !params[TRANSIENT_ERR_DELAYED_FROZEN_LIST].is_array()) {
        BGTASK_LOGE("no key %{public}s", TRANSIENT_ERR_DELAYED_FROZEN_LIST.c_str());
        return false;
    }
    nlohmann::json appArray = params[TRANSIENT_ERR_DELAYED_FROZEN_LIST];
    std::lock_guard<std::mutex> lock(configMutex_);
    transientTaskCloudExemptedQuatoList_.clear();
    for (const auto &app : appArray) {
        transientTaskCloudExemptedQuatoList_.insert(app);
    }
    for (const auto &appName : transientTaskCloudExemptedQuatoList_) {
        BGTASK_LOGI("transientTaskCloudExemptedQuatoList appName: %{public}s", appName.c_str());
    }

    if (!params.contains(CONFIG_JSON_INDEX_SUSPEND_SECOND) ||
        !params[CONFIG_JSON_INDEX_SUSPEND_SECOND].is_object()) {
        BGTASK_LOGE("no key %{public}s", CONFIG_JSON_INDEX_SUSPEND_SECOND.c_str());
        return false;
    }
    nlohmann::json param = jsoparamsnObj[CONFIG_JSON_INDEX_SUSPEND_SECOND];

    if (!param.contains(TRANSIENT_EXEMPTED_QUOTA) ||
        !param[TRANSIENT_EXEMPTED_QUOTA].is_number_integer()) {
        BGTASK_LOGE("no key %{public}s", TRANSIENT_EXEMPTED_QUOTA.c_str());
        return false;
    }
    transientTaskExemptedQuato_ = param[TRANSIENT_EXEMPTED_QUOTA].get<int>();
    BGTASK_LOGI("cloud config transientTaskExemptedQuato: %{public}d", transientTaskExemptedQuato_);
    return true;
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

int32_t BgtaskConfig::GetTransientTaskExemptedQuato()
{
    std::lock_guard<std::mutex> lock(configMutex_);
    return transientTaskExemptedQuato_;
}
}
}
