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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_TRANSIENT_TASK_APP_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_TRANSIENT_TASK_APP_INFO_H

#include <string>

#include <message_parcel.h>

namespace OHOS {
namespace BackgroundTaskMgr {
constexpr int INVAILD_PID = -1;
constexpr int INVAILD_UID = -1;

class TransientTaskAppInfo final {
public:
    TransientTaskAppInfo(std::string packageName, int32_t uid, int32_t pid)
        : packageName_(packageName), uid_(uid), pid_(pid) {}
    TransientTaskAppInfo() = default;
    ~TransientTaskAppInfo() = default;

    bool Marshalling(MessageParcel& out) const;
    static std::shared_ptr<TransientTaskAppInfo> Unmarshalling(MessageParcel& in);

    inline std::string& GetPackageName()
    {
        return packageName_;
    }

    inline int32_t GetUid()
    {
        return uid_;
    }

    inline int32_t GetPid()
    {
        return pid_;
    }

private:
    bool ReadFromParcel(MessageParcel& in);

    std::string packageName_;
    int32_t uid_;
    int32_t pid_;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_TRANSIENT_TASK_APP_INFO_H