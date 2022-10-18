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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_CONFIG_CHANGE_OBSERVER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_CONFIG_CHANGE_OBSERVER_H

#include "configuration_observer_stub.h"
#include "event_handler.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class BgContinuousTaskMgr;

class ConfigChangeObserver : public AppExecFwk::ConfigurationObserverStub {
public:
    ConfigChangeObserver() = default;
    ~ConfigChangeObserver() = default;
    ConfigChangeObserver(const std::shared_ptr<AppExecFwk::EventHandler> handler,
        const std::shared_ptr<BgContinuousTaskMgr> bgContinuousTaskMgr);
    virtual void OnConfigurationUpdated(const AppExecFwk::Configuration& configuration) override;

private:
    bool CheckParamValid();

private:
    std::weak_ptr<AppExecFwk::EventHandler> handler_ {};
    std::weak_ptr<BgContinuousTaskMgr> bgContinuousTaskMgr_ {};
};
}
}
#endif