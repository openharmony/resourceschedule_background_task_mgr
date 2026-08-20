# 短时任务规格

> 文档版本：v1.0
> 更新时间：2026-08-19

短时任务的定义、核心概念（延迟挂起、配额、超时、requestId）、功能边界及与长时任务的区分详见 [docs/knowledge/glossary.md](../knowledge/glossary.md) 和 [docs/knowledge/business_context.md](../knowledge/business_context.md)。

## 接口说明

| 接口名 | 接口描述 |
|--------|---------|
| `requestSuspendDelay(reason: string, callback: Callback[void]): DelaySuspendInfo` | 申请延迟挂起 |
| `cancelSuspendDelay(requestId: number): void` | 取消延迟挂起 |
| `getRemainingDelayTime(requestId: number, callback: AsyncCallback[number]): void` | 获取延迟挂起剩余时间（callback 形式） |
| `getRemainingDelayTime(requestId: number): Promise[number]` | 获取延迟挂起剩余时间（Promise 形式） |

## 规则与约束

### 配额限制

- 单次短时任务必须不超过 3 分钟
- 每日配额必须不超过 10 分钟（系统根据应用场景和系统状态动态调整）
- 配额消耗完后，禁止再申请短时任务

### 时机约束

- 应用必须在前台时或退后台被挂起之前申请延迟挂起
- 应用退到后台默认有一段时间的运行时长窗口，具体由系统决定
- 超时后不取消的，会被系统强制取消

### 取消规则

- 任务完成后应用应当主动取消延迟挂起
- 禁止等待系统回调后再取消，否则会影响应用的后台允许运行时长配额
- 超时后系统自动取消

## 数据模型

以下数据结构体的具体字段定义详见代码设计文档 `docs/design/transient_task.md` 的"数据模型"章节。

- **DelaySuspendInfo**（`interfaces/innerkits/include/delay_suspend_info.h`）：短时任务申请返回信息，包含请求 ID 和实际延迟时间。
- **TransientTaskAppInfo**（`interfaces/innerkits/include/transient_task_app_info.h`）：短时任务应用信息，包含包名、用户 ID 和进程 ID。
- **KeyInfo**（`services/transient_task/include/key_info.h`）：任务键信息（内部使用），以包名、UID、PID 唯一标识一个短时任务申请方。
- **PkgDelaySuspendInfo**（`services/transient_task/include/pkg_delay_suspend_info.h`）：包延迟挂起信息（内部使用），管理配额、计时与请求列表。

## innerAPI 功能

从 `services/transient_task/include/bg_transient_task_mgr.h` 的关键方法表抽取的内部方法签名：

| 方法 | 签名 | 说明 |
|------|------|------|
| `RequestSuspendDelay` | `ErrCode RequestSuspendDelay(const std::u16string& reason, const sptr<IExpiredCallback>& callback, shared_ptr[DelaySuspendInfo] &delayInfo)` | 申请短时任务 |
| `CancelSuspendDelay` | `ErrCode CancelSuspendDelay(int32_t requestId)` | 取消短时任务 |
| `GetRemainingDelayTime` | `ErrCode GetRemainingDelayTime(int32_t requestId, int32_t &delayTime)` | 获取剩余时间 |
| `GetAllTransientTasks` | `ErrCode GetAllTransientTasks(int32_t &remainingQuota, vector[DelaySuspendInfo] &list)` | 获取所有任务 |
| `SubscribeBackgroundTask` | `ErrCode SubscribeBackgroundTask(const sptr[IBackgroundTaskSubscriber]& subscriber)` | 订阅任务状态 |
| `PauseTransientTaskTimeForInner` | `ErrCode PauseTransientTaskTimeForInner(int32_t uid)` | 暂停计时（内部接口） |
| `StartTransientTaskTimeForInner` | `ErrCode StartTransientTaskTimeForInner(int32_t uid)` | 恢复计时（内部接口） |

## API 版本演进

短时任务 API 的演进与版本（从 `docs/transient_task/design.md` 的"演进与版本"章节抽取）：

- 短时任务 API 自 OpenHarmony 初版即提供，接口稳定
- 新增 `PauseTransientTaskTimeForInner` / `StartTransientTaskTimeForInner` 内部接口支持暂停/恢复计时
- 新增 `OnAppCacheStateChanged` 支持应用缓存状态变更通知
- API20 版本新增支持获取应用自身的所有短时任务信息（`getTransientTaskInfo`）

## DFX 设计

### 可靠性设计

短时任务模块的错误码统一定义于 `frameworks/common/include/bgtaskmgr_inner_errors.h`，采用 Syscap|Code|Subcode 三段式布局。短时任务专属错误码段为 `9900001`~`9900004`，覆盖参数非法（`ERR_BGTASK_INVALID_PID_OR_UID`、`ERR_BGTASK_INVALID_REQUEST_ID`）、回调异常（`ERR_BGTASK_INVALID_CALLBACK`、`ERR_BGTASK_CALLBACK_EXISTS`、`ERR_BGTASK_CALLBACK_NOT_EXIST`）、配额约束（`ERR_BGTASK_EXCEEDS_THRESHOLD`、`ERR_BGTASK_TIME_INSUFFICIENT`、`ERR_BGTASK_NOT_IN_PRESET_TIME`）、序列化与服务就绪（`ERR_BGTASK_TRANSIENT_PARCELABLE_FAILED`、`ERR_BGTASK_TRANSIENT_SYS_NOT_READY`）等场景，调用方可据此精确判定失败原因。

