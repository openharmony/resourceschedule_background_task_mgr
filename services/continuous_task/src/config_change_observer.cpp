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

#include "config_change_observer.h"

#include "bg_continuous_task_mgr.h"
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
ConfigChangeObserver::ConfigChangeObserver(const std::shared_ptr<AppExecFwk::EventHandler> handler,
    const std::shared_ptr<BgContinuousTaskMgr> bgContinuousTaskMgr)
{
    handler_ = handler;
    bgContinuousTaskMgr_ = bgContinuousTaskMgr;
}

bool ConfigChangeObserver::CheckParamValid()
{
    if (handler_.expired()) {
        BGTASK_LOGE("ConfigChangeObserver handler is null");
        return false;
    }
    if (bgContinuousTaskMgr_.expired()) {
        BGTASK_LOGE("ConfigChangeObserver bgContinuousTaskMgr is null");
        return false;
    }
    return true;
}

void ConfigChangeObserver::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    if (!CheckParamValid()) {
        return;
    }
    auto task = [this, configuration]() {
        this->bgContinuousTaskMgr_.lock()->OnConfigurationChanged(configuration);
    };
    handler_.lock()->PostTask(task);
}
}
}