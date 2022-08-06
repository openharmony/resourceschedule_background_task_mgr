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

#include "bluetooth_detect.h"

#include "bg_continuous_task_mgr.h"
#include "common_utils.h"
#include "continuous_task_log.h"
#include "task_detection_manager.h"

namespace OHOS {
namespace BackgroundTaskMgr {
void BluetoothDetect::HandleBluetoothSysEvent(const nlohmann::json &root)
{
    if (!CommonUtils::CheckJsonValue(root, {"name_"})) {
        BGTASK_LOGE("hisysevent data domain info lost");
        return;
    }

    std::string eventName = root.at("name_").get<std::string>();
    if (eventName == "BLUETOOTH_BR_SWITCH_STATE" || eventName == "BLUETOOTH_BLE_STATE") {
        HandleBtSwitchState(root);
    } else if (eventName == "BLUETOOTH_SPP_CONNECT_STATE") {
        HandleSppConnect(root);
    } else if (eventName == "BLUETOOTH_GATT_CONNECT_STATE") {
        HandleGattConnect(root);
    } else if (eventName == "BLUETOOTH_GATT_APP_REGISTER") {
        if (!CommonUtils::CheckJsonValue(root, {"ACTION"})) {
            BGTASK_LOGE("Bluetooth connect state event lost important info");
            return;
        }
        std::string action = root.at("ACTION").get<std::string>();
        if (action == "register") {
            HandleGattAppRegister(root);
        } else if (action == "deregister") {
            HandleGattAppDeregister(root);
        }
    } else {
        BGTASK_LOGW("Ignore unrelated event: %{public}s", eventName.c_str());
    }
}

void BluetoothDetect::HandleBtSwitchState(const nlohmann::json &root)
{
    if (!CommonUtils::CheckJsonValue(root, {"STATE"})) {
        BGTASK_LOGE("Bluetooth sys event lost important info");
        return;
    }
    int32_t switchState = root.at("STATE").get<int32_t>();
    std::string eventName = root.at("name_").get<std::string>();
    if (eventName == "BLUETOOTH_BR_SWITCH_STATE") {
        if (switchState == CommonUtils::BT_SWITCH_TURN_ON) {
            isBrSwitchOn_ = true;
        } else if (switchState == CommonUtils::BT_SWITCH_TURN_OFF) {
            isBrSwitchOn_ = false;
        }
    } else if (eventName == "BLUETOOTH_BLE_STATE") {
        if (switchState == CommonUtils::BT_SWITCH_TURN_ON) {
            isBleSwitchOn_ = true;
        } else if (switchState == CommonUtils::BT_SWITCH_TURN_OFF) {
            isBleSwitchOn_ = false;
        }
    }
    if (!isBrSwitchOn_ && !isBleSwitchOn_) {
        TaskDetectionManager::GetInstance()->ReportNeedRecheckTask(CommonUtils::UNSET_UID,
            CommonUtils::BLUETOOTH_INTERACTION_BGMODE_ID);
    }
}

void BluetoothDetect::HandleSppConnect(const nlohmann::json& root)
{
    if (!CommonUtils::CheckJsonValue(root, { "ACTION", "ID", "ADDRESS", "PID", "UID" })) {
        BGTASK_LOGE("Bluetooth HandleSppConnect event lost important info");
        return;
    }
    std::string action = root.at("ACTION").get<std::string>();
    std::string address = root.at("ADDRESS").get<std::string>();
    int32_t socketId = root.at("ID").get<int32_t>();
    int32_t pid = root.at("PID").get<int32_t>();
    int32_t uid = root.at("UID").get<int32_t>();
    if (uid <= CommonUtils::MAX_UID) {
        BGTASK_LOGI("Ignore system service spp connet event");
        return;
    }
    auto findRecord = [socketId, pid, uid](const auto &target) {
        return target->socketId_ == socketId && target->pid_ == pid && target->uid_ == uid;
    };
    auto findRecordIter = find_if(sppConnectRecords_.begin(), sppConnectRecords_.end(), findRecord);
    if (action == "connect") {
        if (findRecordIter != sppConnectRecords_.end()) {
            BGTASK_LOGE("SPP receive same connect event");
            return;
        }
        sppConnectRecords_.emplace_back(std::make_shared<SppConnectStateReocrd>(socketId, pid, uid, address));
    } else if (action == "close") {
        if (findRecordIter == sppConnectRecords_.end()) {
            BGTASK_LOGE("SPP receive close event is not exist");
            return;
        }
        sppConnectRecords_.erase(findRecordIter);
        if (!CheckBluetoothUsingScene(uid)) {
            TaskDetectionManager::GetInstance()->ReportNeedRecheckTask(uid,
                CommonUtils::BLUETOOTH_INTERACTION_BGMODE_ID);
        }
    }
}

void BluetoothDetect::HandleGattConnect(const nlohmann::json &root)
{
    if (!CommonUtils::CheckJsonValue(root, { "ADDRESS", "STATE", "ROLE" })) {
        BGTASK_LOGE("Bluetooth HandleGattConnect state event lost important info");
        return;
    }

    std::string address = root.at("ADDRESS").get<std::string>();
    int32_t state = root.at("STATE").get<int32_t>();
    int32_t role = root.at("ROLE").get<int32_t>();

    auto findRecord = [address, role](const auto &target) {
        return target->address_ == address && target->role_ == role;
    };
    auto findRecordIter = find_if(gattConnectRecords_.begin(), gattConnectRecords_.end(), findRecord);

    if (state == CommonUtils::GATT_CONNECT || state == CommonUtils::GATT_RECONNECT) {
        if (findRecordIter == gattConnectRecords_.end()) {
            gattConnectRecords_.emplace_back(std::make_shared<GattConnectStateRecord>(address, role));
        } else {
            BGTASK_LOGE("Bluetooth connect record to add is already exist");
        }
    } else {
        if (findRecordIter != gattConnectRecords_.end()) {
            auto disconnectRecord = *findRecordIter;
            gattConnectRecords_.erase(findRecordIter);
            HandleGattDisconnect(disconnectRecord);
        } else {
            BGTASK_LOGE("Bluetooth connect record to remove is not exist");
        }
    }
}

void BluetoothDetect::HandleGattAppRegister(const nlohmann::json &root)
{
    if (!CommonUtils::CheckJsonValue(root, { "SIDE", "ADDRESS", "PID", "UID", "APPID" })) {
        BGTASK_LOGE("Bluetooth GattAppRegister event lost important info");
        return;
    }

    std::string side = root.at("SIDE").get<std::string>();
    std::string address = root.at("ADDRESS").get<std::string>();
    int32_t pid = root.at("PID").get<int32_t>();
    int32_t uid = root.at("UID").get<int32_t>();
    int32_t appId = root.at("APPID").get<int32_t>();

    if (uid <= CommonUtils::MAX_UID) {
        BGTASK_LOGI("Ignore gatt register event of system service");
        return;
    }

    auto findRecord = [side, pid, uid, appId](const auto &target) {
        return target->side_ == side && target->appId_ == appId && target->pid_ == pid && target->uid_ == uid;
    };
    auto findRecordIter = find_if(gattAppRegisterInfos_.begin(), gattAppRegisterInfos_.end(), findRecord);
    if (findRecordIter != gattAppRegisterInfos_.end()) {
        BGTASK_LOGE("Gatt app register record already exist");
    } else {
        gattAppRegisterInfos_.emplace_back(std::make_shared<GattAppRegisterInfo>(side, address, appId, pid, uid));
    }
}

void BluetoothDetect::HandleGattAppDeregister(const nlohmann::json &root)
{
    if (!CommonUtils::CheckJsonValue(root, { "SIDE", "APPID" })) {
        BGTASK_LOGE("Bluetooth GattAppDeregister state event lost important info");
        return;
    }

    std::string side = root.at("SIDE").get<std::string>();
    int32_t appId = root.at("APPID").get<int32_t>();

    auto findRecord = [side, appId](const auto &target) {
        return target->side_ == side && target->appId_ == appId;
    };
    auto findRecordIter = find_if(gattAppRegisterInfos_.begin(), gattAppRegisterInfos_.end(), findRecord);
    if (findRecordIter == gattAppRegisterInfos_.end()) {
        BGTASK_LOGE("Gatt app register record to remove is not exist");
    } else {
        auto record = *findRecordIter;
        gattAppRegisterInfos_.erase(findRecordIter);
        if (!CheckBluetoothUsingScene(record->uid_)) {
            TaskDetectionManager::GetInstance()->ReportNeedRecheckTask(record->uid_,
                CommonUtils::BLUETOOTH_INTERACTION_BGMODE_ID);
        }
    }
}

void BluetoothDetect::HandleGattDisconnect(const std::shared_ptr<GattConnectStateRecord> &record)
{
    BGTASK_LOGD("Disconnect record info address: %{public}s, role: %{public}d",
        record->address_.c_str(), record->role_);
    if (record->role_ == CommonUtils::GATT_ROLE_SLAVE) { // means server side connect removed
        HandleSlaveSideDisconnect();
    } else if (record->role_ == CommonUtils::GATT_ROLE_MASTER) { // maeans client side connect removed
        HandleMasterSideDisconnect(record->address_);
    }
}

void BluetoothDetect::HandleMasterSideDisconnect(const std::string &addr)
{
    // data struct to record after one client connect removed if need to stop related continuous tasks.
    std::map<std::pair<int32_t, int32_t>, bool> clientToRemove;
    // this loop is to get all gatt client app's uid and pid that address is the same as the disconnect info.
    for (auto var : gattAppRegisterInfos_) {
        auto pair = std::make_pair(var->uid_, var->pid_);
        if (var->side_ == "client" && var->address_ == addr && clientToRemove.count(pair) == 0) {
            clientToRemove[pair] = false;
        } else if (var->side_ == "client" && var->address_ != addr) {
            clientToRemove[pair] = true;
        }
    }

    // if target uid and pid does not contain any different bluetooth address,
    // report to stop related continous task.
    for (const auto& var : clientToRemove) {
        if (!var.second) {
            TaskDetectionManager::GetInstance()->ReportNeedRecheckTask(var.first.first,
                CommonUtils::BLUETOOTH_INTERACTION_BGMODE_ID);
        }
    }
}

void BluetoothDetect::HandleSlaveSideDisconnect()
{
    std::set<std::string> clientAddr;
    for (auto var : gattConnectRecords_) {
        if (var->role_ == CommonUtils::GATT_ROLE_SLAVE) {
            return;
        }
        clientAddr.emplace(var->address_);
    }
    for (auto var : gattAppRegisterInfos_) {
        if (var->side_ != "server") {
            continue;
        }

        int32_t uid = var->uid_;
        int32_t pid = var->pid_;

        // need to recheck there is no client connect with same uid and pid
        auto findRecord = [uid, pid](const auto &target) {
            return target->side_ == "client" && target->pid_ == pid && target->uid_ == uid;
        };
        auto findRecordIter = find_if(gattAppRegisterInfos_.begin(), gattAppRegisterInfos_.end(), findRecord);
        if (findRecordIter != gattAppRegisterInfos_.end()
            && clientAddr.find((*findRecordIter)->address_) != clientAddr.end()) {
            continue;
        }
        if (CommonUtils::CheckIsUidExist(uid, sppConnectRecords_)) {
            continue;
        }
        TaskDetectionManager::GetInstance()->ReportNeedRecheckTask(uid,
            CommonUtils::BLUETOOTH_INTERACTION_BGMODE_ID);
    }
}

void BluetoothDetect::HandleBluetoothPairState(const std::string &addr, int32_t state)
{
    if (state == CommonUtils::BT_PAIR_PAIRED) {
        devicePairRecords_.emplace(addr);
        return;
    } else if (state == CommonUtils::BT_PAIR_PAIRED) {
        return;
    } else {
        devicePairRecords_.erase(addr);
    }

    std::set<int32_t> uidToCheck;

    for (auto var : sppConnectRecords_) {
        if (var->address_ == addr) {
            uidToCheck.emplace(var->uid_);
        }
    }
    for (int32_t uid : uidToCheck) {
        if (!CheckBluetoothUsingScene(uid)) {
            TaskDetectionManager::GetInstance()->ReportNeedRecheckTask(uid,
                 CommonUtils::BLUETOOTH_INTERACTION_BGMODE_ID);
        }
    }
}

bool BluetoothDetect::CheckBluetoothUsingScene(int32_t uid)
{
    if (uid == CommonUtils::UNSET_UID) {
        return isBrSwitchOn_ || isBleSwitchOn_;
    }
    if (!isBrSwitchOn_ && !isBleSwitchOn_) {
        return false;
    }
    // check if is using SPP function
    for (auto var : sppConnectRecords_) {
        if (var->uid_ == uid && devicePairRecords_.find(var->address_) != devicePairRecords_.end()) {
            return true;
        }
    }
    // check if is using GATT function
    for (auto var : gattAppRegisterInfos_) {
        if (var->uid_ != uid) {
            continue;
        }

        std::string side = var->side_;
        std::string address = var->address_;
        auto findRecord = [side, address](const auto &target) {
            return (side == "client" && target->address_ == address
                && target->role_ == CommonUtils::GATT_ROLE_MASTER)
                || (side == "server" && target->role_ == CommonUtils::GATT_ROLE_SLAVE);
        };
        auto findRecordIter = find_if(gattConnectRecords_.begin(), gattConnectRecords_.end(), findRecord);
        if (findRecordIter != gattConnectRecords_.end()) {
            return true;
        }
    }
    return false;
}

void BluetoothDetect::ParseBluetoothRecordToStr(nlohmann::json &value)
{
    nlohmann::json bluetoothInfo;
    bluetoothInfo["bredr switch"] = isBrSwitchOn_;
    bluetoothInfo["ble switch"] = isBleSwitchOn_;
    auto arrayObj1 = nlohmann::json::array();
    for (auto var: sppConnectRecords_) {
        nlohmann::json sppConnectRecord;
        sppConnectRecord["socketId"] = var->socketId_;
        sppConnectRecord["pid"] = var->pid_;
        sppConnectRecord["uid"] = var->uid_;
        sppConnectRecord["address"] = var->address_;
        arrayObj1.push_back(sppConnectRecord);
    }
    bluetoothInfo["sppConnectRecords"] = arrayObj1;

    nlohmann::json pairsRecords(devicePairRecords_);
    bluetoothInfo["devicePairRecords"] = pairsRecords;

    auto arrayObj2 = nlohmann::json::array();
    for (auto var : gattConnectRecords_) {
        nlohmann::json gattConnectRecord;
        gattConnectRecord["address"] = var->address_;
        gattConnectRecord["role"] = var->role_;
        arrayObj2.push_back(gattConnectRecord);
    }
    bluetoothInfo["gattConnectRecords"] = arrayObj2;

    auto arrayObj3 = nlohmann::json::array();
    for (auto var : gattAppRegisterInfos_) {
        nlohmann::json gattAppRegisterInfo;
        gattAppRegisterInfo["side"] = var->side_;
        gattAppRegisterInfo["address"] = var->address_;
        gattAppRegisterInfo["appId"] = var->appId_;
        gattAppRegisterInfo["pid"] = var->pid_;
        gattAppRegisterInfo["uid"] = var->uid_;
        arrayObj3.push_back(gattAppRegisterInfo);
    }
    bluetoothInfo["gattAppRegisterInfos"] = arrayObj3;

    value["bluetooth"] = bluetoothInfo;
}

bool BluetoothDetect::ParseBluetoothRecordFromJson(const nlohmann::json &value, std::set<int32_t> &uidSet)
{
    if (!CommonUtils::CheckJsonValue(value, {"bluetooth"})) {
        return false;
    }
    nlohmann::json bluetoothInfo = value["bluetooth"];
    this->isBrSwitchOn_ = bluetoothInfo["bredr switch"].get<bool>();
    this->isBleSwitchOn_ = bluetoothInfo["ble switch"].get<bool>();
    int32_t uid;
    for (auto &elem : bluetoothInfo["sppConnectRecords"]) {
        uid = elem["uid"].get<int32_t>();
        uidSet.emplace(uid);
        auto record = std::make_shared<SppConnectStateReocrd>(elem["socketId"].get<int32_t>(),
            elem["pid"].get<int32_t>(), uid, elem["address"].get<std::string>());
        sppConnectRecords_.emplace_back(record);
    }

    for (auto &elem : bluetoothInfo["devicePairRecords"]) {
        devicePairRecords_.emplace(elem.get<std::string>());
    }

    for (auto &elem : bluetoothInfo["gattConnectRecords"]) {
        auto record = std::make_shared<GattConnectStateRecord>(elem["address"].get<std::string>(),
            elem["role"].get<int32_t>());
        gattConnectRecords_.emplace_back(record);
    }

    for (auto &elem : bluetoothInfo["gattAppRegisterInfos"]) {
        uid = elem["uid"].get<int32_t>();
        uidSet.emplace(uid);
        auto record = std::make_shared<GattAppRegisterInfo>(elem["side"].get<std::string>(),
            elem["address"].get<std::string>(), elem["appId"].get<int32_t>(),
            elem["pid"].get<int32_t>(), uid);
        gattAppRegisterInfos_.emplace_back(record);
    }
    return true;
}

void BluetoothDetect::ClearData()
{
    devicePairRecords_.clear();
    sppConnectRecords_.clear();
    gattConnectRecords_.clear();
    gattAppRegisterInfos_.clear();
}
}
}