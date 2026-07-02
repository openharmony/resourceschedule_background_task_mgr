/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "accesstoken_kit.h"
#include "permission_def.h"

namespace OHOS {
namespace {
    int32_t g_mockTokenType = 0;
    bool g_mockAtomicService = true;
}

void BgMockTokenType(int32_t mockTokenType)
{
    g_mockTokenType = mockTokenType;
}

void BgMockAtomicService(bool mockAtomicService)
{
    g_mockAtomicService = mockAtomicService;
}

namespace Security {
namespace AccessToken {
ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    int32_t tokenTypeHap = 1;
    int32_t tokenTypeNative = 2;
    if (g_mockTokenType == tokenTypeHap) {
        return TOKEN_HAP;
    } else if (g_mockTokenType == tokenTypeNative) {
        return TOKEN_NATIVE;
    }
    return TOKEN_INVALID;
}

bool AccessTokenKit::IsAtomicServiceByFullTokenID(uint64_t tokenID)
{
    if (!g_mockAtomicService) {
        return g_mockAtomicService;
    }
    return true;
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
