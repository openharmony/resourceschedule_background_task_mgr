# 业务背景

> 文档版本：v1.0
> 更新时间：2026-08-19
> 本文档介绍后台任务管理组件的业务定位、核心功能、子系统归属、组件信息及与相近子系统的关系。

## 组件定位

后台任务管理（Background Task Manager）是 OpenHarmony 资源调度子系统的核心组件，负责管理后台运行应用的任务生命周期，为应用在后台运行提供免冻结能力。组件位于 `foundation/resourceschedule/background_task_mgr` 目录下。

## 三大核心功能

| 功能 | 说明 | 服务入口类 |
|------|------|------------|
| 短时任务（Transient Task） | 允许应用在后台短时间运行，单次最长 3 分钟，每日配额 10 分钟 | `BgTransientTaskMgr` |
| 长时任务（Continuous Task） | 为用户可感知的后台业务提供持续运行保障，通过通知栏提示用户 | `BgContinuousTaskMgr` |
| 能效资源（Efficiency Resources） | 申请 CPU/软件/硬件资源特权，避免挂起或资源代理 | `BgEfficiencyResourcesMgr` |

## 子系统归属

`resourceschedule` → `background_task_mgr`

## 组件信息

| 项目　　 | 值　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　 |
| ----------| --------------------------------------------------------------------------------|
| 组件名　 | `background_task_mgr`　　　　　　　　　　　　　　　　　　　　　　　　　　　　　|
| 子系统　 | `resourceschedule`　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　 |
| 构建系统 | GN + Ninja　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　 |
| 构建配置 | `BUILD.gn`（根目录）、`bgtaskmgr.gni`（路径定义）、`bundle.json`（组件元数据） |

## 与相近子系统的关系

后台任务管理组件作为资源调度子系统的核心组件，在运行时与多个子系统和系统服务协作完成后台任务的生命周期管理。

各协作关系说明：

- **资源调度子系统**：后台任务管理组件归属资源调度子系统，与子系统中其他组件（如资源调度服务等）共同协作完成应用资源的调度与管控。组件通过 `services/plugin/` 接收资源调度服务框架发送的reportdate事件，并向其他调度组件提供后台任务状态信息。
- **通知子系统（AnsService）**：长时任务申请后系统强制弹出通知提醒用户，`BgContinuousTaskMgr` 通过 `NotificationTools` 发送/取消通知，并通过 `TaskNotificationSubscriber` 监听通知操作（删除/点击）触发任务取消。支持标准通知、子通知、主通知、横幅通知、实况通知等多种通知类型。
- **BundleMgr（Bundle 管理服务）**：长时任务和能效资源均通过 `BundleManagerHelper` 查询包信息、验证后台模式声明、判断系统应用身份。
- **AppMgr（应用管理服务）**：短时任务通过 `AppMgrHelper` 查询应用前后台状态和进程信息；能效资源通过 `AppMgrHelper` 查询运行进程列表、校验应用存活状态。`AppStateObserver` 监听应用前后台切换、进程创建/死亡、Ability 状态变化，应用死亡时通知三个子模块清理各自记录。
- **AccountMgr（账户管理服务）**：长时任务通过 `SystemEventObserver` 监听账户状态变更。
- **AVSession 服务**：音频播放任务关联 AVSession，未使用 AVSession 的音频播放会被系统取消。
- **资源配额管理库**：能效资源通过 `dlopen` 动态加载配额管理库（`resourceQuotaMgrHandle_`），进行 CPU 配额的检查与更新。
