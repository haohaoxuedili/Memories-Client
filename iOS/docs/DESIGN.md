iOS 风格图片浏览器设计系统
1. Vibe 风格定位
设计语言：iOS / Apple Design Language
材质特征：Soft Glassmorphism（柔和毛玻璃）
整体气质：轻盈、通透、精致、现代
2. Color 色彩系统
2.1 浅色模式
表格
Token	说明
background	柔和中性浅色，叠加主题色光晕渐变
foreground	深色主文字
card	白色 → 浅灰的半透明渐变
primary	明快但不刺眼的彩色渐变（主按钮、加载条）
accent	明快彩色渐变（状态页装饰、强调元素）
muted	次要/辅助文字、禁用态
border	低透明度高光边框
gradient-primary	主色渐变
gradient-accent	强调色渐变
gradient-background	背景光晕渐变
gradient-card	卡片背景渐变
2.2 暗色模式
表格
Token	说明
background	深灰（#121212 附近），禁用纯黑
foreground	#F2F4F7 主文字
card	深色半透明渐变
primary	去饱和柔和渐变（避免眩光）
accent	去饱和柔和渐变
muted	#A3A8B3 次要文字
border	低透明度高光边框
gradient-primary	去饱和主色渐变
gradient-accent	去饱和强调色渐变
gradient-background	深色背景渐变
gradient-card	深色卡片渐变
2.3 对比度标准
正文文字：≥ 4.5:1
大文字 / 图标 / 边框：≥ 3:1
3. Typography 字体系统
表格
层级	字体配置
Heading 标题	系统默认 / 思源黑体 / 思源宋体 / 霞鹜文楷 / 阿里巴巴普惠体 / Zpix / OPPO Sans（跟随设置面板）
Body 正文	与 Heading 保持一致，确保阅读舒适度
4. Visual Language 视觉语言
4.1 核心视觉签名
圆角：连续大圆角（约 16px）
毛玻璃：细腻毛玻璃效果（backdrop-blur-2xl）
高光边框：低透明度高光边框（border-white/20 或 border-border/40）
4.2 材质与深度
表格
模式	处理方式
浅色模式	柔和的弥散阴影，营造悬浮层次感
暗色模式	减少阴影，改用 surface 亮度差异 区分层次
4.3 容器与按钮
表格
元素	样式规范
卡片	渐变背景
主按钮（Primary）	渐变填充
次按钮（Secondary）	半透明描边 / 幽灵按钮
选中态	ring 聚焦环 + 缩放反馈
4.4 布局节奏
留白：Generous  generous 间距，呼吸感充足
卡片网格：随屏幕尺寸自适应响应
移动端优化：
增大点击热区
底部安全间距适配
5. Animation 动画系统
5.1 入场动画
表格
场景	动画效果
卡片列表	motion fade-up + stagger 交错入场
状态页（空态/错误态）	scale-in 缩放入场
5.2 交互动画
表格
元素	反馈效果
按钮 / 卡片	active:scale-95 按压缩放
主题 / 模式切换	300ms 颜色平滑过渡
5.3 滚动与过渡
表格
场景	动画效果
图片加载占位	骨架屏 shimmer 位移动画
预览切换	300ms ease-out 滑动过渡
6. Forbidden 禁用项
表格
禁止项	说明
❌ 纯黑背景	暗色模式禁用纯黑（#000000），使用 #121212 附近深灰
❌ 高饱和荧光色	禁止作为大面积背景使用
❌ 无反馈交互	所有按钮必须有 hover / active 状态反馈
7. Additional Notes 补充说明
文案语言：所有用户可见文案为 中文
震动反馈：长按图片卡片触发选择模式时，优先调用设备震动反馈（navigator.vibrate）
加载状态：
图片加载中：使用骨架屏动画占位
图片加载失败：使用图标 + 文字占位提示