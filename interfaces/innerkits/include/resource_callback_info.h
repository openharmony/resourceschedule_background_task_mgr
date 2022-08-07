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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_RESOURCE_CALLBACK_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_RESOURCE_CALLBACK_INFO_H

#include <cstdint>
#include <memory>
#include <string>

#include <message_parcel.h>

namespace OHOS {
namespace BackgroundTaskMgr {
constexpr int32_t INVAILD_PID = -1;
constexpr int32_t INVAILD_UID = -1;

class ResourceCallbackInfo : public Parcelable  {
public:
    ResourceCallbackInfo() = default;
    ResourceCallbackInfo(int32_t uid, int32_t pid, uint32_t resourceNumber, std::string bundleName) :
        uid_(uid), pid_(pid), resourceNumber_(resourceNumber), bundleName_(bundleName) {}
    ~ResourceCallbackInfo() = default;

    /**
     * @brief Marshals a purpose into a parcel.
     *
     * @param parcel Indicates the parcel object for marshalling.
     * @return True if success, else false.
     */
    bool Marshalling(MessageParcel& out) const;

    /**
     * @brief Unmarshals a purpose from a Parcel.
     *
     * @param parcel Indicates the MessageParcel object for unmarshalling.
     * @return App info of transient task.
     */
    static std::shared_ptr<ResourceCallbackInfo> Unmarshalling(MessageParcel& in);

    /**
     * @brief Get the uid.
     *
     * @return The uid of app.
     */
    inline int32_t GetUid()
    {
        return uid_;
    }

    /**
     * @brief Get the uid.
     *
     * @return The pid of app.
     */
    inline int32_t GetPid()
    {
        return pid_;
    }

    /**
     * @brief Get the resourceNumber.
     *
     * @return The type of the resources.
     */
    inline uint32_t GetResourceNumber()
    {
        return resourceNumber_;
    }

   /**
     * @brief Get the bundleName_.
     *
     * @return The bundle name of the app.
     */
    inline std::string GetBundleName()
    {
        return bundleName_;
    }
private:
    bool ReadFromParcel(MessageParcel& in);

    int32_t uid_ {0};
    int32_t pid_ {0};
    uint32_t resourceNumber_ {0};
    std::string bundleName_ {""};
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_RESOURCE_CALLBACK_INFO_H