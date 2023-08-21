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
#include "gmock/gmock.h"

#include "bgtaskmgr_inner_errors.h"
#include "background_task_subscriber.h"
#include "bg_efficiency_resources_mgr.h"
#include "resource_type.h"
#include "system_ability_definition.h"

#include "ibundle_manager_helper_mock.h"

using namespace testing::ext;
using namespace testing;
using testing::Return;
using testing::DoAll;

namespace OHOS {
namespace BackgroundTaskMgr {
static constexpr int32_t CPU_INDEX = 1;
static constexpr int32_t TIMER_INDEX = 3;
static const uint32_t MAX_RESOURCES_TYPE_NUM = ResourceTypeName.size();
static const uint32_t MAX_RESOURCES_MASK = (1 << MAX_RESOURCES_TYPE_NUM) - 1;

class MockEfficiencyResourcesMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static std::shared_ptr<BgEfficiencyResourcesMgr> bgEfficiencyResourcesMgr_;
    static std::shared_ptr<MockBundleManagerHelper> bundleManagerHelperMock_;
};

std::shared_ptr<BgEfficiencyResourcesMgr> MockEfficiencyResourcesMgrTest::bgEfficiencyResourcesMgr_ = nullptr;
std::shared_ptr<MockBundleManagerHelper> MockEfficiencyResourcesMgrTest::bundleManagerHelperMock_ = nullptr;

void MockEfficiencyResourcesMgrTest::SetUpTestCase()
{
    bgEfficiencyResourcesMgr_ = DelayedSingleton<BgEfficiencyResourcesMgr>::GetInstance();
}

void MockEfficiencyResourcesMgrTest::TearDownTestCase()
{
    bundleManagerHelperMock_.reset();
}

void MockEfficiencyResourcesMgrTest::SetUp()
{
    bundleManagerHelperMock_ = std::make_shared<MockBundleManagerHelper>();
    SetBundleManagerHelper(bundleManagerHelperMock_);
}

void MockEfficiencyResourcesMgrTest::TearDown()
{
    CleanBundleManagerHelper();
}

/**
 * @tc.name: GetExemptedResourceType_001
 * @tc.desc: should return 0 when failed to get application info
 * @tc.type: FUNC
 */
HWTEST_F(MockEfficiencyResourcesMgrTest, GetExemptedResourceType_001, TestSize.Level1)
{
    EXPECT_CALL(*bundleManagerHelperMock_, GetApplicationInfo(_, _, _, _)).WillOnce(Return(false));
    auto exemptedType = bgEfficiencyResourcesMgr_->GetExemptedResourceType(MAX_RESOURCES_MASK, -1, "");
    EXPECT_EQ(exemptedType, 0u);
}

/**
 * @tc.name: GetExemptedResourceType_002
 * @tc.desc: should return 0 when resource type is inconsistence with configuration
 * @tc.type: FUNC
 */
HWTEST_F(MockEfficiencyResourcesMgrTest, GetExemptedResourceType_002, TestSize.Level1)
{
    AppExecFwk::ApplicationInfo info {};
    info.resourcesApply = { TIMER_INDEX };
    EXPECT_CALL(*bundleManagerHelperMock_, GetApplicationInfo(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(info), Return(true)));
    auto exemptedType = bgEfficiencyResourcesMgr_->GetExemptedResourceType(ResourceType::CPU, -1, "");
    EXPECT_EQ(exemptedType, 0u);
}

/**
 * @tc.name: GetExemptedResourceType_003
 * @tc.desc: should return origin number when resource list contain 0
 * @tc.type: FUNC
 */
HWTEST_F(MockEfficiencyResourcesMgrTest, GetExemptedResourceType_003, TestSize.Level1)
{
    AppExecFwk::ApplicationInfo info {};
    info.resourcesApply = { 0 };
    EXPECT_CALL(*bundleManagerHelperMock_, GetApplicationInfo(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(info), Return(true)));
    auto exemptedType = bgEfficiencyResourcesMgr_->GetExemptedResourceType(MAX_RESOURCES_MASK, -1, "");
    EXPECT_EQ(exemptedType, MAX_RESOURCES_MASK);
}

/**
 * @tc.name: GetExemptedResourceType_004
 * @tc.desc: should return CPU when resource list contain CPU_INDEX
 * @tc.type: FUNC
 */
HWTEST_F(MockEfficiencyResourcesMgrTest, GetExemptedResourceType_004, TestSize.Level1)
{
    AppExecFwk::ApplicationInfo info {};
    info.resourcesApply = { CPU_INDEX };
    EXPECT_CALL(*bundleManagerHelperMock_, GetApplicationInfo(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(info), Return(true)));
    auto exemptedType = bgEfficiencyResourcesMgr_->GetExemptedResourceType(MAX_RESOURCES_MASK, -1, "");
    EXPECT_EQ(exemptedType, ResourceType::CPU);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
