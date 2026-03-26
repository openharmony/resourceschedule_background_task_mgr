/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_NAPI_INCLUDE_REQUEST_AUTH_FROM_USER_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_NAPI_INCLUDE_REQUEST_AUTH_FROM_USER_H

#include <mutex>

#include "common.h"
#include "expired_callback.h"
#include "ui_content.h"
#include "ability.h"
#include "ability_context.h"
#include "napi_base_context.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class AuthCallbackInstance : public ExpiredCallback {
public:
    AuthCallbackInstance();

    ~AuthCallbackInstance() override;

    void OnExpired() override;

    void OnExpiredAuth(int32_t authResult) override;

    void SetCallbackInfo(const napi_env &env, const napi_ref &ref);

private:
    void DeleteNapiRef();
    struct CallbackInfo {
        napi_env env = nullptr;
        napi_ref ref = nullptr;
    };

    CallbackInfo expiredCallbackInfo_;
};

bool CreateUIExtension(std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> abilityContext,
    const ContinuousTaskParam &taskParam);

class ModalExtensionCallback {
public:
    ModalExtensionCallback();
    ~ModalExtensionCallback();
    void OnRelease(int32_t releaseCode);
    void OnResult(int32_t resultCode, const OHOS::AAFwk::Want& result);
    void OnReceive(const OHOS::AAFwk::WantParams& request);
    void OnError(int32_t code, const std::string& name, const std::string &message);
    void OnRemoteReady(const std::shared_ptr<OHOS::Ace::ModalUIExtensionProxy> &uiProxy);
    void OnDestroy();
    void SetSessionId(int32_t sessionId);
    void SetBundleName(std::string bundleName);
    void SetAbilityContext(std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> abilityContext);
    void ReleaseOrErrorHandle(int32_t code);

private:
    int32_t sessionId_ = 0;
    std::string bundleName_;
    std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> abilityContext_;
};

extern std::map<int32_t, std::shared_ptr<ExpiredCallback>> authCallbackInstances_;
extern std::mutex authCallbackLock_;

napi_value RequestAuthFromUser(napi_env env, napi_callback_info info);
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_KITS_NAPI_INCLUDE_REQUEST_AUTH_FROM_USER_H