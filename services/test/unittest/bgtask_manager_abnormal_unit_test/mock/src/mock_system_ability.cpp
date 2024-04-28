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

#include "system_ability.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "string_ex.h"

namespace OHOS {
SystemAbility::SystemAbility(bool runOnCreate)
{
    isRunning_ = false;
    isRunOnCreate_ = runOnCreate;
    isDistributed_ = false;
    dumpLevel_ = 0;
    dependTimeout_ = 1;
    capability_ = u"";
}

SystemAbility::SystemAbility(int32_t systemAbilityId, bool runOnCreate) : SystemAbility(runOnCreate)
{
    saId_ = systemAbilityId;
}

SystemAbility::~SystemAbility()
{
}

bool SystemAbility::MakeAndRegisterAbility(SystemAbility* systemAbility)
{
    return true;
}

bool SystemAbility::AddSystemAbilityListener(int32_t systemAbilityId)
{
    return true;
}

bool SystemAbility::RemoveSystemAbilityListener(int32_t systemAbilityId)
{
    return true;
}

bool SystemAbility::Publish(sptr<IRemoteObject> systemAbility)
{
    return false;
}


int32_t SystemAbility::GetSystemAbilitId() const
{
    return saId_;
}

void SystemAbility::SetRunOnCreate(bool isRunOnCreate)
{
    isRunOnCreate_ = isRunOnCreate;
}

bool SystemAbility::IsRunOnCreate() const
{
    return isRunOnCreate_;
}

bool SystemAbility::GetRunningStatus() const
{
    return isRunning_;
}

int32_t SystemAbility::GetDependTimeout() const
{
    return dependTimeout_;
}

void SystemAbility::OnDump() {}

void SystemAbility::OnStart() {}

int32_t SystemAbility::OnExtension(const std::string& extension, MessageParcel& data, MessageParcel& reply)
{
    return 0;
}

void SystemAbility::OnStop() {}

void SystemAbility::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) {}

void SystemAbility::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) {}

sptr<IRemoteObject> SystemAbility::GetSystemAbility(int32_t systemAbilityId)
{
    return nullptr;
}
}