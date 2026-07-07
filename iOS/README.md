# iOS Web 客户端

`iOS/` 目录当前承载的是 Memories 的 iOS 风格前端客户端，而不是原生 Swift 工程。项目基于 React 18、TypeScript、Vite 和 Tailwind CSS，使用 `pnpm` 管理依赖，适合在桌面浏览器或移动 WebView 场景下调试和构建。

## 技术栈

- React 18
- TypeScript 5
- Vite（`rolldown-vite`）
- Tailwind CSS
- Biome
- pnpm 10

## 目录结构

```text
iOS/
├── README.md
├── biome.json
├── components.json
├── index.html
├── package.json
├── postcss.config.js
├── tailwind.config.js
├── tsconfig.app.json
├── tsconfig.check.json
├── tsconfig.json
├── tsconfig.node.json
├── vite.config.ts
├── docs/
│   └── DESIGN.md
└── src/
    ├── App.tsx
    ├── main.tsx
    ├── index.css
    ├── routes.tsx
    ├── components/
    ├── contexts/
    ├── hooks/
    ├── lib/
    ├── pages/
    └── types/
```

## 环境要求

- Node.js 20 及以上
- pnpm 10 及以上

建议先确认版本：

```bash
node -v
pnpm -v
```

## 安装与运行

```bash
cd iOS
pnpm install
pnpm dev --host 127.0.0.1
```

默认构建命令：

```bash
cd iOS
pnpm build
```

项目内常用脚本：

- `pnpm dev`：启动开发服务器
- `pnpm build`：生成生产构建
- `pnpm lint`：执行类型检查、Biome 检查，并串行验证构建

## 接口说明

- 主 API 基址：`https://memories-api.mrcwoods.com`
- 图片上传接口：`https://img.scdn.io/api/v1.php`
- 图片信息查询接口：`https://img.scdn.io/api/v1.php?q=文件名`

其中，图片列表、登录态校验和健康检查仍使用主 API；只有图片信息查询单独切到了 `img.scdn.io` 域名。

## 主要功能

- 校园图片广场与分页浏览
- 图片预览、下载、复制链接与元数据查询
- OAuth 登录与上传流程
- PWA 安装、离线打开与主屏使用
- 主题切换、字号调整
- 多款 CDN 字体切换

## PWA 离线分发

当前 iOS Web 客户端已经补齐 PWA 基础能力，可作为“可离线分发”的 Web 安装包使用：

- 构建产物内包含 `manifest.webmanifest`、`sw.js` 和安装图标
- 首次访问后会缓存应用壳、字体资源和部分远程接口响应
- 断网时可打开离线页面，恢复联网后继续刷新数据
- 在 iPhone Safari 中可通过“添加到主屏幕”安装到桌面

发布构建命令：

```bash
cd iOS
pnpm build
```

构建完成后，`dist/` 可直接部署到任意静态站点托管平台。

## 字体与本地缓存

设置面板当前内置以下字体选项：

- 系统默认
- 思源黑体
- 思源宋体
- 霞鹜文楷
- 阿里巴巴普惠体
- Zpix 像素
- OPPO Sans
- 马善政手写
- 站酷小薇
- JetBrains Mono

字体加载策略如下：

- 首次选择 CDN 字体时，先下载字体 CSS
- 解析 CSS 中引用的字体文件并写入浏览器 Cache Storage
- 后续再次切换同一字体时优先复用本地缓存
- 缓存失败时自动回退到远程样式加载，不阻断页面使用

## 故障排查

- 如果依赖安装后 `vite` 相关包异常，优先重新执行 `pnpm install`，不要混用 `npm install`
- 如果 `pnpm lint` 卡在样式检查，通常仍会继续执行；关注最终退出码即可
- 如果需要清理前端产物，可删除 `node_modules/` 和 `dist/` 后重装依赖
