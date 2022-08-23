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

#include "gtest/gtest.h"

#include "bgtaskmgr_inner_errors.h"
#include "background_task_mgr_helper.h"
#include "event_type.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {
class BgTaskClientUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void BgTaskClientUnitTest::SetUpTestCase() {}

void BgTaskClientUnitTest::TearDownTestCase() {}

void BgTaskClientUnitTest::SetUp() {}

void BgTaskClientUnitTest::TearDown() {}

/**
 * @tc.name: ReportStateChangeEvent_001
 * @tc.desc: report some state change infos to bgtask service api test.
 * @tc.type: FUNC
 * @tc.require: issueI5ND60
 */
HWTEST_F(BgTaskClientUnitTest, ReportStateChangeEvent_001, TestSize.Level1)
{
    EXPECT_EQ((int32_t)BackgroundTaskMgrHelper::ReportStateChangeEvent(
        EventType::DIS_COMP_CHANGE, "test"), (int32_t)ERR_OK);
}
}
}