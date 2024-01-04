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

import notification from '@ohos.notification';
import wantAgent from '@ohos.wantAgent';
import particleAbility from '@ohos.ability.particleAbility'
import backgroundTaskManager from '@ohos.backgroundTaskManager'
import featureAbility from '@ohos.ability.featureAbility'

import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index'

describe("ContinuousTaskJsTest", function () {
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
        console.info('afterEach called')
        particleAbility.cancelBackgroundRunning();
        setTimeout(() => { }, 500);
        backgroundTaskManager.stopBackgroundRunning(featureAbility.getContext());
        setTimeout(() => { }, 500);
    })

    /*
     * @tc.name:ContinuousTaskJsTest001
     * @tc.desc:verify new startBackgroundrunning interface promise mode work properly
     * @tc.type: FUNC
     * @tc.require: SR000GGT7U AR000GH6ER AR000GH6EM AR000GH6EN AR000GH6EO
     */
    it("ContinuousTaskJsTest001", 0, async function (done) {
        let wantAgentInfo = {
            wants: [
                {
                    bundleName: "com.continuoustask.test",
                    abilityName: "com.continuoustask.test.MainAbility"
                }
            ],
            operationType: 2,
            requestCode: 0,
            wantAgentFlags: [3]
        };
        wantAgent.getWantAgent(wantAgentInfo).then((data) => {
            backgroundTaskManager.startBackgroundRunning(featureAbility.getContext(),
                backgroundTaskManager.BackgroundMode.DATA_TRANSFER, data).then(() => {
                    console.log("ContinuousTaskJsTest001 startBackgroundRunning success");
                    expect(true).assertTrue();
                    setTimeout(() => {
                        done();
                    }, 500);
                }).catch((err) => {
                    expect(false).assertTrue();
                    console.log("ContinuousTaskJsTest001 startBackgroundRunning failure");
                    setTimeout(() => {
                        done();
                    }, 500);
                });
        });
    })

    /*
     * @tc.name:ContinuousTaskJsTest002
     * @tc.desc:verify new startBackgroundrunning interface callback mode work properly
     * @tc.type: FUNC
     * @tc.require: SR000GGT7T AR000GH6ER AR000GH6EP AR000GJ9PR AR000GH6G8
     */
    it("ContinuousTaskJsTest002", 0, async function (done) {
        function conTaskCallback(err, data) {
            if (err) {
                console.info('ContinuousTaskJsTest002 startBackgroundRunning failed');
                expect(false).assertTrue();
            } else {
                console.info('ContinuousTaskJsTest002 startBackgroundRunning succeed');
                expect(true).assertTrue();
            }
            setTimeout(() => {
                done();
            }, 500);
        }
        let wantAgentInfo = {
            wants: [
                {
                    bundleName: "com.continuoustask.test",
                    abilityName: "com.continuoustask.test.MainAbility2"
                }
            ],
            operationType: 2,
            requestCode: 2,
            wantAgentFlags: [3]
        };
        wantAgent.getWantAgent(wantAgentInfo).then((data) => {
            backgroundTaskManager.startBackgroundRunning(featureAbility.getContext(),
                backgroundTaskManager.BackgroundMode.DATA_TRANSFER, data, conTaskCallback);
        });
    })

    /*
     * @tc.name:ContinuousTaskJsTest003
     * @tc.desc:verify old startBackgroundrunning interface promise mode work properly
     * @tc.type: FUNC
     * @tc.require: SR000GGT7V AR000GH6ER AR000GH6EM AR000GH6G9 AR000GH56K
     */
    it("ContinuousTaskJsTest003", 0, async function (done) {
        let wantAgentInfo = {
            wants: [
                {
                    bundleName: "com.continuoustask.test",
                    abilityName: "com.continuoustask.test.MainAbility3"
                }
            ],
            operationType: 2,
            requestCode: 3,
            wantAgentFlags: [3]
        };
        wantAgent.getWantAgent(wantAgentInfo).then((data) => {
            let basicContent3 = {
                title: "title3",
                text: "text3"
            };

            let notificationContent = {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: basicContent3
            };

            let request = {
                content: notificationContent,
                wantAgent: data
            }

            let id = 1;

            particleAbility.startBackgroundRunning(id, request).then((data) => {
                console.log("ContinuousTaskJsTest003 startBackgroundRunning success");
                expect(true).assertTrue();
                setTimeout(() => {
                    done();
                }, 500);
            }).catch((err) => {
                expect(false).assertTrue();
                console.log("ContinuousTaskJsTest003 startBackgroundRunning failure");
                setTimeout(() => {
                    done();
                }, 500);
            });
        });
    })

    /*
     * @tc.name:ContinuousTaskJsTest004
     * @tc.desc:verify old startBackgroundrunning interface callback mode work properly
     * @tc.type: FUNC
     * @tc.require: SR000GGT81 AR000GH6ER AR000GH6EM AR000GH6G9 AR000GH6ET
     */
    it("ContinuousTaskJsTest004", 0, async function (done) {
        function conTaskCallback(err, data) {
            if (err) {
                console.info('ContinuousTaskJsTest004 startBackgroundRunning failure');
                expect(false).assertTrue();
            } else {
                console.info('ContinuousTaskJsTest004 startBackgroundRunning success');
                expect(true).assertTrue();
            }
            setTimeout(() => {
                done();
            }, 500);
        }
        let wantAgentInfo = {
            wants: [
                {
                    bundleName: "com.continuoustask.test",
                    abilityName: "com.continuoustask.test.MainAbility4"
                }
            ],
            operationType: 2,
            requestCode: 4,
            wantAgentFlags: [3]
        };
        wantAgent.getWantAgent(wantAgentInfo).then((data) => {
            let basicContent4 = {
                title: "title4",
                text: "text4"
            };

            let notificationContent = {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: basicContent4
            };

            let request = {
                content: notificationContent,
                wantAgent: data
            }

            let id = 1;

            particleAbility.startBackgroundRunning(id, request, conTaskCallback);
        });
    })

    /*
     * @tc.name:ContinuousTaskJsTest005
     * @tc.desc:verify new api stopBackgroundrunning interface promise mode work properly
     * @tc.type: FUNC
     * @tc.require: SR000GGT7U AR000GH6ES AR000GH6EM AR000GH6EN AR000GH6EO
     */
    it("ContinuousTaskJsTest005", 0, async function (done) {
        let wantAgentInfo = {
            wants: [
                {
                    bundleName: "com.continuoustask.test",
                    abilityName: "com.continuoustask.test.MainAbility5"
                }
            ],
            operationType: 2,
            requestCode: 5,
            wantAgentFlags: [3]
        };
        wantAgent.getWantAgent(wantAgentInfo).then((data) => {
            backgroundTaskManager.startBackgroundRunning(featureAbility.getContext(),
                backgroundTaskManager.BackgroundMode.DATA_TRANSFER, data).then((data) => {
                    backgroundTaskManager.stopBackgroundRunning(featureAbility.getContext()).then((data) => {
                        console.log("ContinuousTaskJsTest005 cancelBackgroundRunning success");
                        expect(true).assertTrue();
                        setTimeout(() => {
                            done();
                        }, 500);
                    }).catch((err) => {
                        expect(false).assertTrue();
                        console.log("ContinuousTaskJsTest005 cancelBackgroundRunning failure");
                        setTimeout(() => {
                            done();
                        }, 500);
                    });
                })
        });
    })

    /*
     * @tc.name:ContinuousTaskJsTest006
     * @tc.desc:verify new api stopBackgroundrunning interface callback mode work properly
     * @tc.type: FUNC
     * @tc.require: SR000GGT7T AR000GH6ES AR000GH6EP AR000GJ9PR AR000GH6G8
     */
    it("ContinuousTaskJsTest006", 0, async function (done) {
        function conTaskCallback(err, data) {
            if (err) {
                console.info('ContinuousTaskJsTest006 startBackgroundRunning failure');
                expect(false).assertTrue();
            } else {
                console.info('ContinuousTaskJsTest006 startBackgroundRunning success');
                expect(true).assertTrue();
            }
            setTimeout(() => {
                done();
            }, 500);
        }
        let wantAgentInfo = {
            wants: [
                {
                    bundleName: "com.continuoustask.test",
                    abilityName: "com.continuoustask.test.MainAbility6"
                }
            ],
            operationType: 2,
            requestCode: 6,
            wantAgentFlags: [3]
        };
        wantAgent.getWantAgent(wantAgentInfo).then((data) => {
            backgroundTaskManager.startBackgroundRunning(featureAbility.getContext(),
                backgroundTaskManager.BackgroundMode.DATA_TRANSFER, data).then((data) => {
                    backgroundTaskManager.stopBackgroundRunning(featureAbility.getContext(), conTaskCallback);
                })
        });
    })

    /*
     * @tc.name:ContinuousTaskJsTest007
     * @tc.desc:verify old api cancelBackgroundrunning interface promise mode work properly
     * @tc.type: FUNC
     * @tc.require: SR000GGT7V AR000GH6ES AR000GH6EM AR000GH6G9 AR000GH56K
     */
    it("ContinuousTaskJsTest007", 0, async function (done) {
        let wantAgentInfo = {
            wants: [
                {
                    bundleName: "com.continuoustask.test",
                    abilityName: "com.continuoustask.test.MainAbility7"
                }
            ],
            operationType: 2,
            requestCode: 7,
            wantAgentFlags: [3]
        };
        wantAgent.getWantAgent(wantAgentInfo).then((data) => {
            let basicContent7 = {
                title: "title7",
                text: "text7"
            };

            let notificationContent = {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: basicContent7
            };

            let request = {
                content: notificationContent,
                wantAgent: data
            }

            let id = 1;
            particleAbility.startBackgroundRunning(id, request).then((data) => {
                console.log("ContinuousTaskJsTest007 startBackgroundRunning success");
                particleAbility.cancelBackgroundRunning().then(() => {
                    console.log("ContinuousTaskJsTest007 cancelBackgroundRunning success");
                    expect(true).assertTrue();
                    done();
                }).catch((err) => {
                    expect(false).assertTrue();
                    console.log("ContinuousTaskJsTest007 cancelBackgroundRunning failure");
                    done();
                });
            })
        });
    })

    /*
     * @tc.name:ContinuousTaskJsTest008
     * @tc.desc:verify old cancelBackgroundrunning interface callback mode work properly
     * @tc.type: FUNC
     * @tc.require: SR000GGT81 AR000GH6ES AR000GH6EM AR000GH6G9 AR000GH6ET
     */
    it("ContinuousTaskJsTest008", 0, async function (done) {
        function conTaskCallback(err, data) {
            if (err) {
                console.info('ContinuousTaskJsTest008 startBackgroundRunning failure');
                expect(false).assertTrue();
            } else {
                console.info('ContinuousTaskJsTest008 startBackgroundRunning success');
                expect(true).assertTrue();
            }
            setTimeout(() => {
                done();
            }, 500);
        }
        let wantAgentInfo = {
            wants: [
                {
                    bundleName: "com.continuoustask.test",
                    abilityName: "com.continuoustask.test.MainAbility8"
                }
            ],
            operationType: 2,
            requestCode: 8,
            wantAgentFlags: [3]
        };
        wantAgent.getWantAgent(wantAgentInfo).then((data) => {
            let basicContent8 = {
                title: "title8",
                text: "text8"
            };

            let notificationContent = {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: basicContent8
            };
            let request = {
                content: notificationContent,
                wantAgent: data
            }

            let id = 1;
            particleAbility.startBackgroundRunning(id, request).then((data) => {
                console.log("ContinuousTaskJsTest008 startBackgroundRunning success");
                particleAbility.cancelBackgroundRunning(conTaskCallback);
            })
        });
    })

    /*
     * @tc.name:ContinuousTaskJsTest009
     * @tc.desc:verify startBackgroundRunning promise workTime
     * @tc.type: FUNC
     */
    it("ContinuousTaskJsTest009", 0, async function (done) {
        let wantAgentInfo = {
            wants: [
                {
                    bundleName: "com.continuoustask.test",
                    abilityName: "com.continuoustask.test.MainAbility9"
                }
            ],
            operationType: 2,
            requestCode: 9,
            wantAgentFlags: [3]
        };
        let startTime = (new Date()).valueOf()
        wantAgent.getWantAgent(wantAgentInfo).then((data) => {
            backgroundTaskManager.startBackgroundRunning(featureAbility.getContext(),
                backgroundTaskManager.BackgroundMode.DATA_TRANSFER, data).then(() => {
                    let endTime = (new Date()).valueOf()
                    let workTime = endTime - startTime
                    if (workTime < 50) {
                        expect(true).assertTrue()
                    } else {
                        expect(false).assertTrue()
                    }
                    expect(true).assertTrue();
                    setTimeout(() => {
                        done();
                    }, 500);
                }).catch((err) => {
                    expect(false).assertTrue();
                    setTimeout(() => {
                        done();
                    }, 500);
                });
        });
    })

    /*
     * @tc.name:ContinuousTaskJsTest010
     * @tc.desc:verify startBackgroundRunning callback workTime
     * @tc.type: FUNC
     */
    it("ContinuousTaskJsTest010", 0, async function (done) {
        function conTaskCallback(err, data) {
            if (err) {
                console.info('ContinuousTaskJsTest010 startBackgroundRunning failed');
                expect(false).assertTrue();
            } else {
                console.info('ContinuousTaskJsTest010 startBackgroundRunning succeed');
                expect(true).assertTrue();
            }
            setTimeout(() => {
                done();
            }, 500);
        }
        let wantAgentInfo = {
            wants: [
                {
                    bundleName: "com.continuoustask.test",
                    abilityName: "com.continuoustask.test.MainAbility10"
                }
            ],
            operationType: 2,
            requestCode: 10,
            wantAgentFlags: [3]
        };
        wantAgent.getWantAgent(wantAgentInfo).then((data) => {
            let startTime = (new Date()).valueOf()
            backgroundTaskManager.startBackgroundRunning(featureAbility.getContext(),
                backgroundTaskManager.BackgroundMode.DATA_TRANSFER, data, conTaskCallback);
            let endTime = (new Date()).valueOf()
            let workTime = endTime - startTime
            if (workTime < 50) {
                expect(true).assertTrue();
            } else {
                expect(false).assertTrue()
            }
        });
    })

    /*
     * @tc.name:ContinuousTaskJsTest011
     * @tc.desc:verify stopBackgroundRunning promise workTime
     * @tc.type: FUNC
     */
    it("ContinuousTaskJsTest011", 0, async function (done) {
        let wantAgentInfo = {
            wants: [
                {
                    bundleName: "com.continuoustask.test",
                    abilityName: "com.continuoustask.test.MainAbility11"
                }
            ],
            operationType: 2,
            requestCode: 11,
            wantAgentFlags: [3]
        };
        wantAgent.getWantAgent(wantAgentInfo).then((data) => {
            backgroundTaskManager.startBackgroundRunning(featureAbility.getContext(),
                backgroundTaskManager.BackgroundMode.DATA_TRANSFER, data).then((data) => {
                    let startTime = (new Date()).valueOf()
                    backgroundTaskManager.stopBackgroundRunning(featureAbility.getContext()).then((data) => {
                        let endTime = (new Date()).valueOf()
                        let workTime = endTime - startTime
                        if (workTime < 50) {
                            expect(true).assertTrue()
                        } else {
                            expect(false).assertTrue()
                        }
                        setTimeout(() => {
                            done();
                        }, 500);
                    }).catch((err) => {
                        setTimeout(() => {
                            done();
                        }, 500);
                    });
                })
        });
    })

    /*
     * @tc.name:ContinuousTaskJsTest012
     * @tc.desc:verify stopBackgroundRunning callback  workTime
     * @tc.type: FUNC
     */
    it("ContinuousTaskJsTest012", 0, async function (done) {
        function conTaskCallback(err, data) {
            if (err) {
                console.info('ContinuousTaskJsTest012 startBackgroundRunning failure');
                expect(false).assertTrue();
            } else {
                console.info('ContinuousTaskJsTest012 startBackgroundRunning success');
                expect(true).assertTrue();
            }
            setTimeout(() => {
                done();
            }, 500);
        }
        let wantAgentInfo = {
            wants: [
                {
                    bundleName: "com.continuoustask.test",
                    abilityName: "com.continuoustask.test.MainAbility12"
                }
            ],
            operationType: 2,
            requestCode: 12,
            wantAgentFlags: [3]
        };
        wantAgent.getWantAgent(wantAgentInfo).then((data) => {
            backgroundTaskManager.startBackgroundRunning(featureAbility.getContext(),
                backgroundTaskManager.BackgroundMode.DATA_TRANSFER, data).then((data) => {
                    let startTime = (new Date()).valueOf()
                    backgroundTaskManager.stopBackgroundRunning(featureAbility.getContext(), conTaskCallback);
                    let endTime = (new Date()).valueOf()
                    let workTime = endTime - startTime
                    if (workTime < 50) {
                        expect(true).assertTrue()
                    } else {
                        expect(false).assertTrue()
                    }
                })
        });
    })
})