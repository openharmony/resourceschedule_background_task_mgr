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

**主要方法**：
- 短时任务：`RequestSuspendDelay`、`CancelSuspendDelay`、`GetRemainingDelayTime`
- 长时任务：`RequestStartBackgroundRunning`、`RequestStopBackgroundRunning`
- 能效资源：`ApplyEfficiencyResources`、`ResetAllEfficiencyResources`
- 订阅管理：`SubscribeBackgroundTask`、`UnsubscribeBackgroundTask`

## 4. 日志规范

使用 `bgtaskmgr_log_wrapper.h` 定义的宏：

```cpp
#include "bgtaskmgr_log_wrapper.h"

BGTASK_LOGD("Debug message: %{public}d", value);  // 调试日志
BGTASK_LOGI("Info message: %{public}s", str);      // 信息日志
BGTASK_LOGW("Warning message");                     // 警告日志
BGTASK_LOGE("Error message: %{public}d", err);     // 错误日志
BGTASK_LOGF("Fatal message");                       // 致命错误日志
```

**格式化说明**：
- `%{public}` 表示公开字段，可完整输出
- 敏感数据使用 `%{private}` 隐藏

## 5. 错误处理

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