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
     */
    it("EfficiencyResourcesJsTest001", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest001---------------------------');
        let resRequest = {
            resourceType: 1,
            isApply: true,
            timeOut: 10,
            reason: "apply",
        }
        var res = backgroundTaskManager.applyEfficiencyResources(workInfo);
        expect(res).assertEqual(true);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest002
     * @tc.desc: test apply a efficiency resource
     * @tc.type: FUNC
     */
    it("EfficiencyResourcesJsTest002", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest002---------------------------');
        let resRequest = {
            resourceType: 1,
            isApply: true,
            timeOut: 10,
            reason: "apply"
        }
        var res = backgroundTaskManager.applyEfficiencyResources(resRequest);
        expect(res).assertEqual(true);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest003
     * @tc.desc: test apply a efficiency resource without resourceType
     * @tc.type: FUNC
     */
    it("EfficiencyResourcesJsTest003", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest003---------------------------');
        let resRequest = {
            isApply: true,
            timeOut: 0,
            reason: "apply"
        }
        var res = backgroundTaskManager.applyEfficiencyResources(resRequest);
        expect(res).assertEqual(false);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest004
     * @tc.desc: test apply a efficiency resource without isApply
     * @tc.type: FUNC
     */
    it("EfficiencyResourcesJsTest004", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest004---------------------------');
        let resRequest = {
            resourceType: 1,
            timeOut: 10,
            reason: "apply"
        }
        var res = backgroundTaskManager.applyEfficiencyResources(resRequest);
        expect(res).assertEqual(false);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest005
     * @tc.desc: test apply a efficiency resource without timeOut
     * @tc.type: FUNC
     */
    it("EfficiencyResourcesJsTest005", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest005---------------------------');
        let resRequest = {
            resourceType: 1,
            isApply: true,
            reason: "apply"
        }
        var res = backgroundTaskManager.applyEfficiencyResources(resRequest);
        expect(res).assertEqual(false);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest006
     * @tc.desc: test apply a efficiency resource without reason
     * @tc.type: FUNC
     */
    it("EfficiencyResourcesJsTest006", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest006---------------------------');
        let resRequest = {
            resourceType: 1,
            isApply: true,
            timeOut: 10
        }
        var res = backgroundTaskManager.applyEfficiencyResources(resRequest);
        expect(res).assertEqual(false);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest007
     * @tc.desc: test apply a efficiency resource with timeout equals to 0
     * @tc.type: FUNC
     */
    it("EfficiencyResourcesJsTest007", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest007---------------------------');
        let resRequest = {
            resourceType: 1,
            isApply: true,
            timeOut: 0,
            reason: "apply",
            isPersist: false,
        }
        var res = backgroundTaskManager.applyEfficiencyResources(workInfo);
        expect(res).assertEqual(false);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest008
     * @tc.desc: test apply a efficiency resource with resourceType equals to 0
     * @tc.type: FUNC
     */
    it("EfficiencyResourcesJsTest008", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest008---------------------------');
        let resRequest = {
            resourceType: 0,
            isApply: true,
            timeOut: 10,
            reason: "apply",
        }
        var res = backgroundTaskManager.applyEfficiencyResources(workInfo);
        expect(res).assertEqual(false);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest009
     * @tc.desc: test apply a efficiency resource with isPersist
     * @tc.type: FUNC
     */
    it("EfficiencyResourcesJsTest009", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest009---------------------------');
        let resRequest = {
            resourceType: 1,
            isApply: true,
            timeOut: 0,
            reason: "apply",
            isPersist: true,
        }
        var res = backgroundTaskManager.applyEfficiencyResources(workInfo);
        expect(res).assertEqual(true);
        done();
    })

    /*
     * @tc.name: EfficiencyResourcesJsTest010
     * @tc.desc: test apply a efficiency resource with isProcess
     * @tc.type: FUNC
     */
    it("EfficiencyResourcesJsTest010", 0, async function (done) {
        console.info('----------------------EfficiencyResourcesJsTest010---------------------------');
        let resRequest = {
            resourceType: 1,
            isApply: true,
            timeOut: 0,
            reason: "apply",
            isProcess: true,
        }
        var res = backgroundTaskManager.applyEfficiencyResources(workInfo);
        expect(res).assertEqual(true);
        done();
    })
})
