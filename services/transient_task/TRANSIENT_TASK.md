# 短时任务服务开发指南

[← 返回上级目录](../AGENTS.md)

## 1. 功能概述

短时任务服务（Transient Task）负责管理应用的延迟挂起请求，为应用提供短时间的后台运行保障。核心特性：

- **配额管理**：每日配额限制，默认3分钟
- **超时机制**：定时器监控任务超时
- **回调通知**：任务即将过期时通知应用
- **订阅机制**：任务状态变更通知订阅者

## 2. 目录结构

```
transient_task/
├── include/
│   ├── bg_transient_task_mgr.h   # 主管理类
│   ├── decision_maker.h          # 决策器
│   ├── timer_manager.h           # 定时器管理
│   ├── watchdog.h                # 看门狗
│   ├── suspend_controller.h      # 挂起控制器
│   ├── device_info_manager.h     # 设备信息管理
│   ├── input_manager.h           # 输入事件管理
│   ├── key_info.h                # 任务键信息
│   ├── pkg_delay_suspend_info.h  # 短时任务申请信息
│   ├── delay_suspend_info_ex.h   # 短时任务信息扩展
│   ├── event_hub.h               # 事件中心
│   └── event_info.h              # 事件信息
└── src/
    └── [对应实现文件]
```

## 3. 核心类说明

### 3.1 BgTransientTaskMgr

主管理类，协调各子模块完成短时任务管理。

**核心方法**：

| 方法 | 功能 |
|------|------|
| `RequestSuspendDelay` | 申请短时任务 |
| `CancelSuspendDelay` | 取消短时任务 |
| `GetRemainingDelayTime` | 获取剩余时间 |
| `GetAllTransientTasks` | 获取所有任务列表 |
| `SubscribeBackgroundTask` | 订阅任务状态 |
| `PauseTransientTaskTimeForInner` | 暂停计时（内部接口） |

### 3.2 DecisionMaker

决策器，负责判断是否允许申请任务。

**核心逻辑**：
- 检查每日配额是否充足
- 检查应用是否在前台
- 处理屏幕亮灭、电源连接等事件
- 监听应用前后台状态变化

### 3.3 TimerManager

定时器管理，控制任务超时。

**核心功能**：
- 为每个任务设置超时定时器
- 超时前5秒触发过期回调
- 超时后自动取消任务

### 3.4 Watchdog

看门狗，监控任务执行状态。

**核心功能**：
- 监控任务是否正常执行
- 检测异常长时间占用

### 3.5 DeviceInfoManager

设备信息管理，维护设备状态。

**监控状态**：
- 屏幕亮灭状态
- 电源连接状态
- 低电量模式

## 4. 核心流程

### 4.1 申请短时任务流程

```
RequestSuspendDelay()
    │
    ├─→ IsCallingInfoLegal()     // 验证调用合法性
    │
    ├─→ DecisionMaker::Decide()  // 决策器判断
    │       │
    │       ├─→ 检查配额是否充足
    │       ├─→ 检查是否前台应用
    │       └─→ 设置超时定时器
    │
    ├─→ 生成 requestId
    │
    ├─→ 存储回调对象
    │
    └─→ 返回 DelaySuspendInfo
```

### 4.2 任务过期流程

```
TimerManager触发超时
    │
    ├─→ 提前5秒：调用 OnExpired 回调
    │       │
    │       └─→ 应用可选择续期或结束任务
    │
    ├─→ 超时时刻：自动取消任务
    │
    └─→ 通知订阅者任务结束
```

### 4.3 配额重置流程

```
每日配额重置（跨天检测）
    │
    ├─→ IsAfterOneDay() 检测是否跨天
    │
    ├─→ ResetDayQuotaLocked() 重置配额
    │
    └─→ 所有应用恢复默认配额
```

## 5. 数据结构

### 5.1 KeyInfo

任务键信息，标识唯一任务。

```cpp
struct KeyInfo {
    std::string bundleName;  // 包名
    int32_t uid;             // 用户ID
};
```

### 5.2 PkgDelaySuspendInfo

包延迟挂起信息，记录应用的配额使用情况。

```cpp
class PkgDelaySuspendInfo {
    int32_t requestId;           // 请求ID
    int32_t remainingQuota;      // 剩余配额
    bool isBackground;           // 是否后台
    int64_t requestTime;         // 申请时间
};
```

## 6. 线程安全

使用 `std::mutex` 保护共享数据：

```cpp
std::mutex suscriberLock_;           // 订阅者列表保护
std::mutex expiredCallbackLock_;     // 回调映射保护
std::mutex transientUidLock_;        // 暂停UID集合保护
```

## 7. 事件监听

### 7.1 InputManager 监听事件

| 事件类型 | 处理逻辑 |
|----------|----------|
| 屏幕亮起 | 暂停后台任务计时 |
| 屏幕熄灭 | 恢复后台任务计时 |
| 电源连接 | 暂停计时 |
| 电源断开 | 恢复计时 |

### 7.2 应用状态监听

通过 `ApplicationStateObserver` 监听：
- 应用切换到前台：暂停计时
- 应用切换到后台：恢复计时