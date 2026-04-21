[← 返回上级目录](../AGENTS.md)

# 服务层开发指南

## 1. 功能概述

服务层实现后台任务管理的核心业务逻辑，包括短时任务、长时任务和能效资源三大核心功能的管理与调度。

## 2. 目录结构

```
/services
├── common/              # 公共模块
│   ├── include/        # 头文件
│   └── src/            # 实现文件
├── transient_task/      # 短时任务服务
│   ├── include/
│   └── src/
├── continuous_task/     # 长时任务服务
│   ├── include/
│   └── src/
├── efficiency_resources/ # 能效资源服务
│   ├── include/
│   └── src/
├── core/                # 核心服务入口
│   ├── include/
│   └── src/
└── test/                # 单元测试
    └── unittest/
```

## 3. 短时任务服务

**目录**: `transient_task/` → [详细开发指南](transient_task/AGENTS.md)

| 核心类 | 文件 | 功能描述 |
|--------|------|----------|
| `BgTransientTaskMgr` | `bg_transient_task_mgr.h/cpp` | 短时任务管理主类，处理申请、延期、取消 |
| `DecisionMaker` | `decision_maker.h/cpp` | 决策器，判断是否允许申请任务 |
| `TimerManager` | `timer_manager.h/cpp` | 定时器管理，控制任务超时 |
| `Watchdog` | `watchdog.h/cpp` | 看门狗，监控任务执行状态 |

**核心流程**：
- 申请流程：验证调用 → 决策器判断 → 配额检查 → 设置定时器 → 返回结果
- 过期流程：定时器触发 → 提前5秒回调通知 → 自动取消任务 → 通知订阅者

## 4. 长时任务服务

**目录**: `continuous_task/` → [详细开发指南](continuous_task/AGENTS.md)

| 核心类 | 文件 | 功能描述 |
|--------|------|----------|
| `BgContinuousTaskMgr` | `bg_continuous_task_mgr.h/cpp` | 长时任务管理主类 |
| `ContinuousTaskRecord` | `continuous_task_record.h/cpp` | 任务记录，存储任务详情 |
| `NotificationTools` | `notification_tools.h/cpp` | 通知工具，管理后台任务通知 |
| `TaskNotificationSubscriber` | `task_notification_subscriber.h/cpp` | 通知订阅者 |

**核心流程**：
- 开始流程：权限检查 → 模式验证 → 创建记录 → 发送通知 → 通知订阅者 → 持久化
- 停止流程：查找记录 → 取消通知 → 设置原因 → 通知订阅者 → 删除记录

## 5. 能效资源服务

**目录**: `efficiency_resources/` → [详细开发指南](efficiency_resources/AGENTS.md)

| 核心类 | 文件 | 功能描述 |
|--------|------|----------|
| `BgEfficiencyResourcesMgr` | `bg_efficiency_resources_mgr.h/cpp` | 能效资源管理主类 |
| `ResourceApplicationRecord` | `resource_application_record.h/cpp` | 资源申请记录 |
| `ResourcesSubscriberMgr` | `resources_subscriber_mgr.h/cpp` | 资源订阅管理 |

**核心流程**：
- 申请流程：验证调用 → CPU配额检查 → 创建/更新记录 → 通知订阅者 → 持久化
- 重置流程：清理记录 → 通知订阅者 → 更新配额 → 持久化

## 6. 核心服务

**目录**: `core/`

| 类名 | 文件 | 功能描述 |
|------|------|----------|
| `BackgroundTaskMgrService` | `background_task_mgr_service.h/cpp` | 系统服务入口，继承 SystemAbility |

## 7. 公共模块

**目录**: `common/`

| 类名 | 文件 | 功能描述 |
|------|------|----------|
| `AppStateObserver` | `app_state_observer.h/cpp` | 应用状态观察者 |
| `SystemEventObserver` | `system_event_observer.h/cpp` | 系统事件观察者 |
| `BundleManagerHelper` | `bundle_manager_helper.h/cpp` | Bundle 信息查询 Helper |
| `AppMgrHelper` | `app_mgr_helper.h/cpp` | 应用管理 Helper |
| `BgtaskConfig` | `bgtask_config.h/cpp` | 配置管理 |
| `DataStorageHelper` | `data_storage_helper.h/cpp` | 数据存储 Helper |
| `CommonUtils` | `common_utils.h` | 通用工具函数 |
| `TimeProvider` | `time_provider.h/cpp` | 时间管理 |
| `ReportHiSysEventData` | `report_hisysevent_data.h/cpp` | HiSysEvent 上报 |
| `BgtaskHiTraceChain` | `bgtask_hitrace_chain.h/cpp` | HiTrace 链路追踪 |
