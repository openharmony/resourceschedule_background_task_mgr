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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_TASK_STATE_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_TASK_STATE_INFO_H

#include <string>
#include "parcel.h"
#include "user_auth_result.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundTaskStateInfo : public Parcelable {
public:
    BackgroundTaskStateInfo() = default;
    BackgroundTaskStateInfo(int32_t userId, const std::string &bundleName, int32_t appIndex, int32_t authResult = 0)
        : userId_(userId), bundleName_(bundleName), appIndex_(appIndex), authResult_(authResult) {}

    static BackgroundTaskStateInfo* Unmarshalling(Parcel& in);
    bool Marshalling(Parcel& out) const override;
    void SetUserId(int32_t userId);
    void SetBundleName(const std::string &bundleName);
    void SetAppIndex(int32_t appIndex);
    void SetUserAuthResult(int32_t authResult);
    int32_t GetUserId() const;
    std::string GetBundleName() const;
    int32_t GetAppIndex() const;
    int32_t GetUserAuthResult() const;

private:
    bool ReadFromParcel(Parcel& in);

    int32_t userId_ {-1};
    std::string bundleName_ {""};
    int32_t appIndex_ {-1};
    int32_t authResult_ {UserAuthResult::NOT_SUPPORTED};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif