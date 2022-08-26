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

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {
class BgTaskClientUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void BgTaskClientUnitTest::SetUpTestCase() {}

void BgTaskClientUnitTest::TearDownTestCase() {}

void BgTaskClientUnitTest::SetUp() {}

void BgTaskClientUnitTest::TearDown() {}

/**
 * @tc.name: StopContinuousTask_001
 * @tc.desc: request stop target continuous task api test.
 * @tc.type: FUNC
 * @tc.require: issueI5IRJK
 */
HWTEST_F(BgTaskClientUnitTest, StopContinuousTask_001, TestSize.Level1)
{
    EXPECT_EQ((int32_t)BackgroundTaskMgrHelper::StopContinuousTask(1, 1, 1), (int32_t)ERR_OK);
}
}
}