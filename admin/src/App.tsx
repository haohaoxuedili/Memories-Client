import { ChangeEvent, FormEvent, useEffect, useMemo, useState } from 'react';
import {
  ArrowLeft,
  ArrowRight,
  Check,
  Cloud,
  Copy,
  Eye,
  Gauge,
  ImagePlus,
  Loader2,
  Lock,
  RefreshCw,
  Search,
  ShieldCheck,
  Trash2,
  UploadCloud,
} from 'lucide-react';

const MEMORIES_REMOTE_BASE = 'https://memories-api.mrcwoods.com';
const MEMORIES_DEFAULT_BASE = '/memories-api';
const PUBLIC_API_REMOTE = 'https://img.scdn.io/api/v1.php';
const PUBLIC_API_DEFAULT = '/public-image-api';
const MAX_LIST_LIMIT = 20;
const LAST_VIEWED_KEY = 'memories-admin:last-viewed-id';
const SETTINGS_KEY = 'memories-admin:settings:v1';
const PASSPHRASE_KEY = 'memories-admin:device-key:v1';
const FIXED_CDN_DOMAIN = 'cloudflarecnimg.scdn.io';
const FIXED_STORAGE_DESTINATION = 'telegram';

const outputFormats = ['auto', 'jpg', 'jpeg', 'png', 'webp', 'gif', 'webp_animated'];

type ToastKind = 'good' | 'bad' | 'info';

type Toast = {
  kind: ToastKind;
  text: string;
};

type PageKey = 'overview' | 'upload' | 'gallery' | 'query' | 'backup';

const pages: { key: PageKey; label: string; description: string }[] = [
  { key: 'overview', label: '概览', description: '服务状态与快速操作' },
  { key: 'upload', label: '上传', description: '图床上传并写入 API' },
  { key: 'gallery', label: '图库', description: '预览、翻页与删除' },
  { key: 'query', label: '查询', description: '公开元数据检索' },
  { key: 'backup', label: '备份', description: 'WebDAV 配置' },
];

type MemoryImage = {
  id: number;
  url: string;
  uploaded_at: number;
};

type MemoryListResponse = {
  data: MemoryImage[];
  next_after_id: number | null;
  limit: number;
};

type TokenResponse = {
  token: string;
  expires_at: number;
  ttl_seconds: number;
};

type PublicUploadResponse = {
  success: boolean;
  url?: string;
  message?: string;
  error?: string;
  data?: {
    url?: string;
    filename?: string;
    storage_backend?: string;
    original_size?: number;
    compressed_size?: number;
    compression_ratio?: number;
    message?: string;
  };
};

type PublicMetaResponse = {
  success: boolean;
  error?: string;
  message?: string;
  data?: PublicMeta;
};

type PublicMeta = {
  id: number;
  filename: string;
  original_filename?: string;
  size_display?: string;
  upload_date?: string;
  last_accessed?: string | null;
  uploader_masked?: string;
  location?: string;
  tags?: string;
  tags_array?: string[];
  tag_updated_at?: string | null;
  content_description?: string;
  content_desc_updated_at?: string | null;
  storage_backend?: string;
  storage_location?: string;
  image_url?: string;
  cdn_domain?: string;
  password_protected?: boolean;
};

type Settings = {
  memoriesBaseUrl: string;
  adminPath: string;
  adminKey: string;
  publicApiUrl: string;
  backup: {
    enabled: boolean;
    webdav_url: string;
    username: string;
    password: string;
  };
};

const defaultSettings: Settings = {
  memoriesBaseUrl: MEMORIES_DEFAULT_BASE,
  adminPath: '/admin/token',
  adminKey: '',
  publicApiUrl: PUBLIC_API_DEFAULT,
  backup: {
    enabled: false,
    webdav_url: '',
    username: '',
    password: '',
  },
};

function normalizeBaseUrl(value: string) {
  return value.trim().replace(/\/+$/, '');
}

function assertHttpUrl(value: string, label: string) {
  let parsed: URL;
  try {
    parsed = new URL(value.trim());
  } catch {
    throw new Error(`${label} 不是有效 URL`);
  }
  if (!['http:', 'https:'].includes(parsed.protocol)) {
    throw new Error(`${label} 只允许 http 或 https`);
  }
  return parsed.toString().replace(/\/+$/, '');
}

