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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_TRANSIENT_TASK_GUARD_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_TRANSIENT_TASK_GUARD_H

#include <atomic>
#include <functional>
#include <memory>

namespace OHOS {
namespace BackgroundTaskMgr {

class BgTransientTaskMgr;

class TransientTaskGuard {
public:
    explicit TransientTaskGuard(std::shared_ptr<BgTransientTaskMgr> mgr);
    ~TransientTaskGuard();
    void Start();
    void Stop();

private:
    std::shared_ptr<BgTransientTaskMgr> mgr_ {nullptr};
    std::shared_ptr<std::atomic<bool>> running_ {nullptr};
    std::shared_ptr<std::function<void()>> task_ {nullptr};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_TRANSIENT_TASK_GUARD_H
