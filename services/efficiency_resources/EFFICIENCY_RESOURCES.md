# 能效资源服务开发指南

[← 返回上级目录](../AGENTS.md)

## 1. 功能概述

能效资源服务（Efficiency Resources）负责管理应用的能效资源申请，提供资源使用优化和配额管理。核心特性：

- **资源申请**：支持应用级和进程级资源申请
- **配额管理**：CPU效率等级配额控制
- **超时机制**：资源超时自动回收
- **订阅通知**：资源状态变更通知订阅者
- **持久化**：资源申请记录持久化存储

## 2. 目录结构

```
efficiency_resources/
├── include/
│   ├── bg_efficiency_resources_mgr.h  # 主管理类
│   ├── resource_application_record.h   # 资源申请记录
│   └── resources_subscriber_mgr.h      # 订阅者管理
└── src/
    └── [对应实现文件]
```

## 3. 核心类说明

### 3.1 BgEfficiencyResourcesMgr

主管理类，协调资源申请、回收和订阅通知。

**核心方法**：

| 方法 | 功能 |
|------|------|
| `ApplyEfficiencyResources` | 申请能效资源 |
| `ResetAllEfficiencyResources` | 重置所有资源 |
| `GetAllEfficiencyResources` | 获取所有资源申请 |
| `GetEfficiencyResourcesInfos` | 获取资源信息列表 |
| `RemoveProcessRecord` | 移除进程资源记录 |
| `RemoveAppRecord` | 移除应用资源记录 |

### 3.2 ResourceApplicationRecord

资源申请记录，存储申请详细信息。

**核心字段**：

| 字段 | 说明 |
|------|------|
| `uid_` | 用户ID |
| `pid_` | 进程ID |
| `resourceNumber_` | 资源类型编号 |
| `bundleName_` | 包名 |
| `cpuLevel_` | CPU效率等级 |
| `resourceUnitList_` | 资源单元列表 |

### 3.3 ResourcesSubscriberMgr

订阅者管理，维护订阅者列表并通知状态变更。

**核心功能**：
- 添加/移除订阅者
- 通知资源申请事件
- 通知资源重置事件

## 4. 核心流程

### 4.1 申请能效资源流程

```
ApplyEfficiencyResources()
    │
    ├─→ IsCallingInfoLegal()           // 验证调用合法性
    │
    ├─→ CheckOrUpdateCpuApplyQuota()   // CPU配额检查
    │       │
    │       ├─→ 检查CPU效率等级配额
    │       ├─→ 验证申请是否超限
    │       └─→ 更新配额使用量
    │
    ├─→ ApplyResourceForPkgAndProc()   // 应用和进程级申请
    │       │
    │       ├─→ 区分应用级/进程级
    │       ├─→ 创建/更新资源记录
    │       └─→ 设置超时时间
    │
    ├─→ NotifySubscribers()            // 通知订阅者
    │       │
    │       ├─→ OnAppEfficiencyResourcesApply()
    │       └─→ OnProcEfficiencyResourcesApply()
    │
    ├─→ ReportHisysEvent()             // 上报事件
    │
    └─→ 持久化资源数据
```

### 4.2 重置能效资源流程

```
ResetAllEfficiencyResources()
    │
    ├─→ ResetAllEfficiencyResourcesInner()
    │       │
    │       ├─→ 区分应用级/进程级重置
    │       ├─→ 清理资源记录
    │       └─→ 更新配额使用量
    │
    ├─→ NotifySubscribers()
    │       │
    │       ├─→ OnAppEfficiencyResourcesReset()
    │       └─→ OnProcEfficiencyResourcesReset()
    │
    ├─→ ReportHisysEvent()
    │
    └─→ 更新持久化数据
```

### 4.3 资源超时回收流程

```
定时检测超时资源
    │
    ├─→ ResetTimeOutResource()
    │       │
    │       ├─→ 检查 endTime_ 是否过期
    │       ├─→ 非持久资源自动回收
    │       └─→ 持久资源保留
    │
    ├─→ NotifySubscribers()
    │
    └─→ 更新配额使用量
```

### 4.4 进程/应用死亡清理流程

```
OnProcessDied / OnAppDied
    │
    ├─→ RemoveProcessRecord() / RemoveAppRecord()
    │       │
    │       ├─→ 查找相关资源记录
    │       ├─→ 清理所有资源申请
    │       └─→ 通知订阅者
    │
    └─→ 更新配额使用量
```

## 5. 资源类型

### 5.1 支持的资源类型

| 资源编号 | 资源名称 | 说明 |
|----------|----------|------|
| 1 | CPU | CPU资源 |
| 2 | EVENT | 事件资源 |
| 4 | GPS | GPS定位资源 |
| 8 | AUDIO | 音频资源 |
| 16 | NETWORK | 网络资源 |

可通过位运算组合多个资源类型。

### 5.2 CPU效率等级

| 等级 | 说明 |
|------|------|
| DEFAULT | 默认等级 |
| LOW | 低效率等级 |
| MEDIUM | 中效率等级 |
| HIGH | 高效率等级 |

## 6. 数据结构

### 6.1 PersistTime

资源持久时间单元。

```cpp
struct PersistTime {
    uint32_t resourceIndex_;   // 资源索引
    bool isPersist_;           // 是否持久
    int64_t endTime_;          // 结束时间
    std::string reason_;       // 申请原因
    int64_t timeOut_;          // 超时时间
};
```

### 6.2 取消原因枚举

```cpp
enum class CancelReason : uint32_t {
    DEFAULT,                  // 默认
    DUMPER,                   // Dump命令取消
    APPLY_INTERFACE,          // 申请接口触发
    RESET_INTERFACE,          // 重置接口触发
    APP_DIED,                 // 应用死亡
    APP_DIDE_TRIGGER_PROCESS_DIED, // 应用死亡触发进程死亡
    PROCESS_DIED,             // 进程死亡
};
```

## 7. 线程安全

关键锁保护：

```cpp
std::mutex sysAbilityLock_;   // 系统能力状态保护
```

资源记录使用 `std::unordered_map` 存储，通过内部锁机制保护。

## 8. 事件监听

### 8.1 AppStateObserver 监听

| 事件 | 处理逻辑 |
|------|----------|
| 进程死亡 | 清理进程级资源申请 |
| 应用死亡 | 清理应用级资源申请 |

### 8.2 系统能力监听

| 事件 | 处理逻辑 |
|------|----------|
| 系统能力添加 | 初始化依赖服务 |
| 系统能力移除 | 清理相关资源 |

## 9. 持久化

资源申请记录通过 `DataStorageHelper` 持久化存储：

- 应用级资源记录：按 UID 存储
- 进程级资源记录：按 PID 存储
- 设备重启后恢复申请记录