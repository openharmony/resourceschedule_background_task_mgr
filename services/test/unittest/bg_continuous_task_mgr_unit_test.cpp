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
#include <chrono>
#include <thread>

#include "gtest/gtest.h"
#include "bgtask_common.h"
#include "background_task_subscriber.h"
#include "background_task_subscriber_proxy.h"
#include "bgtaskmgr_inner_errors.h"
#include "background_task_subscriber.h"
#include "bg_continuous_task_mgr.h"
#include "common_event_support.h"
#include "want_agent.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr int32_t SLEEP_TIME = 500;
static constexpr int32_t PROMPT_NUMS = 10;
static constexpr uint32_t CONFIGURE_ALL_MODES = 0x1FF;
}
class BgContinuousTaskMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    inline void SleepForFC()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    }

    static std::shared_ptr<BgContinuousTaskMgr> bgContinuousTaskMgr_;
};

std::shared_ptr<BgContinuousTaskMgr> BgContinuousTaskMgrTest::bgContinuousTaskMgr_ = nullptr;

void BgContinuousTaskMgrTest::SetUpTestCase()
{
    bgContinuousTaskMgr_ = BgContinuousTaskMgr::GetInstance();
    std::fill_n(std::back_inserter(bgContinuousTaskMgr_->continuousTaskText_), PROMPT_NUMS, "bgmode_test");
    std::fill_n(std::back_inserter(bgContinuousTaskMgr_->continuousTaskSubText_), PROMPT_NUMS, "bgmsubmode_test");
    bgContinuousTaskMgr_->isSysReady_.store(true);
}

void BgContinuousTaskMgrTest::TearDownTestCase() {}

void BgContinuousTaskMgrTest::SetUp() {}

void BgContinuousTaskMgrTest::TearDown()
{
    std::vector<string> dumpOption;
    dumpOption.emplace_back("-C");
    dumpOption.emplace_back("--cancel_all");
    std::vector<string> dumpInfo;
    bgContinuousTaskMgr_->ShellDump(dumpOption, dumpInfo);
}

class TestBackgroundTaskSubscriber : public BackgroundTaskSubscriber {
public:
    void OnContinuousTaskStart(const std::shared_ptr<ContinuousTaskCallbackInfo>
        &continuousTaskCallbackInfo) override {}

    void OnContinuousTaskStop(const std::shared_ptr<ContinuousTaskCallbackInfo>
        &continuousTaskCallbackInfo) override {}
};

/**
 * @tc.name: BgtaskCreateFileErr_001
 * @tc.desc: use batch api.
 * @tc.type: FUNC
 * @tc.require: issueI94UH9 issueI99HSB
 */
HWTEST_F(BgContinuousTaskMgrTest, BgtaskCreateFileErr_001, TestSize.Level3)
{
    bgContinuousTaskMgr_->continuousTaskText_.push_back("bgmode_test");
    bgContinuousTaskMgr_->cachedBundleInfos_.clear();
    EXPECT_EQ(bgContinuousTaskMgr_->GetBackgroundModeInfo(1, "abilityName"), 0u);
    CachedBundleInfo info = CachedBundleInfo();
    info.abilityBgMode_["ability1"] = CONFIGURE_ALL_MODES;
    info.appName_ = "Entry";
    bgContinuousTaskMgr_->cachedBundleInfos_.emplace(1, info);

    sptr<ContinuousTaskParam> taskParam = new (std::nothrow) ContinuousTaskParam(true, 0,
        std::make_shared<AbilityRuntime::WantAgent::WantAgent>(),
        "ability1", nullptr, "Entry", true, {1, 2, 3}, 1);
    EXPECT_EQ(bgContinuousTaskMgr_->StartBackgroundRunning(taskParam), ERR_BGTASK_CREATE_FILE_ERR);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS