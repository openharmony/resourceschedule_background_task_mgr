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
import backgroundTaskManager from '@ohos.backgroundTaskManager'

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
     * @tc.require: issuesI5OD7X
     */
    it("EfficiencyResourcesJsTest001", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest001---------------------------');
        let resRequest = {
            resourceTypes: 1,
            isApply: true,
            timeOut: 10,
            reason: "apply",
            isPersist: false,
            isProcess: false,
        };
        let res = backgroundTaskManager.applyEfficiencyResources(resRequest);
        expect(res).assertEqual(false);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest002
     * @tc.desc: test reset a efficiency resource
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X
     */
    it("EfficiencyResourcesJsTest002", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest002---------------------------');
        let resRequest = {
            resourceTypes: 1,
            isApply: false,
            timeOut: 10,
            reason: "apply"
        };
        let res = backgroundTaskManager.applyEfficiencyResources(resRequest);
        expect(res).assertEqual(false);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest003
     * @tc.desc: test apply a efficiency resource without resourceTypes
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X
     */
    it("EfficiencyResourcesJsTest003", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest003---------------------------');
        let resRequest = {
            isApply: true,
            timeOut: 0,
            reason: "apply"
        };
        let res = backgroundTaskManager.applyEfficiencyResources(resRequest);
        expect(res).assertEqual(false);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest004
     * @tc.desc: test apply a efficiency resource without isApply
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X
     */
    it("EfficiencyResourcesJsTest004", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest004--------------------------- ');
        let resRequest = {
            resourceTypes: 1,
            timeOut: 10,
            reason: "apply"
        };
        let res = backgroundTaskManager.applyEfficiencyResources(resRequest);
        expect(res).assertEqual(false);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest005
     * @tc.desc: test apply a efficiency resource without timeOut
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X
     */
    it("EfficiencyResourcesJsTest005", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest005---------------------------');
        let resRequest = {
            resourceTypes: 1,
            isApply: true,
            reason: "apply"
        };
        let res = backgroundTaskManager.applyEfficiencyResources(resRequest);
        expect(res).assertEqual(false);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest006
     * @tc.desc: test apply a efficiency resource without reason
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X
     */
    it("EfficiencyResourcesJsTest006", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest006---------------------------');
        let resRequest = {
            resourceTypes: 1,
            isApply: true,
            timeOut: 10
        };
        let res = backgroundTaskManager.applyEfficiencyResources(resRequest);
        console.info('----------------------res---------------------------false ');
        expect(res).assertEqual(false);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest007
     * @tc.desc: test apply a efficiency resource with timeout equals to 0
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X
     */
    it("EfficiencyResourcesJsTest007", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest007---------------------------');
        let resRequest = {
            resourceTypes: 1,
            isApply: true,
            timeOut: 0,
            reason: "apply",
            isPersist: false,
        };
        let res = backgroundTaskManager.applyEfficiencyResources(resRequest);
        expect(res).assertEqual(false);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest008
     * @tc.desc: test apply a efficiency resource with resourceTypes equals to 0
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X
     */
    it("EfficiencyResourcesJsTest008", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest008---------------------------');
        let resRequest = {
            resourceTypes: 0,
            isApply: true,
            timeOut: 10,
            reason: "apply",
        };
        let res = backgroundTaskManager.applyEfficiencyResources(resRequest);
        expect(res).assertEqual(false);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest009
     * @tc.desc: test apply a efficiency resource with isPersist
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X
     */
    it("EfficiencyResourcesJsTest009", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest009---------------------------');
        let resRequest = {
            resourceTypes: 1,
            isApply: true,
            timeOut: 0,
            reason: "apply",
            isPersist: true,
        };
        let res = backgroundTaskManager.applyEfficiencyResources(resRequest);
        expect(res).assertEqual(false);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest010
     * @tc.desc: test apply a efficiency resource with isProcess
     * @tc.type: FUNC
     * @tc.require: issuesI5OD7X
     */
    it("EfficiencyResourcesJsTest010", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest010---------------------------');
        let resRequest = {
            resourceTypes: 1,
            isApply: true,
            timeOut: 10,
            reason: "apply",
            isProcess: true,
        };
        let res = backgroundTaskManager.applyEfficiencyResources(resRequest);
        expect(res).assertEqual(true);
        done();
    })

    /*
     * @tc.name:EfficiencyResourcesJsTest011
     * @tc.desc:verify applyEfficiencyResources  workTime
     * @tc.type: FUNC
     */
    it("EfficiencyResourcesJsTest011", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest011---------------------------');
        let resRequest = {
            resourceTypes: backgroundTaskManager.ResourceType.CPU,
            isApply: true,
            timeOut: 10,
            reason: "apply",
            isProcess: true,
        };
        let startTime = (new Date()).valueOf()
        backgroundTaskManager.applyEfficiencyResources(resRequest);
        let endTime = (new Date()).valueOf()
        let workTime = endTime - startTime 
        if (workTime < 50) {
            expect(true).assertTrue()
        } else {
            expect(false).assertTrue()
        }
        done();
    })

    /*
     * @tc.name:EfficiencyResourcesJsTest012
     * @tc.desc:verify resetAllEfficiencyResources workTime
     * @tc.type: FUNC
     */
    it("EfficiencyResourcesJsTest012", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest012---------------------------');
        let resRequest = {
            resourceTypes: backgroundTaskManager.ResourceType.CPU,
            isApply: true,
            timeOut: 10,
            reason: "apply",
            isProcess: true,
        };
        backgroundTaskManager.applyEfficiencyResources(resRequest);
        let startTime = (new Date()).valueOf()
        backgroundTaskManager.resetAllEfficiencyResources();
        let endTime = (new Date()).valueOf()
        let workTime = endTime - startTime
        if (workTime < 50) {
            expect(true).assertTrue()
        } else {
            expect(false).assertTrue()
        }
        done();
    })
})
