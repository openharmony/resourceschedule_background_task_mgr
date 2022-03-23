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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_KEY_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_KEY_INFO_H

#include <string>

namespace OHOS {
namespace BackgroundTaskMgr {
class KeyInfo {
public:
    KeyInfo(const std::string& pkg, const int32_t& uid, const int32_t& pid = -1): pkg_(pkg), uid_(uid), pid_(pid) {}
    ~KeyInfo() = default;

    inline bool operator<(const KeyInfo &o) const
    {
        return (pkg_ < o.GetPkg()) || ((pkg_ == o.GetPkg()) && (uid_ < o.GetUid()));
    }

    inline bool IsEqual(const KeyInfo& other) const
    {
        return (other.GetPkg() == pkg_ && other.GetUid() == uid_);
    }

    inline bool IsEqual(const std::string& pkg, const int32_t& uid) const
    {
        return (pkg == pkg_ && uid == uid_);
    }

    inline const std::string& GetPkg() const
    {
        return pkg_;
    }

    inline int32_t GetUid() const
    {
        return uid_;
    }

    inline int32_t GetPid() const
    {
        return pid_;
    }

    const std::string ToString() const
    {
        return "pkg: " + pkg_ + " uid: " + std::to_string(uid_) + " pid: " + std::to_string(pid_);
    }

private:
    std::string pkg_ {""};
    int32_t uid_ {-1};
    int32_t pid_ {-1};
};

struct KeyInfoComp {
    bool operator()(const std::shared_ptr<KeyInfo> x, const std::shared_ptr<KeyInfo> y) const
    {
        if (x == nullptr || y == nullptr) {
            return false
        }
        return (x->GetPkg() < y->GetPkg()) || ((x->GetPkg() == y->GetPkg()) && (x->GetUid() < y->GetUid()));
    }
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_TRANSIENT_TASK_INCLUDE_KEY_INFO_H