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

#include <cinttypes>
#include "bgtask_plugin_mgr.h"
#include "bgtaskmgr_log_wrapper.h"
#include "plugin_mgr.h"

namespace OHOS {
namespace BackgroundTaskMgr {
using namespace OHOS::ResourceSchedule;
namespace {
    const std::string_view LIB_NAME = "libbgtaskmgr_service.z.so";
}
IMPLEMENT_SINGLE_INSTANCE(BgtaskPluginMgr);

void BgtaskPluginMgr::RegisterAsyncPluginByValue(const uint32_t resType,
    const std::shared_ptr<BgtaskPlugin> plugin)
{
    auto valueList = plugin->GetPluginValue(resType);
    for (const auto& value : valueList) {
        if (value != RES_VALUE_FOR_ALL) {
            PluginMgr::GetInstance().SubscribeResourceAccurately(std::string(LIB_NAME), resType, value);
        } else {
            PluginMgr::GetInstance().SubscribeResource(std::string(LIB_NAME), resType);
        }
    }
}

void BgtaskPluginMgr::Init()
{
    std::lock_guard<std::mutex> lock(cbMapMutex_);
    for (auto it = asyncCbMap_.begin(); it != asyncCbMap_.end(); it++) {
        if (it->second != nullptr) {
            it->second->Init();
            RegisterAsyncPluginByValue(it->first, it->second);
            BGTASK_LOGI("Subscribe async resType: %{public}u", it->first);
        }
    }
    pluginEnable.store(true);
    BGTASK_LOGI("BgtaskPluginMgr init success, %{public}zu plugins registered", asyncCbMap_.size());
}

void BgtaskPluginMgr::Disable()
{
    pluginEnable.store(false);
    std::lock_guard<std::mutex> lock(cbMapMutex_);
    for (auto it = asyncCbMap_.begin(); it != asyncCbMap_.end(); it++) {
        auto valueList = it->second->GetPluginValue(it->first);
        for (const auto& value : valueList) {
            if (value == RES_VALUE_FOR_ALL) {
                PluginMgr::GetInstance().UnSubscribeResource(std::string(LIB_NAME), it->first);
            } else {
                PluginMgr::GetInstance().UnSubscribeResourceAccurately(std::string(LIB_NAME), it->first, value);
            }
        }
        it->second->Uninit();
    }
}

void BgtaskPluginMgr::DispatchResource(const std::shared_ptr<ResourceSchedule::ResData>& resData)
{
    if (!pluginEnable.load() || resData == nullptr) {
        BGTASK_LOGE("SuspendManagerPluginMgr not enable or data is nullptr");
        return;
    }

    BGTASK_LOGD("BgtaskPluginMgr dispatch resource, type: %{public}u, value: %{public}" PRId64,
        resData->resType, resData->value);

    std::lock_guard<std::mutex> lock(cbMapMutex_);
    auto iter = asyncCbMap_.find(resData->resType);
    if (iter != asyncCbMap_.end() && iter->second != nullptr) {
        iter->second->DispatchResource(resData->resType, resData->value, resData->payload);
    }
}

void BgtaskPluginMgr::RegisterAsyncPlugin(const uint32_t resType, const std::shared_ptr<BgtaskPlugin> plugin)
{
    if (plugin == nullptr) {
        BGTASK_LOGE("plugin is null");
        return;
    }

    BGTASK_LOGD("Register async plugin, type: %{public}u", resType);
    std::lock_guard<std::mutex> lock(cbMapMutex_);
    auto it = asyncCbMap_.find(resType);
    if (it != asyncCbMap_.end() && it->second != nullptr) {
        BGTASK_LOGW("resType:%{public}u registed by %{public}s", resType, it->second->GetPluginName().c_str());
    } else {
        if (pluginEnable.load()) {
            RegisterAsyncPluginByValue(resType, plugin);
            BGTASK_LOGI("Register async plugin, type: %{public}u", resType);
        }
    }
    asyncCbMap_.emplace(resType, plugin);
}

extern "C" bool OnPluginInit(std::string& libName)
{
    if (libName != std::string(LIB_NAME)) {
        BGTASK_LOGE("lib name is not match");
        return false;
    }
    BgtaskPluginMgr::GetInstance().Init();
    BGTASK_LOGD("BgtaskPluginMgr OnPluginInit success");
    return true;
}

extern "C" void OnPluginDisable()
{
    BgtaskPluginMgr::GetInstance().Disable();
    BGTASK_LOGD("BgtaskPluginMgr OnPluginDisable success");
}

extern "C" void OnDispatchResource(const std::shared_ptr<ResourceSchedule::ResData>& resData)
{
    BgtaskPluginMgr::GetInstance().DispatchResource(resData);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS