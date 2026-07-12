import React from 'react';
import { Moon, Sun, Images } from 'lucide-react';
import { Button } from '@/components/ui/button';
import { useSettings } from '@/contexts/SettingsContext';
import SettingsPanel from '@/components/settings/SettingsPanel';

interface HeaderProps {
  title?: string;
  showBack?: boolean;
  onBack?: () => void;
}

const Header: React.FC<HeaderProps> = ({ title = '图片浏览', showBack, onBack }) => {
  const { mode, setMode } = useSettings();

  return (
    <header className="sticky top-0 z-40 w-full ios-frost border-b border-border/50 shadow-[0_4px_20px_-8px_hsl(var(--foreground)/8%)]">
      <div className="mx-auto flex h-14 max-w-7xl items-center justify-between px-4 md:px-6">
        <div className="flex items-center gap-3">
          {showBack ? (
            <Button variant="ghost" size="sm" onClick={onBack} className="rounded-full font-medium h-10 px-4">
              完成
            </Button>
          ) : (
            <div className="flex h-8 w-8 items-center justify-center rounded-lg bg-primary text-primary-foreground">
              <Images className="h-4 w-4" />
            </div>
          )}
          <h1 className="text-lg font-semibold tracking-tight text-foreground">{title}</h1>
        </div>

        <div className="flex items-center gap-1">
          <Button
            variant="ghost"
            size="icon"
            onClick={() => setMode(mode === 'dark' ? 'light' : 'dark')}
            className="rounded-full h-10 w-10 text-foreground hover:bg-muted"
            aria-label={mode === 'dark' ? '切换到亮色模式' : '切换到暗色模式'}
          >
            {mode === 'dark' ? <Sun className="h-5 w-5" /> : <Moon className="h-5 w-5" />}
          </Button>
          <SettingsPanel />
        </div>
      </div>
    </header>
  );
};

export default Header;
