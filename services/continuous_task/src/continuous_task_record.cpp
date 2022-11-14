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

#include "continuous_task_record.h"

#include "common_utils.h"
#include "iremote_object.h"

namespace OHOS {
namespace BackgroundTaskMgr {
const char *g_continuousTaskModeName[10] = {
    "dataTransfer",
    "audioPlayback",
    "audioRecording",
    "location",
    "bluetoothInteraction",
    "multiDeviceConnection",
    "wifiInteraction",
    "voip",
    "taskKeeping",
    "default",
};

std::string ContinuousTaskRecord::GetBundleName() const
{
    return bundleName_;
}

std::string ContinuousTaskRecord::GetAbilityName() const
{
    return abilityName_;
}

bool ContinuousTaskRecord::IsNewApi() const
{
    return isNewApi_;
}

uint32_t ContinuousTaskRecord::GetBgModeId() const
{
    return bgModeId_;
}

int32_t ContinuousTaskRecord::GetUserId() const
{
    return userId_;
}

int32_t ContinuousTaskRecord::GetUid() const
{
    return uid_;
}

pid_t ContinuousTaskRecord::GetPid() const
{
    return pid_;
}

std::string ContinuousTaskRecord::GetNotificationLabel() const
{
    return notificationLabel_;
}

std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> ContinuousTaskRecord::GetWantAgent() const
{
    return wantAgent_;
}

std::string ContinuousTaskRecord::ParseToJsonStr()
{
    nlohmann::json root;
    root["bundleName"] = bundleName_;
    root["abilityName"] = abilityName_;
    root["userId"] = userId_;
    root["uid"] = uid_;
    root["pid"] = pid_;
    root["bgModeId"] = bgModeId_;
    root["isNewApi"] = isNewApi_;
    root["notificationLabel"] = notificationLabel_;
    if (wantAgentInfo_ != nullptr) {
        nlohmann::json info;
        info["bundleName"] = wantAgentInfo_->bundleName_;
        info["abilityName"] = wantAgentInfo_->abilityName_;
        root["wantAgentInfo"] = info;
    }
    return root.dump(CommonUtils::jsonFormat_);
}

bool ContinuousTaskRecord::ParseFromJson(const nlohmann::json &value)
{
    if (value.is_null() || !value.is_object() || !CommonUtils::CheckJsonValue(value, { "bundleName",
        "abilityName", "userId", "uid", "pid", "bgModeId", "isNewApi", "notificationLabel" })) {
        return false;
    }
    this->bundleName_ = value.at("bundleName").get<std::string>();
    this->abilityName_ = value.at("abilityName").get<std::string>();
    this->userId_ = value.at("userId").get<int32_t>();
    this->uid_ = value.at("uid").get<int32_t>();
    this->pid_ = value.at("pid").get<int32_t>();
    this->bgModeId_ = value.at("bgModeId").get<uint32_t>();
    this->isNewApi_ = value.at("isNewApi").get<bool>();
    this->notificationLabel_ = value.at("notificationLabel").get<std::string>();

    if (value.find("wantAgentInfo") != value.end()) {
        nlohmann::json infoVal = value["wantAgentInfo"];
        if (!CommonUtils::CheckJsonValue(infoVal, { "bundleName", "abilityName" })) {
            return false;
        }
        std::shared_ptr<WantAgentInfo> info = std::make_shared<WantAgentInfo>();
        info->bundleName_ = infoVal.at("bundleName").get<std::string>();
        info->abilityName_ = infoVal.at("abilityName").get<std::string>();
        this->wantAgentInfo_ = info;
    }
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS