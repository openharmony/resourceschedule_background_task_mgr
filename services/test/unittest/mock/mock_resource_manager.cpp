/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "resource_manager_impl.h"

namespace OHOS {
namespace Global {
namespace Resource {
ResourceManager *CreateResourceManager()
{
    ResourceManagerImpl *impl = new (std::nothrow) ResourceManagerImpl;
    return impl;
}

ResourceManager::~ResourceManager()
{}

ResourceManagerImpl::ResourceManagerImpl()
{}

ResourceManagerImpl::~ResourceManagerImpl()
{}

bool ResourceManagerImpl::AddResource(const char *path)
{
    return true;
}

RState ResourceManagerImpl::UpdateResConfig(ResConfig &resConfig)
{
    return SUCCESS;
}

void ResourceManagerImpl::GetResConfig(ResConfig &resConfig)
{}

RState ResourceManagerImpl::GetStringById(uint32_t id, std::string &outValue)
{
    outValue = "ENTRY";
    return SUCCESS;
}

RState ResourceManagerImpl::GetStringByName(const char *name, std::string &outValue)
{
    outValue = "bgmode_test";
    return SUCCESS;
}

RState ResourceManagerImpl::GetStringFormatById(std::string &outValue, uint32_t id, ...)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetStringFormatByName(std::string &outValue, const char *name, ...)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetStringArrayById(uint32_t id, std::vector<std::string> &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetStringArrayByName(const char *name, std::vector<std::string> &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetPatternById(uint32_t id, std::map<std::string, std::string> &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetPatternByName(const char *name, std::map<std::string, std::string> &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetPluralStringById(uint32_t id, int quantity, std::string &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetPluralStringByName(const char *name, int quantity, std::string &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetPluralStringByIdFormat(std::string &outValue, uint32_t id, int quantity, ...)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetPluralStringByNameFormat(std::string &outValue, const char *name, int quantity, ...)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetThemeById(uint32_t id, std::map<std::string, std::string> &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetThemeByName(const char *name, std::map<std::string, std::string> &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetBooleanById(uint32_t id, bool &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetBooleanByName(const char *name, bool &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetIntegerById(uint32_t id, int &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetIntegerByName(const char *name, int &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetFloatById(uint32_t id, float &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetFloatById(uint32_t id, float &outValue, std::string &unit)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetFloatByName(const char *name, float &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetFloatByName(const char *name, float &outValue, std::string &unit)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetIntArrayById(uint32_t id, std::vector<int> &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetIntArrayByName(const char *name, std::vector<int> &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetColorById(uint32_t id, uint32_t &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetColorByName(const char *name, uint32_t &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetProfileById(uint32_t id, std::string &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetProfileByName(const char *name, std::string &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetMediaById(uint32_t id, std::string &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetMediaByName(const char *name, std::string &outValue)
{
    return SUCCESS;
}

RState ResourceManagerImpl::GetRawFilePathByName(const std::string &name, std::string &outValue)
{
    return SUCCESS;
}
}
}
}