function assertApiUrl(value: string, label: string) {
  const trimmed = value.trim();
  if (trimmed.startsWith('/')) return trimmed.replace(/\/+$/, '') || '/';
  return assertHttpUrl(trimmed, label);
}

function joinUrl(base: string, path: string) {
  const cleanPath = path.startsWith('/') ? path : `/${path}`;
  return `${assertApiUrl(base, 'API 地址')}${cleanPath}`;
}

function toUrl(value: string) {
  return new URL(value, window.location.origin);
}

async function readJson<T>(response: Response): Promise<T> {
  const text = await response.text();
  let data: { error?: string; message?: string } | null = null;
  try {
    data = text ? JSON.parse(text) : null;
  } catch {
    data = null;
  }
  if (!response.ok) {
    const message = data?.error || data?.message || text.trim() || response.statusText || '请求失败';
    throw new Error(`${response.status} ${message}`);
  }
  if (text && !data) {
    throw new Error('响应不是有效 JSON');
  }
  return data as T;
}

function bytesToBase64(bytes: Uint8Array) {
  return btoa(String.fromCharCode(...bytes));
}

function base64ToBytes(value: string) {
  return Uint8Array.from(atob(value), (char) => char.charCodeAt(0));
}

function toArrayBuffer(bytes: Uint8Array) {
  return bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength) as ArrayBuffer;
}

function getOrCreatePassphrase() {
  const existing = localStorage.getItem(PASSPHRASE_KEY);
  if (existing) return existing;
  const bytes = crypto.getRandomValues(new Uint8Array(32));
  const value = bytesToBase64(bytes);
  localStorage.setItem(PASSPHRASE_KEY, value);
  return value;
}

async function getCryptoKey(salt: Uint8Array) {
  const material = await crypto.subtle.importKey('raw', new TextEncoder().encode(getOrCreatePassphrase()), 'PBKDF2', false, ['deriveKey']);
  return crypto.subtle.deriveKey(
    { name: 'PBKDF2', salt: toArrayBuffer(salt), iterations: 120000, hash: 'SHA-256' },
    material,
    { name: 'AES-GCM', length: 256 },
    false,
    ['encrypt', 'decrypt'],
  );
}

async function encryptSettings(settings: Settings) {
  const iv = crypto.getRandomValues(new Uint8Array(12));
  const salt = crypto.getRandomValues(new Uint8Array(16));
  const key = await getCryptoKey(salt);
  const encrypted = await crypto.subtle.encrypt({ name: 'AES-GCM', iv: toArrayBuffer(iv) }, key, new TextEncoder().encode(JSON.stringify(settings)));
  localStorage.setItem(SETTINGS_KEY, JSON.stringify({ iv: bytesToBase64(iv), salt: bytesToBase64(salt), data: bytesToBase64(new Uint8Array(encrypted)) }));
}

async function decryptSettings(): Promise<Settings | null> {
  const stored = localStorage.getItem(SETTINGS_KEY);
  if (!stored) return null;
  try {
    const parsed = JSON.parse(stored) as { iv: string; salt: string; data: string };
    const key = await getCryptoKey(base64ToBytes(parsed.salt));
    const decrypted = await crypto.subtle.decrypt({ name: 'AES-GCM', iv: toArrayBuffer(base64ToBytes(parsed.iv)) }, key, base64ToBytes(parsed.data));
    const saved = { ...defaultSettings, ...JSON.parse(new TextDecoder().decode(decrypted)) } as Settings;
    const isLocalDev = window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1';
    return {
      ...saved,
      memoriesBaseUrl: isLocalDev ? MEMORIES_DEFAULT_BASE : MEMORIES_REMOTE_BASE,
      adminPath: defaultSettings.adminPath,
      publicApiUrl: isLocalDev ? PUBLIC_API_DEFAULT : PUBLIC_API_REMOTE,
    };
  } catch {
    localStorage.removeItem(SETTINGS_KEY);
    return null;
  }
}

function formatTime(value: number | string | null | undefined) {
  if (!value) return '暂无';
  if (typeof value === 'number') return new Date(value).toLocaleString('zh-CN');
  return value;
}

