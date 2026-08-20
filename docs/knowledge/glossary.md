# 业务术语表

> 文档版本：v1.0
> 更新时间：2026-08-19

## 通用术语

**当前组件**：后台任务管理/Background Task Manager，OpenHarmony 资源调度子系统的核心组件，负责管理后台运行应用的任务生命周期，为应用在后台运行提供免冻结能力。组件位于 `foundation/resourceschedule/background_task_mgr` 目录下。

| 特性/功能中文名 | 特性/功能英文名　　　| 特性描述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　|
| -----------------| ----------------------| -------------------------------------------------------------------------------------------------------------|
| 短时任务　　　　| Transient Task　　　 | 临时性后台运行保障机制，允许应用在后台短时间运行，单次最长 3 分钟，每日配额默认 10 分钟，超时后系统强制取消 |
| 长时任务　　　　| Continuous Task　　　| 为用户可感知且需要持续在后台运行的业务提供长期后台运行保障，通过通知栏强制提示用户　　　　　　　　　　　　　|
| 能效资源　　　　| Efficiency Resources | 资源特权申请机制，申请 CPU/软件/硬件资源，获得在挂起状态下不被代理或不被挂起的特权　　　　　　　　　　　　　|

## 短时任务术语

| 中文名　　　 | 英文名　　　　| 描述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　 |
| --------------| ---------------| ------------------------------------------------------------------------------------|
| 延迟挂起　　 | Delay Suspend | 应用申请短时任务后，系统延迟对该应用的挂起操作　　　　　　　　　　　　　　　　　　 |
| 短时任务配额 | Quota　　　　 | 每个应用每日可使用的短时任务时长，默认 10 分钟，系统根据应用场景和系统状态智能调整 |
| 短时任务超时 | Timeout　　　 | 单次短时任务的时间上限，默认 3 分钟　　　　　　　　　　　　　　　　　　　　　　　　|
| 短时任务ID　 | requestId　　 | 每次申请短时任务分配的唯一标识，用于取消和查询　　　　　　　　　　　　　　　　　　 |

## 长时任务术语

| 中文名　　　　　　| 英文名　　　　　　　　　　　| 描述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　|
| -------------------| -----------------------------| -----------------------------------------------------------------------------------------------------------------------------------|
| 长时任务类型　　　| BackgroundTaskMode　　　　　| API21 版本新增的新类型枚举，定义于 `interfaces/innerkits/include/background_task_mode.h`，替代旧 `BackgroundMode`　　　　　　　　 |
| 长时任务子类型　　| BackgroundTaskSubmode　　　 | API21 版本新增的子模式，定义于 `interfaces/innerkits/include/background_task_submode.h`，支持更细粒度的模式声明，影响通知展现形态 |
| 组合通知/通知合并 | isCombinedTaskNotification_ | API21 版本新增，允许相同类型的多个长时任务合并为一条主通知 + 子通知　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　 |
| 长时任务暂停　　　| Continuous Task Suspend　　 | 系统检测到资源未使用或非法使用时，暂停长时任务的运行，通过 `OnContinuousTaskSuspend` 回调通知　　　　　　　　　　　　　　　　　　 |
| 长时任务激活　　　| Continuous Task Active　　　| 暂停条件消除后，任务恢复运行，通过 `OnContinuousTaskActive` 回调通知　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　|

## 能效资源术语

| 中文名　　　 | 英文名　　　　　　　　　　　| 描述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　 |
| --------------| -----------------------------| --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 能效资源类型 | ResourceType　　　　　　　　| 位掩码枚举，定义于 `interfaces/innerkits/include/resource_type.h`，使用位掩码标识 CPU、COMMON_EVENT、TIMER、WORK_SCHEDULER、BLUETOOTH、GPS、AUDIO、RUNNING_LOCK、SENSOR 等资源 |
| 持久资源　　 | isPersist　　　　　　　　　 | 申请后持续生效，不因超时释放。同时申请同一类持久资源和非持久资源时，持久资源覆盖非持久资源，超时不释放　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　 |
| 非持久资源　 | -　　　　　　　　　　　　　 | 申请后在超时时间后自动释放，由 `ResetTimeOutResource` 定时清理　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　 |
| 应用粒度申请 | -　　　　　　　　　　　　　 | 以应用 UID 为维度申请资源，对 UID 下所有进程生效　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　 |
| 进程粒度申请 | isProcess_　　　　　　　　　| 以进程 PID 为维度申请资源（`isProcess_ = true`），仅对申请的进程生效　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　 |
| CPU 级别　　 | EfficiencyResourcesCpuLevel | CPU 资源分级申请机制，取值 DEFAULT(-1)/SMALL_CPU(0)/MEDIUM_CPU(1)/LARGE_CPU(2)，定义于 `interfaces/innerkits/include/efficiency_resources_cpu_level.h`　　　　　　　　　　　　 |
