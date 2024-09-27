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
const std::string TRANSIENT_ERR_DELAYED_FROZEN_LIST = "transient_err_delayed_frozen_list";
const std::string TRANSIENT_EXEMPTED_QUOTA = "transient_exempted_quota";
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
    if (jsonObj.is_null() || jsonObj.empty()) {
        BGTASK_LOGE("jsonObj null");
        return false;
    }
    nlohmann::json appArray;
    if (!jsonObj.contains(TRANSIENT_ERR_DELAYED_FROZEN_LIST) ||
        !jsonObj[TRANSIENT_ERR_DELAYED_FROZEN_LIST].is_array()) {
        BGTASK_LOGE("no key %{public}s", TRANSIENT_ERR_DELAYED_FROZEN_LIST.c_str());
        return false;
    }
    appArray = jsonObj[TRANSIENT_ERR_DELAYED_FROZEN_LIST];
    if (sourceType == ConfigDataSourceType::CONFIG_CLOUD) {
        transientTaskCloudExemptedQuatoList_.clear();
        for (const auto &app : appArray) {
            transientTaskCloudExemptedQuatoList_.insert(app);
        }
    } else if (sourceType == ConfigDataSourceType::CONFIG_SUSPEND_MANAGER) {
        transientTaskExemptedQuatoList_.clear();
        for (const auto &app : appArray) {
            transientTaskExemptedQuatoList_.insert(app);
        }
    }
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
    transientTaskExemptedQuato_ = jsonObj[TRANSIENT_EXEMPTED_QUOTA].get<int32_t>();
    BGTASK_LOGI("transientTaskExemptedQuato_ %{public}d", transientTaskExemptedQuato_);
}

bool BgtaskConfig::IsTransientTaskExemptedQuatoApp(const std::string &bundleName) const
{
    if (transientTaskCloudExemptedQuatoList_.size() > 0) {
        return transientTaskCloudExemptedQuatoList_.count(bundleName) > 0;
    }
    return transientTaskExemptedQuatoList_.count(bundleName) > 0;
}

int32_t BgtaskConfig::GetTransientTaskExemptedQuato() const
{
    return transientTaskExemptedQuato_;
}
}
}