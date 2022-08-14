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

#define private public

#include "json/json.h"
#include "bgtaskmgr_inner_errors.h"
#include "background_task_subscriber.h"
#include "bg_efficiency_resources_mgr.h"
#include "background_task_subscriber.h"

using namespace testing::ext;

namespace OHOS {
namespace BackgroundTaskMgr {

static constexpr int32_t SLEEP_TIME = 500;

class BgEfficiencyResourcesMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    inline void SleepForFC(){
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    }

    static std::shared_ptr<BgEfficiencyResourcesMgr> bgEfficiencyResourcesMgr_;
};

std::shared_ptr<BgEfficiencyResourcesMgr> BgEfficiencyResourcesMgrTest::bgEfficiencyResourcesMgr_ = nullptr;

void BgEfficiencyResourcesMgrTest::SetUpTestCase()
{
    bgEfficiencyResourcesMgr_ = DelayedSingleton<BgEfficiencyResourcesMgr>::GetInstance();
}

void BgEfficiencyResourcesMgrTest::TearDownTestCase() {}

void BgEfficiencyResourcesMgrTest::SetUp() {}

void BgEfficiencyResourcesMgrTest::TearDown()
{
}

class TestBackgroundTaskSubscriber : public BackgroundTaskMgr::BackgroundTaskSubscriber {
public:
    void OnAppEfficiencyResourcesApply(const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) override {}

    void OnAppEfficiencyResourcesReset(const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) override {}
    
    void OnEfficiencyResourcesApply(const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) override {}

    void OnEfficiencyResourcesReset(const std::shared_ptr<ResourceCallbackInfo> &resourceInfo) override {}
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS