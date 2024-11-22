/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "bgtaskcontinuoustaskrecord_fuzzer.h"
#include "securec.h"

#include "continuous_task_record.h"
#include "common_utils.h"
#include "iremote_object.h"
#include "background_mode.h"
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
    constexpr int32_t U32_AT_SIZE = 4;
    constexpr uint8_t TWENTYFOUR = 24;
    constexpr uint8_t SIXTEEN = 16;
    constexpr uint8_t EIGHT = 8;
    constexpr int32_t THREE = 3;
    constexpr int32_t TWO = 2;

    uint32_t GetU32Data(const char* ptr)
    {
        return (ptr[0] << TWENTYFOUR) | (ptr[1] << SIXTEEN) | (ptr[TWO] << EIGHT) | (ptr[THREE]);
    }

    bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
    {
        auto continuousTaskRecord = std::make_shared<ContinuousTaskRecord>();
        continuousTaskRecord->bundleName_ = "bundleName";
        continuousTaskRecord->bundleName_ = "abilityName";
        continuousTaskRecord->uid_ = 1;
        continuousTaskRecord->pid_ = 1;
        continuousTaskRecord->bgModeId_ = 1;
        continuousTaskRecord->isBatchApi_ = true;
        continuousTaskRecord->bgModeIds_ = {1};
        continuousTaskRecord->abilityId_ = 1;
        continuousTaskRecord->GetBundleName();
        continuousTaskRecord->GetAbilityName();
        continuousTaskRecord->IsNewApi();
        continuousTaskRecord->IsFromWebview();
        continuousTaskRecord->GetBgModeId();
        continuousTaskRecord->GetUserId();
        continuousTaskRecord->GetUid();
        continuousTaskRecord->GetPid();
        continuousTaskRecord->GetNotificationLabel();
        continuousTaskRecord->GetNotificationId();
        continuousTaskRecord->GetWantAgent();
        uint32_t modeData = GetU32Data(data);
        std::vector<uint32_t> bgmodes = {1, 2, modeData};
        continuousTaskRecord->ToString(bgmodes);
        std::string modeStr = "1";
        continuousTaskRecord->ToVector(modeStr);
        continuousTaskRecord->GetAbilityId();
        continuousTaskRecord->IsSystem();
        continuousTaskRecord->ParseToJsonStr();
        nlohmann::json json;
        json["bundleName"] = "bundleName";
        json["abilityName"] = "abilityName";
        json["userId"] = 1;
        json["uid"] = 1;
        json["pid"] = 1;
        json["bgModeId"] = 1;
        json["isNewApi"] = false;
        json["isFromWebview"] = false;
        json["notificationLabel"] = "notificationLabel";
        json["isSystem"] = false;
        json["isBatchApi"] = false;
        json["bgModeIds"] = "1";
        nlohmann::json info;
        info["bundleName"] = "wantAgentBundleName";
        info["abilityName"] = "wantAgentAbilityName";
        info["wantAgentInfo"] = info;
        json["notificationId"] = 1;
        continuousTaskRecord->ParseFromJson(json);
        return true;
    }
} // BackgroundTaskMgr
} // OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    if (size < OHOS::BackgroundTaskMgr::U32_AT_SIZE) {
        return 0;
    }

    char* ch = static_cast<char *>(malloc(size + 1));
    if (ch == nullptr) {
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::BackgroundTaskMgr::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}
