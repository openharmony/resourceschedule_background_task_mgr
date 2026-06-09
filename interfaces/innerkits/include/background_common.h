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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_COMMON_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_COMMON_H

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
    // 查询用户授权API接口-起始版本
    static constexpr int32_t API_VERSION_CHECK_SPECIAL_USER_AUTH = 22;
    // 查询用户授权API 26接口
    static constexpr int32_t API_VERSION_CHECK_SPECIAL_USER_AUTH_RESULT = 26;
    // 请求用户授权API接口-起始版本
    static constexpr int32_t API_VERSION_REQUEST_SPECIAL_USER_AUTH = 22;
    // 请求用户授权API 26接口
    static constexpr int32_t API_VERSION_REQUEST_SPECIAL_USER_AUTH_BY_DIALOG = 26;
    // 长时任务自有hap包名
    static constexpr char BUNDLE_NAME_CONTINUOUS_TASK_RESOURCE[] = "com.ohos.backgroundtaskmgr.resources";
    // 特殊类型长时任务自定义授权弹窗Extension ability name
    static constexpr char EXTENSION_ABILITY_NAME_SPECIAL_MODE_PERMISSION_DIALOG[] = "EnableBgTaskAuthDialog";
    // 通过UEC拉起extension固定配置key
    static constexpr char TYPE_KEY_START_UIEXTENSION[] = "ability.want.params.uiExtensionType";
    // 特殊类型长时任务自定义授权弹窗-传参appIndex
    static constexpr char PERMISSION_DIALOG_PARAM_APP_INDEX[] = "appIndex";
    // 特殊类型长时任务自定义授权弹窗-传参taskMode（长时任务子类型）
    static constexpr char PERMISSION_DIALOG_PARAM_SPECIAL_SUB_MODE[] = "taskMode";
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_INTERFACES_INNERKITS_INCLUDE_BACKGROUND_COMMON_H