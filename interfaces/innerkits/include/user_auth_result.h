/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_USER_AUTH_RESULT_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_USER_AUTH_RESULT_H

#include <string>

namespace OHOS {
namespace BackgroundTaskMgr {
class UserAuthResult {
public:
    virtual ~UserAuthResult() = default;
    enum Type : uint32_t {
        NOT_SUPPORTED = 0,
        NOT_DETERMINED = 1,
        DENIED = 2,
        GRANTED_ONCE = 3,
        GRANTED_ALWAYS = 4,
        END,
    };
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_USER_AUTH_RESULT_H