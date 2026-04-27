[← 返回上级目录](../AGENTS.md)

# 接口层开发指南

## 1. 功能概述

接口层负责定义和实现后台任务管理组件的 API，分为：
- **内部 API（innerkits）**：C++ 接口，供系统内部模块调用
- **外部 API（kits）**：多语言接口，供应用层调用

## 2. 目录结构

```
interfaces/
├── innerkits/           # 内部 C++ API
│   ├── include/        # 头文件定义
│   ├── src/            # 数据模型实现
│   ├── *.idl           # IPC 接口定义
│   └── BUILD.gn        # 构建配置
├── kits/               # 外部多语言 API
│   ├── napi/           # Node-API（ArkTS/JS）
│   ├── c/              # C API（Native Development Kit）
│   ├── cj/             # CJ FFI（仓颉语言）
│   └── ets/taihe/      # ANI（ArkTS Native Interface）
└── test/               # 接口层测试
    └── unittest/       # 单元测试
```

## 3. IDL 规范

### 3.1 接口文件

| IDL 文件 | 用途 | 说明 |
|---------|------|------|
| `IBackgroundTaskMgr.idl` | 主服务接口 | 定义短时任务、长时任务、能效资源的所有操作 |
| `IBackgroundTaskSubscriber.idl` | 订阅者接口 | 定义任务状态变更回调（OnTransientTaskStart/OnContinuousTaskStop 等） |
| `IExpiredCallback.idl` | 回调接口 | 定义任务过期回调（OnExpired/OnExpiredAuth） |

## 4. 内部 API（innerkits）

### 4.1 关键头文件

| 头文件 | 用途 |
|--------|------|
| `background_task_mgr_helper.h` | API 访问辅助类，简化服务获取 |
| `background_task_subscriber.h` | 订阅者基类 |
| `expired_callback.h` | 超时回调基类 |
| `delay_suspend_info.h` | 短时任务信息 |
| `continuous_task_info.h` | 长时任务信息 |
| `continuous_task_param.h` | 长时任务参数 |
| `efficiency_resource_info.h` | 能效资源信息 |
| `transient_task_app_info.h` | 短时任务应用信息 |
| `background_mode.h` | 长时任务旧类型 |
| `background_task_mode.h` | 长时任务新类型——主类型 |
| `background_task_submode.h` | 长时任务新类型——子类型 |
| `continuous_task_cancel_reason.h` | 长时任务取消原因 |
| `continuous_task_suspend_reason.h` | 长时任务暂停原因 |
| `continuous_task_callback_info.h` | 长时任务回调信息 |
| `resource_type.h` | 资源类型定义 |

## 5. 外部 API（kits）

### 5.1 多语言接口

| 语言 | 目录 | 目标 | 说明 |
|------|------|------|------|
| **NAPI** | `kits/napi/` | `backgroundtaskmanager` | ArkTS/JS API，模块路径 `@ohos.backgroundTaskManager` |
| **NAPI** | `kits/napi/` | `backgroundtaskmanager_napi` | 扩展 NAPI，含订阅者功能 |
| **C API** | `kits/c/` | `transient_task` | Native Development Kit for C API，供原生应用调用 |
| **CJ FFI** | `kits/cj/` | `cj_background_task_mgr_ffi` | 仓颉语言 FFI 绑定 |
| **ANI** | `kits/ets/taihe/` | `background_task_manager_taihe` | ArkTS Native Interface |

### 5.2 NAPI 核心实现

#### 5.2.1 模块初始化

| 源文件 | 功能 |
|--------|------|
| `init.cpp` | 模块入口，注册 `@ohos.backgroundTaskManager` 模块 |
| `init_bgtaskmgr.cpp` | 扩展模块入口，注册 `@ohos.resourceschedule.backgroundTaskManager` 模块 |

#### 5.2.2 短时任务 API

