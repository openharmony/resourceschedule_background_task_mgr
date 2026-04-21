# 系统能力配置开发指南

[← 返回上级目录](../AGENTS.md)

## 1. 功能概述

本目录定义后台任务管理服务的系统能力配置，用于向 OpenHarmony 系统注册系统服务。

## 2. 目录结构

```
sa_profile/
├── BUILD.gn        # 构建配置文件
└── 1903.json       # 系统能力配置文件
```

## 3. 配置文件说明

### 1903.json 配置项

| 配置项 | 值 | 说明 |
|--------|-----|------|
| `name` | 1903 | 后台任务管理服务的saId |
| `process` | resource_schedule_service | 服务运行在资源调度进程 |
| `libpath` | libbgtaskmgr_service.z.so | 服务实现动态库 |
| `run-on-create` | true | 系统启动时自动创建服务 |
| `distributed` | false | 不支持分布式部署 |
| `dump_level` | 1 | dump 信息级别 |
| `extension` | ["backup", "restore"] | 支持备份恢复扩展 |

## 4. 修改规范

- 服务 ID（1903）为固定值，定义在 `system_ability_definition.h`
- 进程名需与资源调度子系统其他服务一致
- 新增配置项需同步更新 `BUILD.gn` 中的 `sa_profile` 目标

> 💡 **提示**：修改配置后需重新构建并重启设备验证服务注册状态。