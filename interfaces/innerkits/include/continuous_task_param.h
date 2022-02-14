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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_PARAMS_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_CONTINUOUS_TASK_PARAMS_H

#include "iremote_object.h"
#include "parcel.h"
#include "want_agent.h"

namespace OHOS {
namespace BackgroundTaskMgr {
struct ContinuousTaskParam : public Parcelable {
    bool isNewApi_ {false};
    uint32_t bgModeId_ {0};
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent_ {nullptr};
    std::string abilityName_ {""};
    sptr<IRemoteObject> abilityToken_ {nullptr};
    
    ContinuousTaskParam() = default;
    ContinuousTaskParam(bool isNewApi, uint32_t bgModeId,
        const std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent, const std::string abilityName,
        const sptr<IRemoteObject> abilityToken)
        : isNewApi_(isNewApi), bgModeId_(bgModeId), wantAgent_(wantAgent),
        abilityName_(abilityName), abilityToken_(abilityToken) {}

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static ContinuousTaskParam *Unmarshalling(Parcel &parcel);
};
}
}

#endif