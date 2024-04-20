/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef BGTASK_FRAMEWORK_MOCK_MESSAGEPARCEL_INCLUDE_H
#define BGTASK_FRAMEWORK_MOCK_MESSAGEPARCEL_INCLUDE_H

namespace OHOS {
class MessageParcelHelper {
public:
    static void BgTaskFwkAbnormalSetWriteRemoteObjectFlag(bool flag);
    static void BgTaskFwkAbnormalSetWriteReadRemoteObjectFlag(bool flag);
    static void BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(bool flag);
    static void BgTaskFwkAbnormalSetReadInterfaceTokenFlag(bool flag);
    static void BgTaskFwkAbnormalSetWriteString16Flag(bool flag);
    static void BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(bool flag);
    static void BgTaskFwkAbnormalSetWriteInt32WithParamFlag(bool flag);
    static void BgTaskFwkAbnormalSetWriteParcelableFlag(bool flag);
    static void BgTaskFwkAbnormalSetWriteStringFlag(bool flag);
    static void BgTaskFwkAbnormalSetWriteUint32Flag(bool flag);
    static void BgTaskFwkAbnormalSetReadInt32Flag(bool flag);
};
}  // namespace OHOS
#endif  // BGTASK_FRAMEWORK_MOCK_INCLUDE_H
