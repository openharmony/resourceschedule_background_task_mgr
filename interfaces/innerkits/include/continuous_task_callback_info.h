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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_LONGTIME_TASK_EVENT_DATA_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_LONGTIME_TASK_EVENT_DATA_H

#include<string>

#include "parcel.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class ContinuousTaskCallbackInfo : public Parcelable {
public:
    ContinuousTaskCallbackInfo();
    ContinuousTaskCallbackInfo(uint32_t typeId, int32_t creatorUid, pid_t creatorPid, std::string abilityName);

    /**
     * @brief Get the id of type.
     *
     * @return The id of type.
     */
    uint32_t GetTypeId() const;

    /**
     * @brief Get the uid of notification crector.
     *
     * @return The uid of the notification creator.
     */
    int32_t GetCreatorUid() const;

    /**
     * @brief Get the pid of notification crector.
     *
     * @return The pid of the notification creator.
     */
    pid_t GetCreatorPid() const;

    /**
     * @brief Get the name of ability.
     *
     * @return The name of ability.
     */
    std::string GetAbilityName() const;

    /**
     * @brief Marshals a purpose into a parcel.
     *
     * @param parcel Indicates the parcel object for marshalling.
     * @return True if success, else false.
     */
    bool Marshalling(Parcel &parcel) const;
    static ContinuousTaskCallbackInfo *Unmarshalling(Parcel &parcel);

private:
    bool ReadFromParcel(Parcel &parcel);

private:
    uint32_t typeId_ {0};
    int32_t creatorUid_ {0};
    pid_t creatorPid_ {0};
    std::string abilityName_ {""};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_LONGTIME_TASK_EVENT_DATA_H