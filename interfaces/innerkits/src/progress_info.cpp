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

#include "progress_info.h"

#include "bgtaskmgr_log_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace BackgroundTaskMgr {

std::string ProgressInfo::GetTitle() const
{
    return title_;
}

void ProgressInfo::SetTitle(const std::string &title)
{
    title_ = title;
}

std::string ProgressInfo::GetFileName() const
{
    return fileName_;
}

void ProgressInfo::SetFileName(const std::string &fileName)
{
    fileName_ = fileName;
}

int32_t ProgressInfo::GetProgressValue() const
{
    return progressValue_;
}

void ProgressInfo::SetProgressValue(int32_t progressValue)
{
    progressValue_ = progressValue;
}

bool ProgressInfo::IsMute() const
{
    return isMute_;
}

void ProgressInfo::SetIsMute(bool isMute)
{
    isMute_ = isMute;
}

bool ProgressInfo::Marshalling(Parcel& out) const
{
    if (!out.WriteString16(Str8ToStr16(title_))) {
        BGTASK_LOGE("Failed to write title");
        return false;
    }
    if (!out.WriteString16(Str8ToStr16(fileName_))) {
        BGTASK_LOGE("Failed to write fileName");
        return false;
    }
    if (!out.WriteInt32(progressValue_)) {
        BGTASK_LOGE("Failed to write progressValue");
        return false;
    }
    if (!out.WriteBool(isMute_)) {
        BGTASK_LOGE("Failed to write isMute");
        return false;
    }
    return true;
}

ProgressInfo* ProgressInfo::Unmarshalling(Parcel& in)
{
    ProgressInfo* info = new (std::nothrow) ProgressInfo();
    if (info && !info->ReadFromParcel(in)) {
        BGTASK_LOGE("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool ProgressInfo::ReadFromParcel(Parcel& in)
{
    std::u16string u16Title;
    if (!in.ReadString16(u16Title)) {
        BGTASK_LOGE("read parcel title error");
        return false;
    }
    title_ = Str16ToStr8(u16Title);

    std::u16string u16FileName;
    if (!in.ReadString16(u16FileName)) {
        BGTASK_LOGE("read parcel fileName error");
        return false;
    }
    fileName_ = Str16ToStr8(u16FileName);

    if (!in.ReadInt32(progressValue_)) {
        BGTASK_LOGE("read parcel progressValue error");
        return false;
    }
    if (!in.ReadBool(isMute_)) {
        BGTASK_LOGE("read parcel isMute error");
        return false;
    }
    return true;
}

std::string ProgressInfo::ParseToJsonStr() const
{
    nlohmann::json root;
    root["title"] = title_;
    root["fileName"] = fileName_;
    root["progressValue"] = progressValue_;
    root["isMute"] = isMute_;
    return root.dump();
}

bool ProgressInfo::ParseFromJson(const nlohmann::json &value)
{
    if (value.is_null() || !value.is_object()) {
        BGTASK_LOGE("progressInfo json is invalid");
        return false;
    }
    if (!value["title"].is_string() || !value["fileName"].is_string() ||
        !value["progressValue"].is_number_integer() || !value["isMute"].is_boolean()) {
        BGTASK_LOGE("progressInfo json field type mismatch");
        return false;
    }
    title_ = value.at("title").get<std::string>();
    fileName_ = value.at("fileName").get<std::string>();
    progressValue_ = value.at("progressValue").get<int32_t>();
    isMute_ = value.at("isMute").get<bool>();
    return true;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
