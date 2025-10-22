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

#ifndef CALLBACK_FFI_H
#define CALLBACK_FFI_H

#include <mutex>
#include <string>

#include "delay_suspend_info.h"
#include "expired_callback.h"

namespace OHOS {
namespace BackgroundTaskMgr {
class CallbackFFI : public ExpiredCallback {
public:
    CallbackFFI();

    ~CallbackFFI() override;

    void OnExpired() override;

    void OnExpiredAuth(int32_t authResult) override;

    void SetCallbackInfo(void (*callback)());

private:
    std::function<void()> ffiCallback_;
};

extern std::map<int32_t, std::shared_ptr<CallbackFFI>> callbackInstances_;
extern std::mutex callbackLock_;

}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // CALLBACK_FFI_H