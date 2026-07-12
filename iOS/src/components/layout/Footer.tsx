import React from 'react';
import { Globe, Heart, Mail } from 'lucide-react';

const Footer: React.FC = () => {
  return (
    <footer className="w-full mt-4">
      {/* 顶部渐变分割线 */}
      <div className="h-px w-full" style={{ background: 'var(--gradient-primary)', opacity: 0.25 }} />

      <div className="ios-frost border-t border-border/30">
        <div className="mx-auto max-w-7xl px-4 py-8 md:px-6">
          {/* 主内容区 */}
          <div className="flex flex-col items-center gap-6 md:flex-row md:items-start md:justify-between">
            {/* 左侧品牌区 */}
            <div className="flex flex-col items-center md:items-start gap-2">
              <div className="flex items-center gap-2">
                <span
                  className="text-lg font-bold gradient-text select-none tracking-wide"
                >
                  memories
                </span>
              </div>
              <p className="text-xs text-muted-foreground text-center md:text-left leading-relaxed max-w-48">
                轻量、优雅的图片浏览工具<br />让每一张图片都值得珍藏
              </p>
            </div>

            {/* 右侧链接区 */}
            <div className="flex flex-col items-center md:items-end gap-3">
              <p className="text-xs font-medium text-muted-foreground uppercase tracking-widest">联系方式</p>
              <div className="flex flex-col items-center md:items-end gap-2.5">
                <a
                  href="https://memories.mrcwoods.com"
                  target="_blank"
                  rel="noreferrer noopener"
                  className="group inline-flex items-center gap-2 text-sm text-muted-foreground hover:text-primary transition-all duration-200 min-h-10"
                >
                  <Globe className="h-4 w-4 transition-transform duration-200 group-hover:scale-110" />
                  <span>memories.mrcwoods.com</span>
                </a>
                <a
                  href="mailto:mail@mrcwoods.com"
                  className="group inline-flex items-center gap-2 text-sm text-muted-foreground hover:text-primary transition-all duration-200 min-h-10"
                >
                  <Mail className="h-4 w-4 transition-transform duration-200 group-hover:scale-110" />
                  <span>mail@mrcwoods.com</span>
                </a>
              </div>
            </div>
          </div>

          {/* 底部版权栏 */}
          <div className="mt-6 pt-5 border-t border-border/30 flex items-center justify-center gap-1.5 text-xs text-muted-foreground/70">
            <span>Made with</span>
            <Heart className="h-3 w-3 text-primary fill-primary animate-pulse" />
            <span>by mrcwoods · {new Date().getFullYear()}</span>
          </div>
        </div>
      </div>
    </footer>
  );
};

export default Footer;
