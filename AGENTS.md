CODEAGENT.md
This file provides guidance to CodeAgent when working with code in this reposity.

# 后台任务管理组件开发指南

本文档为 OpenHarmony 资源调度子系统后台任务管理组件的开发指南，采用渐进式披露原则组织内容。

## 1. 项目概述

**功能定位**：管理短时任务（Transient Task）、长时任务（Continuous Task）和能效资源（Efficiency Resources），为后台运行应用提供免冻结能力。

**技术栈**：IPC 通信基于 IDL 自动生成的代理/桩代码。

**子系统归属**：`resourceschedule` → `background_task_mgr`

## 2. 目录结构

```
/foundation/resourceschedule/background_task_mgr
├── frameworks/           # 框架层 - 客户端代理实现
│   ├── common/          # 公共工具、日志宏、错误码
│   ├── include/         # 头文件定义
│   ├── src/             # BackgroundTaskManager 实现
│   └── test/            # 框架层单元测试
│   └── [FRAMEWORKS.md](frameworks/FRAMEWORKS.md)  # 框架层开发指南
│
├── interfaces/           # 接口层 - API 定义
│   ├── innerkits/       # 内部 C++ API（IDL 接口、数据模型）
│   ├── kits/            # 外部 API（NAPI/NDK/CJ/ANI）
│   ├── test/            # 接口层测试
│   └── [INTERFACES.md](interfaces/INTERFACES.md)  # 接口层开发指南
│
├── services/             # 服务层 - 服务端实现
│   ├── common/          # 公共服务（Helper、Observer、Config）
│   ├── transient_task/  # 短时任务服务（延迟挂起管理）
│   ├── continuous_task/ # 长时任务服务（后台运行管理）
│   ├── efficiency_resources/ # 能效资源服务
│   ├── core/            # 核心服务入口（BackgroundTaskMgrService）
│   ├── test/            # 服务层单元测试
│   └── [AGENTS.md](services/AGENTS.md)  # 服务层开发指南
│
├── test/                 # 测试层
│   ├── fuzztest/        # 模糊测试（IPC 安全测试）
│   ├── systemtest/      # 系统测试（Dump 功能测试）
│
├── sa_profile/           # 系统能力配置 → [SA_PROFILE.md](sa_profile/SA_PROFILE.md)
├── resources/            # 国际化资源 → [RESOURCES.md](resources/RESOURCES.md)
└── figures/              # 文档图片
```

## 3. 三大核心功能

| 功能类型 | 服务模块 | 核心类 | 详细文档 |
|---------|---------|--------|---------|
| **短时任务** | `services/transient_task/` | `BgTransientTaskMgr` | [TRANSIENT_TASK.md](services/transient_task/TRANSIENT_TASK.md) |
| **长时任务** | `services/continuous_task/` | `BgContinuousTaskMgr` | [CONTINUOUS_TASK.md](services/continuous_task/CONTINUOUS_TASK.md) |
| **能效资源** | `services/efficiency_resources/` | `BgEfficiencyResourcesMgr` | [EFFICIENCY_RESOURCES.md](services/efficiency_resources/EFFICIENCY_RESOURCES.md) |

## 4. 整体开发规范摘要

| 规范项 | 要求 | 详细文档 |
|--------|------|---------|
| **命名空间** | `OHOS::BackgroundTaskMgr` | [frameworks/FRAMEWORKS.md](frameworks/FRAMEWORKS.md#命名规范) |
| **错误码** | 使用 `bgtaskmgr_inner_errors.h` 定义 | [frameworks/FRAMEWORKS.md](frameworks/FRAMEWORKS.md#错误处理) |
| **日志** | 使用 `BGTASK_LOG*` 宏 | [frameworks/FRAMEWORKS.md](frameworks/FRAMEWORKS.md#日志规范) |
| **IPC 接口** | 修改 IDL 文件后重新生成代理/桩 | [interfaces/INTERFACES.md](interfaces/INTERFACES.md#IDL规范) |

## 5. 文档导航

- **框架层开发** → [frameworks/FRAMEWORKS.md](frameworks/FRAMEWORKS.md)
- **接口层开发** → [interfaces/INTERFACES.md](interfaces/INTERFACES.md)
- **服务层开发** → [services/AGENTS.md](services/AGENTS.md)
- **系统能力配置** → [sa_profile/SA_PROFILE.md](sa_profile/SA_PROFILE.md)
- **资源配置** → [resources/RESOURCES.md](resources/RESOURCES.md)

---

> 💡 **提示**：本目录文档采用渐进式披露原则，仅提供概述和导航。具体实现细节请查阅对应子目录的 `AGENTS.md` 文件。