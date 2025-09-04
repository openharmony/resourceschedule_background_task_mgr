/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_REQUEST_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_REQUEST_H

#include <memory>

#include "parcel.h"
#include "want_agent.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class ContinuousTaskRequest : public std::enable_shared_from_this<ContinuousTaskRequest>, public Parcelable {
public:
    ~ContinuousTaskRequest() = default;
    ContinuousTaskRequest(const std::vector<uint32_t> &continuousTaskModes = {},
        const std::vector<uint32_t> &continuousTaskSubmodes = {},
        const std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent = nullptr,
        bool combinedTaskNotification = false, int32_t continuousTaskId = -1, bool isBuildByRequest = false)
        : continuousTaskModes_(continuousTaskModes), continuousTaskSubmodes_(continuousTaskSubmodes),
        wantAgent_(wantAgent), combinedTaskNotification_(combinedTaskNotification),
        continuousTaskId_(continuousTaskId), isBuildByRequest_(isBuildByRequest) {}
    /**
     * @brief Unmarshals a purpose from a Parcel.
     *
     * @param in Indicates the parcel object for unmarshalling.
     * @return The info continuous task request.
     */
    static ContinuousTaskRequest* Unmarshalling(Parcel& in);

    /**
     * @brief Marshals a purpose into a parcel.
     *
     * @param out indicates the parcel object for marshalling.
     * @return True if success, else false.
     */
    bool Marshalling(Parcel& out) const override;

    /**
     * @brief Get WantAgent.
     *
     * @return WantAgent.
     */
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> GetWantAgent() const;

    /**
     * @brief Get ContinuousTaskModes.
     *
     * @return ContinuousTaskModes.
     */
    std::vector<uint32_t> GetContinuousTaskModes() const;

    /**
     * @brief Get ContinuousTaskSubmodes.
     *
     * @return ContinuousTaskSubmodes.
     */
    std::vector<uint32_t> GetContinuousTaskSubmodes() const;

    /**
     * @brief IsBuildByRequest.
     *
     * @return IsBuildByRequest.
     */
    bool IsBuildByRequest() const;

    /**
     * @brief Set IsBuildByRequest.
     *
     * @param isBuildByRequest.
     */
    void SetIsBuildByRequest(bool isBuildByRequest);

    /**
     * @brief Get continuous task Id.
     *
     * @return continuous task Id
     */
    int32_t GetContinuousTaskId() const;

    void SetContinuousTaskId(const int32_t continuousTaskId);
    void SetCombinedTaskNotification(const bool combinedTaskNotification);
    void SetWantAgent(const std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent);
    bool IsCombinedTaskNotification() const;
    void SetContinuousTaskMode(const std::vector<uint32_t> &continuousTaskMode);
    void SetContinuousTaskSubMode(const std::vector<uint32_t> &continuousTaskSubMode);
private:
    bool ReadFromParcel(Parcel& in);

    std::vector<uint32_t> continuousTaskModes_ {};
    std::vector<uint32_t> continuousTaskSubmodes_ {};
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent_ {nullptr};
    bool combinedTaskNotification_ {false};
    int32_t continuousTaskId_ {-1};
    bool isBuildByRequest_ {false};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_REQUEST_H