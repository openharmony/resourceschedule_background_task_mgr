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

#include "task_detection_manager.h"

#include <new>
#include <unistd.h>
#include <random>

#include "bg_continuous_task_mgr.h"
#include "continuous_task_log.h"
#include "distributed_component_listener_stub.h"
#include "distributed_sched_proxy.h"
#include "iservice_registry.h"
#include "json/value.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace BackgroundTaskMgr {
TaskDetectionManager::TaskDetectionManager() {}

TaskDetectionManager::~TaskDetectionManager() {}

bool TaskDetectionManager::Init(const std::shared_ptr<DataStorage> &dataStorage,
    const std::shared_ptr<AppExecFwk::EventHandler> &handler)
{
    BGTASK_LOGD("TaskDetectionManager::Init begin");
    if (!dataStorage || !handler) {
        BGTASK_LOGE("TaskDetectionManager::Init dataStorage or handler is null");
        return false;
    }
    dataStorage_ = dataStorage;
    handler_ = handler;
    if (!AddSystemAbilityListener()) {
        return false;
    }
    audioDetect_ = std::make_shared<AudioDetect>();
    bluetoothDetect_ = std::make_shared<BluetoothDetect>();
    locationDetect_ = std::make_shared<LocationDetect>();
    multiDeviceDetect_ = std::make_shared<MultiDeviceDetect>();
    return true;
}

bool TaskDetectionManager::InitHiSysEventListener()
{
    BGTASK_LOGD("InitHiSysEventListener begin");
    hiEventListener_ = std::make_shared<HiSysEventListener>();
    HiviewDFX::ListenerRule brSwitchState("BLUETOOTH", "BLUETOOTH_BR_SWITCH_STATE");
    HiviewDFX::ListenerRule bleSwitchState("BLUETOOTH", "BLUETOOTH_BLE_STATE");
    HiviewDFX::ListenerRule sppConnectState("BLUETOOTH", "SPP_CONNECT_STATE");
    HiviewDFX::ListenerRule gattConnectState("BLUETOOTH", "GATT_CONNECT_STATE");
    HiviewDFX::ListenerRule gattAppRegister("BLUETOOTH", "GATT_APP_REGISTER");
    HiviewDFX::ListenerRule locationSwitchState("LOCATION", "LOCATION_SWITCH_STATE");
    HiviewDFX::ListenerRule gnssState("LOCATION", "GNSS_STATE");
    std::vector<HiviewDFX::ListenerRule> sysRules;
    sysRules.push_back(brSwitchState);
    sysRules.push_back(bleSwitchState);
    sysRules.push_back(sppConnectState);
    sysRules.push_back(gattConnectState);
    sysRules.push_back(gattAppRegister);
    sysRules.push_back(locationSwitchState);
    sysRules.push_back(gnssState);
    int32_t ret = HiviewDFX::HiSysEventManager::AddEventListener(hiEventListener_, sysRules);
    if (ret != ERR_OK) {
        BGTASK_LOGE("Hisysevent listener is added failed");
        return false;
    }
    return true;
}

bool TaskDetectionManager::InitDisCompChangeObserver()
{
    BGTASK_LOGD("TaskDetectionManager::InitDisCompChangeObserver begin");
    if (!GetDisSchedProxy()) {
        return false;
    }
    disCompListener_ = new (std::nothrow) DistributedComponentListenerStub();
    if (disSched_->RegisterDistributedComponentListener(disCompListener_->AsObject()) != ERR_OK) {
        return false;
    }
    return true;
}

bool TaskDetectionManager::InitAudioStateChangeListener()
{
    BGTASK_LOGD("TaskDetectionManager::InitAudioStateChangeListener begin");

    // generete random number as audio event listener id.
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int32_t> dist(0x01);
    int32_t clientId = dist(mt);
    BGTASK_LOGI("Generate random clientId: %{public}d", clientId);
    playerListener_ = std::make_shared<AudioRendererStateChangeListener>();
    recorderListener_ = std::make_shared<AudioCapturerStateChangeListener>();
    if (AudioStandard::AudioStreamManager::GetInstance()->RegisterAudioRendererEventListener(
        clientId, playerListener_) != ERR_OK) {
        BGTASK_LOGE("RegisterAudioRendererEventListener failed");
        return false;
    }
    if (AudioStandard::AudioStreamManager::GetInstance()->RegisterAudioCapturerEventListener(
        clientId, recorderListener_) != ERR_OK) {
        BGTASK_LOGE("RegisterAudioCapturerEventListener failed");
        return false;
    }
    return true;
}

