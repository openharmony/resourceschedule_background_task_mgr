/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "transient_task_guard.h"

#include <functional>

#include "ffrt.h"

#include "bg_transient_task_mgr.h"
#include "time_provider.h"
#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
    constexpr int64_t GUARD_INTERVAL_MS = MSEC_PER_HOUR; // 60 minutes
}

TransientTaskGuard::TransientTaskGuard(std::shared_ptr<BgTransientTaskMgr> mgr)
    : mgr_(mgr), running_(std::make_shared<std::atomic<bool>>(false)) {}

TransientTaskGuard::~TransientTaskGuard()
{
    Stop();
}

void TransientTaskGuard::Start()
{
    if (running_->exchange(true)) {
        BGTASK_LOGW("TransientTaskGuard is already running");
        return;
    }
    auto flag = running_;
    auto mgr = mgr_;
    auto task = std::make_shared<std::function<void()>>();
    *task = [mgr, flag, task]() {
        if (!flag->load()) {
            return;
        }
        mgr->CheckAndCancelOvertimeTasks();
        if (!flag->load()) {
            return;
        }
        ffrt::submit(*task, {}, {}, ffrt::task_attr().delay(GUARD_INTERVAL_MS));
    };
    ffrt::submit(*task, {}, {}, ffrt::task_attr().delay(GUARD_INTERVAL_MS));
    BGTASK_LOGI("TransientTaskGuard started, first check after %{public}lldms",
        static_cast<long long>(GUARD_INTERVAL_MS));
}

void TransientTaskGuard::Stop()
{
    running_->store(false);
    BGTASK_LOGI("TransientTaskGuard stopped");
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
