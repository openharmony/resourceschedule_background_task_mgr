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
 
#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_COMMON_INCLUDE_GAME_PRE_LAUNCH_MGR_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_COMMON_INCLUDE_GAME_PRE_LAUNCH_MGR_H

#ifdef GAME_PRE_LAUNCH_ENABLE
#include <set>
#include <memory>
#include <mutex>
#include "singleton.h"

namespace OHOS {
namespace BackgroundTaskMgr {

class GamePreLaunchMgr : public DelayedSingleton<GamePreLaunchMgr> {
public:
    GamePreLaunchMgr();
    ~GamePreLaunchMgr();

    /**
     * @brief Add application uid to game pre-launch app set.
     * @param uid Application uid.
     */
    void AddGamePreLaunchApp(const int32_t uid);

    /**
     * @brief Remove application uid from game pre-launch app set.
     * @param uid Application uid.
     */
    void RemoveGamePreLaunchApp(const int32_t uid);

    /**
     * @brief Check if application is in pre-launch state.
     * @param uid Application uid.
     * @return True if application is in pre-launch state, false otherwise.
     */
    bool IsGamePreLaunchApp(const int32_t uid) const;
private:
    mutable std::mutex gamePreLaunchMutex_;
    std::set<int32_t> gamePreLaunchAppUids_{};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // GAME_PRE_LAUNCH_ENABLE
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_COMMON_INCLUDE_GAME_PRE_LAUNCH_MGR_H