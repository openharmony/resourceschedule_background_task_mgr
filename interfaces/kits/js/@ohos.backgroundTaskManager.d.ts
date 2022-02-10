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

import { AsyncCallback , Callback} from './basic';

/**
 * Manages background tasks.
 *
 * @since 7
 * @sysCap SystemCapability.ResourceSchedule.BackgroundTaskMgr.TransientTask
 */
declare namespace backgroundTaskManager {
    /**
     * The info of delay suspend.
     *
     * @name DelaySuspendInfo
     * @since 7
     * @sysCap SystemCapability.ResourceSchedule.BackgroundTaskMgr.TransientTask
     */
    interface DelaySuspendInfo {
        /**
         * The unique identifier of the delay request.
         */
        requestId: number;
        /**
         * The actual delay duration (ms).
         */
        actualDelayTime: number;
    }

    /**
     * Cancels delayed transition to the suspended state.
     *
     * @since 7
     * @sysCap SystemCapability.ResourceSchedule.BackgroundTaskMgr.TransientTask
     * @param requestId Indicates the identifier of the delay request.
     */
    function cancelSuspendDelay(requestId: number): void;

    /**
     * Obtains the remaining time before an application enters the suspended state.
     *
     * @since 7
     * @sysCap SystemCapability.ResourceSchedule.BackgroundTaskMgr.TransientTask
     * @param requestId Indicates the identifier of the delay request.
     * @return The remaining delay time
     */
    function getRemainingDelayTime(requestId: number, callback: AsyncCallback<number>): void;
    function getRemainingDelayTime(requestId: number): Promise<number>;

    /**
     * Requests delayed transition to the suspended state.
     *
     * @since 7
     * @sysCap SystemCapability.ResourceSchedule.BackgroundTaskMgr.TransientTask
     * @param reason Indicates the reason for delayed transition to the suspended state.
     * @param callback The callback delay time expired.
     * @return Info of delay request
     */
    function requestSuspendDelay(reason: string, callback: Callback<void>): DelaySuspendInfo;
}

export default backgroundTaskManager;
