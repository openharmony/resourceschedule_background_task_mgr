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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_DATA_TRANSFER_PROGRESS_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_DATA_TRANSFER_PROGRESS_H

#include <memory>

#include "nlohmann/json.hpp"
#include "parcel.h"
#include "progress_info.h"
#include "want_agent.h"

namespace OHOS {
namespace BackgroundTaskMgr {
struct DataTransferProgress : public Parcelable {
    DataTransferProgress() = default;
    DataTransferProgress(int32_t continuousTaskId,
        const std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> &wantAgent,
        const std::shared_ptr<ProgressInfo> &progressInfo)
        : continuousTaskId_(continuousTaskId), wantAgent_(wantAgent), progressInfo_(progressInfo) {}
    ~DataTransferProgress() = default;

    /**
     * @brief Get continuous task id.
     *
     * @return Continuous task id.
     */
    int32_t GetContinuousTaskId() const;

    /**
     * @brief Set continuous task id.
     *
     * @param continuousTaskId Continuous task id.
     */
    void SetContinuousTaskId(int32_t continuousTaskId);

    /**
     * @brief Get want agent.
     *
     * @return Want agent.
     */
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> GetWantAgent() const;

    /**
     * @brief Set want agent.
     *
     * @param wantAgent Want agent.
     */
    void SetWantAgent(const std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent);

    /**
     * @brief Get progress info.
     *
     * @return Progress info.
     */
    std::shared_ptr<ProgressInfo> GetProgressInfo() const;

    /**
     * @brief Set progress info.
     *
     * @param progressInfo Progress info.
     */
    void SetProgressInfo(const std::shared_ptr<ProgressInfo> progressInfo);

    bool Marshalling(Parcel& out) const override;
    static DataTransferProgress* Unmarshalling(Parcel& in);

    std::string ParseToJsonStr() const;
    bool ParseFromJson(const nlohmann::json &value);

private:
    bool ReadFromParcel(Parcel& in);

    int32_t continuousTaskId_ {-1};
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent_ {nullptr};
    std::shared_ptr<ProgressInfo> progressInfo_ {nullptr};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_DATA_TRANSFER_PROGRESS_H
