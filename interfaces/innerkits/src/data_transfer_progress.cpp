/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "data_transfer_progress.h"
#include "bgtaskmgr_log_wrapper.h"

namespace OHOS {
namespace BackgroundTaskMgr {

int32_t DataTransferProgress::GetContinuousTaskId() const
{
    return continuousTaskId_;
}

void DataTransferProgress::SetContinuousTaskId(int32_t continuousTaskId)
{
    continuousTaskId_ = continuousTaskId;
}

std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> DataTransferProgress::GetWantAgent() const
{
    return wantAgent_;
}

void DataTransferProgress::SetWantAgent(
    const std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent)
{
    wantAgent_ = wantAgent;
}

std::shared_ptr<ProgressInfo> DataTransferProgress::GetProgressInfo() const
{
    return progressInfo_;
}

void DataTransferProgress::SetProgressInfo(const std::shared_ptr<ProgressInfo> progressInfo)
{
    progressInfo_ = progressInfo;
}

bool DataTransferProgress::Marshalling(Parcel& out) const
{
    if (!out.WriteInt32(continuousTaskId_)) {
        BGTASK_LOGE("Failed to write continuousTaskId");
        return false;
    }
    bool valid = wantAgent_ != nullptr;
    if (!out.WriteBool(valid)) {
        BGTASK_LOGE("Failed to write the flag which indicate whether wantAgent is null");
        return false;
    }
    if (valid) {
        if (!out.WriteParcelable(wantAgent_.get())) {
            BGTASK_LOGE("Failed to write wantAgent");
            return false;
        }
    }
    bool progressValid = progressInfo_ != nullptr;
    if (!out.WriteBool(progressValid)) {
        BGTASK_LOGE("Failed to write progressInfo valid flag");
        return false;
    }
    if (progressValid) {
        if (!out.WriteParcelable(progressInfo_.get())) {
            BGTASK_LOGE("Failed to write progressInfo");
            return false;
        }
    }
    return true;
}

DataTransferProgress* DataTransferProgress::Unmarshalling(Parcel& in)
{
    DataTransferProgress* info = new (std::nothrow) DataTransferProgress();
    if (info && !info->ReadFromParcel(in)) {
        BGTASK_LOGE("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool DataTransferProgress::ReadFromParcel(Parcel& in)
{
    if (!in.ReadInt32(continuousTaskId_)) {
        BGTASK_LOGE("read parcel continuousTaskId error");
        return false;
    }
    bool valid = in.ReadBool();
    if (valid) {
        wantAgent_ = std::shared_ptr<AbilityRuntime::WantAgent::WantAgent>(
            in.ReadParcelable<AbilityRuntime::WantAgent::WantAgent>());
        if (!wantAgent_) {
            BGTASK_LOGE("read parcel wantAgent error");
            return false;
        }
    }
    bool progressValid = in.ReadBool();
    if (progressValid) {
        progressInfo_ = std::shared_ptr<ProgressInfo>(in.ReadParcelable<ProgressInfo>());
        if (!progressInfo_) {
            BGTASK_LOGE("read parcel progressInfo error");
            return false;
        }
    }
    return true;
}

std::string DataTransferProgress::ParseToJsonStr() const
{
    nlohmann::json root;
    root["continuousTaskId"] = continuousTaskId_;
    if (progressInfo_ != nullptr) {
        nlohmann::json progressJson = nlohmann::json::parse(progressInfo_->ParseToJsonStr());
        if (!progressJson.is_discarded()) {
            root["progressInfo"] = progressJson;
        }
    }
    return root.dump(jsonFormat_, ' ', false, nlohmann::json::error_handler_t::replace);
}

bool DataTransferProgress::ParseFromJson(const nlohmann::json &value)
{
    if (value.is_null() || !value.is_object()) {
        BGTASK_LOGE("DataTransferProgress json is invalid");
        return false;
    }
    if (!value["continuousTaskId"].is_number_integer()) {
        BGTASK_LOGE("DataTransferProgress json field type mismatch");
        return false;
    }
    continuousTaskId_ = value.at("continuousTaskId").get<int32_t>();
    if (value.contains("progressInfo") && value["progressInfo"].is_object()) {
        auto progressInfo = std::make_shared<ProgressInfo>();
        if (!progressInfo->ParseFromJson(value["progressInfo"])) {
            BGTASK_LOGE("parse progressInfo from json failed");
            return false;
        }
        progressInfo_ = progressInfo;
    }
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
