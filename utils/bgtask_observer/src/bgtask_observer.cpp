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

#include "bgtask_observer.h"

#include <sstream>

#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
void BgTaskObserver::OnContinuousTaskStart(const std::shared_ptr<OHOS::BackgroundTaskMgr
    ::ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("Continuous Task Start");
    if (continuousTaskCallbackInfo == nullptr) {
        return;
    }
    std::stringstream stream;
    int32_t typeId = continuousTaskCallbackInfo->GetTypeId();
    int32_t uid = continuousTaskCallbackInfo->GetCreatorUid();
    pid_t pid = continuousTaskCallbackInfo->GetCreatorPid();
    std::string abiliytName = continuousTaskCallbackInfo->GetAbilityName();
    stream.str("");
    stream.clear();
    stream << "Continuous Task Start:" << "\n";
    stream << "\ttask typeId: " << typeId << "\n";
    stream << "\ttask uid: " << uid << "\n";
    stream << "\ttask pid: " << pid << "\n";
    stream << "\ttask abiliytName: " << abiliytName << "\n";

    std::cout << stream.str() << std::endl;
}

void BgTaskObserver::OnContinuousTaskStop(const std::shared_ptr<OHOS::BackgroundTaskMgr
    ::ContinuousTaskCallbackInfo> &continuousTaskCallbackInfo)
{
    BGTASK_LOGI("Continuous Task Stop");
    if (continuousTaskCallbackInfo == nullptr) {
        return;
    }
    std::stringstream stream;
    int32_t typeId = continuousTaskCallbackInfo->GetTypeId();
    int32_t uid = continuousTaskCallbackInfo->GetCreatorUid();
    pid_t pid = continuousTaskCallbackInfo->GetCreatorPid();
    std::string abiliytName = continuousTaskCallbackInfo->GetAbilityName();
    stream.str("");
    stream.clear();
    stream << "Continuous Task Stop:" << "\n";
    stream << "\ttask typeId: " << typeId << "\n";
    stream << "\ttask uid: " << uid << "\n";
    stream << "\ttask pid: " << pid << "\n";
    stream << "\ttask abiliytName: " << abiliytName << "\n";

    std::cout << stream.str() << std::endl;
}

void BgTaskObserver::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    BGTASK_LOGW("remote service died");
    isRemoteDied_.store(true);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS