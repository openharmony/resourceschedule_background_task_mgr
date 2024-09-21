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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_COMMON_BGTASK_CONFIG_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_COMMON_BGTASK_CONFIG_H
#include <string>
#include <set>

#include "singleton.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace BackgroundTaskMgr {
class BgtaskConfig : public DelayedSingleton<BgtaskConfig> {
public:
    void Init();
    bool IsTransientTaskExemptedQuatoApp(const std::string &bundleName) const;
    int32_t GetTransientTaskExemptedQuato() const;

private:
    void LoadConfigFile();
    void ParseTransientTaskExemptedQuatoList(const nlohmann::json &jsonObj);
    void ParseTransientTaskExemptedQuato(const nlohmann::json &jsonObj);

private:
    bool isInit_ = false;
    std::set<std::string> transientTaskExemptedQuatoList_ {};
    int32_t transientTaskExemptedQuato_ = 10 * 1000; // 10s
};
}
}
#endif