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

**核心流程**：
- 申请流程：验证调用 → 决策器判断 → 配额检查 → 设置定时器 → 返回结果
- 过期流程：定时器触发 → 提前5秒回调通知 → 自动取消任务 → 通知订阅者

## 4. 长时任务服务

**目录**: `continuous_task/` → [详细开发指南](continuous_task/AGENTS.md)

**核心流程**：
- 开始流程：权限检查 → 模式验证 → 创建记录 → 发送通知 → 通知订阅者 → 持久化
- 停止流程：查找记录 → 取消通知 → 设置原因 → 通知订阅者 → 删除记录

## 5. 能效资源服务

**目录**: `efficiency_resources/` → [详细开发指南](efficiency_resources/AGENTS.md)

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

提供后台任务管理服务的公共基础设施和辅助工具：

| 类名 | 功能描述 |
|------|----------|
| `AppStateObserver` | 应用状态观察者，监听应用前后台切换、进程创建/死亡、Ability状态变化等事件 |
| `SystemEventObserver` | 系统事件观察者，监听用户状态变更、Bundle信息变更、系统公共事件等 |
| `BundleManagerHelper` | Bundle信息查询助手，获取应用包信息、检查权限声明、判断系统应用身份 |
| `AppMgrHelper` | 应用管理代理，获取运行进程列表、订阅应用状态观察者、查询Ability状态 |
| `BgtaskConfig` | 配置管理器，解析配置文件（豁免应用列表、恶意应用黑名单、配额参数等） |
| `DataStorageHelper` | 数据持久化助手，负责任务记录的JSON序列化存储和设备重启后的恢复 |
| `CommonUtils` | 通用工具函数，提供JSON校验、后台模式检查、字符串处理等公共方法 |
| `TimeProvider` | 时间提供者，提供多种时钟类型的精确时间获取，支持单调时钟和实时时钟 |
| `ReportHiSysEventData` | HiSysEvent上报数据结构，封装任务申请/取消/超时等事件的上报字段 |
| `BgtaskHiTraceChain` | HiTrace链路追踪，为关键操作添加性能追踪点，支持跨进程调用链追踪 |