bool TaskDetectionManager::InitAVSessionStateChangeListener()
{
#ifdef AV_SESSION_PART_ENABLE
    avSessionStateListener_ = std::make_shared<SessionStateListener>();
    if (AVSession::AVSessionManager::GetInstance().RegisterSessionListener(avSessionStateListener_) != ERR_OK) {
        BGTASK_LOGE("RegisterSessionListener failed");
        return false;
    }
#endif // AV_SESSION_PART_ENABLE
    return true;
}

bool TaskDetectionManager::InitBluetoothStateChangeObserver()
{
#ifdef BLUETOOTH_PART_ENABLE
    btRemoteDeviceObserver_ = std::make_shared<BluetoothRemoteDeviceObserver>();
    Bluetooth::BluetoothHost::GetDefaultHost().RegisterRemoteDeviceObserver(*(btRemoteDeviceObserver_.get()));
#endif // BLUETOOTH_PART_ENABLE
    return true;
}

void TaskDetectionManager::OnAddSystemAbility(int32_t systemAbilityId)
{
    handler_->PostTask([=]() {
        HandleSystemAbilityAdded(systemAbilityId);
    });
}

void TaskDetectionManager::HandleSystemAbilityAdded(int32_t systemAbilityId)
{
    switch (systemAbilityId) {
        case DISTRIBUTED_SCHED_SA_ID:
            InitDisCompChangeObserver();
            break;
        case AUDIO_POLICY_SERVICE_ID:
            InitAudioStateChangeListener();
            break;
        case DFX_SYS_EVENT_SERVICE_ABILITY_ID:
            InitHiSysEventListener();
            break;
        case AVSESSION_SERVICE_ID:
            InitAVSessionStateChangeListener();
            break;
        case BLUETOOTH_HOST_SYS_ABILITY_ID:
            InitBluetoothStateChangeObserver();
            break;
        default:
            break;
    }
}

void TaskDetectionManager::HandlePersistenceData(const std::vector<AppExecFwk::RunningProcessInfo> &allProcesses)
{
    if (dataStorage_ == nullptr) {
        BGTASK_LOGE("DataStorage is null");
        return;
    }
    std::set<int32_t> allUidHasTasks;
    nlohmann::json value;
    if (dataStorage_->RestoreTaskDetectionInfo(value) != ERR_OK) {
        BGTASK_LOGE("Get detection persistent info failed");
        return;
    }
    bool hasNoRunningUid = true;
    if (!ParseRecordFromJson(value, allUidHasTasks)) {
        BGTASK_LOGE("ParseRecordFromJson failed");
        return;
    }
    for (auto process : allProcesses) {
        if (allUidHasTasks.find(process.uid_) != allUidHasTasks.end()) {
            hasNoRunningUid = false;
            break;
        }
    }
#ifdef BLUETOOTH_PART_ENABLE
    bluetoothDetect_->isBrSwitchOn_ = Bluetooth::BluetoothHost::GetDefaultHost().GetBtState()
        == CommonUtils::CLASSICAL_BT_SWITCH_ON ? true : false;
    bluetoothDetect_->isBleSwitchOn_ = Bluetooth::BluetoothHost::GetDefaultHost().IsBleEnabled();
#else // BLUETOOTH_PART_ENABLE
    bluetoothDetect_->isBrSwitchOn_ = false;
    bluetoothDetect_->isBleSwitchOn_ = false;
#endif // BLUETOOTH_PART_ENABLE
    if (hasNoRunningUid && !bluetoothDetect_->isBrSwitchOn_ && !bluetoothDetect_->isBleSwitchOn_) {
        ClearAllData();
    }
    dataStorage_->RefreshTaskDetectionInfo(ParseRecordToStr());
}

void TaskDetectionManager::ReportBluetoothPairState(const std::string &addr, int32_t state)
{
    BGTASK_LOGI("ReportBluetoothPairState begin, address: %{public}s, state: %{public}d", addr.c_str(), state);
    handler_->PostTask([=]() {
        bluetoothDetect_->HandleBluetoothPairState(addr, state);
        dataStorage_->RefreshTaskDetectionInfo(ParseRecordToStr());
    });
}

void TaskDetectionManager::ClearAllData()
{
    audioDetect_->ClearData();
    bluetoothDetect_->ClearData();
    locationDetect_->ClearData();
    multiDeviceDetect_->ClearData();
}

