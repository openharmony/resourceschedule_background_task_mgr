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
#include "audio_renderer_info_plugin_data.h"
#include "bgtaskmgr_log_wrapper.h"
#include "bgtask_config.h"
#include "bgtask_plugin_mgr.h"
#include "res_type.h"
#include "res_data.h"
#include "res_sched_json_util.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace BackgroundTaskMgr {
using namespace OHOS::ResourceSchedule;
namespace {
    const bool SELF_REGISTER = EventMsgHandlerPluginAdapter::SelfRegister();
    const char PLUGIN_NAME[] = "EventMsgHandlerPluginAdapter";
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
    cbMap[ResType::RES_TYPE_RSS_CLOUD_CONFIG_UPDATE] = {
        {-1, [this](const int32_t stateType, const nlohmann::json &payload) {
                this->HandleCloudConfigUpdateEvent(stateType, payload);
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
        if (auto instance = AudioRendererInfoPluginData::GetInstance()) {
            instance->AfterAddSaListener();
        } else {
            BGTASK_LOGE("AudioRendererInfoPluginData instance is null");
        }
    }
}

void EventMsgHandlerPluginAdapter::HandleCloudConfigUpdateEvent(const int32_t stateType, const nlohmann::json &payload)
{
    if (stateType == SUSPEND_MANAGER_SYSTEM_ABILITY_ID) {
        bool ret = DelayedSingleton<BgtaskConfig>::GetInstance()->UpdateSusMgrCloudConfig(payload);
        if (!ret) {
            BGTASK_LOGE("suspend manager cloud config update fail.");
        }
    } else if (stateType == BACKGROUND_TASK_MANAGER_SERVICE_ID) {
        bool ret = DelayedSingleton<BgtaskConfig>::GetInstance()->UpdateBgMgrCloudConfig(payload);
        if (!ret) {
            BGTASK_LOGE("background task cloud config update fail.");
        }
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