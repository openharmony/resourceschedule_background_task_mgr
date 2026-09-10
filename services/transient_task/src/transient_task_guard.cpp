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

#include "ffrt.h"

#include "bg_transient_task_mgr.h"
#include "time_provider.h"
#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
constexpr int64_t GUARD_INTERVAL_US = 60LL * 60 * USEC_PER_SEC; // 60 minutes
}

TransientTaskGuard::TransientTaskGuard(std::shared_ptr<BgTransientTaskMgr> mgr) : mgr_(mgr) {}

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
    uint64_t gen = generation_.fetch_add(1) + 1;
    ScheduleNext(gen);
}

void TransientTaskGuard::ScheduleNext(uint64_t gen)
{
    std::weak_ptr<TransientTaskGuard> weakSelf = shared_from_this();
    ffrt::submit([weakSelf, gen]() {
        auto self = weakSelf.lock();
        if (self == nullptr) {
            return;
        }
        if (!self->running_.load() || gen != self->generation_.load()) {
            return;
        }
        auto mgr = self->mgr_.lock();
        if (mgr == nullptr) {
            return;
        }
        mgr->CheckAndCancelOvertimeTasks();
        if (!self->running_.load() || gen != self->generation_.load()) {
            return;
        }
        self->ScheduleNext(gen);
    }, {}, {}, ffrt::task_attr().delay(GUARD_INTERVAL_US));
}

void TransientTaskGuard::Stop()
{
    running_.store(false);
    generation_.fetch_add(1);
    BGTASK_LOGI("TransientTaskGuard stopped");
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
