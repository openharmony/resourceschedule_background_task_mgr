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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_INTERFACES_INNERKITS_TRANSIENT_TASK_INCLUDE_DELAY_SUSPEND_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_INTERFACES_INNERKITS_TRANSIENT_TASK_INCLUDE_DELAY_SUSPEND_INFO_H

#include <message_parcel.h>

namespace OHOS {
namespace BackgroundTaskMgr {
class DelaySuspendInfo : public Parcelable {
public:
    static std::shared_ptr<DelaySuspendInfo> Unmarshalling(Parcel& in);

    bool Marshalling(Parcel& out) const override;

    inline bool IsSameRequestId(int32_t requestId) const { return requestId_ == requestId; }
    inline int32_t GetRequestId() const { return requestId_; }
    inline int32_t GetActualDelayTime() const { return actualDelayTime_; }
    inline void SetRequestId(int32_t id) { requestId_ = id; }
    inline void SetActualDelayTime(int32_t time) { actualDelayTime_ = time; }

private:
    bool ReadFromParcel(Parcel& in);

    int32_t requestId_{-1};
    int32_t actualDelayTime_{0};
};
} // namespace BackgroundTaskMgr
} // namespace OHOS
#endif