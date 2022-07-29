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

#include "common_utils.h"

namespace OHOS {
namespace BackgroundTaskMgr {
CommonUtils::CommonUtils() {}

CommonUtils::~CommonUtils() {}

bool CommonUtils::CheckJsonValue(const Json::Value &value, std::initializer_list<std::string> params)
{
    for (auto param : params) {
        if (!value.isMember(param) || value[param].isNull()) {
            return false;
        }
    }
    return true;
}

template<typename T>
bool CommonUtils::CheckIsUidExist(int32_t uid, const T &t)
{
    for (auto var : t) {
        if (var->uid_ == uid) {
            return true;
        }
    }
}
}
}