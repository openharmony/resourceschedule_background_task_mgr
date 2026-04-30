/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_COMMON_UTILS_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_COMMON_UTILS_H

#include <regex>
#include <set>
#include "nlohmann/json.hpp"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
    static constexpr uint32_t NOTIFICATION_TEXT_MEDIA_PROCESS_INDEX = 1;
    static constexpr uint32_t NOTIFICATION_TEXT_VIDEO_BROADCAST_INDEX = 2;
    static constexpr uint32_t NOTIFICATION_TEXT_WORK_OUT_INDEX = 3;
    // 系统API 注册subscriber回调
    static constexpr uint32_t SUBSCRIBER_BACKGROUND_TASK_STATE = 1 << 3;
    // 长时任务授权Dialog点击事件
    static constexpr char BGTASK_AUTH_DIALOG_EVENT_NAME[] = "OnBgTaskServiceDialogClicked";
    // 点击公共事件只接收长时hap自己发送的
    static constexpr char BGTASK_BUNDLE_NAME[] = "com.ohos.backgroundtaskmgr.resources";
}
class CommonUtils {
public:
    static bool CheckJsonValue(const nlohmann::json &value, std::initializer_list<std::string> params)
    {
        for (auto param : params) {
            if (value.find(param) == value.end()) {
                return false;
            }
        }
        return true;
    }

    static bool CheckExistMode(const std::vector<uint32_t> &bgModeIds, uint32_t bgMode)
    {
        auto iter = std::find(bgModeIds.begin(), bgModeIds.end(), bgMode);
        return iter != bgModeIds.end();
    }

    static bool CheckExistNotification(const std::vector<int32_t> &notificaitonIds, const int32_t notificaitonId)
    {
        auto iter = std::find(notificaitonIds.begin(), notificaitonIds.end(), notificaitonId);
        return iter == notificaitonIds.end();
    }

    static bool CheckModesSame(const std::vector<uint32_t> &oldBgModeIds, const std::vector<uint32_t> &newBgModeIds)
    {
        std::set<uint32_t> oldModesSet(oldBgModeIds.begin(), oldBgModeIds.end());
        std::set<uint32_t> newModesSet(newBgModeIds.begin(), newBgModeIds.end());
        return oldModesSet == newModesSet;
    }

    static std::string ModesToString(const std::vector<uint32_t> &bgmodes)
    {
        std::string modeStr;
        for (const auto &mode : bgmodes) {
            modeStr += std::to_string(mode);
        }
        return modeStr;
    }

    static bool CheckApplyMode(const std::vector<uint32_t> &applyBgModeIds,
        const std::vector<uint32_t> &checkBgModeIds)
    {
        for (const auto &mode : applyBgModeIds) {
            auto iter = std::find(checkBgModeIds.begin(), checkBgModeIds.end(), mode);
            if (iter == checkBgModeIds.end()) {
                return false;
            }
        }
        return true;
    }

    static bool CheckExistOtherMode(
        const std::vector<uint32_t> &bgModeIds, uint32_t bgMode, const std::set<uint32_t> &liveViewTypes)
    {
        std::set<uint32_t> taskTypesSet(bgModeIds.begin(), bgModeIds.end());
        std::set<uint32_t> otherTypes = liveViewTypes;
        otherTypes.erase(bgMode);
        for (const auto& type : otherTypes) {
            if (taskTypesSet.find(type) != taskTypesSet.end()) {
                return true;
            }
        }
        return false;
    }

    static bool CheckStrToNum(const std::string &value)
    {
        std::regex pattern(R"(^\d+$)");
        return std::regex_match(value, pattern);
    }
public:
    static constexpr int32_t jsonFormat_ = 4;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_COMMON_UTILS_H