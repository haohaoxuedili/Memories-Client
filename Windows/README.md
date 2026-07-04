# Memories Client (C++)

跨平台 Linux 图片管理桌面客户端，使用 C++/Qt6 构建。高性能、低资源占用、支持无图形化模式启动。

## 功能概览

| 功能模块 | 说明 |
|---------|------|
| **服务检测** | 启动时自动 GET `/health` 健康检查 |
| **图片广场** | 分页浏览图片列表，缩略图网格展示 |
| **图片预览** | 旋转、翻转、缩放、拖拽，分享/下载/复制URL/打印/设壁纸 |
| **图片上传** | 单张/批量上传，经 scdn.io → Memories API 两段式上传，可控延迟防限速 |
| **上传队列** | 队列管理、进度追踪、完成通知、记录清理 |
| **批量操作** | 全选、批量下载/分享/打印/复制URL |
| **图片信息** | 查询元数据（名称/存储位置/标签/AI描述） |
| **OAuth 登录** | 授权码流程 + Token 手动输入双模式 |
| **个性化设置** | 下载位置、缓存清理、主题/字号/字体、上传默认参数 |

## 安装方式

```bash
# 给安装脚本添加执行权限
chmod +x scripts/install.sh

# 完整 GUI 安装（自动检测发行版、安装依赖、编译、安装）
./scripts/install.sh install

# 仅无图形化版本
./scripts/install.sh install-headless

# 生成 .deb 包
./scripts/install.sh deb

# 生成 .rpm 包
./scripts/install.sh rpm

# 生成全部包格式
./scripts/install.sh all-packages

# 卸载
./scripts/install.sh uninstall
```

## 从源码构建

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=ON
cmake --build . --parallel $(nproc)
```

### 构建无 GUI 版本

```bash
mkdir build-headless && cd build-headless
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=OFF
cmake --build . --parallel $(nproc)
```

## 使用

### GUI 模式

```bash
memories-client
# 或
./build/MemoriesClient
```

### 无图形化模式

```bash
memories-client --headless
# 或
memories-client-headless --health
memories-client-headless --list
```

## 依赖

- CMake >= 3.20
- Qt 6 (Core, Gui, Widgets, Network, Quick, Svg, OpenGLWidgets)
- C++20 编译器 (GCC 11+ / Clang 14+)
- X11 开发库

## API 端点

| 端点 | 方法 | 说明 |
|------|------|------|
| `/health` | GET | 服务健康检查 |
| `/images?after_id=0` | GET | 分页获取图片列表（每页20条） |
| `/images` | POST | 写入图片URL，返回自增ID |
| `img.scdn.io/api/v1.php` | POST | 上传图片文件 |
| `img.scdn.io/api/v1.php?q=` | GET | 查询图片元数据 |

## 项目结构

```
cpp-client/
├── CMakeLists.txt
├── src/
│   ├── main.cpp              # 入口（GUI/Headless 双模式）
│   ├── app/
│   │   ├── Application.h/cpp  # 应用核心（单例）
│   │   └── Settings.h/cpp     # 持久化设置（QSettings）
│   ├── network/
│   │   ├── ApiClient.h/cpp    # Memories API 客户端
│   │   ├── HealthChecker.h/cpp # 健康检查器
│   │   └── ImageUploader.h/cpp # 上传引擎（两段式+限速）
│   ├── models/
│   │   ├── ImageModel.h       # 图片数据结构
│   │   └── UploadQueue.h/cpp  # 上传队列模型
│   ├── ui/
│   │   ├── MainWindow.h/cpp   # 主窗口（菜单/工具栏/状态栏/托盘）
│   │   ├── GalleryWidget.h/cpp # 图片广场（网格/批量操作）
│   │   ├── ImageViewer.h/cpp  # 图片预览器（变换/操作）
│   │   ├── UploadDialog.h/cpp # 上传对话框
│   │   ├── SettingsDialog.h/cpp # 设置对话框
│   │   ├── LoginDialog.h/cpp  # OAuth 登录
│   │   └── ImageInfoDialog.h/cpp # 图片信息查询
│   └── utils/
│       ├── ImageCache.h/cpp   # 图片缓存（内存+磁盘）
│       └── Logger.h/cpp       # 日志系统
├── resources/
└── scripts/
    └── install.sh
```
