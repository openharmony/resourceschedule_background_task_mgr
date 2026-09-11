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

#include <ffrt.h>

#include "bg_transient_task_mgr.h"
#include "singleton.h"
#include "time_provider.h"
#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
constexpr int64_t GUARD_INTERVAL_US = 60LL * 60 * USEC_PER_SEC; // 60 minutes
}

TransientTaskGuard::~TransientTaskGuard()
{
    Stop();
}

void TransientTaskGuard::Start()
{
    if (running_.exchange(true)) {
        BGTASK_LOGW("TransientTaskGuard is already running");
        return;
    }
    ScheduleNext();
}

void TransientTaskGuard::ScheduleNext()
{
    std::weak_ptr<TransientTaskGuard> weakSelf = shared_from_this();
    ffrt::submit(
        [weakSelf]() {
            auto self = weakSelf.lock();
            if (self == nullptr) {
                BGTASK_LOGE("Guard task expired but guard already destroyed");
                return;
            }
            if (self->IsExpired()) {
                BGTASK_LOGE("Guard task expired but stopped, running: %{public}d",
                    static_cast<int>(self->running_.load()));
                return;
            }
            auto mgr = DelayedSingleton<BgTransientTaskMgr>::GetInstance();
            if (mgr == nullptr) {
                BGTASK_LOGE("Guard task expired but BgTransientTaskMgr is null");
                return;
            }
            mgr->CheckAndCancelOvertimeTasks();
            if (self->IsExpired()) {
                BGTASK_LOGE("Guard task stopped during check, running: %{public}d",
                    static_cast<int>(self->running_.load()));
                return;
            }
            self->ScheduleNext();
        },
        {},
        {},
        ffrt::task_attr().delay(GUARD_INTERVAL_US));
}

bool TransientTaskGuard::IsExpired() const
{
    return !running_.load();
}

void TransientTaskGuard::Stop()
{
    running_.store(false);
    BGTASK_LOGI("TransientTaskGuard stopped");
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
