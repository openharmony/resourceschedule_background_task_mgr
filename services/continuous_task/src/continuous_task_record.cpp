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

#include "iremote_object.h"
#include "json/value.h"

namespace OHOS {
namespace BackgroundTaskMgr {
const char *ContinuousTaskModeName[10] = {
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

ContinuousTaskRecord::ContinuousTaskRecord(const std::string &bundleName, const std::string &abilityName,
    const std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> &wantAgent, int32_t userId, int32_t uid,
    pid_t pid, uint32_t bgModeId, bool isNewApi, const std::string &appName)
    : bundleName_(bundleName), abilityName_(abilityName), wantAgent_(wantAgent), userId_(userId),
    uid_(uid), pid_(pid), bgModeId_(bgModeId), isNewApi_(isNewApi), appName_(appName) {}

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
    Json::Value root;
    root["bundleName"] = bundleName_;
    root["abilityName"] = abilityName_;
    root["userId"] = userId_;
    root["uid"] = uid_;
    root["pid"] = pid_;
    root["bgModeId"] = bgModeId_;
    root["isNewApi"] = isNewApi_;
    root["notificationLabel"] = notificationLabel_;
    if (wantAgentInfo_ != nullptr) {
        Json::Value info;
        info["bundleName"] = wantAgentInfo_->bundleName_;
        info["abilityName"] = wantAgentInfo_->abilityName_;
        root["wantAgentInfo"] = info;
    }
    Json::StreamWriterBuilder writerBuilder;
    std::ostringstream os;
    std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
    jsonWriter->write(root, &os);
    std::string result = os.str();
    return result;
}

bool ContinuousTaskRecord::ParseFromJson(const Json::Value value)
{
    if (value.empty()) {
        return false;
    }
    this->bundleName_ = value["bundleName"].asString();
    this->abilityName_ = value["abilityName"].asString();
    this->userId_ = value["userId"].asInt();
    this->uid_ = value["uid"].asInt();
    this->pid_ = value["pid"].asInt();
    this->bgModeId_ = value["bgModeId"].asUInt();
    this->isNewApi_ = value["isNewApi"].asBool();
    this->notificationLabel_ = value["notificationLabel"].asString();

    if (value.isMember("wantAgentInfo")) {
        Json::Value infoVal = value["wantAgentInfo"];
        std::shared_ptr<WantAgentInfo> info = std::make_shared<WantAgentInfo>();
        info->bundleName_ = infoVal["bundleName"].asString();
        info->abilityName_ = infoVal["abilityName"].asString();
        this->wantAgentInfo_ = info;
    }
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS