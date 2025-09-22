/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_NOTIFICATION_TOOLS_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_NOTIFICATION_TOOLS_H

#include "singleton.h"
#include "continuous_task_record.h"
#include "bgtaskmgr_inner_errors.h"

void BgContinuousTaskMgr::OnAppStateChanged(int32_t uid)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return;
    }
    applyTaskOnForeground.erase(uid);
    std::vector<uint32_t> appliedModeIds {};
    for (const auto &task : continuousTaskInfosMap_) {
        if (!task.second) {
            continue;
        }
        if (task.second->GetUid() == uid) {
            std::vector<uint32_t> bgModeIds = task.second->bgModeIds_;
            for (const auto &mode : bgModeIds) {
                if (!std::count(appliedModeIds.begin(), appliedModeIds.end(), mode)) {
                    appliedModeIds.push_back(mode);
                }
            }
        }
    }
    applyTaskOnForeground.emplace(uid, appliedModeIds);
    std::string modeStr = CommonUtils::ModesToString(appliedModeIds);
    BGTASK_LOGD("uid: %{public}d to background, continuous modes: %{public}s", uid, modeStr.c_str());
}

ErrCode BgContinuousTaskMgr::AllowApplyContinuousTask(const std::shared_ptr<ContinuousTaskRecord> record)
{
    if (!record->isByRequestObject_) {
        return ERR_OK;
    }
    // 申请数量是否超过10个
    ErrCode ret = CheckAbilityTaskNum(record);
    if (ret != ERR_OK) {
        return ret;
    }
    // 需要豁免的情况
    if (record->IsFromWebview()) {
        return ERR_OK;
    }
    // 应用退后台前已申请过的类型，外加播音类型
    std::vector<uint32_t> checkBgModeIds {};
    int32_t uid = record->GetUid();
    if (applyTaskOnForeground.find(uid) != applyTaskOnForeground.end()) {
        checkBgModeIds = applyTaskOnForeground.at(uid);
    }
    checkBgModeIds.push_back(BackgroundMode::AUDIO_PLAYBACK);
    if (CommonUtils::CheckApplyMode(record->bgModeIds_, checkBgModeIds)) {
        return ERR_OK;
    }
    // 应用在前台
    std::string bundleName = record->GetBundleName();
    bool isForeApp = DelayedSingleton<BgTransientTaskMgr>::GetInstance()->IsFrontApp(bundleName, uid);
    if (isForeApp) {
        return ERR_OK;
    }
    BGTASK_LOGE("uid: %{public}d, bundleName: %{public}s check allow apply continuous task fail.",
        uid, bundleName.c_str());
    return ERR_BGTASK_CONTINUOUS_NOT_APPLY_ONBACKGROUND;
}

void BgContinuousTaskMgr::RestoreApplyRecord()
{
    applyTaskOnForeground.clear();
    for (const auto &task : continuousTaskInfosMap_) {
        if (!task.second) {
            continue;
        }
        int32_t uid = task.second->GetUid();
        std::vector<uint32_t> bgModeIds = task.second->bgModeIds_;
        if (applyTaskOnForeground.find(uid) == applyTaskOnForeground.end()) {
            std::string modeStr = CommonUtils::ModesToString(bgModeIds);
            BGTASK_LOGI("uid: %{public}d restore apply record, continuous modes: %{public}s", uid, modeStr.c_str());
            applyTaskOnForeground.emplace(uid, bgModeIds);
        } else {
            std::vector<uint32_t> appliedModeIds = applyTaskOnForeground.at(uid);
            applyTaskOnForeground.erase(uid);
            for (const auto &mode : bgModeIds) {
                if (!std::count(appliedModeIds.begin(), appliedModeIds.end(), mode)) {
                    appliedModeIds.push_back(mode);
                }
            }
            std::string modeStr = CommonUtils::ModesToString(appliedModeIds);
            BGTASK_LOGI("uid: %{public}d restore apply record, continuous modes: %{public}s", uid, modeStr.c_str());
            applyTaskOnForeground.emplace(uid, appliedModeIds);
        }
    }
}


namespace OHOS {
namespace BackgroundTaskMgr {
class NotificationTools : public DelayedSingleton<NotificationTools> {
public:
    static void SetNotificationIdIndex(const int32_t id);
    ErrCode PublishNotification(const std::shared_ptr<ContinuousTaskRecord> &continuousTaskRecord,
        const std::string &appName, const std::string &prompt, int32_t serviceUid);
    ErrCode CancelNotification(const std::string &label, int32_t id);
    void GetAllActiveNotificationsLabels(std::set<std::string> &notificationLabels);
    void RefreshContinuousNotifications(
        const std::map<std::string, std::pair<std::string, std::string>> &newPromptInfos, int32_t serviceUid);
    ErrCode RefreshContinuousNotificationWantAndContext(int32_t serviceUid,
        const std::map<std::string, std::pair<std::string, std::string>> &newPromptInfos,
        const std::shared_ptr<ContinuousTaskRecord> continuousTaskRecord, bool updateContent = false);

private:
    static int32_t notificationIdIndex_;

    DECLARE_DELAYED_SINGLETON(NotificationTools)
};
} // BackgroundTaskMgr
} // OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_SERVICES_CONTINUOUS_TASK_INCLUDE_NOTIFICATION_TOOLS_H
