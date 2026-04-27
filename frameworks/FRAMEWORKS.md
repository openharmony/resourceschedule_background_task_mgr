[← 返回上级目录](../AGENTS.md)

# 框架层开发指南

## 1. 功能概述

框架层作为客户端代理，封装所有后台任务操作，通过 IPC 调用服务端接口。主要职责：
- 提供统一的 `BackgroundTaskManager` 接口供上层调用
- 管理服务代理的生命周期和死亡监听
- 封装短时任务、长时任务、能效资源三类操作

## 2. 目录结构

```
frameworks/
├── common/              # 公共模块
│   ├── include/        # 日志宏、错误码、IPC 工具
│   └── src/            # 错误码映射实现
├── include/            # 框架层头文件
│   └── background_task_manager.h
├── src/                # 框架层实现
│   └── background_task_manager.cpp
└── test/               # 单元测试
    └── unittest/       # 框架层单元测试
```

## 3. 核心类

### BackgroundTaskManager

位置：`include/background_task_manager.h`

**职责**：客户端代理管理器，封装所有后台任务 IPC 调用

**关键成员**：
| 成员 | 类型 | 说明 |
|------|------|------|
| `proxy_` | `sptr<IBackgroundTaskMgr>` | 服务端代理 |
| `mutex_` | `std::mutex` | 线程安全锁 |
| `recipient_` | `sptr<BgTaskMgrDeathRecipient>` | 死亡监听 |

## 4. 主要方法

### 4.1 短时任务 API

| 方法 | 作用描述 |
|------|----------|
| `RequestSuspendDelay` | 申请短时任务，获取 requestId 和延迟时间，应用可在后台短暂运行 |
| `CancelSuspendDelay` | 取消短时任务，释放后台运行资源 |
| `GetRemainingDelayTime` | 获取剩余延迟时间，应用可据此判断是否需要续期 |
| `GetAllTransientTasks` | 获取系统所有短时任务列表（内部接口） |

### 4.2 长时任务 API

| 方法 | 作用描述 |
|------|----------|
| `StartBackgroundRunning` | 开始长时任务，申请指定长时任务类型，系统发布通知提示用户 |
| `StopBackgroundRunning` | 停止长时任务，取消后台运行和通知 |
| `UpdateBackgroundRunning` | 更新长时任务模式，动态调整后台长时任务类型 |
| `GetAllContinuousTasks` | 查询已申请的长时任务 |
| `OnOnContinuousTaskCallback` | 注册长时任务回调，监听取消/暂停/激活事件 |
| `OffOnContinuousTaskCallback` | 取消注册长时任务回调 |

### 4.3 能效资源 API

| 方法 | 作用描述 |
|------|----------|
| `ApplyEfficiencyResources` | 申请能效资源（如 CPU、GPS、音频等），优化后台运行效率 |
| `ResetAllEfficiencyResources` | 重置所有已申请的能效资源 |
| `GetEfficiencyResourcesInfos` | 获取应用级和进程级的能效资源申请信息 |

### 4.4 订阅管理 API

| 方法 | 作用描述 |
|------|----------|
| `SubscribeBackgroundTask` | 订阅后台任务状态变更事件 |
| `UnsubscribeBackgroundTask` | 取消订阅后台任务状态变更事件 |

## 5. 日志规范

使用 `bgtaskmgr_log_wrapper.h` 定义的宏：

```cpp
#include "bgtaskmgr_log_wrapper.h"

BGTASK_LOGD("Debug message: %{public}d", value);  // 调试日志
BGTASK_LOGI("Info message: %{public}s", str);      // 信息日志
BGTASK_LOGW("Warning message");                     // 警告日志
BGTASK_LOGE("Error message: %{public}d", err);     // 错误日志
BGTASK_LOGF("Fatal message");                       // 致命错误日志
```

## 6. 错误处理

使用 `bgtaskmgr_inner_errors.h` 中定义的错误码：

```cpp
#include "bgtaskmgr_inner_errors.h"

// 常用错误码
ERR_BGTASK_PERMISSION_DENIED       // 201 权限拒绝
ERR_BGTASK_NOT_SYSTEM_APP          // 202 非系统应用
ERR_BGTASK_INVALID_PARAM            // 401 参数无效
ERR_BGTASK_SERVICE_NOT_CONNECTED    // 服务未连接
ERR_BGTASK_NO_MEMORY                // 内存不足
ERR_BGTASK_PARCELABLE_FAILED        // 序列化失败

// 获取错误描述
std::string msg = BusinessErrorMap::GetSaErrMsg(errCode);
```