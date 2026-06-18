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

#include "game_pre_launch_mgr.h"
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {

GamePreLaunchMgr::GamePreLaunchMgr()
{
}

GamePreLaunchMgr::~GamePreLaunchMgr()
{
}

void GamePreLaunchMgr::AddGamePreLaunchApp(const int32_t uid)
{
    std::lock_guard<std::mutex> lock(gamePreLaunchMutex_);
    gamePreLaunchAppUids_.insert(uid);
    BGTASK_LOGI("Add game pre-launch app uid: %{public}d", uid);
}

void GamePreLaunchMgr::RemoveGamePreLaunchApp(const int32_t uid)
{
    std::lock_guard<std::mutex> lock(gamePreLaunchMutex_);
    gamePreLaunchAppUids_.erase(uid);
    BGTASK_LOGI("Remove game pre-launch app uid: %{public}d", uid);
}

bool GamePreLaunchMgr::IsGamePreLaunchApp(const int32_t uid) const
{
    std::lock_guard<std::mutex> lock(gamePreLaunchMutex_);
    return gamePreLaunchAppUids_.find(uid) != gamePreLaunchAppUids_.end();
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS