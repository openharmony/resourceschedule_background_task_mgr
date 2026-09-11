# 短时任务守卫线程规格

> 文档版本：v1.0
> 更新时间：2026-09-11

## 实体概念

### 定义

短时任务守卫线程（Transient Task Guard）是短时任务超时处理的第三级兜底机制。它独立于系统事件循环，通过 ffrt 延迟任务周期性扫描全部活跃短时任务，对已超时但未被前两级机制（定时器、看门狗）清理的任务执行强制取消，防止应用长期占用后台配额。

### 定位与背景

短时任务超时处理原有两级机制均依赖系统事件循环（`EventHandler`）：

| 级别 | 机制 | 依赖 | 行为 |
|------|------|------|------|
| 第一级 | 定时器（`TimerManager`） | `EventHandler` | 投递定时器，到期前提前回调通知应用 |
| 第二级 | 看门狗（`Watchdog`） | `EventHandler` | 宽限 `WATCHDOG_DELAY_TIME`（6 秒）后 `ForceCancelSuspendDelay` |
| 第三级 | 守卫线程（`TransientTaskGuard`） | ffrt 延迟任务 | 每 60 分钟扫描，对超时任务强制取消 |

当事件循环阻塞或异常时，前两级机制可能同时失效。守卫线程基于 ffrt 延迟任务，不依赖 `EventHandler`，因此在事件循环故障时仍能兜底清理超时任务。

### 核心概念

| 概念 | 说明 |
|------|------|
| 守卫线程（Guard） | `TransientTaskGuard` 实例，管理周期性扫描的延迟任务链 |
| 扫描周期 | 固定 60 分钟（`GUARD_INTERVAL_US`），每轮扫描全部活跃短时任务 |
| 运行标志（`running_`） | `atomic<bool>`，控制守卫线程启停，延迟任务到期后检查此标志决定是否继续 |
| 任务链 | `Start` 发起的首轮延迟任务到期后递归提交下一轮，形成持续运行的任务链 |
| 双重检查 | 执行 `CheckAndCancelOvertimeTasks` 前后各检查一次 `running_`，覆盖扫描期间被 `Stop` 的时序窗口 |

## 规则与约束

### 扫描触发规则

- 守卫线程必须在 `BgTransientTaskMgr` 初始化就绪（`isReady_=true` 且 `SetReady` 完成）后启动
- 每轮扫描间隔固定为 60 分钟（`GUARD_INTERVAL_US = 60LL * 60 * USEC_PER_SEC`），不可配置
- 首轮扫描在 `Start()` 后延迟 60 分钟执行，不立即扫描
- 扫描入口为 `BgTransientTaskMgr::CheckAndCancelOvertimeTasks()`

### 超时判定规则

- 对每个活跃短时任务调用 `DecisionMaker::GetRemainingDelayTime(key, requestId)` 获取剩余时间
- 剩余时间 `<= 0` 的任务判定为超时，必须强制取消
- 剩余时间 `> 0` 的任务跳过，不处理

### 强制取消规则

- 超时任务在强制取消前必须通过 `HiSysEventWrite` 上报 `BGTASK_ERR` 打点
- 强制取消调用 `ForceCancelSuspendDelay(requestId)`，该方法内部加锁清理 `keyInfoMap_` 与回调记录
- 扫描时必须先快照 `keyInfoMap_` 后释放锁，禁止持锁调用 `ForceCancelSuspendDelay`（该方法内部也会加 `expiredCallbackLock_`，持锁调用会死锁）

### 启停规则

| 操作 | 行为 | 幂等性 |
|------|------|--------|
| `Start()` | `running_.exchange(true)`；旧值为 `true` 则告警返回，旧值为 `false` 则提交首轮延迟任务 | 重复 `Start` 不产生多条任务链 |
| `Stop()` | `running_.store(false)`；已排队的延迟任务到期后通过 `IsExpired()` 检查退出 | 重复 `Stop` 无副作用 |
| 析构 | `~TransientTaskGuard()` 调用 `Stop()` | 幂等，与显式 `Stop` 可共存 |

### 任务链退出规则

延迟任务到期后，以下任一条件满足时必须退出不重调度：

| 退出条件 | 判定方式 | 场景 |
|---------|---------|------|
| 守卫已析构 | `weakSelf.lock()` 返回 `nullptr` | `BgTransientTaskMgr` 析构后 `shared_ptr` 释放 |
| 已停止 | `IsExpired()` 返回 `true`（`!running_.load()`） | `Stop()` 调用后 |
| 管理器为空 | `DelayedSingleton<BgTransientTaskMgr>::GetInstance()` 返回 `nullptr` | 单例未初始化或已销毁 |

所有退出分支必须输出 `BGTASK_LOGE` 日志，携带 `running` 状态供问题定位。

### 双重检查规则

- 第一次检查：`CheckAndCancelOvertimeTasks()` 调用前检查 `IsExpired()`，已停止则不执行扫描
- 第二次检查：`CheckAndCancelOvertimeTasks()` 调用后检查 `IsExpired()`，扫描期间被 `Stop` 则不重调度
- 两次检查均通过后，调用 `ScheduleNext()` 递归提交下一轮延迟任务

## innerAPI 功能

从 `services/transient_task/include/transient_task_guard.h` 的关键方法：

| 方法 | 签名 | 说明 |
|------|------|------|
| `Start` | `void Start()` | 启动守卫线程，`exchange(true)` 保证幂等 |
| `Stop` | `void Stop()` | 停止守卫线程，`store(false)` 使已排队任务到期后退出 |
| `ScheduleNext` | `void ScheduleNext()`（private） | 提交 60 分钟延迟 ffrt 任务，到期后检查存活与运行状态，通过后执行扫描并递归提交下一轮 |
| `IsExpired` | `bool IsExpired() const`（private） | 返回 `!running_.load()`，判断是否应退出 |

`BgTransientTaskMgr` 侧新增方法：

| 方法 | 签名 | 说明 |
|------|------|------|
| `CheckAndCancelOvertimeTasks` | `void CheckAndCancelOvertimeTasks()` | 守卫线程周期调用的扫描入口，快照 `keyInfoMap_` 后逐项检查超时并强制取消 |

## 生命周期约束

### 生命周期阶段

| 阶段 | 触发位置 | 行为 |
|------|---------|------|
| 创建+启动 | `BgTransientTaskMgr::InitNecessaryState` | `make_shared<TransientTaskGuard>()` + `Start()`，在 `isReady_=true` 和 `SetReady` 之后 |
| 停止 | `BgTransientTaskMgr::~BgTransientTaskMgr` | 显式 `taskGuard_->Stop()`，`shared_ptr` 随析构自动释放 |
| 析构 | `TransientTaskGuard::~TransientTaskGuard` | 调用 `Stop()`，幂等 |
| 延迟任务释放 | ffrt 延迟任务到期 | `weakSelf.lock()` 返回 `nullptr` 时直接 return；不持有对象引用，对象可正常析构 |

### 引用关系约束

- `BgTransientTaskMgr` 持有 `shared_ptr<TransientTaskGuard>`（强引用，控制生命周期）
- `TransientTaskGuard` 不持有 `BgTransientTaskMgr` 的引用，通过 `DelayedSingleton<BgTransientTaskMgr>::GetInstance()` 获取，不构成循环引用
- ffrt 延迟任务以 `weak_ptr<TransientTaskGuard>` 捕获 guard 自身，不阻止对象析构
- 引用链无环：`BgTransientTaskMgr →(strong)→ TransientTaskGuard`，guard 反向通过单例获取管理器

### ffrt 任务约束

- 延迟任务通过 `ffrt::submit(func, {}, {}, task_attr().delay(GUARD_INTERVAL_US))` 提交
- 第 2/3 参数为空依赖向量，表示独立任务不参与数据流调度
- 延迟任务 lambda 以 `weak_ptr` 捕获 guard 自身，不强引用守卫对象
- `Stop()` 后已排队任务无法取消，但到期后通过 `IsExpired()` 安全退出

## DFX 设计

### 可靠性设计

- 守卫线程独立于系统事件循环，通过 ffrt 延迟任务调度，在 `EventHandler` 阻塞或异常时仍能兜底清理超时任务
- `running_` 使用 `atomic<bool>` 保证 `Start`/`Stop` 的线程安全与幂等性
- `Start()` 使用 `exchange(true)` 原子操作保证不会产生多条并发任务链
- 扫描时先快照 `keyInfoMap_` 到局部向量后释放锁，避免持锁调用 `ForceCancelSuspendDelay` 导致死锁
- 双重检查机制覆盖扫描期间被 `Stop` 的时序窗口，确保 `Stop` 后不再提交下一轮任务
- `weak_ptr` 捕获确保 guard 对象可随 `BgTransientTaskMgr` 析构正常释放，避免对象永生
- 延迟任务到期时若 guard 已析构（`weakSelf.lock()` 返回 `nullptr`），直接安全退出

### 可维护设计

日志：统一使用 `BGTASK_LOG*` 宏（映射 HILOG），短时任务模块专用 LOG_TAG 为 `TRANSIENT_TASK`。所有退出分支均输出 `BGTASK_LOGE` 日志，携带 `running` 状态供问题定位。正常扫描流程输出 `BGTASK_LOGI` 标记开始与结束。

HiSysevent 事件埋点：

| 上报场景 | 事件名 | 域 | 级别 | 上报字段 |
|---------|--------|-----|------|---------|
| 守卫线程兜底强制取消超时任务 | `BGTASK_ERR` | BACKGROUND_TASK | STATISTIC/CRITICAL | APP_UID、APP_PID、APP_NAME、UIABILITY_IDENTITY、MODULE_NAME、FUNC_NAME、ERR_CODE、ERR_MSG |

打点字段说明：

| 字段 | 取值 | 说明 |
|------|------|------|
| APP_UID | `task.second->GetUid()` | 超时任务所属应用 UID |
| APP_PID | `task.second->GetPid()` | 超时任务所属进程 PID |
| APP_NAME | `task.second->GetPkg()` | 超时任务所属包名 |
| UIABILITY_IDENTITY | `-1` | 短时任务无 Ability 维度，固定填充 |
| MODULE_NAME | `BgTransientTaskMgr` | 触发模块 |
| FUNC_NAME | `CheckAndCancelOvertimeTasks` | 触发函数 |
| ERR_CODE | `remainTime` | 剩余时间（`<= 0` 的负值或零） |
| ERR_MSG | `Transient task overtime, force cancelled by guard` | 固定描述 |

### 可测试性设计

| 测试用例 | 验证内容 |
|---------|---------|
| `TransientTaskGuard_001` | Start/Stop 基本生命周期，`running_` 状态切换 |
| `TransientTaskGuard_002` | 重复 Start 幂等，不产生多链 |
| `CheckAndCancelOvertimeTasks_001` | 空 `keyInfoMap` 下不崩溃 |
| `CheckAndCancelOvertimeTasks_002` | 未超时任务（`delayTime=MSEC_PER_MIN`）不被取消 |
| `CheckAndCancelOvertimeTasks_003` | 已超时任务（`delayTime=0`）被强制取消，`keyInfoMap` 清空 |

## 数据模型

以下数据结构体定义详见代码设计文档 `docs/design/transient_task_guard.md` 的"核心类"章节。

- **TransientTaskGuard**（`services/transient_task/include/transient_task_guard.h`）：守卫线程类，继承 `std::enable_shared_from_this<TransientTaskGuard>`，持有 `running_` 运行标志，提供 `Start`/`Stop`/`ScheduleNext`/`IsExpired` 方法。
- **KeyInfo**（`services/transient_task/include/key_info.h`）：任务键信息（内部使用），以包名、UID、PID 唯一标识一个短时任务申请方，守卫线程扫描时从 `keyInfoMap_` 快照获取。

## 关键参数

| 参数 | 所在位置 | 类型 | 默认值 | 可配 | 说明 |
|------|---------|------|--------|------|------|
| `GUARD_INTERVAL_US` | `transient_task_guard.cpp:28` | `constexpr int64_t` | `60LL * 60 * USEC_PER_SEC`（60 分钟） | 否 | 守卫线程扫描间隔 |
| `running_` | `TransientTaskGuard` | `atomic<bool>` | `false` | — | 运行标志，`Start` 时 `exchange(true)`，`Stop` 时 `store(false)` |
| `taskGuard_` | `BgTransientTaskMgr` | `shared_ptr<TransientTaskGuard>` | `nullptr` | — | 守卫线程实例，`InitNecessaryState` 创建，析构时 Stop |
| `USEC_PER_SEC` | `time_provider.h` | `int64_t` | `1000000LL` | 否 | 微秒/秒常量，用于 `GUARD_INTERVAL_US` 计算 |