function storageLabel(value?: string) {
  if (value === 'telegram') return 'Telegram';
  if (value === 'r2') return 'Cloudflare R2';
  return value || '默认';
}

function extractMemoryImage(value: MemoryImage | { data?: MemoryImage }): MemoryImage | null {
  if ('id' in value) return value;
  return value.data || null;
}

export default function App() {
  const [settings, setSettings] = useState<Settings>(defaultSettings);
  const [settingsLoaded, setSettingsLoaded] = useState(false);
  const [token, setToken] = useState<TokenResponse | null>(null);
  const [images, setImages] = useState<MemoryImage[]>([]);
  const [nextAfterId, setNextAfterId] = useState<number | null>(0);
  const [previewIndex, setPreviewIndex] = useState<number | null>(null);
  const [lastViewedId, setLastViewedId] = useState<number>(() => Number(localStorage.getItem(LAST_VIEWED_KEY) || 0));
  const [query, setQuery] = useState('');
  const [meta, setMeta] = useState<PublicMeta | null>(null);
  const [toast, setToast] = useState<Toast | null>(null);
  const [busy, setBusy] = useState<string | null>(null);
  const [activePage, setActivePage] = useState<PageKey>('overview');
  const [unlockKey, setUnlockKey] = useState('');
  const [uploadFile, setUploadFile] = useState<File | null>(null);
  const [imageUrl, setImageUrl] = useState('');
  const [outputFormat, setOutputFormat] = useState('auto');
  const [uploadResult, setUploadResult] = useState<PublicUploadResponse | null>(null);
  const [tokenExpiryPrompt, setTokenExpiryPrompt] = useState<number | null>(null);

  useEffect(() => {
    decryptSettings().then((saved) => {
      if (saved) {
        setSettings(saved);
      } else {
        const isLocalDev = window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1';
        setSettings({
          ...defaultSettings,
          memoriesBaseUrl: isLocalDev ? MEMORIES_DEFAULT_BASE : MEMORIES_REMOTE_BASE,
          publicApiUrl: isLocalDev ? PUBLIC_API_DEFAULT : PUBLIC_API_REMOTE,
        });
      }
      setSettingsLoaded(true);
    });
  }, []);

  useEffect(() => {
    if (!toast) return;
    const timer = window.setTimeout(() => setToast(null), 3800);
    return () => window.clearTimeout(timer);
  }, [toast]);

  useEffect(() => {
    if (!token) {
      setTokenExpiryPrompt(null);
      return;
    }
    const expiresAt = token.expires_at < 1000000000000 ? token.expires_at * 1000 : token.expires_at;
    const timers = [60, 30]
      .map((seconds) => {
        const delay = expiresAt - Date.now() - seconds * 1000;
        if (delay <= 0) return null;
        return window.setTimeout(() => setTokenExpiryPrompt(seconds), delay);
      })
      .filter((timer): timer is number => timer !== null);
    return () => timers.forEach((timer) => window.clearTimeout(timer));
  }, [token]);

  const currentImage = previewIndex === null ? null : images[previewIndex] || null;
  const healthUrl = useMemo(() => joinUrl(settings.memoriesBaseUrl, '/health'), [settings.memoriesBaseUrl]);
  const currentPage = pages.find((page) => page.key === activePage) || pages[0];
  const loadingText = busy === 'upload'
    ? '正在上传并写入 API...'
    : busy === 'images'
      ? '正在读取图库...'
      : busy === 'query'
        ? '正在查询元数据...'
        : busy === 'backup'
          ? '正在保存备份配置...'
          : busy === 'health'
            ? '正在检查服务...'
            : busy === 'token'
              ? '正在获取管理员 token...'
              : busy?.startsWith('delete-')
                ? '正在删除图片...'
                : null;

  function notify(kind: ToastKind, text: string) {
    setToast({ kind, text });
  }

  async function checkHealth() {
    setBusy('health');
    try {
      await readJson<{ ok: boolean }>(await fetch(healthUrl));
      notify('good', 'Memories 服务可用');
    } catch (error) {
      notify('bad', error instanceof Error ? error.message : '健康检查失败');
    } finally {
      setBusy(null);
    }
  }

  function sanitizeKey(raw: string) {
    return raw.replace(/[\u200B-\u200D\uFEFF\u00A0\u2018\u2019\u201C\u201D\u3002\uFF0C\u3001\u2014\u2026\u300A\u300B\u300C\u300D\u300E\u300F\u2018\u2019\u201C\u201D\uFF01\uFF1F\uFF1A\uFF1B\u2013\u2014\u2022\u2032\u2033\u02C6\u02DC\u203E\u00B4\u2018\u2019\u201A\u201B\u201C\u201D\u201E\u201F\u2039\u203A\u00AB\u00BB\u3000]/g, '').trim();
  }

  async function getAdminToken(adminKey = settings.adminKey) {
    const cleanKey = sanitizeKey(adminKey);
    if (!cleanKey) {
      notify('bad', '请先填写管理员 key');
      return null;
    }
    setBusy('token');
    try {
      const isLocalDev = window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1';
    const tokenBase = isLocalDev ? '' : MEMORIES_REMOTE_BASE;
    const url = tokenBase ? new URL(joinUrl(tokenBase, settings.adminPath)) : toUrl(settings.adminPath);
      url.searchParams.set('key', cleanKey);
      const nextToken = await readJson<TokenResponse>(await fetch(url));
      setToken(nextToken);
      notify('good', '管理员 token 已获取');
      return nextToken.token;
    } catch (error) {
      notify('bad', error instanceof Error ? error.message : '获取 token 失败');
      return null;
    } finally {
      setBusy(null);
    }
  }

  async function unlockAdmin(event: FormEvent) {
    event.preventDefault();
    const nextToken = await getAdminToken(unlockKey);
    if (nextToken) {
      updateSettings('adminKey', unlockKey);
      setUnlockKey('');
    }
  }

  async function loadImages(afterId = 0, append = false) {
    setBusy('images');
    try {
      const url = toUrl(joinUrl(settings.memoriesBaseUrl, '/images'));
      url.searchParams.set('after_id', String(afterId));
      const response = await readJson<MemoryListResponse>(await fetch(url));
      setImages((current) => (append ? [...current, ...response.data] : response.data));
      setNextAfterId(response.next_after_id);
      notify('good', `已读取 ${response.data.length} 条图片`);
    } catch (error) {
      notify('bad', error instanceof Error ? error.message : '读取图片失败');
    } finally {
      setBusy(null);
    }
  }

  async function deleteImage(id: number) {
    const adminToken = token?.token || (await getAdminToken());
    if (!adminToken) return;
    setBusy(`delete-${id}`);
    try {
      await readJson<{ deleted: boolean; id: number }>(await fetch(joinUrl(settings.memoriesBaseUrl, '/admin/images/delete'), {
        method: 'POST',
        headers: { 'content-type': 'application/json' },
        body: JSON.stringify({ token: adminToken, id }),
      }));
      setImages((current) => current.filter((image) => image.id !== id));
      setPreviewIndex((index) => {
        if (index === null) return null;
        const remaining = images.filter((image) => image.id !== id);
        return remaining.length ? Math.min(index, remaining.length - 1) : null;
      });
      notify('good', `图片 ${id} 已删除`);
    } catch (error) {
      notify('bad', error instanceof Error ? error.message : '删除失败');
    } finally {
      setBusy(null);
    }
  }

  async function saveBackup(event: FormEvent) {
    event.preventDefault();
    const adminToken = token?.token || (await getAdminToken());
    if (!adminToken) return;
    setBusy('backup');
    try {
      if (settings.backup.enabled) assertHttpUrl(settings.backup.webdav_url, 'WebDAV 地址');
      const response = await readJson<{ ok: boolean; enabled: boolean }>(await fetch(joinUrl(settings.memoriesBaseUrl, '/admin/backup'), {
        method: 'POST',
        headers: { 'content-type': 'application/json' },
        body: JSON.stringify({ token: adminToken, ...settings.backup }),
      }));
      await encryptSettings(settings);
      notify('good', response.enabled ? 'WebDAV 备份已开启' : 'WebDAV 备份已关闭');
    } catch (error) {
      notify('bad', error instanceof Error ? error.message : 'WebDAV 配置失败');
    } finally {
      setBusy(null);
    }
  }

  async function uploadPublicImage(event: FormEvent) {
    event.preventDefault();
    if (!!uploadFile === !!imageUrl.trim()) {
      notify('bad', '请选择文件或填写图片 URL，二者只能选一个');
      return;
    }
    setBusy('upload');
    try {
      assertApiUrl(settings.publicApiUrl, '公共 API 地址');
      const formData = new FormData();
      if (uploadFile) formData.set('image', uploadFile);
      if (imageUrl.trim()) formData.set('image_url', assertHttpUrl(imageUrl, '图片 URL'));
      formData.set('outputFormat', outputFormat);
      formData.set('cdn_domain', FIXED_CDN_DOMAIN);
      formData.set('storage_destination', FIXED_STORAGE_DESTINATION);
      const result = await readJson<PublicUploadResponse>(await fetch(settings.publicApiUrl, { method: 'POST', body: formData }));
      if (!result.success) throw new Error(result.error || result.message || '上传失败');
      const uploadedUrl = result.url || result.data?.url;
      if (!uploadedUrl) throw new Error('图床未返回可写入 Memories API 的 URL');
      const storedImage = await readJson<MemoryImage | { data?: MemoryImage }>(await fetch(joinUrl(settings.memoriesBaseUrl, '/images'), {
        method: 'POST',
        headers: { 'content-type': 'application/json' },
        body: JSON.stringify({ url: uploadedUrl }),
      }));
      const nextImage = extractMemoryImage(storedImage);
      if (nextImage?.id) {
        setImages((current) => [nextImage, ...current.filter((image) => image.id !== nextImage.id)]);
        setLastViewedId(nextImage.id);
        localStorage.setItem(LAST_VIEWED_KEY, String(nextImage.id));
      }
      setUploadResult(result);
      if (result.data?.filename) setQuery(result.data.filename);
      notify('good', nextImage?.id ? `已上传并写入 Memories #${nextImage.id}` : '已上传并写入 Memories API');
    } catch (error) {
      notify('bad', error instanceof Error ? error.message : '上传失败');
    } finally {
      setBusy(null);
    }
  }

  async function queryPublicMeta(event?: FormEvent) {
    event?.preventDefault();
    const trimmed = query.trim();
    if (!trimmed) {
      notify('bad', '请输入图片 ID 或完整文件名');
      return;
    }
    if (!/^\d+$/.test(trimmed) && !/^[A-Za-z0-9_.-]+$/.test(trimmed)) {
      notify('bad', '查询值只能是数字 ID 或完整文件名');
      return;
    }
    setBusy('query');
    try {
      const url = toUrl(settings.publicApiUrl);
      url.searchParams.set('q', trimmed);
      const result = await readJson<PublicMetaResponse>(await fetch(url));
      if (!result.success || !result.data) throw new Error(result.error || result.message || '未查询到图片');
      setMeta(result.data);
      notify('good', `已读取 ${result.data.filename}`);
    } catch (error) {
      setMeta(null);
      notify('bad', error instanceof Error ? error.message : '查询失败');
    } finally {
      setBusy(null);
    }
  }

  function openPreview(index: number) {
    const image = images[index];
    setPreviewIndex(index);
    setLastViewedId(image.id);
    localStorage.setItem(LAST_VIEWED_KEY, String(image.id));
  }

  function movePreview(direction: -1 | 1) {
    setPreviewIndex((index) => {
      if (index === null) return null;
      const next = Math.min(Math.max(index + direction, 0), images.length - 1);
      const image = images[next];
      if (image) {
        setLastViewedId(image.id);
        localStorage.setItem(LAST_VIEWED_KEY, String(image.id));
      }
      return next;
    });
  }

  async function copyText(value: string) {
    await navigator.clipboard.writeText(value);
    notify('good', '已复制到剪贴板');
  }

  function updateSettings<T extends keyof Settings>(key: T, value: Settings[T]) {
    setSettings((current) => ({ ...current, [key]: value }));
  }

  function updateBackup<T extends keyof Settings['backup']>(key: T, value: Settings['backup'][T]) {
    setSettings((current) => ({ ...current, backup: { ...current.backup, [key]: value } }));
  }

  function logout() {
    setToken(null);
    setTokenExpiryPrompt(null);
    setPreviewIndex(null);
    setActivePage('overview');
  }

  async function refreshTokenFromPrompt() {
    setTokenExpiryPrompt(null);
    await getAdminToken();
  }

  function handleFile(event: ChangeEvent<HTMLInputElement>) {
    setUploadFile(event.target.files?.[0] || null);
    if (event.target.files?.[0]) setImageUrl('');
  }

  if (!settingsLoaded) {
    return <main className="boot">正在解密本地配置...</main>;
  }

  if (!token) {
    return (
      <main className="lock-screen">
        {toast && <div className={`toast ${toast.kind}`}>{toast.text}</div>}
        <section className="lock-card">
          <div className="panel-title"><Lock /><div><h1>管理员解锁</h1><p>输入管理员 key 获取临时 token 后进入控制台。</p></div></div>
          <form onSubmit={unlockAdmin} className="form-grid">
            <label>管理员 key<input autoFocus type="password" autoComplete="current-password" value={unlockKey} onChange={(event) => setUnlockKey(event.target.value)} /></label>
            <button className="dark" disabled={busy === 'token'}>{busy === 'token' ? <Loader2 className="spin" /> : <ShieldCheck />}进入管理台</button>
          </form>
        </section>
      </main>
    );
  }

  return (
    <main className="app-shell">
      {toast && <div className={`toast ${toast.kind}`}>{toast.text}</div>}

      <section className="hero">
        <div style={{ display: 'flex', alignItems: 'center', gap: '1rem' }}>
          <AppIcon width={48} height={48} style={{ flexShrink: 0 }} />
          <div>
            <p className="eyebrow">Memories Serves</p>
            <h1>Memories</h1>
            <p className="hero-copy">上传、查询公开元数据、管理 WebDAV 备份，并在预览时快速删除不合要求的记录。</p>
          </div>
        </div>
        <div className="hero-actions">
          <button onClick={checkHealth} disabled={busy === 'health'}>{busy === 'health' ? <Loader2 className="spin" /> : <ShieldCheck />}服务检查</button>
          <button className="dark" onClick={() => loadImages(0, false)} disabled={busy === 'images'}>{busy === 'images' ? <Loader2 className="spin" /> : <RefreshCw />}刷新列表</button>
          <button className="secondary" onClick={logout}><Lock />退出</button>
        </div>
      </section>

      <nav className="page-tabs" aria-label="管理页面">
        {pages.map((page) => <button key={page.key} className={page.key === activePage ? 'active' : ''} onClick={() => setActivePage(page.key)}>{page.label}</button>)}
      </nav>

      <section className="status-strip" aria-label="状态">
        <div><span>上次看到 ID</span><strong>{lastViewedId || '暂无记录'}</strong></div>
        <div><span>列表上限</span><strong>{MAX_LIST_LIMIT} / 页</strong></div>
      </section>

      <section className="page-heading">
        <span>{currentPage.label}</span>
        <div><h2>{currentPage.description}</h2><p>当前页面聚焦一个工作流，减少滚动和误触。</p></div>
      </section>

      {activePage === 'overview' && <div className="workspace overview-page">
        <section className="panel quick-panel">
          <div className="panel-title"><Gauge /><div><h2>快速操作</h2><p>常用入口集中在这里，适合先检查服务再进入图库或上传。</p></div></div>
          <div className="quick-grid">
            <button onClick={checkHealth} disabled={busy === 'health'}>{busy === 'health' ? <Loader2 className="spin" /> : <ShieldCheck />}服务检查</button>
            <button className="dark" onClick={() => { setActivePage('gallery'); loadImages(0, false); }} disabled={busy === 'images'}>{busy === 'images' ? <Loader2 className="spin" /> : <RefreshCw />}读取图库</button>
            <button className="secondary" onClick={() => setActivePage('upload')}><UploadCloud />上传图片</button>
            <button className="secondary" onClick={() => setActivePage('query')}><Search />查询元数据</button>
          </div>
        </section>
        <section className="panel quick-panel">
          <div className="panel-title"><Eye /><div><h2>查看进度</h2><p>保留本机最近浏览位置，回到图库后可以继续处理后续图片。</p></div></div>
          <div className="locked-config">
            <div><span>上次看到 ID</span><strong>{lastViewedId || '暂无记录'}</strong></div>
            <div><span>当前列表</span><strong>{images.length ? `${images.length} 张图片` : '尚未读取'}</strong></div>
            <div><span>下一页游标</span><strong>{nextAfterId ?? '没有更多'}</strong></div>
          </div>
        </section>
      </div>}

      {activePage === 'upload' && <section className="panel single-panel upload-panel">
        <div className="panel-title"><UploadCloud /><div><h2>上传到失控图床</h2><p>支持文件或远程 URL，查询与上传限流互不占用。</p></div></div>
        <form onSubmit={uploadPublicImage} className="form-grid">
          <label className="file-drop"><ImagePlus /><span>{uploadFile ? uploadFile.name : '选择图片或短视频文件'}</span><input type="file" accept="image/*,video/mp4,video/webm,video/quicktime,video/x-msvideo" onChange={handleFile} /></label>
          <label>或通过 URL 上传<input value={imageUrl} placeholder="https://example.com/a.jpg" onChange={(event) => { setImageUrl(event.target.value); if (event.target.value) setUploadFile(null); }} /></label>
          <div className="inline-fields">
            <label>输出格式<select value={outputFormat} onChange={(event) => setOutputFormat(event.target.value)}>{outputFormats.map((format) => <option key={format}>{format}</option>)}</select></label>
            <label>存储位置<input value="Telegram" readOnly /></label>
          </div>
          <label>CDN 域名<input value="cloudflarecnimg.scdn.io" readOnly /></label>
          <button className="dark" disabled={busy === 'upload'}>{busy === 'upload' ? <Loader2 className="spin" /> : <UploadCloud />}上传并写入 API</button>
        </form>
        {uploadResult?.url && <div className="result-card"><img src={uploadResult.url} alt="上传结果预览" loading="lazy" /><div><strong>{uploadResult.data?.filename || '上传成功'}</strong><p>{uploadResult.data?.storage_backend ? `存储：${storageLabel(uploadResult.data.storage_backend)}` : uploadResult.message}</p><button className="secondary" onClick={() => copyText(uploadResult.url || '')}><Copy />复制链接</button></div></div>}
      </section>}

      {activePage === 'query' && <section className="panel single-panel query-panel">
        <div className="panel-title"><Search /><div><h2>查询图片信息</h2><p>按数字 ID 或完整文件名精确查询标签与 AI 画面简述。</p></div></div>
        <form onSubmit={queryPublicMeta} className="search-form"><input value={query} onChange={(event) => setQuery(event.target.value)} placeholder="12345 或 filename.webp" /><button disabled={busy === 'query'}>{busy === 'query' ? <Loader2 className="spin" /> : <Search />}查询</button></form>
        {meta && <article className="meta-card">{meta.image_url && <img src={meta.image_url} alt={meta.filename} loading="lazy" />}<div className="meta-body"><div className="meta-heading"><h3>{meta.filename}</h3>{meta.password_protected && <span>加密图</span>}</div><p className="description">{meta.content_description || '暂无 AI 画面简述'}</p><div className="tag-list">{(meta.tags_array?.length ? meta.tags_array : meta.tags ? meta.tags.split(/[、,]/) : []).map((tag) => <span key={tag}>{tag}</span>)}</div><dl className="meta-grid"><div><dt>ID</dt><dd>{meta.id}</dd></div><div><dt>大小</dt><dd>{meta.size_display || '暂无'}</dd></div><div><dt>位置</dt><dd>{meta.location || '暂无'}</dd></div><div><dt>存储</dt><dd>{meta.storage_location || storageLabel(meta.storage_backend)}</dd></div><div><dt>上传</dt><dd>{formatTime(meta.upload_date)}</dd></div><div><dt>最后访问</dt><dd>{formatTime(meta.last_accessed)}</dd></div></dl></div></article>}
      </section>}

      {activePage === 'backup' && <section className="panel single-panel backup-panel">
        <div className="panel-title"><Cloud /><div><h2>WebDAV 备份</h2><p>使用管理员 token 更新服务端备份配置。</p></div></div>
        <form onSubmit={saveBackup} className="form-grid">
          <label className="switch-line"><input type="checkbox" checked={settings.backup.enabled} onChange={(event) => updateBackup('enabled', event.target.checked)} />开启 WebDAV 备份</label>
          <label>WebDAV 地址<input value={settings.backup.webdav_url} onChange={(event) => updateBackup('webdav_url', event.target.value)} placeholder="https://dav.example.com/backups/" /></label>
          <div className="inline-fields"><label>用户名<input autoComplete="username" value={settings.backup.username} onChange={(event) => updateBackup('username', event.target.value)} /></label><label>密码<input type="password" autoComplete="current-password" value={settings.backup.password} onChange={(event) => updateBackup('password', event.target.value)} /></label></div>
          <button disabled={busy === 'backup'}>{busy === 'backup' ? <Loader2 className="spin" /> : <Cloud />}保存备份配置</button>
        </form>
      </section>}

      {activePage === 'gallery' && <section className="panel gallery-panel">
        <div className="panel-title split-title"><div className="title-left"><Eye /><div><h2>Memories 图片列表</h2><p>点击图片预览，翻页时自动记录看到的 ID。</p></div></div><div className="button-row compact"><button className="secondary" onClick={() => loadImages(lastViewedId, false)} disabled={!lastViewedId || busy === 'images'}>从记录继续</button><button onClick={() => loadImages(0, false)} disabled={busy === 'images'}>{busy === 'images' ? <Loader2 className="spin" /> : <RefreshCw />}读取</button></div></div>
        <div className="gallery-grid">{images.map((image, index) => <article key={image.id} className={image.id <= lastViewedId && lastViewedId ? 'seen tile' : 'tile'}><button className="thumb" onClick={() => openPreview(index)} aria-label={`预览图片 ${image.id}`}><img src={image.url} alt={`图片 ${image.id}`} loading="lazy" /></button><div className="tile-footer"><div><strong>#{image.id}</strong><span>{formatTime(image.uploaded_at)}</span></div><button className="icon-danger" onClick={() => deleteImage(image.id)} disabled={busy === `delete-${image.id}`} aria-label="删除图片">{busy === `delete-${image.id}` ? <Loader2 className="spin" /> : <Trash2 />}</button></div></article>)}</div>
        {!images.length && <div className="empty-state">还没有读取列表，点击“读取”加载第一页。</div>}
        {nextAfterId !== null && images.length > 0 && <button className="load-more" onClick={() => loadImages(nextAfterId, true)} disabled={busy === 'images'}>读取下一页</button>}
      </section>}

      {loadingText && <div className="loading-overlay" role="status" aria-live="polite"><div className="loading-card"><Loader2 className="spin" /><strong>{loadingText}</strong><span>请稍候，当前操作正在处理中。</span></div></div>}

      {tokenExpiryPrompt && <div className="modal-backdrop" role="dialog" aria-modal="true"><section className="token-modal"><div className="panel-title"><Lock /><div><h2>Token 即将过期</h2><p>管理员 token 还剩约 {tokenExpiryPrompt} 秒，继续操作前建议刷新。</p></div></div><div className="button-row"><button className="danger" onClick={logout}>退出</button><button className="dark" onClick={refreshTokenFromPrompt} disabled={busy === 'token'}>{busy === 'token' ? <Loader2 className="spin" /> : <RefreshCw />}刷新 token</button></div></section></div>}

      {currentImage && <div className="preview-backdrop" role="dialog" aria-modal="true"><section className="preview-panel"><div className="preview-stage"><button onClick={() => movePreview(-1)} disabled={previewIndex === 0} aria-label="上一张"><ArrowLeft /></button><img src={currentImage.url} alt={`图片 ${currentImage.id}`} /><button onClick={() => movePreview(1)} disabled={previewIndex === images.length - 1} aria-label="下一张"><ArrowRight /></button></div><div className="preview-toolbar"><div><strong>#{currentImage.id}</strong><span>{formatTime(currentImage.uploaded_at)}</span></div><div className="button-row compact"><button className="secondary" onClick={() => copyText(currentImage.url)}><Copy />复制</button><button className="danger" onClick={() => deleteImage(currentImage.id)}><Trash2 />内容不符合，删除</button><button onClick={() => setPreviewIndex(null)}>关闭</button></div></div></section></div>}
    </main>
  );
}