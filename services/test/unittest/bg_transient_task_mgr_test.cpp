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
#include <ctime>

#define private public
#include "bg_transient_task_mgr.h"
#include "background_task_subscriber.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {
class TestBackgroundTaskSubscriber : public BackgroundTaskSubscriber {
public:
    void OnTransientTaskStart(const std::shared_ptr<TransientTaskAppInfo>& info) override {}
    void OnTransientTaskEnd(const std::shared_ptr<TransientTaskAppInfo>& info) override {}
};

class BgTransientTaskMgrTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: SubscribeTransientTask_001
 * @tc.desc: subscribe transient task event.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET AR000GH86P AR000GH86O
 */
HWTEST_F(BgTransientTaskMgrTest, SubscribeTransientTask_001, TestSize.Level1)
{
    auto subscriber = std::make_shared<TestBackgroundTaskSubscriber>();
    auto ret = DelayedSingleton<BgTransientTaskMgr>::GetInstance()->SubscribeBackgroundTask(subscriber->GetImpl());
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.name: SubscribeTransientTask_002
 * @tc.desc: subscribe transient task event.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET AR000GH86P AR000GH86O
 */
HWTEST_F(BgTransientTaskMgrTest, SubscribeTransientTask_002, TestSize.Level1)
{
    auto ret = DelayedSingleton<BgTransientTaskMgr>::GetInstance()->SubscribeBackgroundTask(nullptr);
    EXPECT_FALSE(ret == ERR_OK);
}

/**
 * @tc.name: SubscribeTransientTask_003
 * @tc.desc: subscribe transient task event.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET AR000GH86P AR000GH86O
 */
HWTEST_F(BgTransientTaskMgrTest, SubscribeTransientTask_003, TestSize.Level1)
{
    auto subscriber = std::make_shared<TestBackgroundTaskSubscriber>();
    auto ret = DelayedSingleton<BgTransientTaskMgr>::GetInstance()->SubscribeBackgroundTask(subscriber->GetImpl());
    EXPECT_TRUE(ret == ERR_OK);

    ret = DelayedSingleton<BgTransientTaskMgr>::GetInstance()->SubscribeBackgroundTask(subscriber->GetImpl());
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.name: UnsubscribeTransientTask_001
 * @tc.desc: unsubscribe transient task event.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET AR000GH86P AR000GH86O
 */
HWTEST_F(BgTransientTaskMgrTest, UnsubscribeTransientTask_001, TestSize.Level1)
{
    auto subscriber = std::make_shared<TestBackgroundTaskSubscriber>();
    auto ret = DelayedSingleton<BgTransientTaskMgr>::GetInstance()->SubscribeBackgroundTask(subscriber->GetImpl());
    EXPECT_TRUE(ret == ERR_OK);

    usleep(1000);

    ret = DelayedSingleton<BgTransientTaskMgr>::GetInstance()->UnsubscribeBackgroundTask(subscriber->GetImpl());
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.name: UnsubscribeTransientTask_002
 * @tc.desc: unsubscribe transient task event.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET AR000GH86P AR000GH86O
 */
HWTEST_F(BgTransientTaskMgrTest, UnsubscribeTransientTask_002, TestSize.Level1)
{
    auto subscriber = std::make_shared<TestBackgroundTaskSubscriber>();
    auto ret = DelayedSingleton<BgTransientTaskMgr>::GetInstance()->UnsubscribeBackgroundTask(subscriber->GetImpl());
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.name: UnsubscribeTransientTask_003
 * @tc.desc: unsubscribe transient task event.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET AR000GH86P AR000GH86O
 */
HWTEST_F(BgTransientTaskMgrTest, UnsubscribeTransientTask_003, TestSize.Level1)
{
    auto ret = DelayedSingleton<BgTransientTaskMgr>::GetInstance()->UnsubscribeBackgroundTask(nullptr);
    EXPECT_FALSE(ret == ERR_OK);
}

/**
 * @tc.name: Marshalling_001
 * @tc.desc: marshalling transient task app info.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET AR000GH86Q
 */
HWTEST_F(BgTransientTaskMgrTest, Marshalling_001, TestSize.Level1)
{
    auto appInfo = std::make_shared<TransientTaskAppInfo>();
    MessageParcel data;
    EXPECT_TRUE(appInfo->Marshalling(data));

    EXPECT_TRUE(TransientTaskAppInfo::Unmarshalling(data) != nullptr);
}

/**
 * @tc.name: Unmarshalling_001
 * @tc.desc: unmarshalling transient task app info.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET AR000GH86Q
 */
HWTEST_F(BgTransientTaskMgrTest, Unmarshalling_001, TestSize.Level1)
{
    MessageParcel data;
    EXPECT_TRUE(TransientTaskAppInfo::Unmarshalling(data) == nullptr);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
