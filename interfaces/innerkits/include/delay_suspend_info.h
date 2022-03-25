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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_DELAY_SUSPEND_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_DELAY_SUSPEND_INFO_H

#include <message_parcel.h>

namespace OHOS {
namespace BackgroundTaskMgr {
class DelaySuspendInfo : public Parcelable {
public:
    
    /**
     * @brief Unmarshals a purpose from a Parcel.
     *
     * @param parcel Indicates the parcel object for unmarshalling.
     * @return DelaySuspendInfo.
     */
    static std::shared_ptr<DelaySuspendInfo> Unmarshalling(Parcel& in);

    /**
     * @brief Marshals a purpose into a parcel.
     *
     * @param parcel Indicates the parcel object for marshalling.
     * @return True if success, else false.
     */
    bool Marshalling(Parcel& out) const override;

    /**
     * @brief Judge whether the request id is same.
     *
     * @param requestId Id use to judge.
     * @return True if success, else false.
     */
    inline bool IsSameRequestId(int32_t requestId) const
    {
        return requestId_ == requestId;
    }
    
    /**
     * @brief Get the request id.
     *
     * @return Request id.
     */
    inline int32_t GetRequestId() const
    {
        return requestId_;
    }

    /**
     * @brief Get delay time.
     *
     * @return The delay time.
     */
    inline int32_t GetActualDelayTime() const
    {
        return actualDelayTime_;
    }

    /**
     * @brief Set id of the request.
     *
     * @param id Id of the request.
     */
    inline void SetRequestId(int32_t id)
    {
        requestId_ = id;
    }

    /**
     * @brief Set the actual delay time. 
     *
     * @param time The delay time
     */
    inline void SetActualDelayTime(int32_t time)
    {
        actualDelayTime_ = time;
    }

private:
    bool ReadFromParcel(Parcel& in);

    int32_t requestId_ {-1};
    int32_t actualDelayTime_ {0};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_DELAY_SUSPEND_INFO_H