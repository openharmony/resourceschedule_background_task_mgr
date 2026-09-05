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

#include <chrono>

#include "ffrt.h"

#include "bg_transient_task_mgr.h"
#include "transient_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
    constexpr int32_t GUARD_INTERVAL_MS = 1 * 60 * 60 * 1000; // 60 minutes
    constexpr int32_t STOP_WAIT_TIMEOUT_S = 5; // 5 seconds
}

TransientTaskGuard::TransientTaskGuard(BgTransientTaskMgr* mgr) : mgr_(mgr) {}

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
    exitPromise_ = std::make_shared<std::promise<void>>();
    exitFuture_ = exitPromise_->get_future();
    auto promise = exitPromise_;
    ffrt::submit([this, promise]() {
        BGTASK_LOGI("TransientTaskGuard thread started");
        while (running_.load()) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait_for(lock, std::chrono::milliseconds(GUARD_INTERVAL_MS),
                [this]() { return !running_.load(); });
            lock.unlock();
            if (!running_.load()) {
                break;
            }
            mgr_->CheckAndCancelOvertimeTasks();
        }
        BGTASK_LOGI("TransientTaskGuard thread exited");
        promise->set_value();
    });
}

void TransientTaskGuard::Stop()
{
    if (!running_.exchange(false)) {
        return;
    }
    cv_.notify_all();
    if (exitFuture_.valid()) {
        auto status = exitFuture_.wait_for(std::chrono::seconds(STOP_WAIT_TIMEOUT_S));
        if (status == std::future_status::timeout) {
            BGTASK_LOGW("TransientTaskGuard thread did not exit within %{public}ds", STOP_WAIT_TIMEOUT_S);
        }
    }
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
