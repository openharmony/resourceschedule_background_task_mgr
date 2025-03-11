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

#include "notification_helper.h"
#include "ans_notification.h"
#include "singleton.h"

namespace OHOS {
namespace {
int32_t g_publishContinuousTaskNotificationFlag = 0;
int32_t g_cancelContinuousTaskNotificationFlag = 0;
int32_t g_getAllActiveNotificationsFlag = 0;
}

void SetPublishContinuousTaskNotificationFlag(int32_t flag)
{
    g_publishContinuousTaskNotificationFlag = flag;
}

void SetCancelContinuousTaskNotificationFlag(int32_t flag)
{
    g_cancelContinuousTaskNotificationFlag = flag;
}

void SetGetAllActiveNotificationsFlag(int32_t flag)
{
    g_getAllActiveNotificationsFlag = flag;
}

namespace Notification {
namespace {
constexpr int32_t TEST_NUM_ONE = 1;
constexpr int32_t TEST_NUM_TWO = 2;
}

ErrCode NotificationHelper::PublishContinuousTaskNotification(const NotificationRequest &request)
{
    if (g_publishContinuousTaskNotificationFlag == 1) {
        return -1;
    }
    return 0;
}

ErrCode NotificationHelper::CancelContinuousTaskNotification(const std::string &label, int32_t notificationId)
{
    if (g_cancelContinuousTaskNotificationFlag == 1) {
        return -1;
    }
    return 0;
}

ErrCode NotificationHelper::SubscribeNotification(const NotificationSubscriber &subscriber)
{
    return 0;
}

ErrCode NotificationHelper::SubscribeNotification(
    const NotificationSubscriber &subscriber, const NotificationSubscribeInfo &subscribeInfo)
{
    return 0;
}

ErrCode NotificationHelper::UnSubscribeNotification(NotificationSubscriber &subscriber)
{
    return 0;
}

ErrCode NotificationHelper::UnSubscribeNotification(
    NotificationSubscriber &subscriber, NotificationSubscribeInfo subscribeInfo)
{
    return 0;
}

ErrCode NotificationHelper::GetAllActiveNotifications(std::vector<sptr<Notification>> &notification)
{
    std::shared_ptr<NotificationNormalContent> normalContent
        = std::make_shared<NotificationNormalContent>();
    normalContent->SetTitle("appName1");
    normalContent->SetText("prompt1");
    if (g_getAllActiveNotificationsFlag == TEST_NUM_ONE) {
        sptr<NotificationRequest> notificationRequest1 = sptr<NotificationRequest>(new NotificationRequest());
        notificationRequest1->SetContent(std::make_shared<NotificationContent>(normalContent));
        notificationRequest1->SetLabel("label1");
        notificationRequest1->SetCreatorUid(0);
        auto notification1 = sptr<Notification>(new Notification(notificationRequest1));
        sptr<NotificationRequest> notificationRequest2 = sptr<NotificationRequest>(new NotificationRequest());
        notificationRequest2->SetContent(std::make_shared<NotificationContent>(normalContent));
        notificationRequest2->SetLabel("label");
        notificationRequest2->SetCreatorUid(-1);
        auto notification2 = sptr<Notification>(new Notification(notificationRequest1));
        sptr<NotificationRequest> notificationRequest3 = sptr<NotificationRequest>(new NotificationRequest());
        notificationRequest3->SetContent(std::make_shared<NotificationContent>(normalContent));
        notificationRequest3->SetLabel("label1");
        notificationRequest3->SetCreatorUid(-1);
        auto notification3 = sptr<Notification>(new Notification(notificationRequest1));
        sptr<NotificationRequest> notificationRequest4 = sptr<NotificationRequest>(new NotificationRequest());
        notificationRequest4->SetContent(std::make_shared<NotificationContent>(normalContent));
        notificationRequest4->SetLabel("label");
        notificationRequest4->SetCreatorUid(0);
        auto notification4 = sptr<Notification>(new Notification(notificationRequest1));
        notification.emplace_back(notification1);
        notification.emplace_back(notification2);
        notification.emplace_back(notification3);
        notification.emplace_back(notification4);
        return 0;
    }

    if (g_getAllActiveNotificationsFlag == TEST_NUM_TWO) {
        sptr<NotificationRequest> notificationRequest = sptr<NotificationRequest>(new NotificationRequest());
        notificationRequest->SetContent(std::make_shared<NotificationContent>(normalContent));
        notificationRequest->SetLabel("label");
        notificationRequest->SetCreatorUid(0);
        auto notification5 = sptr<Notification>(new Notification(notificationRequest));
        notification.emplace_back(notification5);
        return 0;
    }
    return 0;
}

ErrCode NotificationHelper::GetActiveNotifications(std::vector<sptr<NotificationRequest>> &request)
{
    std::shared_ptr<NotificationNormalContent> normalContent
        = std::make_shared<NotificationNormalContent>();
    normalContent->SetTitle("appName1");
    normalContent->SetText("prompt1");
    sptr<NotificationRequest> notificationRequest = sptr<NotificationRequest>(new NotificationRequest());
    notificationRequest->SetContent(std::make_shared<NotificationContent>(normalContent));
    notificationRequest->SetLabel("label");
    notificationRequest->SetCreatorUid(0);
    request.emplace_back(notificationRequest);
    return ERR_OK;
}
}  // namespace Notification
}  // namespace OHOS
