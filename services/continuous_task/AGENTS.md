# 长时任务服务开发指南

[← 返回上级目录](../AGENTS.md)

## 1. 功能概述

长时任务服务（Continuous Task）负责管理应用的后台长时运行请求，提供持续的后台运行保障。核心特性：

- **通知管理**：必须显示通知提示用户
- **模式验证**：验证申请的后台模式是否合法
- **暂停机制**：支持任务暂停和恢复
- **订阅通知**：任务状态变更通知订阅者
- **权限校验**：检查应用是否有对应后台模式权限

## 2. 目录结构

```
continuous_task/
├── include/
│   ├── bg_continuous_task_mgr.h      # 主管理类
│   ├── continuous_task_record.h      # 任务记录实体
│   ├── notification_tools.h          # 通知工具
│   ├── task_notification_subscriber.h # 通知订阅者
│   ├── banner_notification_record.h  # 横幅通知记录
│   ├── config_change_observer.h      # 配置变更观察者
│   ├── bg_continuous_task_dumper.h   # Dump工具
│   └── remote_death_recipient.h      # 远程死亡监听
└── src/
    └── [对应实现文件]
```

## 3. 核心类说明

### 3.1 BgContinuousTaskMgr

主管理类，协调各子模块完成长时任务管理。

**核心方法**：

| 方法 | 功能 |
|------|------|
| `StartBackgroundRunning` | 开始长时任务 |
| `UpdateBackgroundRunning` | 更新长时任务模式 |
| `StopBackgroundRunning` | 停止长时任务 |
| `GetAllContinuousTasks` | 获取所有任务列表 |
| `RequestBackgroundRunningForInner` | 内部接口申请 |
| `StopContinuousTask` | 停止指定任务 |
| `SuspendContinuousTask` | 暂停任务 |
| `ActiveContinuousTask` | 恢复任务 |
| `RequestAuthFromUser` | 请求用户授权 |

### 3.2 ContinuousTaskRecord

任务记录实体，存储任务详细信息。

**核心字段**：

| 字段 | 说明 |
|------|------|
| `bundleName_` | 包名 |
| `abilityName_` | Ability名称 |
| `uid_` | 用户ID |
| `pid_` | 进程ID |
| `bgModeId_` | 后台模式ID |
| `bgModeIds_` | 后台模式列表（新API） |
| `notificationLabel_` | 通知标签 |
| `suspendState_` | 是否暂停状态 |
| `isBatchApi_` | 是否批量API |

### 3.3 NotificationTools

通知工具类，管理后台任务通知。

**核心功能**：
- 发布后台任务通知
- 取消后台任务通知
- 更新通知内容
- 处理通知点击事件

### 3.4 TaskNotificationSubscriber

通知订阅者，监听通知取消事件。

**监听事件**：
- 用户点击通知取消按钮
- 系统取消通知
- 通知被删除

## 4. 核心流程

### 4.1 开始长时任务流程

```
StartBackgroundRunning()
    │
    ├─→ CheckIsSysReadyAndPermission()  // 系统就绪和权限检查
    │
    ├─→ CheckBgmodeType()               // 长时任务类型校验
    │       │
    │       ├─→ 检查应用是否有该模式权限
    │       ├─→ 检查模式是否合法
    │       └─→ 检查是否支持批量申请
    │
    ├─→ 创建 ContinuousTaskRecord
    │
    ├─→ SendContinuousTaskNotification() // 发送通知
    │       │
    │       ├─→ 获取通知文本
    │       ├─→ 构建通知内容
    │       └─→ 发布通知
    │
    ├─→ 存储任务记录
    │
    ├─→ NotifySubscribersTaskStart()     // 通知订阅者
    │
    └─→ 持久化任务数据
```

### 4.2 停止长时任务流程

```
StopBackgroundRunning()
    │
    ├─→ 查找任务记录
    │
    ├─→ 取消通知
    │       │
    │       ├─→ NotificationTools::CancelNotification()
    │       └─→ 清理通知记录
    │
    ├─→ 设置停止原因
    │       │
    │       ├─→ USER_CANCEL         // 用户取消
    │       ├─→ REMOVE_NOTIFICATION // 删除通知
    │       ├─→ FREEZE_CANCEL       // 系统冻结
    │       └─→ SYSTEM_CANCEL       // 系统取消
    │
    ├─→ NotifySubscribersTaskCancel()   // 通知订阅者
    │
    ├─→ 删除任务记录
    │
    └─→ 更新持久化数据
```

### 4.3 暂停/恢复任务流程

```
SuspendContinuousTask()
    │
    ├─→ 设置 suspendState_ = true
    │
    ├─→ 设置 suspendReason_
    │
    ├─→ 更新通知状态
    │
    └─→ NotifySubscribersTaskSuspend()

ActiveContinuousTask()
    │
    ├─→ 设置 suspendState_ = false
    │
    ├─→ 清除 suspendReason_
    │
    ├─→ 恢复通知状态
    │
    └─→ NotifySubscribersTaskActive()
```

## 5. 长时任务类型

### 5.1 支持的长时任务类型

| 类型ID | 类型名称 | 说明 |
|--------|----------|------|
| 1 | dataTransfer | 数据传输 |
| 2 | audioPlayback | 音频播放 |
| 3 | audioRecording | 音频录制 |
| 4 | location | 定位 |
| 5 | bluetoothInteraction | 蓝牙交互 |
| 6 | multiDeviceConnection | 多设备连接 |
| 7 | wifiInteraction | WiFi交互 |
| 8 | voip | 网络通话 |
| 9 | taskKeeping | 任务保持 |

### 5.2 子类型（SubMode）

| 子类型ID | 子类型名称 |
|----------|------------|
| 1 | carKey | 车钥匙 |
| 2 | mediaProcessing | 媒体处理 |
| 3 | videoProjection | 视频投播 |
| 4 | sportTask | 运动任务 |

## 6. 数据结构

### 6.1 SubscriberInfo

订阅者信息。

```cpp
struct SubscriberInfo {
    sptr<IBackgroundTaskSubscriber> subscriber_;
    int uid_;
    int pid_;
    bool isHap_;
    uint32_t flag_;
};
```

### 6.2 CachedBundleInfo

缓存的Bundle信息。

```cpp
struct CachedBundleInfo {
    std::unordered_map<std::string, uint32_t> abilityBgMode_;
    std::string appName_;
};
```

## 7. 线程安全

关键锁保护：

```cpp
std::mutex delayTasksMutex_;         // 延迟任务保护
std::mutex liveViewInfoMutex_;       // 实时视图信息保护
```

## 8. 事件监听

### 8.1 AppStateObserver 监听

| 事件 | 处理逻辑 |
|------|----------|
| 应用停止 | 清理该应用所有任务 |
| 应用状态变更 | 更新前台任务列表 |
| Ability状态变更 | 检查任务合法性 |

### 8.2 SystemEventObserver 监听

| 事件 | 处理逻辑 |
|------|----------|
| 账户状态变更 | 清理非活跃账户任务 |
| Bundle信息变更 | 更新缓存信息 |

### 8.3 ConfigChangeObserver 监听

| 事件 | 处理逻辑 |
|------|----------|
| 配置变更 | 更新通知文本语言 |