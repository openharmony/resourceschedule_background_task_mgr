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

#include <iostream>
#include <memory>
#include <thread>

#include "background_task_mgr_helper.h"
#include "background_task_subscriber.h"
#include "bgtask_observer.h"

namespace {
static constexpr int32_t DELAY_TIME = 1000;
static constexpr int32_t MAX_CHAR_NUMS = 100;

void CycleTest()
{
    auto bgtaskObserver = std::make_shared<OHOS::BackgroundTaskMgr::BgTaskObserver>();
    if (OHOS::BackgroundTaskMgr::BackgroundTaskMgrHelper::SubscribeBackgroundTask(*bgtaskObserver)
        == OHOS::ERR_OK) {
        std::cout << "register succeed in cycletest" << std::endl;
    }
    while (true) {
        if (bgtaskObserver->isRemoteDied_.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_TIME));
            if (OHOS::BackgroundTaskMgr::BackgroundTaskMgrHelper::SubscribeBackgroundTask(*bgtaskObserver)
                == OHOS::ERR_OK) {
                std::cout << "try to subscribe again succeed" << std::endl;
                bgtaskObserver->isRemoteDied_.store(false);
            }
        }
    }
}

void PatchTest(int nums)
{
    std::list<std::shared_ptr<OHOS::BackgroundTaskMgr::BgTaskObserver>> observerList;
    for (int i = 0; i < nums; i++) {
        auto bgtaskObserver = std::make_shared<OHOS::BackgroundTaskMgr::BgTaskObserver>();
        observerList.emplace_back(bgtaskObserver);
    }

    int index = 1;
    for (auto observer : observerList) {
        if (OHOS::BackgroundTaskMgr::BackgroundTaskMgrHelper::SubscribeBackgroundTask(*observer)
            == OHOS::ERR_OK) {
            std::cout << "register succeed No." << index << std::endl;
        }
        index++;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_TIME));
    index = 1;
    for (auto observer : observerList) {
        if (OHOS::BackgroundTaskMgr::BackgroundTaskMgrHelper::UnsubscribeBackgroundTask(*observer)
            == OHOS::ERR_OK) {
            std::cout << "unregister succeed No." << index << std::endl;
        }
        index++;
    }
}
}

int main(int argc, char *argv[])
{
    auto bgtaskObserver = std::make_shared<OHOS::BackgroundTaskMgr::BgTaskObserver>();
    char command[MAX_CHAR_NUMS];
    int nums;
    while (std::cin >> command >> nums) {
        std::string commandStr = command;
        if (commandStr == "register") {
            if (OHOS::BackgroundTaskMgr::BackgroundTaskMgrHelper::SubscribeBackgroundTask(*bgtaskObserver)
                == OHOS::ERR_OK) {
                std::cout << "register once succeed" << std::endl;
            }
        }
        if (commandStr == "unregister") {
            if (OHOS::BackgroundTaskMgr::BackgroundTaskMgrHelper::UnsubscribeBackgroundTask(*bgtaskObserver)
                == OHOS::ERR_OK) {
                std::cout << "unregister once succeed" << std::endl;
            }
        }
        if (commandStr == "patchtest") {
            PatchTest(nums);
        }
        if (commandStr == "cycletest") {
            CycleTest();
        }
    }
    return 0;
}