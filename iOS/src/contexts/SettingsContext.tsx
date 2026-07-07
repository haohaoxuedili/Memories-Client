import React, { createContext, useContext, useEffect, useState } from 'react';
import type { AppSettings, AppTheme, AppMode, FontFamily } from '@/types';
import { DEFAULT_SETTINGS } from '@/types';

const FONT_CACHE_NAME = 'ios-gallery-font-cache-v1';

interface FontConfig {
  family: string;
  url?: string;
  injectStyle?: string;
}

interface SettingsContextValue extends AppSettings {
  setTheme: (theme: AppTheme) => void;
  setMode: (mode: AppMode) => void;
  setFont: (font: FontFamily) => void;
  setFontScale: (scale: number) => void;
  themeLabel: string;
}

const SETTINGS_KEY = 'ios-gallery-settings-v1';

const themeLabels: Record<AppTheme, string> = {
  minimal: '简约黑白',
  moss: '苔光晨雾',
  sunset: '霞橙晴空',
  cyan: '青蓝玻璃',
  neon: '夜航霓光',
};

const fontMap: Record<FontFamily, FontConfig> = {
  system: { family: '-apple-system, BlinkMacSystemFont, "SF Pro Text", "Segoe UI", "PingFang SC", "Microsoft YaHei", sans-serif' },
  'noto-sans': {
    family: '"Noto Sans SC", sans-serif',
    url: 'https://fonts.googleapis.com/css2?family=Noto+Sans+SC:wght@400;500;600;700&display=swap',
  },
  'noto-serif': {
    family: '"Noto Serif SC", serif',
    url: 'https://fonts.googleapis.com/css2?family=Noto+Serif+SC:wght@400;600;700&display=swap',
  },
  lxgw: {
    family: '"LXGW WenKai", "PingFang SC", sans-serif',
    url: 'https://cdn.jsdelivr.net/npm/lxgw-wenkai-webfont@1.7.0/style.css',
  },
  alibabapuhuiti: {
    family: '"Alibaba PuHuiTi 2.0", "PingFang SC", sans-serif',
    url: 'https://cdn.jsdelivr.net/npm/alibaba-puhuiti-2/Alibaba-PuHuiTi-Bold/Alibaba-PuHuiTi-Bold.css',
  },
  zpix: {
    family: '"Zpix", monospace',
    url: 'https://cdn.jsdelivr.net/npm/zpix-pixel-font/fonts/stylesheet.css',
  },
  opposans: {
    family: '"OPPO Sans", "PingFang SC", sans-serif',
    injectStyle: `
      @font-face {
        font-family: 'OPPO Sans';
        src: url('https://statics.moonshot.cn/kimi-poster/font/OPPOSans/OPPO-Sans-Regular.woff2') format('woff2');
        font-weight: 400;
        font-style: normal;
        font-display: swap;
      }
    `,
  },
  'ma-shan-zheng': {
    family: '"Ma Shan Zheng", "KaiTi", cursive',
    url: 'https://fonts.googleapis.com/css2?family=Ma+Shan+Zheng&display=swap',
  },
  'zcool-xiaowei': {
    family: '"ZCOOL XiaoWei", "STKaiti", serif',
    url: 'https://fonts.googleapis.com/css2?family=ZCOOL+XiaoWei&display=swap',
  },
  'jetbrains-mono': {
    family: '"JetBrains Mono", "SFMono-Regular", Consolas, monospace',
    url: 'https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@400;500;700&display=swap',
  },
};

const SettingsContext = createContext<SettingsContextValue | undefined>(undefined);

async function getCachedText(url: string): Promise<string | null> {
  try {
    const cache = await caches.open(FONT_CACHE_NAME);
    const cached = await cache.match(url);
    if (!cached) return null;
    return await cached.text();
  } catch {
    return null;
  }
}

async function putCachedResponse(url: string, response: Response): Promise<void> {
  try {
    const cache = await caches.open(FONT_CACHE_NAME);
    await cache.put(url, response.clone());
  } catch {
    // 缓存失败不影响字体使用
  }
}

async function fetchTextWithCache(url: string): Promise<string> {
  const cachedText = await getCachedText(url);
  if (cachedText) {
    return cachedText;
  }

  const response = await fetch(url, { mode: 'cors', credentials: 'omit' });
  if (!response.ok) {
    throw new Error(`字体样式下载失败: ${response.status}`);
  }

  await putCachedResponse(url, response);
  return await response.text();
}