bool TaskDetectionManager::AddSystemAbilityListener()
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgrProxy) {
        BGTASK_LOGE("failed to get samgrProxy");
        return false;
    }
    statusChangeListener_ = new (std::nothrow) SystemAbilityListener();
    if (samgrProxy->SubscribeSystemAbility(DFX_SYS_EVENT_SERVICE_ABILITY_ID, statusChangeListener_) != ERR_OK) {
        BGTASK_LOGE("failed to listen hiview sa");
        return false;
    }
    if (samgrProxy->SubscribeSystemAbility(DISTRIBUTED_SCHED_SA_ID, statusChangeListener_) != ERR_OK) {
        BGTASK_LOGE("failed to listen dis sched sa");
        return false;
    }
    if (samgrProxy->SubscribeSystemAbility(AUDIO_POLICY_SERVICE_ID, statusChangeListener_) != ERR_OK) {
        BGTASK_LOGE("failed to listen audio service sa");
        return false;
    }
    if (samgrProxy->SubscribeSystemAbility(AVSESSION_SERVICE_ID, statusChangeListener_) != ERR_OK) {
        BGTASK_LOGE("failed to listen avsession service sa");
        return false;
    }
    if (samgrProxy->SubscribeSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, statusChangeListener_) != ERR_OK) {
        BGTASK_LOGE("failed to listen bluetooth service sa");
        return false;
    }
    return true;
}

bool TaskDetectionManager::GetDisSchedProxy()
{
    if (disSched_ != nullptr) {
        return true;
    }
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        BGTASK_LOGE("fail to get samgr.");
        return false;
    }
    sptr<IRemoteObject> remote = samgrProxy->GetSystemAbility(DISTRIBUTED_SCHED_SA_ID);
    if (remote == nullptr) {
        BGTASK_LOGE("Get DMS failed");
        return false;
    }
    disSched_ = iface_cast<DistributedSchedule::IDistributedSched>(remote);
    if (disSched_ == nullptr) {
        BGTASK_LOGE("Get DMS proxy failed");
        return false;
    }
    return true;
}

void TaskDetectionManager::HandleAudioStreamInfo(const std::list<std::tuple<int32_t, int32_t, int32_t>> &streamInfos,
    const std::string &type)
{
    BGTASK_LOGI("HandleAudioStreamInfo begin, type: %{public}s", type.c_str());
    handler_->PostTask([=]() {
        audioDetect_->HandleAudioStreamInfo(streamInfos, type);
        dataStorage_->RefreshTaskDetectionInfo(ParseRecordToStr());
    });
}

#ifdef AV_SESSION_PART_ENABLE
void TaskDetectionManager::HandleAVSessionInfo(const AVSession::AVSessionDescriptor &descriptor,
    const std::string &action)
{
    handler_->PostTask([=]() {
        audioDetect_->HandleAVSessionInfo(descriptor, action);
        dataStorage_->RefreshTaskDetectionInfo(ParseRecordToStr());
    });
}
#endif // AV_SESSION_PART_ENABLE

void TaskDetectionManager::HandleBluetoothSysEvent(const nlohmann::json &root)
{
    handler_->PostTask([=]() {
        bluetoothDetect_->HandleBluetoothSysEvent(root);
        dataStorage_->RefreshTaskDetectionInfo(ParseRecordToStr());
    });
}

void TaskDetectionManager::HandleLocationSysEvent(const nlohmann::json &root)
{
    handler_->PostTask([=]() {
        locationDetect_->HandleLocationSysEvent(root);
        dataStorage_->RefreshTaskDetectionInfo(ParseRecordToStr());
    });
}

void TaskDetectionManager::HandleProcessDied(int32_t uid, int32_t pid)
{
    bluetoothDetect_->sppConnectRecords_.remove_if(
        [uid, pid](auto record) {return (record->uid_ == uid && record->pid_ == pid);});
    bluetoothDetect_->gattAppRegisterInfos_.remove_if(
        [uid, pid](auto record) {return (record->uid_ == uid && record->pid_ == pid);});
    audioDetect_->audioPlayerInfos_.remove_if([uid](auto record) {return (record->uid_ == uid);});
    audioDetect_->audioRecorderInfos_.remove_if([uid](auto record) {return (record->uid_ == uid);});
    audioDetect_->avSessionInfos_.remove_if(
        [uid, pid](auto record) {return (record->uid_ == uid && record->pid_ == pid);});
    locationDetect_->locationUsingRecords_.remove_if(
        [uid, pid](auto record) {return (record.first == uid && record.second == pid);});
    multiDeviceDetect_->calleeRecords_.erase(uid);
    multiDeviceDetect_->callerRecords_.erase(uid);
    dataStorage_->RefreshTaskDetectionInfo(ParseRecordToStr());
}

