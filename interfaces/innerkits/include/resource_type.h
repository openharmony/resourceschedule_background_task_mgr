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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_RESOURCE_TYPE_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_RESOURCE_TYPE_H

namespace OHOS {
namespace BackgroundTaskMgr {
class ResourceType {
public:
    virtual ~ResourceType() = default;
    enum Type : uint32_t {
        CPU = 1,
        COMMON_EVENT = 1 << 1,
        TIMER = 1 << 2,
        WORK_SCHEDULER = 1 << 3,
        BLUETOOTH = 1 << 4,
        GPS = 1 << 5,
        AUDIO = 1 << 6
    };
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_RESOURCE_TYPE_H