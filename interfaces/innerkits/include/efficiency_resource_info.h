/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_EFFICIENCY_RESOURCE_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_EFFICIENCY_RESOURCE_INFO_H

#include <string>
#include <memory>

#include "parcel.h"
namespace OHOS {
namespace BackgroundTaskMgr {
class EfficiencyResourceInfo : public Parcelable {
public:
    EfficiencyResourceInfo() = default;;
    EfficiencyResourceInfo(uint32_t resourceNumber, bool isApply, int32_t timeOut, std::string reason,
        bool isPersist = false, bool isProcess = false) : resourceNumber_(resourceNumber), isApply_(isApply),
        timeOut_(timeOut), reason_(reason), isPersist_(isPersist), isProcess_(isProcess) {}

    /**
     * @brief Unmarshals a purpose from a Parcel.
     *
     * @param parcel Indicates the parcel object for unmarshalling.
     * @return The info of delay suspend.
     */
    static EfficiencyResourceInfo* Unmarshalling(Parcel& in);

    /**
     * @brief Marshals a purpose into a parcel.
     *
     * @param parcel Indicates the parcel object for marshalling.
     * @return True if success, else false.
     */
    bool Marshalling(Parcel& out) const override;

    /**
     * @brief Get the resource number used to represent resource type.
     *
     * @return the resource number.
     */
    inline uint32_t GetResourceNumber() const
    {
        return resourceNumber_;
    }

    /**
     * @brief Get the apply status.
     *
     * @return True if the app begin to use, else false..
     */
    inline bool IsApply() const
    {
        return isApply_;
    }

    /**
     * @brief Get the timeout.
     *
     * @return the timeout.
     */
    inline int32_t GetTimeOut() const
    {
        return timeOut_;
    }

    /**
     * @brief Get the reason.
     *
     * @return the reason.
     */
    inline std::string GetReason() const
    {
        return reason_;
    }
    
    /**
     * @brief persist or not.
     *
     * @return True if persist, else false.
     */
    inline bool IsPersist() const
    {
        return isPersist_;
    }

    /**
     * @brief is process or not.
     *
     * @return True if apply for the process, false if apply for the app.
     */
    inline bool IsProcess() const
    {
        return isProcess_;
    }

    /**
     * @brief Set the Resource Number object
     * 
     * @param resourceNumber resource type
     */
    inline void SetResourceNumber(uint32_t resourceNumber)
    {
        resourceNumber_ = resourceNumber;
    }

private:
    bool ReadFromParcel(Parcel& in);

    uint32_t resourceNumber_;
    bool isApply_;
    int32_t timeOut_;
    std::string reason_;
    bool isPersist_ {false};
    bool isProcess_ {false};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_EFFICIENCY_RESOURCE_INFO_H