短时任务状态全部维护在内存中，不落盘持久化。服务重启后短时任务状态丢失，应用必须重新申请延迟挂起；模块不保证跨重启的状态延续。

超时处理采用定时器加看门狗两级机制：定时器到期后先触发过期回调，随后启动看门狗宽限期（`WATCHDOG_DELAY_TIME`），宽限期内仍未取消则强制取消，避免应用长期占用后台配额。提前回调机制在到期前一定时间提前通知应用，便于应用主动收尾。

模块通过 `isReady` 原子状态门控所有入口：服务未就绪时必须拒绝请求并返回系统未就绪错误码。依赖的服务（应用管理、公共事件、包管理、资源调度）未就绪时按固定间隔轮询重试，不阻塞初始化。

回调死亡恢复机制保证状态一致性：当过期回调远端死亡时清理对应记录；订阅者死亡时从订阅列表移除。

边界处理覆盖：配额耗尽（剩余配额低于阈值时拒绝申请）、重复申请（回调已存在则拒绝）、重复取消（记录不存在则拒绝）、非法 requestId（归属校验失败则拒绝）、未在准入时间窗（超过后台运行窗口则拒绝）、前台应用（暂停/恢复计时对前台应用拒绝）。

### 安全性设计

Atomic Service（元服务）禁止申请短时任务，申请入口通过 Token 类型判断拦截，返回权限拒绝错误码。

内部接口（`PauseTransientTaskTimeForInner`、`StartTransientTaskTimeForInner`、`SetBgTaskConfig`、`GetTransientTaskApps`）实施严格的调用者限制：暂停/恢复计时与配置下发仅允许 `resource_schedule_service`、`hidumper_service` 等指定系统进程调用，非白名单进程返回非法进程名错误码；查询全部任务与配置下发仅允许 `TOKEN_NATIVE`/`TOKEN_SHELL` 类型调用方。

Dump 诊断入口需同时满足 ENG 模式（`const.debuggable=1`）与 `ohos.permission.DUMP` 权限，防止非调试设备泄露运行态信息。

### 可扩展设计

短时任务通过 `IBackgroundTaskSubscriber` 订阅者机制对外暴露任务状态变更事件，包括任务开始、任务结束、应用级任务开始/结束与任务错误，外部系统可订阅以感知短时任务运行态。

本模块不提供插件化扩展机制（无动态库加载），不涉及多用户维度（以 uid 为隔离粒度，无 userId 分域）。配额与时长参数可通过系统属性与云配置运行时调整，无需改代码。

### 可配置设计

可通过系统属性（`persist.*`）运行时调整的参数：

| 参数名　　　　　　　　　　　　　　　　| 作用　　　　　　　　 | 默认值　|
| ---------------------------------------| ----------------------| ---------|
| `persist.sys.bgtask_delaytime_normal` | 正常模式单次延迟时长 | 3 分钟　|
| `persist.sys.bgtask_init_quota`　　　 | 每日初始配额　　　　 | 10 分钟 |
| `persist.sys.bgtask_quota_update`　　 | 配额刷新周期　　　　 | 1 天　　|

硬编码阈值（不可配）：单应用最大并发请求数 `MAX_REQUEST_ID=3`、配额下限 `MIN_ALLOW_QUOTA_TIME`、看门狗宽限 `WATCHDOG_DELAY_TIME`、低电量模式延迟时长等。

### 可维护设计

日志：统一使用 `BGTASK_LOGD`/`BGTASK_LOGI`/`BGTASK_LOGW`/`BGTASK_LOGE`/`BGTASK_LOGF` 宏（映射 HILOG），短时任务模块专用 LOG_TAG 为 `TRANSIENT_TASK`，日志自动附加函数与行号定位信息，含隐私格式化标注。

HiSysevent 事件埋点：`TRANSIENT_TASK_APPLY`（申请，携带 UID/PID/包名/任务ID/延迟时长）与 `TRANSIENT_TASK_CANCEL`（取消，携带 UID/PID/包名/任务ID），归 `STATISTIC/MINOR/PowerStats`，用于功耗统计。

Dump 诊断：经 `hidumper` 调用，`-T` 路由到短时任务，支持子命令 `All`（列出全部请求与配额）、`BATTARY_LOW`/`BATTARY_OKAY`（模拟低电量/电量恢复）、`PAUSE`/`START` 加 uid（暂停/恢复计时）、`DUMP_CANCEL`（退出 dump 模式）。

HiTrace 链路追踪在关键入口标注，便于跨进程调用链分析。

### 兼容性设计

C API 版本演进：`OH_BackgroundTaskManager_RequestSuspendDelay`/`GetRemainingDelayTime`/`CancelSuspendDelay` 自 `@since 13` 提供；`OH_BackgroundTaskManager_GetTransientTaskInfo` 与 `TRANSIENT_TASK_MAX_NUM` 自 `@since 20` 新增。错误码部分自 `@since 13`、部分自 `@since 20` 演进。

NAPI 双入口兼容：提供不抛异常（返回 null）与抛异常（抛 JS 异常）两个入口，兼容老版本不抛异常行为与新版本规范异常行为。
