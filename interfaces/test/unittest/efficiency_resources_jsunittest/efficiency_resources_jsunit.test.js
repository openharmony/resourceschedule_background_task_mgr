/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
import backgroundTaskManager from '@ohos.resourceschedule.backgroundTaskManager'

import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index'

describe("EfficiencyResourcesJsTest", function () {
    beforeAll(function() {
        /*
         * @tc.setup: setup invoked before all testcases
         */
        console.info('beforeAll called')
    })

    afterAll(function() {
        /*
         * @tc.teardown: teardown invoked after all testcases
         */
        console.info('afterAll called')
    })

    beforeEach(function() {
        /*
         * @tc.setup: setup invoked before each testcases
         */
        console.info('beforeEach called')
    })

    afterEach(function() {
        /*
         * @tc.teardown: teardown invoked after each testcases
         */
        backgroundTaskManager.resetAllEfficiencyResources();
        console.info('afterEach called')
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest001
     * @tc.desc: test apply a efficiency resource
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X issueI5QJR8
     */
    it("EfficiencyResourcesJsTest001", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest001---------------------------');
        let resRequest = {
            resourceTypes: 1,
            isApply: true,
            timeOut: 10,
            reason: "applyEfficiencyResourcesJsTest001",
            isPersist: false,
            isProcess: false,
        };
        try {
            backgroundTaskManager.applyEfficiencyResources(resRequest);
            expect(false).assertEqual(true);
        } catch (error) {
            expect(true).assertEqual(true);
        }
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest002
     * @tc.desc: test reset a efficiency resource
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X issueI5QJR8
     */
    it("EfficiencyResourcesJsTest002", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest002---------------------------');
        let resRequest = {
            resourceTypes: 1,
            isApply: false,
            timeOut: 10,
            reason: "applyEfficiencyResourcesJsTest002"
        };
        try {
            backgroundTaskManager.applyEfficiencyResources(resRequest);
            expect(false).assertEqual(true);
        } catch (error) {
            expect(true).assertEqual(true);
        }
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest003
     * @tc.desc: test apply a efficiency resource without resourceTypes
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X issueI5QJR8
     */
    it("EfficiencyResourcesJsTest003", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest003---------------------------');
        let resRequest = {
            isApply: true,
            timeOut: 0,
            reason: "applyEfficiencyResourcesJsTest003"
        };
        try {
            backgroundTaskManager.applyEfficiencyResources(resRequest);
            expect(false).assertEqual(true);
        } catch (error) {
            expect(true).assertEqual(true);
        }
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest004
     * @tc.desc: test apply a efficiency resource without isApply
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X issueI5QJR8
     */
    it("EfficiencyResourcesJsTest004", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest004--------------------------- ');
        let resRequest = {
            resourceTypes: 1,
            timeOut: 10,
            reason: "applyEfficiencyResourcesJsTest004"
        };
        try {
            backgroundTaskManager.applyEfficiencyResources(resRequest);
            expect(false).assertEqual(true);
        } catch (error) {
            expect(true).assertEqual(true);
        }
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest005
     * @tc.desc: test apply a efficiency resource without timeOut
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X issueI5QJR8
     */
    it("EfficiencyResourcesJsTest005", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest005---------------------------');
        let resRequest = {
            resourceTypes: 1,
            isApply: true,
            reason: "applyEfficiencyResourcesJsTest005"
        };
        try {
            backgroundTaskManager.applyEfficiencyResources(resRequest);
            expect(false).assertEqual(true);
        } catch (error) {
            expect(true).assertEqual(true);
        }
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest006
     * @tc.desc: test apply a efficiency resource without reason
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X issueI5QJR8
     */
    it("EfficiencyResourcesJsTest006", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest006---------------------------');
        let resRequest = {
            resourceTypes: 1,
            isApply: true,
            timeOut: 10
        };
        try {
            backgroundTaskManager.applyEfficiencyResources(resRequest);
            expect(false).assertEqual(true);
        } catch (error) {
            expect(true).assertEqual(true);
        }
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest007
     * @tc.desc: test apply a efficiency resource with timeout equals to 0
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X issueI5QJR8
     */
    it("EfficiencyResourcesJsTest007", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest007---------------------------');
        let resRequest = {
            resourceTypes: 1,
            isApply: true,
            timeOut: 0,
            reason: "applyEfficiencyResourcesJsTest007",
            isPersist: false,
        };
        try {
            backgroundTaskManager.applyEfficiencyResources(resRequest);
            expect(false).assertEqual(true);
        } catch (error) {
            expect(true).assertEqual(true);
        }
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest008
     * @tc.desc: test apply a efficiency resource with resourceTypes equals to 0
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X issueI5QJR8
     */
    it("EfficiencyResourcesJsTest008", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest008---------------------------');
        let resRequest = {
            resourceTypes: 0,
            isApply: true,
            timeOut: 10,
            reason: "applyEfficiencyResourcesJsTest008",
        };
        try {
            backgroundTaskManager.applyEfficiencyResources(resRequest);
            expect(false).assertEqual(true);
        } catch (error) {
            expect(true).assertEqual(true);
        }
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest009
     * @tc.desc: test apply a efficiency resource with isPersist
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X issueI5QJR8
     */
    it("EfficiencyResourcesJsTest009", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest009---------------------------');
        let resRequest = {
            resourceTypes: 1,
            isApply: true,
            timeOut: 0,
            reason: "applyEfficiencyResourcesJsTest009",
            isPersist: true,
        };
        try {
            backgroundTaskManager.applyEfficiencyResources(resRequest);
            expect(false).assertEqual(true);
        } catch (error) {
            expect(true).assertEqual(true);
        }
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest010
     * @tc.desc: test apply a efficiency resource with isProcess
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X issueI5QJR8
     */
    it("EfficiencyResourcesJsTest010", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest010---------------------------');
        let resRequest = {
            resourceTypes: 1,
            isApply: true,
            timeOut: 10,
            reason: "applyEfficiencyResourcesJsTest010",
            isProcess: true,
        };
        try {
            backgroundTaskManager.applyEfficiencyResources(resRequest);
            expect(false).assertEqual(true);
        } catch (error) {
            expect(true).assertEqual(true);
        }
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest011
     * @tc.desc: test apply a efficiency resource with cpuLevel
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X issueI5QJR8
     */
    it("EfficiencyResourcesJsTest011", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest011---------------------------');
        let resRequest = {
            resourceTypes: 1,
            isApply: true,
            timeOut: 10,
            reason: "EfficiencyResourcesJsTest011",
            isProcess: true,
            cpuLevel: backgroundTaskManager.EfficiencyResourcesCpuLevel.MEDIUM_CPU,
        };
        try {
            backgroundTaskManager.applyEfficiencyResources(resRequest);
            expect(false).assertEqual(true);
        } catch (error) {
            expect(true).assertEqual(true);
        }
        done();
    })
})
