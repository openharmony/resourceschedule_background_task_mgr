/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_PARAMS_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_PARAMS_H

#include <string>

#include "background_mode.h"
#include "iremote_object.h"
#include "parcel.h"
#include "refbase.h"
#include "want_agent.h"

namespace OHOS {
namespace BackgroundTaskMgr {
struct ContinuousTaskParam : public Parcelable {
    bool isNewApi_ {false};
    uint32_t bgModeId_ {0};
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent_ {nullptr};
    std::string abilityName_ {""};
    sptr<IRemoteObject> abilityToken_ {nullptr};
    std::string appName_ {""};
    bool isBatchApi_ {false};
    std::vector<uint32_t> bgModeIds_ {};
    int32_t abilityId_ {-1};
    int32_t notificationId_ {-1}; // out
    int32_t continuousTaskId_ {-1}; // out

    ContinuousTaskParam() = default;
    ContinuousTaskParam(bool isNewApi, uint32_t bgModeId,
        const std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent, const std::string abilityName,
        const sptr<IRemoteObject> abilityToken, const std::string &appName, bool isBatchApi = false,
        const std::vector<uint32_t> &bgModeIds = {}, int32_t abilityId = -1)
        : isNewApi_(isNewApi), bgModeId_(bgModeId), wantAgent_(wantAgent), abilityName_(abilityName),
          abilityToken_(abilityToken), appName_(appName), isBatchApi_(isBatchApi), bgModeIds_(bgModeIds),
          abilityId_(abilityId) {
            if (isBatchApi_ && bgModeIds_.size() > 0) {
                auto findNonDataTransfer = [](const auto &target) {
                    return  target != BackgroundMode::DATA_TRANSFER;
                };
                auto iter = std::find_if(bgModeIds_.begin(), bgModeIds_.end(), findNonDataTransfer);
                if (iter != bgModeIds_.end()) {
                    bgModeId_ = *iter;
                } else {
                    bgModeId_ = bgModeIds_[0];
                }
            } else {
                bgModeIds_.push_back(bgModeId);
            }
        }
    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static ContinuousTaskParam *Unmarshalling(Parcel &parcel);
};

struct ContinuousTaskParamForInner : public Parcelable {
    int32_t uid_ {0};
    uint32_t bgModeId_ {0};
    bool isStart_ {false};
    int32_t abilityId_ {-1};
    uint64_t tokenId_ {0};
    int32_t pid_ {0};

    ContinuousTaskParamForInner() = default;
    ContinuousTaskParamForInner(int32_t uid, uint32_t bgModeId, bool isStart, int32_t abilityId = -1,
        uint64_t tokenId = 0)
        : uid_(uid), bgModeId_(bgModeId), isStart_(isStart), abilityId_(abilityId), tokenId_(tokenId) {}

    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static ContinuousTaskParamForInner *Unmarshalling(Parcel &parcel);
    void SetPid(int32_t pid);
    int32_t GetPid();
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_PARAMS_H