| 源文件 | NAPI 函数 | 功能 |
|--------|-----------|------|
| `request_suspend_delay.cpp` | `RequestSuspendDelay` | 申请短时任务，返回 DelaySuspendInfo |
| `cancel_suspend_delay.cpp` | `CancelSuspendDelay` | 取消短时任务 |
| `get_remaining_delay_time.cpp` | `GetRemainingDelayTime` | 获取短时任务剩余时间（Promise/Callback） |
| `get_all_transient_tasks.cpp` | `GetAllTransientTasks` | 获取所有短时任务 |

#### 5.2.3 长时任务 API

| 源文件 | NAPI 函数 | 功能 |
|--------|-----------|------|
| `bg_continuous_task_napi_module.cpp` | `StartBackgroundRunning` | 申请长时任务 |
| `bg_continuous_task_napi_module.cpp` | `StartBackgroundRunningThrow` | 申请长时任务（带异常抛出） |
| `bg_continuous_task_napi_module.cpp` | `StopBackgroundRunning` | 取消长时任务 |
| `bg_continuous_task_napi_module.cpp` | `StopBackgroundRunningThrow` | 取消长时任务（带异常抛出） |
| `bg_continuous_task_napi_module.cpp` | `UpdateBackgroundRunningThrow` | 更新长时任务（带异常抛出） |
| `bg_continuous_task_napi_module.cpp` | `GetAllContinuousTasksThrow` | 长时任务查询（带异常抛出） |
| `bg_continuous_task_napi_module.cpp` | `ObtainAllContinuousTasks` | 长时任务查询（包含暂停任务） |
| `bg_continuous_task_napi_module.cpp` | `OnOnContinuousTaskCallback` | 注册长时任务回调（监听取消/暂停/激活事件） |
| `bg_continuous_task_napi_module.cpp` | `OffOnContinuousTaskCallback` | 取消注册长时任务回调 |
| `bg_continuous_task_napi_module.cpp` | `SubscribeContinuousTaskState` | 订阅长时任务状态变更 |
| `bg_continuous_task_napi_module.cpp` | `UnSubscribeContinuousTaskState` | 取消订阅长时任务状态 |
| `bg_continuous_task_napi_module.cpp` | `IsModeSupported` | 长时任务模式支持情况查询 |
| `bg_continuous_task_napi_module.cpp` | `CheckSpecialScenarioAuth` | 特殊场景授权检查 |
| `bg_continuous_task_napi_module.cpp` | `SetBackgroundTaskState` | 设置后台任务状态 |
| `bg_continuous_task_napi_module.cpp` | `GetBackgroundTaskState` | 获取后台任务状态 |

#### 5.2.4 能效资源 API

| 源文件 | NAPI 函数 | 功能 |
|--------|-----------|------|
| `efficiency_resources_operation.cpp` | `ApplyEfficiencyResources` | 申请能效资源 |
| `efficiency_resources_operation.cpp` | `ResetAllEfficiencyResources` | 重置所有能效资源 |
| `efficiency_resources_operation.cpp` | `GetEfficiencyResourcesInfos` | 获取能效资源信息 |

#### 5.2.5 订阅者封装

`js_backgroundtask_subscriber.h` 定义 `JsBackgroundTaskSubscriber` 类：

- 继承 `BackgroundTaskSubscriber` 基类
- 实现回调方法：`OnContinuousTaskStart/Update/Stop/Suspend/Active`
- 管理 JS 回调对象：`AddJsObserverObject/RemoveJsObserverObject`
- 系统服务状态监听：`JsBackgroudTaskSystemAbilityStatusChange`

#### 5.2.6 公共工具类

`common.h` 定义 `Common` 工具类，提供：

- 异步工作数据：`AsyncWorkData` 结构
- NAPI 调用宏：`BGTASK_NAPI_CALL`
- 类型转换：`GetNapiContinuousTaskInfo/GetNapiDelaySuspendInfo/GetNapiEfficiencyResourcesInfo`
- 参数解析：`GetContinuousTaskRequest/GetInt32NumberValue/GetStringValue`
- 错误处理：`HandleErrCode/GetCallbackErrorValue`
- Promise/Callback 统一处理：`ReturnCallbackPromise/SetPromise/SetCallback`