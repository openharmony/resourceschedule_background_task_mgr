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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_INFO_H

#include <string>
#include <memory>

#include "parcel.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class ContinuousTaskInfo : public Parcelable {
public:
    ContinuousTaskInfo() = default;
    ContinuousTaskInfo(std::string abilityName, int32_t uid, int32_t pid, bool isFromWebView,
        std::vector<uint32_t> backgroundModes, std::vector<uint32_t> backgroundSubModes, int32_t notificationId,
        int32_t continuousTaskId, int32_t abilityId, std::string wantAgentBundleName, std::string wantAgentAbilityName)
        : abilityName_(abilityName), uid_(uid), pid_(pid), isFromWebView_(isFromWebView),
        backgroundModes_(backgroundModes), backgroundSubModes_(backgroundSubModes), notificationId_(notificationId),
        continuousTaskId_(continuousTaskId), abilityId_(abilityId), wantAgentBundleName_(wantAgentBundleName),
        wantAgentAbilityName_(wantAgentAbilityName) {}

    /**
     * @brief Unmarshals a purpose from a Parcel.
     *
     * @param parcel Indicates the parcel object for unmarshalling.
     * @return The info continuous task.
     */
    static ContinuousTaskInfo* Unmarshalling(Parcel& in);

    /**
     * @brief Marshals a purpose into a parcel.
     *
     * @param parcel Indicates the parcel object for marshalling.
     * @return True if success, else false.
     */
    bool Marshalling(Parcel& out) const override;

    /**
     * @brief Get the ability name.
     *
     * @return the ability name.
     */
    std::string GetAbilityName() const;

    /**
     * @brief Get the want agent bundle name.
     *
     * @return the want agent bundle name.
     */
    std::string GetWantAgentBundleName() const;

    /**
     * @brief Get the want agent ability name.
     *
     * @return the want agent ability name.
     */
    std::string GetWantAgentAbilityName() const;
    /**
     * @brief Get the uid.
     *
     * @return the uid.
     */
    int32_t GetUid() const;

    /**
     * @brief Get the pid.
     *
     * @return the pid.
     */
    int32_t GetPid() const;

    /**
     * @brief Get the notificationId.
     *
     * @return the notification Id.
     */
    int32_t GetNotificationId() const;

    /**
     * @brief Get the continuousTaskId.
     *
     * @return the continuousTask Id.
     */
    int32_t GetContinuousTaskId() const;

    /**
     * @brief Get the abilityId.
     *
     * @return the ability Id.
     */
    int32_t GetAbilityId() const;

    /**
     * @brief Get the backgroundModes.
     *
     * @return the background modes.
     */
    std::vector<uint32_t> GetBackgroundModes() const;
    /**
     * @brief Get the backgroundSubModes_.
     *
     * @return the background sub modes.
     */
    std::vector<uint32_t> GetBackgroundSubModes() const;

    /**
     * @brief Get the isFromWebView.
     *
     * @return the is from webview.
     */
    bool IsFromWebView() const;

    /**
     * @brief Get suspendState.
     *
     * @return the suspend state.
     */
    bool GetSuspendState() const;

    std::string ToString(const std::vector<uint32_t> &modes) const;

private:
    bool ReadFromParcel(Parcel& in);

    std::string abilityName_ {""};
    int32_t uid_ {0};
    int32_t pid_ {0};
    bool isFromWebView_ {false};
    std::vector<uint32_t> backgroundModes_ {};
    std::vector<uint32_t> backgroundSubModes_ {};
    int32_t notificationId_ {-1};
    int32_t continuousTaskId_ {-1};
    int32_t abilityId_ {-1};
    std::string wantAgentBundleName_ {""};
    std::string wantAgentAbilityName_ {""};
    bool suspendState_ {false};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_INFO_H