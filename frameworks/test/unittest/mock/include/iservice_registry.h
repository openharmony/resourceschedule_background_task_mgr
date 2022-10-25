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

#ifndef BGTASK_FRAMEWORK_MOCK_INCLUDE_H
#define BGTASK_FRAMEWORK_MOCK_INCLUDE_H

#include "ipc_types.h"
#include <iremote_broker.h>
#include "ipc_object_stub.h"
#include "iremote_proxy.h"
#include "if_system_ability_manager.h"

namespace OHOS {
class SystemAbilityManagerClient {
public:
    static SystemAbilityManagerClient& GetInstance();

    sptr<ISystemAbilityManager> GetSystemAbilityManager();
    sptr<IRemoteObject> GetRegistryRemoteObject();
    SystemAbilityManagerClient() = default;
    ~SystemAbilityManagerClient() = default;

    sptr<ISystemAbilityManager> systemAbilityManager_;
    std::string action_ {""};
    std::mutex systemAbilityManagerLock_;
};
}  // namespace OHOS
#endif  // BGTASK_FRAMEWORK_MOCK_INCLUDE_H
