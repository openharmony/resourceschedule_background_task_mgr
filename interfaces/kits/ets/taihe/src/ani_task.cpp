/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ani_task.h"

#include <memory>
#include <thread>
#include "bgtaskmgr_inner_errors.h"
#include "bgtaskmgr_log_wrapper.h"

namespace OHOS {
namespace BackgroundTaskMgr {
std::shared_ptr<OHOS::AppExecFwk::EventHandler> AniTask::mainHandler_ = nullptr;
std::mutex mainHandlerMutex_;

AniTask::AniTask()
{
    BGTASK_LOGD("AniTask()");
}

AniTask::~AniTask()
{
    std::lock_guard<std::mutex> lock(mainHandlerMutex_);
    mainHandler_ = nullptr;
    BGTASK_LOGD("~AniTask()");
}
ani_status AniTask::AniSendEvent(const std::function<void()> task)
{
    BGTASK_LOGD("AniSendEvent");
    if (task == nullptr) {
        BGTASK_LOGE("null task");
        return ani_status::ANI_INVALID_ARGS;
    }
    std::lock_guard<std::mutex> lock(mainHandlerMutex_);
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        if (!runner) {
            BGTASK_LOGE("null EventRunner");
            return ani_status::ANI_NOT_FOUND;
        }
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    }
    if (mainHandler_ == nullptr) {
        BGTASK_LOGE("null mainHandler");
        return ani_status::ANI_NOT_FOUND;
    }
    mainHandler_->PostTask(std::move(task));
    return ani_status::ANI_OK;
}
} // namespace BackgroundTaskMgr
} // namespace OHOS