async function fetchBlobUrlWithCache(url: string): Promise<string> {
  try {
    const cache = await caches.open(FONT_CACHE_NAME);
    const cached = await cache.match(url);
    if (cached) {
      const blob = await cached.blob();
      return URL.createObjectURL(blob);
    }

    const response = await fetch(url, { mode: 'cors', credentials: 'omit' });
    if (!response.ok) {
      throw new Error(`字体文件下载失败: ${response.status}`);
    }

    await cache.put(url, response.clone());
    const blob = await response.blob();
    return URL.createObjectURL(blob);
  } catch {
    return url;
  }
}

function resolveFontAssetUrl(assetUrl: string, baseUrl: string): string {
  try {
    return new URL(assetUrl, baseUrl).toString();
  } catch {
    return assetUrl;
  }
}

async function inlineFontAssets(cssText: string, sourceUrl: string): Promise<string> {
  const matches = Array.from(cssText.matchAll(/url\((['"]?)(.*?)\1\)/g));
  if (!matches.length) {
    return cssText;
  }

  const replacements = await Promise.all(
    matches.map(async ([fullMatch, , rawUrl]) => {
      const trimmedUrl = rawUrl.trim();
      if (!trimmedUrl || trimmedUrl.startsWith('data:')) {
        return [fullMatch, fullMatch] as const;
      }

      const resolvedUrl = resolveFontAssetUrl(trimmedUrl, sourceUrl);
      const localUrl = await fetchBlobUrlWithCache(resolvedUrl);
      const nextValue = fullMatch.replace(rawUrl, localUrl);
      return [fullMatch, nextValue] as const;
    }),
  );

  return replacements.reduce((result, [searchValue, replaceValue]) => result.split(searchValue).join(replaceValue), cssText);
}

async function loadFontCSS(font: FontFamily) {
  const config = fontMap[font];
  if (!config) return;

  const id = `font-css-${font}`;
  if (document.getElementById(id)) return;

  if (config.injectStyle) {
    const style = document.createElement('style');
    style.id = id;
    style.textContent = config.injectStyle;
    document.head.appendChild(style);
    return;
  }

  if (config.url) {
    const style = document.createElement('style');
    style.id = id;
    style.setAttribute('data-font-loading', 'true');
    document.head.appendChild(style);

    try {
      const cssText = await fetchTextWithCache(config.url);
      style.textContent = await inlineFontAssets(cssText, config.url);
    } catch {
      document.head.removeChild(style);

      const link = document.createElement('link');
      link.id = id;
      link.rel = 'stylesheet';
      link.href = config.url;
      link.crossOrigin = 'anonymous';
      document.head.appendChild(link);
    }
  }
}

function applySettings(settings: AppSettings) {
  const html = document.documentElement;

  html.classList.remove('theme-minimal', 'theme-moss', 'theme-sunset', 'theme-cyan', 'theme-neon', 'dark');
  html.classList.add(`theme-${settings.theme}`);
  if (settings.mode === 'dark') {
    html.classList.add('dark');
  }

  html.style.setProperty('--font-scale', String(settings.fontScale));

  const config = fontMap[settings.font];
  document.body.style.fontFamily = config.family;
  void loadFontCSS(settings.font);
}

function loadSettings(): AppSettings {
  try {
    const raw = localStorage.getItem(SETTINGS_KEY);
    if (raw) {
      const parsed = JSON.parse(raw) as Partial<AppSettings>;
      return { ...DEFAULT_SETTINGS, ...parsed };
    }
  } catch {
    // 忽略读取错误
  }
  return DEFAULT_SETTINGS;
}

function saveSettings(settings: AppSettings) {
  try {
    localStorage.setItem(SETTINGS_KEY, JSON.stringify(settings));
  } catch {
    // 忽略写入错误
  }
}

export const SettingsProvider: React.FC<{ children: React.ReactNode }> = ({ children }) => {
  const [settings, setSettings] = useState<AppSettings>(loadSettings);

  useEffect(() => {
    applySettings(settings);
    saveSettings(settings);
  }, [settings]);

  const value: SettingsContextValue = {
    ...settings,
    themeLabel: themeLabels[settings.theme],
    setTheme: (theme) => setSettings((s) => ({ ...s, theme })),
    setMode: (mode) => setSettings((s) => ({ ...s, mode })),
    setFont: (font) => setSettings((s) => ({ ...s, font })),
    setFontScale: (fontScale) => setSettings((s) => ({ ...s, fontScale })),
  };

  return <SettingsContext.Provider value={value}>{children}</SettingsContext.Provider>;
};

export function useSettings(): SettingsContextValue {
  const context = useContext(SettingsContext);
  if (!context) {
    throw new Error('useSettings 必须在 SettingsProvider 内使用');
  }
  return context;
}
