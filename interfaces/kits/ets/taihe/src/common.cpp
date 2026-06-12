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

#include "bgtaskmgr_inner_errors.h"
#include "common.h"
#include "continuous_task_log.h"
#include <unordered_map>

namespace OHOS {
namespace BackgroundTaskMgr {
std::string Common::FindErrMsg(const int32_t errCode)
{
    if (errCode == ERR_OK) {
        return "";
    }
    std::string errMsg = BusinessErrorMap::GetSaErrMsg(errCode);
    if (errMsg != "") {
        std::string errMessage = "BusinessError ";
        int32_t errCodeInfo = FindErrCode(errCode);
        errMessage.append(std::to_string(errCodeInfo)).append(": ").append(errMsg);
        return errMessage;
    }
    errMsg = BusinessErrorMap::GetParamErrMsg(errCode);
    if (errMsg != "") {
        std::string errMessage = "BusinessError 401: Parameter error. ";
        errMessage.append(errMsg);
        return errMessage;
    }
    return "Inner error.";
}

int32_t Common::FindErrCode(const int32_t errCodeIn)
{
    if (BusinessErrorMap::GetParamErrMsg(errCodeIn) != "") {
        return ERR_BGTASK_INVALID_PARAM;
    }
    return errCodeIn > THRESHOLD ? errCodeIn / OFFSET : errCodeIn;
}

bool CreateUIExtension(std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> abilityContext,
    const ContinuousTaskParam &taskParam)
{
    if (abilityContext == nullptr) {
        BGTASK_LOGE("null context");
        return false;
    }
    auto uiContent = abilityContext->GetUIContent();
    if (uiContent == nullptr) {
        BGTASK_LOGE("null uiContent");
        return false;
    }
    if (taskParam.bgSubModeIds_.size() == 0) {
        return false;
    }
    AAFwk::Want want;
    std::string targetBundleName = "com.ohos.backgroundtaskmgr.resources";
    std::string targetAbilityName = "EnableBgTaskAuthDialog";
    want.SetElementName(targetBundleName, targetAbilityName);
    std::string typeKey = "ability.want.params.uiExtensionType";
    std::string typeValue = "sysDialog/common";
    want.SetParam(typeKey, typeValue);
    int32_t appIndex = abilityContext->GetAbilityInfo()->appIndex;
    std::string bundleName = abilityContext->GetBundleName();
    want.SetParam("appIndex", appIndex);
    int32_t taskMode = static_cast<int32_t>(taskParam.bgSubModeIds_[0]);
    want.SetParam("taskMode", taskMode);
    auto uiExtCallback = std::make_shared<ModalExtensionCallback>();
    uiExtCallback->SetAbilityContext(abilityContext);
    uiExtCallback->SetBundleName(bundleName);
    Ace::ModalUIExtensionCallbacks uiExtensionCallbacks = {
        .onRelease = std::bind(&ModalExtensionCallback::OnRelease, uiExtCallback, std::placeholders::_1),
        .onResult = std::bind(&ModalExtensionCallback::OnResult, uiExtCallback,
            std::placeholders::_1, std::placeholders::_2),
        .onReceive = std::bind(&ModalExtensionCallback::OnReceive, uiExtCallback, std::placeholders::_1),
        .onError = std::bind(&ModalExtensionCallback::OnError, uiExtCallback,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        .onRemoteReady = std::bind(&ModalExtensionCallback::OnRemoteReady, uiExtCallback, std::placeholders::_1),
        .onDestroy = std::bind(&ModalExtensionCallback::OnDestroy, uiExtCallback),
    };
    Ace::ModalUIExtensionConfig config;
    config.isProhibitBack = true;
    int32_t sessionId = uiContent->CreateModalUIExtension(want, uiExtensionCallbacks, config);
    if (sessionId == 0) {
        BGTASK_LOGE("Create component failed, sessionId is 0");
        return false;
    }
    uiExtCallback->SetSessionId(sessionId);
    return true;
}
 
ModalExtensionCallback::ModalExtensionCallback()
{}

ModalExtensionCallback::~ModalExtensionCallback()
{}

/*
* when UIExtensionAbility use terminateSelfWithResult
*/
void ModalExtensionCallback::OnResult(int32_t resultCode, const AAFwk::Want& result)
{
    BGTASK_LOGI("called OnResult");
}

/*
* when UIExtensionAbility send message to UIExtensionComponent
*/
void ModalExtensionCallback::OnReceive(const AAFwk::WantParams& receive)
{
    BGTASK_LOGI("called OnReceive");
}

/*
* when UIExtensionAbility disconnect or use terminate or process die
* releaseCode is 0 when process normal exit
*/
void ModalExtensionCallback::OnRelease(int32_t releaseCode)
{
    BGTASK_LOGI("called OnRelease");
    ReleaseOrErrorHandle(releaseCode);
}

/*
* when UIExtensionComponent init or turn to background or destroy UIExtensionAbility occur error
*/
void ModalExtensionCallback::OnError(int32_t code, const std::string& name, const std::string& message)
{
    BGTASK_LOGI("called OnError, name = %{public}s, message = %{public}s", name.c_str(), message.c_str());
    ReleaseOrErrorHandle(code);
}

/*
* when UIExtensionComponent connect to UIExtensionAbility, ModalUIExtensionProxy will init,
* UIExtensionComponent can send message to UIExtensionAbility by ModalUIExtensionProxy
*/
void ModalExtensionCallback::OnRemoteReady(const std::shared_ptr<Ace::ModalUIExtensionProxy>& uiProxy)
{
    BGTASK_LOGI("called OnRemoteReady");
}

/*
* when UIExtensionComponent destructed
*/
void ModalExtensionCallback::OnDestroy()
{
    BGTASK_LOGI("called OnDestroy");
}

void ModalExtensionCallback::SetSessionId(int32_t sessionId)
{
    this->sessionId_ = sessionId;
}

void ModalExtensionCallback::SetBundleName(std::string bundleName)
{
    this->bundleName_ = bundleName;
}

void ModalExtensionCallback::SetAbilityContext(std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> abilityContext)
{
    this->abilityContext_ = abilityContext;
}

void ModalExtensionCallback::ReleaseOrErrorHandle(int32_t code)
{
    Ace::UIContent* uiContent = this->abilityContext_->GetUIContent();
    if (uiContent == nullptr) {
        BGTASK_LOGE("null uiContent");
        return;
    }
    uiContent->CloseModalUIExtension(this->sessionId_);
    return;
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS