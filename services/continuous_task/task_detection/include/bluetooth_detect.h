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

#ifndef FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_BLUETOOTH_DETECT_H
#define FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_BLUETOOTH_DETECT_H

#include <list>
#include <set>

#include "nlohmann/json.hpp"

namespace OHOS {
namespace BackgroundTaskMgr {
struct SppConnectStateReocrd {
    int32_t socketId_ {-1};
    int32_t pid_ {-1};
    int32_t uid_ {-1};
    std::string address_ {""};
    SppConnectStateReocrd() = default;
    SppConnectStateReocrd(int32_t socketId, int32_t pid, int32_t uid, const std::string &address)
        : socketId_(socketId), pid_(pid), uid_(uid), address_(address) {};
};

struct GattConnectStateRecord {
    std::string address_ {""};
    int32_t role_ {-1}; // GATT_ROLE_MASTER = 0x00; GATT_ROLE_SLAVE = 0x01; GATT_ROLE_INVALID = 0xFF;
    GattConnectStateRecord() = default;
    GattConnectStateRecord(const std::string &address, int32_t role)
        : address_(address), role_(role) {};
};

struct GattAppRegisterInfo {
    std::string side_ {""}; // client or server
    std::string address_ {""};
    int32_t appId_ {-1};
    int32_t pid_ {-1};
    int32_t uid_ {-1};
    GattAppRegisterInfo() = default;
    GattAppRegisterInfo(const std::string &side, const std::string &address, int32_t appId, int32_t pid, int32_t uid)
        : side_(side), address_(address), appId_(appId), pid_(pid), uid_(uid) {};
};

class BluetoothDetect {
public:
    bool CheckBluetoothUsingScene(int32_t uid);
    void HandleBluetoothSysEvent(const nlohmann::json &root);
    void HandleBtSwitchState(const nlohmann::json &root);
    void HandleSppConnect(const nlohmann::json &root);
    void HandleGattConnect(const nlohmann::json &root);
    void HandleGattAppRegister(const nlohmann::json &root);
    void HandleGattAppDeregister(const nlohmann::json &root);
    void HandleGattDisconnect(const std::shared_ptr<GattConnectStateRecord> &record);
    void HandleMasterSideDisconnect(const std::string &addr);
    void HandleSlaveSideDisconnect();
    void HandleBluetoothPairState(const std::string &addr, int32_t state);
    void ParseBluetoothRecordToStr(nlohmann::json &value);
    bool ParseBluetoothRecordFromJson(const nlohmann::json &value, std::set<int32_t> &uidSet);
    void ClearData();

private:
    bool isBrSwitchOn_ {false};
    bool isBleSwitchOn_ {false};
    std::set<std::string> devicePairRecords_ {};
    std::list<std::shared_ptr<SppConnectStateReocrd>> sppConnectRecords_ {};
    std::list<std::shared_ptr<GattConnectStateRecord>> gattConnectRecords_ {};
    std::list<std::shared_ptr<GattAppRegisterInfo>> gattAppRegisterInfos_ {};

    friend class TaskDetectionManager;
};
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_BACKGROUND_TASK_MGR_CONTINUOUS_TASK_BLUETOOTH_DETECT_H