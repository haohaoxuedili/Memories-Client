# Memories

> 校园记忆胶片杂志 — 每一张照片，都是青春的底片。

Memories 是一款专为校园场景打造的跨平台客户端，支持 Android、iOS、Windows、macOS 和 Linux。每所学校拥有独立的客户端包，数据隔离、安全可靠。

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)

---

## 项目结构

```
Memories-Client/
├── web/          # 官网（React + Vite + Tailwind CSS）
├── Android/      # Android 原生客户端（Java）
├── iOS/          # iOS 客户端
├── macOS/        # macOS 客户端
├── Windows/      # Windows 客户端
├── Linux/        # Linux 客户端
├── admin/        # 管理后台
└── LICENSE       # GPL v3
```

## 官网

基于 React 18 + Vite + Tailwind CSS + shadcn/ui 构建。

```bash
cd web
pnpm install
pnpm dev        # 启动开发服务器
pnpm build      # 构建静态文件到 dist/
```

## Android 客户端

原生 Java 项目，使用 Android Studio 或 Gradle 构建。

```bash
cd Android
cp app-config.example.properties app/src/main/assets/app-config.properties
# 编辑配置文件填入你的参数
./gradlew :app:assembleDebug
```

详细说明见 [Android/README.md](Android/README.md)。

## 主要功能

- 📷 **校园广场** — 浏览全校师生的照片记忆
- 📤 **多图上传** — 支持批量选择、队列上传、进度展示
- 🔗 **OAuth 登录** — 通过校园墙账号安全登录
- 🎨 **主题定制** — 亮色 / 暗色主题自由切换
- 📦 **一校一包** — 每所学校独立客户端，数据完全隔离
- 🌐 **跨平台** — Android / iOS / Windows / macOS / Linux 全覆盖

## 技术栈

| 模块 | 技术 |
|------|------|
| 官网 | React 18, Vite, Tailwind CSS, shadcn/ui, Framer Motion |
| Android | Java, Android SDK, Gradle |
| 多语言 | 简体中文 / 繁體中文 / English / 日本語 |

## 开源协议

本项目基于 [GNU General Public License v3.0](LICENSE) 开源。
