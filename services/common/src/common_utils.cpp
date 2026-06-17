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


#include "common_utils.h"

namespace OHOS {
namespace BackgroundTaskMgr {
bool CommonUtils::CheckJsonValue(const nlohmann::json &value, std::initializer_list<std::string> params)
{
    for (const auto &param : params) {
        if (value.find(param) == value.end()) {
            return false;
        }
    }
    return true;
}

bool CommonUtils::CheckExistMode(const std::vector<uint32_t> &bgModeIds, uint32_t bgMode)
{
    auto iter = std::find(bgModeIds.begin(), bgModeIds.end(), bgMode);
    return iter != bgModeIds.end();
}

bool CommonUtils::CheckExistNotification(const std::vector<int32_t> &notificationIds, const int32_t notificationId)
{
    auto iter = std::find(notificationIds.begin(), notificationIds.end(), notificationId);
    return iter == notificationIds.end();
}

bool CommonUtils::CheckModesSame(const std::vector<uint32_t> &oldBgModeIds, const std::vector<uint32_t> &newBgModeIds)
{
    std::set<uint32_t> oldModesSet(oldBgModeIds.begin(), oldBgModeIds.end());
    std::set<uint32_t> newModesSet(newBgModeIds.begin(), newBgModeIds.end());
    return oldModesSet == newModesSet;
}

std::string CommonUtils::ModesToString(const std::vector<uint32_t> &bgmodes)
{
    std::string modeStr;
    for (const auto &mode : bgmodes) {
        modeStr += std::to_string(mode);
    }
    return modeStr;
}

bool CommonUtils::CheckApplyMode(const std::vector<uint32_t> &applyBgModeIds,
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

bool CommonUtils::CheckExistOtherMode(
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

bool CommonUtils::CheckStrToNum(const std::string &value)
{
    std::regex pattern(R"(^\d+$)");
    return std::regex_match(value, pattern);
}

void CommonUtils::SortMode(std::vector<uint32_t> &bgModeIds)
{
    std::vector<uint32_t> result;
    std::vector<uint32_t> target = {
        BackgroundMode::DATA_TRANSFER,
        BackgroundMode::AUDIO_RECORDING,
        BackgroundMode::LOCATION
    };
    for (const auto mode : target) {
        if (std::find(bgModeIds.begin(), bgModeIds.end(), mode) != bgModeIds.end()) {
            result.push_back(mode);
        }
    }
    for (const auto mode : bgModeIds) {
        if (std::find(target.begin(), target.end(), mode) == target.end()) {
            result.push_back(mode);
        }
    }
    bgModeIds = result;
}
}  // namespace SuspendManager
}  // namespace OHOS