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
#include "continuous_task_log.h"

namespace OHOS {
namespace BackgroundTaskMgr {
namespace {
static constexpr int32_t UNSET_UID = -1;
static constexpr int32_t UNSET_PID = -1;
static constexpr uint32_t BLUETOOTH_INTERACTION_BGMODE_ID = 5;
static constexpr uint32_t SOFTBUS_SA_UID = 1024;
static constexpr int32_t GATT_ROLE_MASTER = 0;
static constexpr int32_t GATT_ROLE_SLAVE = 1;
}

void BluetoothDetect::HandleBluetoothSysEvent(const Json::Value &root)
{
    if (!root.isMember("name_") || root["name_"].isNull()) {
        BGTASK_LOGE("hisysevent data domain info lost");
        return;
    }
    std::string eventName = root["name_"].asString();
    if (eventName == "BLUETOOTH_BR_SWITCH_STATE" || eventName == "BLUETOOTH_BLE_STATE") {
        HandleBtSwitchState(root);
    } else if (eventName == "BLUETOOTH_SPP_CONNECT_STATE") {
        HandleSppConnect(root);
    } else if (eventName == "BLUETOOTH_GATT_CONNECT_STATE") {
        HandleGattConnect(root);
    } else if (eventName == "BLUETOOTH_GATT_APP_REGISTER") {
        if (!root.isMember("ACTION")) {
            BGTASK_LOGE("Bluetooth connect state event lost important info");
            return;
        }
        std::string action = root["ACTION"].asString();
        if (action == "register") {
            HandleGattGattAppRegister(root);
        } else if (action == "deregister") {
            HandleGattGattAppDeregister(root);
        }
    } else {
        BGTASK_LOGW("Ignore unrelated event: %{public}s", eventName.c_str());
    }
}

void BluetoothDetect::HandleBtSwitchState(const Json::Value& root)
{
    if (!root.isMember("STATE")) {
        BGTASK_LOGE("Bluetooth sys event lost important info");
        return;
    }
    std::string switchState = root["STATE"].asString();
    std::string eventName = root["name_"].asString();
    if (eventName == "BLUETOOTH_BR_SWITCH_STATE") {
        if (switchState == "1") { // 1 means state turn on
            isBrSwitchOn_ = true;
        } else if (switchState == "3") { // 3 means state turn off
            isBrSwitchOn_ = false;
        }
    } else if (eventName == "BLUETOOTH_BLE_STATE") {
        if (switchState == "1") {
            isBleSwitchOn_ = true;
        } else if (switchState == "3") {
            isBleSwitchOn_ = false;
        }
    }
    if (!isBrSwitchOn_ && !isBleSwitchOn_) {
        BgContinuousTaskMgr::GetInstance()->ReportTaskRunningStateUnmet(UNSET_UID,
            UNSET_PID, BLUETOOTH_INTERACTION_BGMODE_ID);
    }
}

void BluetoothDetect::HandleSppConnect(const Json::Value& root)
{
    if (!root.isMember("ACTION") || !root.isMember("ID")
        || !root.isMember("PID") || !root.isMember("UID")) {
        BGTASK_LOGE("Bluetooth connect state event lost important info");
        return;
    }
    std::string action = root["ACTION"].asString();
    int32_t socketId = atoi(root["ID"].asString().c_str());
    int32_t pid = atoi(root["PID"].asString().c_str());
    int32_t uid = atoi(root["UID"].asString().c_str());
    if (uid == SOFTBUS_SA_UID) {
        BGTASK_LOGI("Ignore spp server app register event about softbus");
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
        sppConnectRecords_.emplace_back(std::make_shared<SppConnectStateReocrd>(socketId, pid, uid));
    } else if (action == "close") {
        if (findRecordIter == sppConnectRecords_.end()) {
            BGTASK_LOGE("SPP receive close event is not exist");
            return;
        }
        sppConnectRecords_.erase(findRecordIter);
        if (!CheckBluetoothUsingScene(uid)) {
            BgContinuousTaskMgr::GetInstance()->ReportTaskRunningStateUnmet(uid,
                UNSET_PID, BLUETOOTH_INTERACTION_BGMODE_ID);
        }
    }
}

void BluetoothDetect::HandleGattConnect(const Json::Value& root)
{
    if (!root.isMember("ADDRESS") || !root.isMember("STATE") || !root.isMember("ROLE")
        || !root.isMember("TRANSPORT")) {
        BGTASK_LOGE("Bluetooth connect state event lost important info");
        return;
    }
    std::string address = root["ADDRESS"].asString();
    std::string state = root["STATE"].asString();
    int32_t role = atoi(root["ROLE"].asString().c_str());
    int32_t transport = atoi(root["TRANSPORT"].asString().c_str());
    
    auto findRecord = [address, transport, role](const auto &target) {
        return target->address_ == address && target->transport_ == transport && target->role_ == role;
    };
    auto findRecordIter = find_if(gattConnectRecords_.begin(), gattConnectRecords_.end(), findRecord);
    if (state == "0" || state == "4") { // 0 means device connected, 4 means device reconnected.
        if (findRecordIter == gattConnectRecords_.end()) {
            gattConnectRecords_.emplace_back(std::make_shared<GattConnectStateRecord>(address, role, transport));
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

void BluetoothDetect::HandleGattGattAppRegister(const Json::Value& root)
{
    if (!root.isMember("SIDE") || !root.isMember("ADDRESS") || !root.isMember("PID")
        || !root.isMember("UID") || !root.isMember("APPID")) {
        BGTASK_LOGE("Bluetooth connect state event lost important info");
        return;
    }
    std::string side = root["SIDE"].asString();
    std::string address = root["ADDRESS"].asString();
    int32_t pid = atoi(root["PID"].asString().c_str());
    int32_t uid = atoi(root["UID"].asString().c_str());
    int32_t appId = atoi(root["APPID"].asString().c_str());

    if (uid == SOFTBUS_SA_UID) {
        BGTASK_LOGI("Ignore gatt server app register event about softbus");
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

void BluetoothDetect::HandleGattGattAppDeregister(const Json::Value& root)
{
    if (!root.isMember("SIDE") || !root.isMember("APPID")) {
        BGTASK_LOGE("Bluetooth connect state event lost important info");
        return;
    }
    std::string side = root["SIDE"].asString();
    int32_t appId = atoi(root["APPID"].asString().c_str());

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
            BgContinuousTaskMgr::GetInstance()->ReportTaskRunningStateUnmet(record->uid_,
                UNSET_PID, BLUETOOTH_INTERACTION_BGMODE_ID);
        }
    }
}

void BluetoothDetect::HandleGattDisconnect(const std::shared_ptr<GattConnectStateRecord> &record)
{
    BGTASK_LOGD("Disconnect record info address: %{public}s, role: %{public}d",
        record->address_.c_str(), record->role_);
    if (record->role_ == GATT_ROLE_SLAVE) { // means server side connect removed
        HandleSlaveSideDisconnect();
    } else if (record->role_ == GATT_ROLE_MASTER) { // maeans client side connect removed
        HandleMasterSideDisconnect(record->address_);
    }
}

void BluetoothDetect::HandleMasterSideDisconnect(const std::string &addr)
{
    // data struct to record after one client connect removed if need to stop related continuous tasks.
    std::map<std::pair<int32_t, int32_t>, bool> clientToRemove;
    // this loop is to get all gatt client app's uid and pid that address is the same as the disconnect info.
    for (auto var : gattAppRegisterInfos_) {
        if (var->side_ == "client" && var->address_ == addr) {
            clientToRemove[std::make_pair(var->uid_, var->pid_)] = false;
        }
    }
    // this loop is check whether there is target gatt client app(with same uid and pid)
    // that connect different remote device(namely different address)
    for (auto var : gattAppRegisterInfos_) {
        if (var->side_ == "client" && var->address_ != addr) {
            clientToRemove[std::make_pair(var->uid_, var->pid_)] = true;
        }
    }
    // after second loop check, if target uid and pid does not contain any different bluetooth address,
    // report to stop related continous task.
    for (const auto& var : clientToRemove) {
        if (var.second == false) {
            BgContinuousTaskMgr::GetInstance()->ReportTaskRunningStateUnmet(var.first.first,
                var.first.second, BLUETOOTH_INTERACTION_BGMODE_ID);
        }
    }
}

void BluetoothDetect::HandleSlaveSideDisconnect()
{
    for (auto var : gattConnectRecords_) {
        if (var->role_ == GATT_ROLE_SLAVE) {
            return;
        }
    }
    for (auto var : gattAppRegisterInfos_) {
        if (var->side_ != "server") {
            continue;
        }
        bool isClientHasSameUidPid = false;
        // need to recheck there is no client connect with same uid and pid
        for (auto innerClient : gattAppRegisterInfos_) {
            if (innerClient->side_ == "client" && innerClient->uid_ == var->uid_ && innerClient->pid_ == var->pid_) {
                isClientHasSameUidPid = true;
                break;
            }
        }
        if (!isClientHasSameUidPid) {
            BgContinuousTaskMgr::GetInstance()->ReportTaskRunningStateUnmet(var->uid_, var->pid_,
                BLUETOOTH_INTERACTION_BGMODE_ID);
        }
    }
}

bool BluetoothDetect::CheckBluetoothUsingScene(int32_t uid)
{
    if (!isBrSwitchOn_ && !isBleSwitchOn_) {
        return false;
    }
    // check if is using SPP function
    for (auto var : sppConnectRecords_) {
        if (var->uid_ == uid) {
            return true;
        }
    }
    // check if is using GATT function
    for (auto var : gattAppRegisterInfos_) {
        if (var->uid_ != uid) {
            continue;
        }
        if (var->side_ == "client") {
            for (auto record : gattConnectRecords_) {
                if (var->address_ == record->address_ && record->role_ == GATT_ROLE_MASTER) {
                    return true;
                }
            }
        } else if (var->side_ == "server") {
            for (auto record : gattConnectRecords_) {
                if (record->role_ == GATT_ROLE_SLAVE) {
                    return true;
                }
            }
        }
    }
    return false;
}

void BluetoothDetect::ParseBluetoothRecordToStr(Json::Value &value)
{
    Json::Value bluetoothInfo;
    bluetoothInfo["bredr switch"] = isBrSwitchOn_;
    bluetoothInfo["ble switch"] = isBleSwitchOn_;
    Json::Value arrayObj1;
    for (auto var: sppConnectRecords_) {
        Json::Value sppConnectRecord;
        sppConnectRecord["socketId"] = var->socketId_;
        sppConnectRecord["pid"] = var->pid_;
        sppConnectRecord["uid"] = var->uid_;
        bluetoothInfo["sppConnectRecords"].append(sppConnectRecord);
    }

    for (auto var : gattConnectRecords_) {
        Json::Value gattConnectRecord;
        gattConnectRecord["address"] = var->address_;
        gattConnectRecord["role"] = var->role_;
        gattConnectRecord["transport"] = var->transport_;
        bluetoothInfo["gattConnectRecords"].append(gattConnectRecord);
    }

    for (auto var : gattAppRegisterInfos_) {
        Json::Value gattAppRegisterInfo;
        gattAppRegisterInfo["side"] = var->side_;
        gattAppRegisterInfo["address"] = var->address_;
        gattAppRegisterInfo["appId"] = var->appId_;
        gattAppRegisterInfo["pid"] = var->pid_;
        gattAppRegisterInfo["uid"] = var->uid_;
        bluetoothInfo["gattAppRegisterInfos"].append(gattAppRegisterInfo);
    }

    value["bluetooth"] = bluetoothInfo;
}

bool BluetoothDetect::ParseBluetoothRecordFromJson(const Json::Value &value, std::set<int32_t> &uidSet)
{
    if (!value.isMember("bluetooth")) {
        return false;
    }
    Json::Value bluetoothInfo = value["bluetooth"];
    this->isBrSwitchOn_ = bluetoothInfo["bredr switch"].asBool();
    this->isBleSwitchOn_ = bluetoothInfo["ble switch"].asBool();
    Json::Value arrayObj = bluetoothInfo["sppConnectRecords"];
    int32_t uid;
    for (uint32_t i = 0; i < arrayObj.size(); i++) {
        uid = arrayObj[i]["uid"].asInt();
        uidSet.emplace(uid);
        auto record = std::make_shared<SppConnectStateReocrd>(arrayObj[i]["socketId"].asInt(),
            arrayObj[i]["pid"].asInt(), uid);
        sppConnectRecords_.emplace_back(record);
    }

    arrayObj = bluetoothInfo["gattConnectRecords"];
    for (uint32_t i = 0; i < arrayObj.size(); i++) {
        auto record = std::make_shared<GattConnectStateRecord>(arrayObj[i]["address"].asString(),
            arrayObj[i]["role"].asInt(), arrayObj[i]["transport"].asInt());
        gattConnectRecords_.emplace_back(record);
    }

    arrayObj = bluetoothInfo["gattAppRegisterInfos"];
    for (uint32_t i = 0; i < arrayObj.size(); i++) {
        uid = arrayObj[i]["uid"].asInt();
        uidSet.emplace(uid);
        auto record = std::make_shared<GattAppRegisterInfo>(arrayObj[i]["side"].asString(),
            arrayObj[i]["address"].asString(), arrayObj[i]["appId"].asInt(),
            arrayObj[i]["pid"].asInt(), uid);
        gattAppRegisterInfos_.emplace_back(record);
    }
    return true;
}

void BluetoothDetect::ClearData()
{
    sppConnectRecords_.clear();
    gattConnectRecords_.clear();
    gattAppRegisterInfos_.clear();
}
}
}