void TaskDetectionManager::HandleDisComponentChange(const std::string &info)
{
    BGTASK_LOGI("MultiDeviceDetect::HandleDisComponentChange info: %{public}s", info.c_str());
    handler_->PostTask([=]() {
        multiDeviceDetect_->HandleDisComponentChange(info);
        dataStorage_->RefreshTaskDetectionInfo(ParseRecordToStr());
    });
}

void TaskDetectionManager::ReportNeedRecheckTask(int32_t uid, uint32_t taskType)
{
    auto reportTask = [=]() {
        if (!CheckTaskRunningState(uid, taskType)) {
            BgContinuousTaskMgr::GetInstance()->ReportTaskRunningStateUnmet(uid,
                CommonUtils::UNSET_PID, taskType);
        }
    };
    handler_->PostTask(reportTask, CommonUtils::RECHECK_DELAY_TIME);
}

bool TaskDetectionManager::CheckTaskRunningState(int32_t uid, uint32_t taskType)
{
    if (taskType == CommonUtils::AUDIO_PLAYBACK_BGMODE_ID) {
        return audioDetect_->CheckAudioCondition(uid, CommonUtils::AUDIO_PLAYBACK_BGMODE_ID);
    } else if (taskType == CommonUtils::AUDIO_RECORDING_BGMODE_ID) {
        return audioDetect_->CheckAudioCondition(uid, CommonUtils::AUDIO_RECORDING_BGMODE_ID);
    } else if (taskType == CommonUtils::LOCATION_BGMODE_ID) {
        return locationDetect_->CheckLocationCondition(uid);
    } else if (taskType == CommonUtils::BLUETOOTH_INTERACTION_BGMODE_ID) {
        return bluetoothDetect_->CheckBluetoothUsingScene(uid);
    } else if (taskType == CommonUtils::MULTIDEVICE_CONNECTION_BGMODE_ID) {
        return multiDeviceDetect_->CheckIsDisSchedScene(uid);
    } else {
        BGTASK_LOGE("Invalid taskType");
    }
    return false;
}

void TaskDetectionManager::Dump(std::vector<std::string> &dumpInfo)
{
    std::string recordInfo = ParseRecordToStr();
    dumpInfo.emplace_back(recordInfo);
}

std::string TaskDetectionManager::ParseRecordToStr()
{
    nlohmann::json root;
    locationDetect_->ParseLocationRecordToStr(root);
    bluetoothDetect_->ParseBluetoothRecordToStr(root);
    multiDeviceDetect_->ParseDisSchedRecordToStr(root);
    audioDetect_->ParseAudioRecordToStr(root);
    return root.dump(CommonUtils::JSON_FORMAT);
}

bool TaskDetectionManager::ParseRecordFromJson(const nlohmann::json &value, std::set<int32_t> &uidSet)
{
    if (value.is_null() || !value.is_object()) {
        BGTASK_LOGE("ParseRecordFromJson json value is empty");
        return false;
    }

    locationDetect_->ParseLocationRecordFromJson(value, uidSet);
    bluetoothDetect_->ParseBluetoothRecordFromJson(value, uidSet);
    multiDeviceDetect_->ParseDisSchedRecordFromJson(value, uidSet);
    audioDetect_->ParseAudioRecordFromJson(value, uidSet);

    std::string allUid;
    for (auto var : uidSet) {
        allUid += (std::to_string(var) + ", ");
    }
    BGTASK_LOGD("All record running uids are: %{public}s", allUid.c_str());
    return true;
}

TaskDetectionManager::HiSysEventListener::HiSysEventListener() : HiSysEventSubscribeCallBack() {}

TaskDetectionManager::HiSysEventListener::~HiSysEventListener() {}

void TaskDetectionManager::HiSysEventListener::OnHandle(const std::string &domain,
    const std::string &eventName, const int32_t eventType, const std::string &eventDetail)
{
    BGTASK_LOGI("OnHandle get hisysevent info: %{public}s", eventDetail.c_str());
    nlohmann::json root = nlohmann::json::parse(eventDetail, nullptr, false);
    if (root.is_discarded()) {
        BGTASK_LOGE("Parse hisysevent data failed");
        return;
    }
    if (!CommonUtils::CheckJsonValue(root, {"domain_"})) {
        BGTASK_LOGE("hisysevent data domain info lost");
        return;
    }
    std::string domainName = root.at("domain_").get<std::string>();
    if (domainName == "BLUETOOTH") {
        TaskDetectionManager::GetInstance()->HandleBluetoothSysEvent(root);
    } else if (domainName == "LOCATION") {
        TaskDetectionManager::GetInstance()->HandleLocationSysEvent(root);
    } else {
        BGTASK_LOGW("Get unrelated event");
    }
}
void TaskDetectionManager::HiSysEventListener::OnServiceDied() {}

