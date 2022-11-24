/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "message_parcel.h"

namespace OHOS {
namespace {
bool g_bgTaskFwkAbnormalWriteRemoteObjectFlag = true;
bool g_bgTaskFwkAbnormalReadRemoteObjectFlag = true;
bool g_bgTaskFwkAbnormalWriteInterfaceTokenFlag = true;
bool g_bgTaskFwkAbnormalReadInterfaceTokenFlag = true;
bool g_bgTaskFwkAbnormalWriteString16Flag = true;
bool g_bgTaskFwkAbnormalReadInt32WithParamFlag = true;
bool g_bgTaskFwkAbnormalWriteInt32WithParamFlag = true;
bool g_bgTaskFwkAbnormalWriteParcelableFlag = true;
bool g_bgTaskFwkAbnormalWriteStringFlag = true;
bool g_bgTaskFwkAbnormalWriteUint32Flag = true;
bool g_bgTaskFwkAbnormalReadInt32Flag = true;
}

void BgTaskFwkAbnormalSetWriteRemoteObjectFlag(bool flag)
{
    g_bgTaskFwkAbnormalWriteRemoteObjectFlag = flag;
}

void BgTaskFwkAbnormalSetWriteReadRemoteObjectFlag(bool flag)
{
    g_bgTaskFwkAbnormalReadRemoteObjectFlag = flag;
}

void BgTaskFwkAbnormalSetWriteInterfaceTokenFlag(bool flag)
{
    g_bgTaskFwkAbnormalWriteInterfaceTokenFlag = flag;
}

void BgTaskFwkAbnormalSetReadInterfaceTokenFlag(bool flag)
{
    g_bgTaskFwkAbnormalReadInterfaceTokenFlag = flag;
}

void BgTaskFwkAbnormalSetWriteString16Flag(bool flag)
{
    g_bgTaskFwkAbnormalWriteString16Flag = flag;
}

void BgTaskFwkAbnormalSetWriteReadInt32WithParamFlag(bool flag)
{
    g_bgTaskFwkAbnormalReadInt32WithParamFlag = flag;
}

void BgTaskFwkAbnormalSetWriteInt32WithParamFlag(bool flag)
{
    g_bgTaskFwkAbnormalWriteInt32WithParamFlag = flag;
}

void BgTaskFwkAbnormalSetWriteParcelableFlag(bool flag)
{
    g_bgTaskFwkAbnormalWriteParcelableFlag = flag;
}

void BgTaskFwkAbnormalSetWriteStringFlag(bool flag)
{
    g_bgTaskFwkAbnormalWriteStringFlag = flag;
}

void BgTaskFwkAbnormalSetWriteUint32Flag(bool flag)
{
    g_bgTaskFwkAbnormalWriteUint32Flag = flag;
}

void BgTaskFwkAbnormalSetReadInt32Flag(bool flag)
{
    g_bgTaskFwkAbnormalReadInt32Flag = flag;
}

bool MessageParcel::WriteRemoteObject(const sptr<IRemoteObject> &object)
{
    return g_bgTaskFwkAbnormalWriteRemoteObjectFlag;
}

bool MessageParcel::WriteInterfaceToken(std::u16string name)
{
    return g_bgTaskFwkAbnormalWriteInterfaceTokenFlag;
}

std::u16string MessageParcel::ReadInterfaceToken()
{
    if (g_bgTaskFwkAbnormalReadInterfaceTokenFlag) {
        return std::u16string(u"string");
    }
    return std::u16string();
}

bool Parcel::WriteString16(const std::u16string &value)
{
    return g_bgTaskFwkAbnormalWriteString16Flag;
}

bool Parcel::ReadInt32(int32_t &value)
{
    return g_bgTaskFwkAbnormalReadInt32WithParamFlag;
}

bool Parcel::WriteInt32(int32_t value)
{
    return g_bgTaskFwkAbnormalWriteInt32WithParamFlag;
}

bool Parcel::WriteParcelable(const Parcelable *object)
{
    return g_bgTaskFwkAbnormalWriteParcelableFlag;
}

bool Parcel::WriteString(const std::string &value)
{
    return g_bgTaskFwkAbnormalWriteStringFlag;
}

bool Parcel::WriteUint32(uint32_t value)
{
    return g_bgTaskFwkAbnormalWriteUint32Flag;
}

int32_t Parcel::ReadInt32()
{
    return 1;
}
} // namespace OHOS
