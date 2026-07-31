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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_PROGRESS_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_PROGRESS_INFO_H

#include <string>
#include "nlohmann/json.hpp"
#include "parcel.h"

namespace OHOS {
namespace BackgroundTaskMgr {
static constexpr int32_t jsonFormat_ = 4;
struct ProgressInfo : public Parcelable {
    ProgressInfo() = default;
    ProgressInfo(const std::string &title, const std::string &fileName, int32_t progressValue, bool isMute)
        : title_(title), fileName_(fileName), progressValue_(progressValue), isMute_(isMute) {}
    ~ProgressInfo() = default;

    /**
     * @brief Get title.
     *
     * @return Title string.
     */
    std::string GetTitle() const;

    /**
     * @brief Set title.
     *
     * @param title Title string.
     */
    void SetTitle(const std::string &title);

    /**
     * @brief Get file name.
     *
     * @return File name string.
     */
    std::string GetFileName() const;

    /**
     * @brief Set file name.
     *
     * @param fileName File name string.
     */
    void SetFileName(const std::string &fileName);

    /**
     * @brief Get progress value.
     *
     * @return Progress value.
     */
    int32_t GetProgressValue() const;

    /**
     * @brief Set progress value.
     *
     * @param progressValue Progress value.
     */
    void SetProgressValue(int32_t progressValue);

    /**
     * @brief Check if mute.
     *
     * @return True if mute.
     */
    bool IsMute() const;

    /**
     * @brief Set mute state.
     *
     * @param isMute Mute state.
     */
    void SetIsMute(bool isMute);

    bool Marshalling(Parcel& out) const override;
    static ProgressInfo* Unmarshalling(Parcel& in);

    std::string ParseToJsonStr() const;
    bool ParseFromJson(const nlohmann::json &value);

private:
    bool ReadFromParcel(Parcel& in);

    std::string title_ {""};
    std::string fileName_ {""};
    int32_t progressValue_ {-1};
    bool isMute_ {false};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_PROGRESS_INFO_H
