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
#include "background_mode.h"
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
const char *g_continuousTaskModeName[11] = {
    "dataTransfer",
    "audioPlayback",
    "audioRecording",
    "location",
    "bluetoothInteraction",
    "multiDeviceConnection",
    "wifiInteraction",
    "voip",
    "taskKeeping",
    "workout",
    "default",
};

ContinuousTaskRecord::ContinuousTaskRecord(const std::string &bundleName, const std::string &abilityName, int32_t uid,
    int32_t pid, uint32_t bgModeId, bool isBatchApi, const std::vector<uint32_t> &bgModeIds, int32_t abilityId)
    : bundleName_(bundleName), abilityName_(abilityName), uid_(uid), pid_(pid), bgModeId_(bgModeId),
      isBatchApi_(isBatchApi), bgModeIds_(bgModeIds), abilityId_(abilityId) {
    if (isBatchApi_) {
        auto findNonDataTransfer = [](const auto &target) {
            return  target != BackgroundMode::DATA_TRANSFER;
        };
        auto iter = std::find_if(bgModeIds_.begin(), bgModeIds_.end(), findNonDataTransfer);
        if (iter != bgModeIds_.end()) {
            bgModeId_ = *iter;
            BGTASK_LOGI("batch api, find non-datatransfer mode, set %{public}d", bgModeId_);
        } else {
            bgModeId_ = bgModeIds_[0];
        }
    } else {
        bgModeIds_.push_back(bgModeId);
    }
}

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

bool ContinuousTaskRecord::IsFromWebview() const
{
    return isFromWebview_;
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

int32_t ContinuousTaskRecord::GetNotificationId() const
{
    return notificationId_;
}

std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> ContinuousTaskRecord::GetWantAgent() const
{
    return wantAgent_;
}

std::string ContinuousTaskRecord::ToString(std::vector<uint32_t> &bgmodes)
{
    std::string result;
    for (auto it : bgmodes) {
        result += std::to_string(it);
        result += ",";
    }
    return result;
}

std::vector<uint32_t> ContinuousTaskRecord::ToVector(std::string &str)
{
    std::vector<std::string> stringTokens;
    std::vector<uint32_t> modeTokens;
    OHOS::SplitStr(str, ",", stringTokens);
    for (auto mode : stringTokens) {
        modeTokens.push_back(std::atoi(mode.c_str()));
    }
    return modeTokens;
}

int32_t ContinuousTaskRecord::GetAbilityId() const
{
    return abilityId_;
}

bool ContinuousTaskRecord::IsSystem() const
{
    return isSystem_;
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
    root["isFromWebview"] = isFromWebview_;
    root["notificationLabel"] = notificationLabel_;
    root["notificationId"] = notificationId_;
    root["isBatchApi"] = isBatchApi_;
    root["bgModeIds"] = ToString(bgModeIds_);
    root["isSystem"] = isSystem_;
    if (wantAgentInfo_ != nullptr) {
        nlohmann::json info;
        info["bundleName"] = wantAgentInfo_->bundleName_;
        info["abilityName"] = wantAgentInfo_->abilityName_;
        root["wantAgentInfo"] = info;
    }
    root["continuousTaskId"] = continuousTaskId_;
    root["abilityId"] = abilityId_;
    return root.dump(CommonUtils::jsonFormat_);
}

bool CheckContinuousRecod(const nlohmann::json &value)
{
    return !value["bundleName"].is_string() || !value["abilityName"].is_string()
        || !value["userId"].is_number_integer() || !value["uid"].is_number_integer()
        || !value["pid"].is_number_integer() || !value["bgModeId"].is_number_integer()
        || !value["isNewApi"].is_boolean() || !value["isFromWebview"].is_boolean()
        || !value["notificationLabel"].is_string() || !value["isSystem"].is_boolean()
        || !value["continuousTaskId"].is_number_integer()
        || !value["abilityId"].is_number_integer();
}

bool ContinuousTaskRecord::ParseFromJson(const nlohmann::json &value)
{
    if (value.is_null() || !value.is_object() || !CommonUtils::CheckJsonValue(value, { "bundleName",
        "abilityName", "userId", "uid", "pid", "bgModeId", "isNewApi", "isFromWebview", "notificationLabel",
        "isSystem", "continuousTaskId", "abilityId"})) {
        BGTASK_LOGE("continuoustaskrecord no key");
        return false;
    }
    if (CheckContinuousRecod(value)) {
        BGTASK_LOGE("continuoustaskrecord parse from json fail");
        return false;
    }
    this->bundleName_ = value.at("bundleName").get<std::string>();
    this->abilityName_ = value.at("abilityName").get<std::string>();
    this->userId_ = value.at("userId").get<int32_t>();
    this->uid_ = value.at("uid").get<int32_t>();
    this->pid_ = value.at("pid").get<int32_t>();
    this->bgModeId_ = value.at("bgModeId").get<uint32_t>();
    this->isNewApi_ = value.at("isNewApi").get<bool>();
    this->isFromWebview_ = value.at("isFromWebview").get<bool>();
    this->notificationLabel_ = value.at("notificationLabel").get<std::string>();
    this->isSystem_ = value.at("isSystem").get<bool>();
    this->continuousTaskId_ = value.at("continuousTaskId").get<int32_t>();
    this->abilityId_ = value.at("abilityId").get<int32_t>();
    if (value.contains("isBatchApi") && value["isBatchApi"].is_boolean()) {
        this->isBatchApi_ = value.at("isBatchApi").get<bool>();
    }
    if (value.contains("bgModeIds") && value["bgModeIds"].is_string()) {
        auto modes = value.at("bgModeIds").get<std::string>();
        this->bgModeIds_ = ToVector(modes);
    }
    if (value.find("wantAgentInfo") != value.end()) {
        nlohmann::json infoVal = value["wantAgentInfo"];
        if (!CommonUtils::CheckJsonValue(infoVal, { "bundleName", "abilityName" })
            || !infoVal["bundleName"].is_string() || !infoVal["abilityName"].is_string()) {
            return false;
        }
        std::shared_ptr<WantAgentInfo> info = std::make_shared<WantAgentInfo>();
        info->bundleName_ = infoVal.at("bundleName").get<std::string>();
        info->abilityName_ = infoVal.at("abilityName").get<std::string>();
        this->wantAgentInfo_ = info;
    }
    if (value.contains("notificationId") && value["notificationId"].is_number_integer()) {
        this->notificationId_ = value.at("notificationId").get<int32_t>();
    }
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS