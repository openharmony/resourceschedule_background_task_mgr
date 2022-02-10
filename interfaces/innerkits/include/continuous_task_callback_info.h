/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
    ContinuousTaskCallbackInfo(int32_t typeId, uid_t creatorUid, pid_t creatorPid, std::string abilityName);

    int32_t GetTypeId();
    uid_t GetCreatorUid();
    pid_t GetCreatorPid();
    std::string GetAbilityName();
    bool Marshalling(Parcel &parcel) const;
    static ContinuousTaskCallbackInfo *Unmarshalling(Parcel &parcel);

private:
    bool ReadFromParcel(Parcel &parcel);

private:
    int32_t typeId_ ;
    uid_t creatorUid_ ;
    pid_t creatorPid_ ;
    std::string abilityName_ ;
};
}
}
#endif