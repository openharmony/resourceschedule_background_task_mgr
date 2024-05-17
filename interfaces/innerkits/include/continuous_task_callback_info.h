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

#include <cstdint>
#include <sys/types.h>
#include <string>
#include "parcel.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class ContinuousTaskCallbackInfo : public Parcelable {
public:
    ContinuousTaskCallbackInfo();
    ContinuousTaskCallbackInfo(uint32_t typeId, int32_t creatorUid,
        pid_t creatorPid, std::string abilityName, bool isFromWebview = false, bool isBatchApi = false,
        const std::vector<uint32_t> &typeIds = {}, int32_t abilityId = -1, uint64_t tokenId = 0)
        : typeId_(typeId), creatorUid_(creatorUid), creatorPid_(creatorPid), abilityName_(abilityName),
          isFromWebview_(isFromWebview), isBatchApi_(isBatchApi), typeIds_(typeIds), abilityId_(abilityId),
          tokenId_(tokenId) {}

    /**
     * @brief Get the id of type.
     *
     * @return The id of type.
     */
    uint32_t GetTypeId() const;

    /**
     * @brief Get the uid of notification creator.
     *
     * @return The uid of the notification creator.
     */
    int32_t GetCreatorUid() const;

    /**
     * @brief Get the pid of notification creator.
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
     * @brief Judge whether this ability come from webview.
     *
     * @return True if success, else false.
     */
    bool IsFromWebview() const;

    /**
     * @brief Get the id of types.
     *
     * @return The id of types.
     */
    const std::vector<uint32_t>& GetTypeIds() const;

    /**
     * @brief Get the batch api flag.
     *
     * @return The flag of batch api.
     */
    bool IsBatchApi() const;

    /**
     * @brief Get the id of ability.
     *
     * @return The id of ability.
     */
    int GetAbilityId() const;

    /**
     * @brief Get the tokenId.
     *
     * @return The tokenId.
     */
    uint64_t GetTokenId() const;

    /**
     * @brief Marshals a purpose into a parcel.
     *
     * @param parcel Indicates the parcel object for marshalling.
     * @return True if success, else false.
     */
    bool Marshalling(Parcel &parcel) const override;

    static ContinuousTaskCallbackInfo *Unmarshalling(Parcel &parcel);

private:
    bool ReadFromParcel(Parcel &parcel);

private:
    uint32_t typeId_ {0};
    int32_t creatorUid_ {0};
    pid_t creatorPid_ {0};
    std::string abilityName_ {""};
    bool isFromWebview_ {false};
    bool isBatchApi_ {false};
    std::vector<uint32_t> typeIds_ {};
    int32_t abilityId_ {-1};
    uint64_t tokenId_ {0};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_LONGTIME_TASK_EVENT_DATA_H