import React from 'react';
import { Moon, Sun, Type, Palette, SlidersHorizontal, Check } from 'lucide-react';
import { Sheet, SheetContent, SheetHeader, SheetTitle, SheetTrigger } from '@/components/ui/sheet';
import { Button } from '@/components/ui/button';
import { Label } from '@/components/ui/label';
import { Slider } from '@/components/ui/slider';
import { useSettings } from '@/contexts/SettingsContext';
import type { AppTheme, AppMode, FontFamily } from '@/types';

const themes: { value: AppTheme; label: string; gradient: string }[] = [
  { value: 'minimal', label: '简约黑白', gradient: 'linear-gradient(135deg, hsl(0 0% 25%), hsl(0 0% 0%))' },
  { value: 'moss', label: '苔光晨雾', gradient: 'linear-gradient(135deg, hsl(120 25% 45%), hsl(160 30% 55%))' },
  { value: 'sunset', label: '霞橙晴空', gradient: 'linear-gradient(135deg, hsl(17 100% 65%), hsl(35 100% 62%))' },
  { value: 'cyan', label: '青蓝玻璃', gradient: 'linear-gradient(135deg, hsl(211 100% 55%), hsl(190 90% 65%))' },
  { value: 'neon', label: '夜航霓光', gradient: 'linear-gradient(135deg, hsl(280 85% 65%), hsl(320 80% 65%))' },
];

const fonts: { value: FontFamily; label: string }[] = [
  { value: 'system', label: '系统默认' },
  { value: 'noto-sans', label: '思源黑体' },
  { value: 'noto-serif', label: '思源宋体' },
  { value: 'lxgw', label: '霞鹜文楷' },
  { value: 'alibabapuhuiti', label: '阿里巴巴普惠体' },
  { value: 'zpix', label: 'Zpix 像素' },
  { value: 'opposans', label: 'OPPO Sans' },
  { value: 'ma-shan-zheng', label: '马善政手写' },
  { value: 'zcool-xiaowei', label: '站酷小薇' },
  { value: 'jetbrains-mono', label: 'JetBrains Mono' },
];

const SettingsPanel: React.FC = () => {
  const {
    theme,
    mode,
    font,
    fontScale,
    setTheme,
    setMode,
    setFont,
    setFontScale,
    themeLabel,
  } = useSettings();

  return (
    <Sheet>
      <SheetTrigger asChild>
        <Button
          variant="ghost"
          size="icon"
          className="rounded-full text-foreground hover:bg-muted"
          aria-label="打开设置"
        >
          <SlidersHorizontal className="h-5 w-5" />
        </Button>
      </SheetTrigger>
      <SheetContent side="bottom" className="bg-sidebar border-border rounded-t-3xl pb-8 max-h-[85dvh] overflow-y-auto">
        <SheetHeader className="text-left pb-4 border-b border-border">
          <SheetTitle className="text-lg font-semibold">个性化设置</SheetTitle>
        </SheetHeader>

        <div className="space-y-8 py-6">
          {/* 主题选择 */}
          <section>
            <div className="flex items-center gap-2 mb-4">
              <Palette className="h-4 w-4 text-primary" />
              <Label className="text-base font-medium">主题选择</Label>
              <span className="ml-auto text-sm text-muted-foreground">{themeLabel}</span>
            </div>
            <div className="grid grid-cols-5 gap-3">
              {themes.map((t) => (
                <button
                  key={t.value}
                  type="button"
                  onClick={() => setTheme(t.value)}
                  className={`flex flex-col items-center gap-2 rounded-xl p-2 transition-all ${
                    theme === t.value ? 'bg-primary/10 ring-2 ring-primary' : 'hover:bg-muted'
                  }`}
                >
                  <span
                    className="h-10 w-10 rounded-full shadow-sm flex items-center justify-center ring-2 ring-transparent"
                    style={{ background: t.gradient }}
                  >
                    {theme === t.value && <Check className="h-4 w-4 text-white" />}
                  </span>
                  <span className="text-xs text-center leading-tight">{t.label}</span>
                </button>
              ))}
            </div>
          </section>

          {/* 界面模式 */}
          <section>
            <div className="flex items-center gap-2 mb-4">
              {mode === 'dark' ? <Moon className="h-4 w-4 text-primary" /> : <Sun className="h-4 w-4 text-primary" />}
              <Label className="text-base font-medium">界面模式</Label>
            </div>
            <div className="flex gap-3">
              <ModeButton active={mode === 'light'} onClick={() => setMode('light')} label="亮色模式" icon={Sun} />
              <ModeButton active={mode === 'dark'} onClick={() => setMode('dark')} label="暗色模式" icon={Moon} />
            </div>
          </section>

          {/* 字体选择 */}
          <section>
            <div className="flex items-center gap-2 mb-4">
              <Type className="h-4 w-4 text-primary" />
              <Label className="text-base font-medium">字体选择</Label>
            </div>
            <div className="grid grid-cols-2 gap-2">
              {fonts.map((f) => (
                <button
                  key={f.value}
                  type="button"
                  onClick={() => setFont(f.value)}
                  className={`rounded-xl px-4 py-3.5 text-left text-sm transition-all ${
                    font === f.value
                      ? 'text-primary-foreground shadow-sm'
                      : 'bg-muted text-foreground hover:bg-muted/80'
                  }`}
                  style={font === f.value ? { background: 'var(--gradient-primary)' } : undefined}
                >
                  {f.label}
                </button>
              ))}
            </div>
          </section>

          {/* 字号调整 */}
          <section>
            <div className="flex items-center justify-between mb-4">
              <div className="flex items-center gap-2">
                <Type className="h-4 w-4 text-primary" />
                <Label className="text-base font-medium">字号调整</Label>
              </div>
              <span className="text-sm text-muted-foreground">{Math.round(fontScale * 100)}%</span>
            </div>
            <Slider
              value={[fontScale]}
              min={0.8}
              max={1.4}
              step={0.05}
              onValueChange={([value]) => setFontScale(value)}
            />
            <p className="mt-2 text-xs text-muted-foreground">拖动滑块调整全局字号，即时生效。</p>
          </section>
        </div>
      </SheetContent>
    </Sheet>
  );
};

const ModeButton: React.FC<{
  active: boolean;
  onClick: () => void;
  label: string;
  icon: React.ElementType;
}> = ({ active, onClick, label, icon: Icon }) => (
  <button
    type="button"
    onClick={onClick}
    className={`flex flex-1 items-center justify-center gap-2 rounded-xl py-3.5 transition-all ${
      active ? 'text-primary-foreground shadow-sm' : 'bg-muted text-foreground hover:bg-muted/80'
    }`}
    style={active ? { background: 'var(--gradient-primary)' } : undefined}
  >
    <Icon className="h-4 w-4" />
    <span className="text-sm font-medium">{label}</span>
  </button>
);

export default SettingsPanel;
