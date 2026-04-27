# 国际化资源配置开发指南

[← 返回上级目录](../AGENTS.md)

## 1. 功能概述

本目录包含后台任务管理器的国际化资源，打包为 HAP 包安装到系统中，用于：
- 长时任务通知提示文本
- 后台运行权限对话框文本
- 多语言国际化支持（70+ 语言）

## 2. 目录结构

```
resources/
├── BUILD.gn                    # 资源构建配置
├── BackgroundTaskResources.p7b # HAP 签名配置
├── publicity.xml               # 包信息公开文件
├── signature/
│   └── BackgroundTaskResources.gni  # 签名路径配置
└── main/
    ├── config.json             # HAP 配置文件
    └── resources/              # 多语言资源目录
        ├── base/element/string.json    # 基础语言（英文）
        ├── zh_CN/element/string.json   # 简体中文
        ├── zh_TW/element/string.json   # 繁体中文（中国台湾）
        ├── zh_HK/element/string.json   # 繁体中文（中国香港）
        └── ... (共约70个语言目录)
```

## 3. 核心字符串资源

| 类别 | 示例键名 | 说明 |
|------|----------|------|
| 后台模式通知 | `ohos_bgmode_prompt_*` | 数据传输、音视频播放、录制、定位、蓝牙、WiFi、通话等任务通知 |
| 子类型通知 | `ohos_bgsubmode_prompt_*` | 车钥匙、媒体处理、视频投播、运动任务通知 |
| 对话框文本 | `text_notification_*` | 后台运行权限询问对话框 |
| 按钮文本 | `btn_allow_time`, `btn_allow_allowed`, `btn_not_allowed` | 本次允许/始终允许/不允许 |

## 4. HAP 配置信息

| 配置项 | 值 |
|--------|-----|
| BundleName | `com.ohos.backgroundtaskmgr.resources` |
| 安装路径 | `app/com.ohos.backgroundtaskmgr.resources` |
| 支持设备 | tablet, default, tv, car, wearable, 2in1 |

## 5. 新增语言资源步骤

1. 在 `main/resources/` 下创建语言目录（如 `fr_FR/element/`）
2. 复制 `base/element/string.json` 到新目录
3. 翻译字符串值
4. 更新 `BUILD.gn` 中的资源列表

> 💡 **提示**：修改资源字符串后需重新构建并验证通知显示效果。