void TaskDetectionManager::AudioRendererStateChangeListener::OnRendererStateChange(
    const std::vector<std::unique_ptr<AudioStandard::AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    std::list<std::tuple<int32_t, int32_t, int32_t>> streamInfos;
    for (const auto &var : audioRendererChangeInfos) {
        streamInfos.emplace_back(std::make_tuple(var->clientUID, var->sessionId,
            static_cast<int32_t>(var->rendererState)));
    }
    TaskDetectionManager::GetInstance()->HandleAudioStreamInfo(streamInfos, "player");
}

void TaskDetectionManager::AudioCapturerStateChangeListener::OnCapturerStateChange(
    const std::vector<std::unique_ptr<AudioStandard::AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    std::list<std::tuple<int32_t, int32_t, int32_t>> streamInfos;
    for (const auto &var : audioCapturerChangeInfos) {
        streamInfos.emplace_back(std::make_tuple(var->clientUID, var->sessionId,
            static_cast<int32_t>(var->capturerState)));
    }
    TaskDetectionManager::GetInstance()->HandleAudioStreamInfo(streamInfos, "recorder");
}

void TaskDetectionManager::SystemAbilityListener::OnAddSystemAbility(
    int32_t systemAbilityId, const std::string &deviceId)
{
    BGTASK_LOGI("System ability added: %{public}d", systemAbilityId);
    TaskDetectionManager::GetInstance()->OnAddSystemAbility(systemAbilityId);
}

void TaskDetectionManager::SystemAbilityListener::OnRemoveSystemAbility(
    int32_t systemAbilityId, const std::string &deviceId) {}

#ifdef AV_SESSION_PART_ENABLE
void TaskDetectionManager::SessionStateListener::OnSessionCreate(
    const AVSession::AVSessionDescriptor &descriptor)
{
    BGTASK_LOGI("OnSessionCreate begin");
    TaskDetectionManager::GetInstance()->HandleAVSessionInfo(descriptor, "create");
}
void TaskDetectionManager::SessionStateListener::OnSessionRelease(
    const AVSession::AVSessionDescriptor &descriptor)
{
    BGTASK_LOGI("OnSessionRelease begin");
    TaskDetectionManager::GetInstance()->HandleAVSessionInfo(descriptor, "release");
}
void TaskDetectionManager::SessionStateListener::OnTopSessionChange(
    const AVSession::AVSessionDescriptor &descriptor) {}
#endif // AV_SESSION_PART_ENABLE

#ifdef BLUETOOTH_PART_ENABLE
void TaskDetectionManager::BluetoothRemoteDeviceObserver::OnPairStatusChanged(
    const Bluetooth::BluetoothRemoteDevice &device, int status)
{
    std::string addr = device.GetDeviceAddr();
    TaskDetectionManager::GetInstance()->ReportBluetoothPairState(addr, status);
}

void TaskDetectionManager::BluetoothRemoteDeviceObserver::OnRemoteUuidChanged(
    const Bluetooth::BluetoothRemoteDevice &device, const std::vector<Bluetooth::ParcelUuid> &uuids) {}

void TaskDetectionManager::BluetoothRemoteDeviceObserver::OnRemoteNameChanged(
    const Bluetooth::BluetoothRemoteDevice &device, const std::string &deviceName) {}

void TaskDetectionManager::BluetoothRemoteDeviceObserver::OnRemoteAliasChanged(
    const Bluetooth::BluetoothRemoteDevice &device, const std::string &alias) {}

void TaskDetectionManager::BluetoothRemoteDeviceObserver::OnRemoteCodChanged(
    const Bluetooth::BluetoothRemoteDevice &device, const Bluetooth::BluetoothDeviceClass &cod) {}

void TaskDetectionManager::BluetoothRemoteDeviceObserver::OnRemoteBatteryLevelChanged(
    const Bluetooth::BluetoothRemoteDevice &device, int batteryLevel) {}

void TaskDetectionManager::BluetoothRemoteDeviceObserver::OnReadRemoteRssiEvent(
    const Bluetooth::BluetoothRemoteDevice &device, int rssi, int status) {}
#endif // BLUETOOTH_PART_ENABLE
}  // namespace BackgroundTaskMgr
}  // namespace OHOS