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

#include "event_msg_handler_plugin_adapter.h"
#include "bgtask_data_mgr.h"
#include "bgtaskmgr_log_wrapper.h"
#include "bgtask_plugin_mgr.h"
#include "bg_continuous_task_mgr.h"
#include "res_type.h"
#include "res_value.h"
#include "res_data.h"
#include "res_sched_json_util.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace BackgroundTaskMgr {
using namespace OHOS::ResourceSchedule;
namespace {
    const bool SELF_REGISTER = EventMsgHandlerPluginAdapter::SelfRegister();
    const char PLUGIN_NAME[] = "EventMsgHandlerPluginAdapter";
    constexpr int32_t MULTI_DEVICE_CAST_STATE_START = 1;
    constexpr int32_t MULTI_DEVICE_CAST_STATE_STOP = 2;
}

bool EventMsgHandlerPluginAdapter::SelfRegister()
{
    auto obj = EventMsgHandlerPluginAdapter::GetInstance();
    if (obj == nullptr) {
        BGTASK_LOGE("EventMsgHandlerPluginAdapter null.");
        return false;
    }
 
    obj->InitCbMap(obj->cbMap_);
    for (const auto &info : obj->cbMap_) {
        BgtaskPluginMgr::GetInstance().RegisterAsyncPlugin(info.first, obj);
    }
    return true;
}

void EventMsgHandlerPluginAdapter::InitCbMap(CallBackMap &cbMap)
{
    cbMap[ResType::RES_TYPE_OBSERVER_MANAGER_STATUS_CHANGE] = {
        {ResType::SystemAbilitySign::ADD_SYSTEM_ABILITY,
            [this](const int32_t stateType, const nlohmann::json &payload) { this->AfterAddSaListener(payload); }}};
    cbMap[ResType::RES_TYPE_BGTASK_INNER_EVENT] = {
        {ResType::BgtaskInnerEvent::MULTI_DEVICE_CAST_EVENT,
            [this](const int32_t stateType, const nlohmann::json &payload) {
                this->HandleMultiDeviceCastEvent(payload);
            }}
    };
}

void EventMsgHandlerPluginAdapter::AfterAddSaListener(const nlohmann::json &payload)
{
    int32_t saId = -1;
    if (!ResCommonUtil::ParseIntParameterFromJson("saId", saId, payload)) {
        BGTASK_LOGE("no said");
        return;
    }

    if (saId == AUDIO_POLICY_SERVICE_ID) {
        if (auto instance = BgtaskDataMgr::GetInstance()) {
            instance->AfterAddSaListener();
        } else {
            BGTASK_LOGE("BgtaskDataMgr instance is null");
        }
    }
}

void EventMsgHandlerPluginAdapter::HandleMultiDeviceCastEvent(const nlohmann::json &payload)
{
    int32_t uid = -1;
    int32_t said = -1;
    int32_t state = -1;
    if (!ResCommonUtil::ParseIntParameterFromJson("uid", uid, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("said", said, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("state", state, payload)) {
        BGTASK_LOGE("HandleMultiDeviceCastEvent parse payload error");
        return;
    }

    BGTASK_LOGI("uid:%{public}d said:%{public}d state:%{public}d", uid, said, state);
    if (state == MULTI_DEVICE_CAST_STATE_START) {
        BgContinuousTaskMgr::GetInstance()->NotifyAudioStart(uid);
        BgtaskDataMgr::GetInstance()->HandleMultiDeviceCastStart(uid, said);
    } else if (state == MULTI_DEVICE_CAST_STATE_STOP) {
        BgtaskDataMgr::GetInstance()->HandleMultiDeviceCastStop(uid, said);
    }
}

void EventMsgHandlerPluginAdapter::Init()
{
    BGTASK_LOGI("EventMsgHandlerPluginAdapter init");
}

void EventMsgHandlerPluginAdapter::Uninit()
{
    BGTASK_LOGI("EventMsgHandlerPluginAdapter uninit");
}

std::string EventMsgHandlerPluginAdapter::GetPluginName() const
{
    return PLUGIN_NAME;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS