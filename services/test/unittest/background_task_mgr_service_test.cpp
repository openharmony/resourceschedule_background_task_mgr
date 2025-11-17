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

#include <functional>
#include <gtest/gtest.h>

#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "background_task_mgr_service.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {
class BackgroundTaskMgrServiceTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: BackgroundTaskMgrServiceTest_001
 * @tc.desc: Test BackgroundTaskMgrService service ready.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET SR000GMUG8 AR000GH86O AR000GH86Q AR000GMUIA AR000GMUHN
 */
HWTEST_F(BackgroundTaskMgrServiceTest, BackgroundTaskMgrServiceTest_001, TestSize.Level3)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr) << "BackgroundTaskMgrServiceTest fail to get GetSystemAbilityManager";
    sptr<IRemoteObject> remoteObject = sm->CheckSystemAbility(BACKGROUND_TASK_MANAGER_SERVICE_ID);
    EXPECT_TRUE(remoteObject != nullptr) << "GetSystemAbility failed";
}

/**
 * @tc.name: SetBackgroundTaskState_001
 * @tc.desc: SetBackgroundTaskState test.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgContinuousTaskMgrTest, SetBackgroundTaskState_001, TestSize.Level1)
{
    bgContinuousTaskMgr_->isSysReady_.store(false);
    EXPECT_EQ(bgContinuousTaskMgr_->SetBackgroundTaskState(nullptr), ERR_BGTASK_SYS_NOT_READY);

    bgContinuousTaskMgr_->isSysReady_.store(true);
    EXPECT_EQ(bgContinuousTaskMgr_->SetBackgroundTaskState(nullptr), ERR_BGTASK_CHECK_TASK_PARAM);
    // userId参数非法
    std::shared_ptr<BackgroundTaskStateInfo> taskParam = std::make_shared<BackgroundTaskStateInfo>();
    taskParam->SetUserId(-1);
    EXPECT_EQ(bgContinuousTaskMgr_->SetBackgroundTaskState(taskParam),
        ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_PARAM_INVALID);
    // appIndex参数非法
    taskParam->SetUserId(100);
    taskParam->SetAppIndex(-1);
    EXPECT_EQ(bgContinuousTaskMgr_->SetBackgroundTaskState(taskParam),
        ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_PARAM_INVALID);
    // bundleName参数非法
    taskParam->SetAppIndex(0);
    taskParam->SetBundleName("");
    EXPECT_EQ(bgContinuousTaskMgr_->SetBackgroundTaskState(taskParam),
        ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_PARAM_INVALID);
    // 权限参数非法
    taskParam->SetBundleName("bundleName");
    taskParam->SetUserAuthResult(-1);
    EXPECT_EQ(bgContinuousTaskMgr_->SetBackgroundTaskState(taskParam),
        ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_PARAM_INVALID);
    taskParam->SetUserAuthResult(static_cast<int32_t>(UserAuthResult::END));
    EXPECT_EQ(bgContinuousTaskMgr_->SetBackgroundTaskState(taskParam),
        ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_PARAM_INVALID);
    // 无记录时，添加
    taskParam->SetUserAuthResult(static_cast<int32_t>(UserAuthResult::GRANTED_ONCE));
    bgContinuousTaskMgr_->bannerNotificationRecord_.clear();
    bgContinuousTaskMgr_->SetBackgroundTaskState(taskParam);
    EXPECT_FALSE(bgContinuousTaskMgr_->bannerNotificationRecord_.empty());
    // 有记录时更新
    taskParam->SetUserAuthResult(static_cast<int32_t>(UserAuthResult::GRANTED_ALWAYS));
    EXPECT_EQ(bgContinuousTaskMgr_->SetBackgroundTaskState(taskParam), ERR_OK);
}

/**
 * @tc.name: GetBackgroundTaskState_001
 * @tc.desc: GetBackgroundTaskState test.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BgContinuousTaskMgrTest, GetBackgroundTaskState_001, TestSize.Level1)
{
    bgContinuousTaskMgr_->isSysReady_.store(false);
    uint32_t authResult = 0;
    EXPECT_EQ(bgContinuousTaskMgr_->GetBackgroundTaskState(nullptr, authResult), ERR_BGTASK_SYS_NOT_READY);

    bgContinuousTaskMgr_->isSysReady_.store(true);
    EXPECT_EQ(bgContinuousTaskMgr_->GetBackgroundTaskState(nullptr, authResult), ERR_BGTASK_CHECK_TASK_PARAM);
    // userId参数非法
    std::shared_ptr<BackgroundTaskStateInfo> taskParam = std::make_shared<BackgroundTaskStateInfo>();
    taskParam->SetUserId(-1);
    EXPECT_EQ(bgContinuousTaskMgr_->GetBackgroundTaskState(taskParam, authResult),
        ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_PARAM_INVALID);
    // appIndex参数非法
    taskParam->SetUserId(100);
    taskParam->SetAppIndex(-1);
    EXPECT_EQ(bgContinuousTaskMgr_->GetBackgroundTaskState(taskParam, authResult),
        ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_PARAM_INVALID);
    // bundleName参数非法
    taskParam->SetAppIndex(0);
    taskParam->SetBundleName("");
    EXPECT_EQ(bgContinuousTaskMgr_->GetBackgroundTaskState(taskParam, authResult),
        ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_PARAM_INVALID);
    // 无对应记录时
    taskParam->SetBundleName("bundleName");
    bgContinuousTaskMgr_->bannerNotificationRecord_.clear();
    EXPECT_EQ(bgContinuousTaskMgr_->GetBackgroundTaskState(taskParam, authResult),
        ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_NOT_APPLY_RECORD);
    // 有记录时更新
    taskParam->SetUserAuthResult(static_cast<int32_t>(UserAuthResult::GRANTED_ALWAYS));
    bgContinuousTaskMgr_->bannerNotificationRecord_.clear();
    bgContinuousTaskMgr_->SetBackgroundTaskState(taskParam);
    EXPECT_EQ(bgContinuousTaskMgr_->GetBackgroundTaskState(taskParam, authResult), ERR_OK);
}

ErrCode BgContinuousTaskMgr::SetBackgroundTaskState(std::shared_ptr<BackgroundTaskStateInfo> taskParam)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    if (!taskParam) {
        BGTASK_LOGE("SetBackgroundTaskState task param is null!");
        return ERR_BGTASK_CHECK_TASK_PARAM;
    }
    if (taskParam->GetUserId() == -1 || taskParam->GetAppIndex() == -1 || taskParam->GetBundleName() == "" ||
        taskParam->GetUserAuthResult() >= static_cast<int32_t>(UserAuthResult::END) ||
        taskParam->GetUserAuthResult() < static_cast<int32_t>(UserAuthResult::NOT_SUPPORTED)) {
        BGTASK_LOGE("SetBackgroundTaskState task param is invaild!");
        return ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_PARAM_INVALID;
    }
    ErrCode result = ERR_OK;
    handler_->PostSyncTask([this, taskParam, &result]() {
        result = this->SetBackgroundTaskStateInner(taskParam);
        }, AppExecFwk::EventQueue::Priority::HIGH);
    return result;
}

ErrCode BgContinuousTaskMgr::SetBackgroundTaskStateInner(std::shared_ptr<BackgroundTaskStateInfo> taskParam)
{
    int32_t userId = taskParam->GetUserId();
    std::string bundleName = taskParam->GetBundleName();
    int32_t appIndex = taskParam->GetAppIndex();
    int32_t authResult = taskParam->GetUserAuthResult();
    auto findRecord = [userId, bundleName, appIndex](const auto &target) {
        return userId == target.second->GetUserId() && appIndex == target.second->GetAppIndex() &&
            bundleName == target.second->GetBundleName();
    };
    auto findRecordIter = find_if(bannerNotificationRecord_.begin(), bannerNotificationRecord_.end(), findRecord);
    if (findRecordIter == bannerNotificationRecord_.end()) {
        // 不存在，新增:例如直接在设置里关闭了权限开关
        std::shared_ptr<BannerNotificationRecord> bannerNotification = std::make_shared<BannerNotificationRecord>();
        bannerNotification->SetBundleName(bundleName);
        bannerNotification->SetUserId(userId);
        bannerNotification->SetAppIndex(appIndex);
        bannerNotification->SetAuthResult(authResult);
        std::string key = NotificationTools::GetInstance()->CreateBannerNotificationLabel(bundleName,
            userId, appIndex);
        bannerNotificationRecord_.emplace(key, bannerNotification);
    } else {
        // 存在记录，直接更新
        findRecordIter->second->SetAuthResult(authResult);
    }
    RefreshAuthRecord();
    return ERR_OK;
}

ErrCode BgContinuousTaskMgr::GetBackgroundTaskState(std::shared_ptr<BackgroundTaskStateInfo> taskParam,
    uint32_t &authResult)
{
    if (!isSysReady_.load()) {
        BGTASK_LOGW("manager is not ready");
        return ERR_BGTASK_SYS_NOT_READY;
    }
    if (!taskParam) {
        BGTASK_LOGE("SetBackgroundTaskState task param is null!");
        return ERR_BGTASK_CHECK_TASK_PARAM;
    }
    if (taskParam->GetUserId() == -1 || taskParam->GetAppIndex() == -1 || taskParam->GetBundleName() == "") {
        BGTASK_LOGE("GetBackgroundTaskState task param is invaild!");
        return ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_PARAM_INVALID;
    }
    ErrCode result = ERR_OK;
    handler_->PostSyncTask([this, taskParam, &result, &authResult]() {
        result = this->GetBackgroundTaskStateInner(taskParam, authResult);
        }, AppExecFwk::EventQueue::Priority::HIGH);
    return result;
}

ErrCode BgContinuousTaskMgr::GetBackgroundTaskStateInner(std::shared_ptr<BackgroundTaskStateInfo> taskParam,
    uint32_t &authResult)
{
    int32_t userId = taskParam->GetUserId();
    std::string bundleName = taskParam->GetBundleName();
    int32_t appIndex = taskParam->GetAppIndex();
    BGTASK_LOGI("GetBackgroundTaskStateInner bundleName: %{public}s, appIndex: %{public}d",
        bundleName.c_str(), appIndex);
    auto findRecord = [userId, bundleName, appIndex](const auto &target) {
        return userId == target.second->GetUserId() && appIndex == target.second->GetAppIndex() &&
            bundleName == target.second->GetBundleName();
    };
    auto findRecordIter = find_if(bannerNotificationRecord_.begin(), bannerNotificationRecord_.end(), findRecord);
    if (findRecordIter == bannerNotificationRecord_.end()) {
        return ERR_BGTASK_CONTINUOUS_BACKGROUND_TASK_NOT_APPLY_RECORD;
    } else {
        authResult = findRecordIter->second->GetAuthResult();
    }
    return ERR_OK;
}

#include "background_task_state_info.h"

ErrCode SetBackgroundTaskState(std::shared_ptr<BackgroundTaskStateInfo> taskParam);
    ErrCode GetBackgroundTaskState(std::shared_ptr<BackgroundTaskStateInfo> taskParam, uint32_t &authResult);

ErrCode SetBackgroundTaskStateInner(std::shared_ptr<BackgroundTaskStateInfo> taskParam);
    ErrCode GetBackgroundTaskStateInner(std::shared_ptr<BackgroundTaskStateInfo> taskParam, uint32_t &authResult);

}  // namespace BackgroundTaskMgr
}  // namespace OHOS
