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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_SERVICES_TRANSIENT_TASK_INCLUDE_EVENT_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUNDTASKMANAGER_SERVICES_TRANSIENT_TASK_INCLUDE_EVENT_INFO_H

#include <string>
#include <vector>

#include <refbase.h>

#include "event_info.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "want.h"

namespace OHOS {
namespace BackgroundTaskMgr {
enum EventId {
    EVENT_APP_RUN_FRONT = 1,
    EVENT_SCREEN_ON,
    EVENT_SCREEN_OFF,
    EVENT_SCREEN_UNLOCK,
    EVENT_BATTERY_LOW,
    EVENT_BATTERY_OKAY,
    EVENT_MAX,
};

class EventInfo{
public:
    EventInfo() {}
    virtual ~EventInfo() = default;

    inline void SetEventId(const int32_t& eventId)
    {
        eventId_ = eventId;
    }

    inline const int32_t& GetEventId() const
    {
        return eventId_;
    }

    inline void SetIntArgs(const std::vector<int32_t> args)
    {
        intArgs_ = args;
    }

    inline const std::vector<int32_t>& GetIntArgs() const
    {
        return intArgs_;
    }

    inline void SetStringArgs(const std::vector<std::string> args)
    {
        stringArgs_ = args;
    }

    inline const std::vector<std::string>& GetStringArgs() const
    {
        return stringArgs_;
    }

    virtual inline const std::string ToString() const
    {
        std::string ret = "eventId : " + std::to_string(eventId_) + " intArgs_ : [";
        for_each(intArgs_.cbegin(), intArgs_.cend(),
            [&ret](const int32_t& val)->void { ret += ("" + std::to_string(val));});
        ret += "], stringArgs_ : [";
        for_each(stringArgs_.cbegin(), stringArgs_.cend(),
            [&ret](const std::string& val)->void { ret += ("" + val);});
        ret += "]";
        return ret;
    }
private:
    int32_t eventId_ {0};
    std::vector<int32_t> intArgs_;
    std::vector<std::string> stringArgs_;
};
} // namespace BackgroundTaskMgr
} // namespace OHOS
#endif