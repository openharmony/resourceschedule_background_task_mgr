/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <gtest/gtest.h>
#include "bgtask_data_mgr.h"
#include "audio_info.h"
#include "audio_stream_manager.h"

using namespace testing::ext;

namespace OHOS {

void BgMockGetCurrentRendererChangeInfos(int32_t mockRet);

namespace BackgroundTaskMgr {

class BgtaskDataMgrTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        BgtaskDataMgr::GetInstance()->ClearAll();
    }

    static void TearDownTestCase()
    {
        BgtaskDataMgr::GetInstance()->ClearAll();
    }

    void SetUp() override
    {
        BgtaskDataMgr::GetInstance()->ClearAll();
    }

    void TearDown() override
    {
        BgtaskDataMgr::GetInstance()->ClearAll();
    }
};

/**
 * @tc.name: BgtaskDataMgrTest_001
 * @tc.desc: test CheckAppIsPlaying method with empty list.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskDataMgrTest, BgtaskDataMgrTest_001, TestSize.Level2)
{
    bool result = BgtaskDataMgr::GetInstance()->CheckAppIsPlaying(1001);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: BgtaskDataMgrTest_002
 * @tc.desc: test CheckAppIsPlaying method with playing app.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskDataMgrTest, BgtaskDataMgrTest_002, TestSize.Level2)
{
    auto audioInfo = std::make_shared<AudioInfo>(1001, 1);
    BgtaskDataMgr::GetInstance()->AddAudioPlayerInfo(audioInfo);
    bool result = BgtaskDataMgr::GetInstance()->CheckAppIsPlaying(1001);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: BgtaskDataMgrTest_003
 * @tc.desc: test CheckAppIsPlaying method with non-playing app.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskDataMgrTest, BgtaskDataMgrTest_003, TestSize.Level2)
{
    auto audioInfo = std::make_shared<AudioInfo>(1001, 1);
    BgtaskDataMgr::GetInstance()->AddAudioPlayerInfo(audioInfo);
    bool result = BgtaskDataMgr::GetInstance()->CheckAppIsPlaying(1002);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: BgtaskDataMgrTest_004
 * @tc.desc: test AddAudioPlayerInfo method with valid info.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskDataMgrTest, BgtaskDataMgrTest_004, TestSize.Level2)
{
    auto audioInfo = std::make_shared<AudioInfo>(1001, 1);
    bool result = BgtaskDataMgr::GetInstance()->AddAudioPlayerInfo(audioInfo);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: BgtaskDataMgrTest_005
 * @tc.desc: test AddAudioPlayerInfo method with nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskDataMgrTest, BgtaskDataMgrTest_005, TestSize.Level2)
{
    bool result = BgtaskDataMgr::GetInstance()->AddAudioPlayerInfo(nullptr);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: BgtaskDataMgrTest_006
 * @tc.desc: test AddAudioPlayerInfo method with duplicate info.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskDataMgrTest, BgtaskDataMgrTest_006, TestSize.Level2)
{
    auto audioInfo1 = std::make_shared<AudioInfo>(1001, 1);
    auto audioInfo2 = std::make_shared<AudioInfo>(1001, 1);
    BgtaskDataMgr::GetInstance()->AddAudioPlayerInfo(audioInfo1);
    bool result = BgtaskDataMgr::GetInstance()->AddAudioPlayerInfo(audioInfo2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: BgtaskDataMgrTest_007
 * @tc.desc: test AddAudioPlayerInfo method with different session id.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskDataMgrTest, BgtaskDataMgrTest_007, TestSize.Level2)
{
    auto audioInfo1 = std::make_shared<AudioInfo>(1001, 1);
    auto audioInfo2 = std::make_shared<AudioInfo>(1001, 2);
    BgtaskDataMgr::GetInstance()->AddAudioPlayerInfo(audioInfo1);
    bool result = BgtaskDataMgr::GetInstance()->AddAudioPlayerInfo(audioInfo2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: BgtaskDataMgrTest_008
 * @tc.desc: test RemoveAudioPlayerInfo method with valid info.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskDataMgrTest, BgtaskDataMgrTest_008, TestSize.Level2)
{
    int32_t uid = 1001;
    int32_t sessionId = 1;
    auto audioInfo = std::make_shared<AudioInfo>(uid, sessionId);
    BgtaskDataMgr::GetInstance()->AddAudioPlayerInfo(audioInfo);
    BgtaskDataMgr::GetInstance()->RemoveAudioPlayerInfo(uid, sessionId);
    bool result = BgtaskDataMgr::GetInstance()->CheckAppIsPlaying(uid);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: BgtaskDataMgrTest_009
 * @tc.desc: test ClearAll method clears all data.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskDataMgrTest, BgtaskDataMgrTest_009, TestSize.Level2)
{
    auto audioInfo1 = std::make_shared<AudioInfo>(1001, 1);
    auto audioInfo2 = std::make_shared<AudioInfo>(1002, 2);
    BgtaskDataMgr::GetInstance()->AddAudioPlayerInfo(audioInfo1);
    BgtaskDataMgr::GetInstance()->AddAudioPlayerInfo(audioInfo2);
    BgtaskDataMgr::GetInstance()->ClearAll();
    EXPECT_FALSE(BgtaskDataMgr::GetInstance()->CheckAppIsPlaying(1001));
    EXPECT_FALSE(BgtaskDataMgr::GetInstance()->CheckAppIsPlaying(1002));
}

/**
 * @tc.name: BgtaskDataMgrTest_010
 * @tc.desc: test AfterAddSaListener method with empty list
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskDataMgrTest, BgtaskDataMgrTest_010, TestSize.Level2)
{
    BgMockGetCurrentRendererChangeInfos(-1);
    BgtaskDataMgr::GetInstance()->AfterAddSaListener();
    EXPECT_FALSE(BgtaskDataMgr::GetInstance()->CheckAppIsPlaying(1));

    BgMockGetCurrentRendererChangeInfos(0);
    BgtaskDataMgr::GetInstance()->AfterAddSaListener();
    EXPECT_FALSE(BgtaskDataMgr::GetInstance()->CheckAppIsPlaying(1));

    BgMockGetCurrentRendererChangeInfos(1);
    BgtaskDataMgr::GetInstance()->AfterAddSaListener();
    EXPECT_TRUE(BgtaskDataMgr::GetInstance()->CheckAppIsPlaying(1));
}

/**
 * @tc.name: BgtaskDataMgrTest_011
 * @tc.desc: test HandleMultiDeviceCastStart and HandleMultiDeviceCastStop with multiple calls.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskDataMgrTest, BgtaskDataMgrTest_011, TestSize.Level2)
{
    int32_t uid = 1001;
    int32_t said = 3009;
    BgtaskDataMgr::GetInstance()->HandleMultiDeviceCastStart(uid, said);
    BgtaskDataMgr::GetInstance()->HandleMultiDeviceCastStart(uid, said);
    BgtaskDataMgr::GetInstance()->HandleMultiDeviceCastStart(uid, said);
    EXPECT_TRUE(BgtaskDataMgr::GetInstance()->IsMultiDeviceCast(uid));

    BgtaskDataMgr::GetInstance()->HandleMultiDeviceCastStop(uid, said);
    EXPECT_TRUE(BgtaskDataMgr::GetInstance()->IsMultiDeviceCast(uid));

    BgtaskDataMgr::GetInstance()->HandleMultiDeviceCastStop(uid, said);
    EXPECT_TRUE(BgtaskDataMgr::GetInstance()->IsMultiDeviceCast(uid));

    BgtaskDataMgr::GetInstance()->HandleMultiDeviceCastStop(uid, said);
    EXPECT_FALSE(BgtaskDataMgr::GetInstance()->IsMultiDeviceCast(uid));
}

/**
 * @tc.name: BgtaskDataMgrTest_012
 * @tc.desc: test RemoveMultiDeviceInfoByUid method.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskDataMgrTest, BgtaskDataMgrTest_012, TestSize.Level2)
{
    int32_t uid = 1001;
    int32_t said = 3009;
    BgtaskDataMgr::GetInstance()->HandleMultiDeviceCastStart(uid, said);
    EXPECT_TRUE(BgtaskDataMgr::GetInstance()->IsMultiDeviceCast(uid));
    BgtaskDataMgr::GetInstance()->OnAppStopped(uid);
    EXPECT_FALSE(BgtaskDataMgr::GetInstance()->IsMultiDeviceCast(uid));
}

/**
 * @tc.name: BgtaskDataMgrTest_013
 * @tc.desc: test RemoveMultiDeviceInfoBySaid method.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskDataMgrTest, BgtaskDataMgrTest_013, TestSize.Level2)
{
    int32_t uid = 1001;
    int32_t said = 3009;
    BgtaskDataMgr::GetInstance()->HandleMultiDeviceCastStart(uid, said);
    EXPECT_TRUE(BgtaskDataMgr::GetInstance()->IsMultiDeviceCast(uid));
    BgtaskDataMgr::GetInstance()->OnRemoveSystemAbility(said);
    EXPECT_FALSE(BgtaskDataMgr::GetInstance()->IsMultiDeviceCast(uid));
}

/**
 * @tc.name: BgtaskDataMgrTest_014
 * @tc.desc: test ClearAll method clears multiDeviceInfo.
 * @tc.type: FUNC
 */
HWTEST_F(BgtaskDataMgrTest, BgtaskDataMgrTest_014, TestSize.Level2)
{
    int32_t uid1 = 1001;
    int32_t uid2 = 1002;
    int32_t said = 3009;
    BgtaskDataMgr::GetInstance()->HandleMultiDeviceCastStart(uid1, said);
    BgtaskDataMgr::GetInstance()->HandleMultiDeviceCastStart(uid2, said);
    EXPECT_TRUE(BgtaskDataMgr::GetInstance()->IsMultiDeviceCast(uid1));
    EXPECT_TRUE(BgtaskDataMgr::GetInstance()->IsMultiDeviceCast(uid2));
    BgtaskDataMgr::GetInstance()->ClearAll();
    EXPECT_FALSE(BgtaskDataMgr::GetInstance()->IsMultiDeviceCast(uid1));
    EXPECT_FALSE(BgtaskDataMgr::GetInstance()->IsMultiDeviceCast(uid2));
}
} // namespace BackgroundTaskMgr
} // namespace OHOS