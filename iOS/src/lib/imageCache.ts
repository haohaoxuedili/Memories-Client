const CACHE_NAME = 'ios-gallery-image-cache-v1';

export async function cacheImage(url: string): Promise<boolean> {
  try {
    const cache = await caches.open(CACHE_NAME);
    const cached = await cache.match(url);
    if (cached) return true;

    const response = await fetch(url, { mode: 'no-cors' });
    // 跨域图片可能是 opaque 响应（status 为 0），仍可写入 Cache API
    if (response.type === 'opaque' || response.ok) {
      await cache.put(url, response.clone());
      return true;
    }
  } catch {
    // 缓存失败不影响主流程
  }
  return false;
}

export async function getCachedImage(url: string): Promise<string | null> {
  try {
    const cache = await caches.open(CACHE_NAME);
    const cached = await cache.match(url);
    if (cached) {
      const blob = await cached.blob();
      return URL.createObjectURL(blob);
    }
  } catch {
    // 读取失败返回原 URL
  }
  return null;
}

export async function downloadImage(url: string, filename?: string): Promise<boolean> {
  const name = filename || `image-${Date.now()}`;

  // 优先尝试 fetch 获取 Blob 后触发真正的文件下载
  try {
    const response = await fetch(url);
    if (!response.ok) {
      throw new Error(`下载失败：${response.status}`);
    }
    const blob = await response.blob();
    const objectUrl = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = objectUrl;
    link.download = name;
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    setTimeout(() => URL.revokeObjectURL(objectUrl), 1000);
    return true;
  } catch {
    // 跨域图床 fetch 失败时，使用带 download 属性的锚链接触发浏览器下载
    return new Promise((resolve) => {
      const link = document.createElement('a');
      link.href = url;
      link.download = name;
      link.target = '_blank';
      link.rel = 'noopener noreferrer';
      document.body.appendChild(link);
      link.click();
      document.body.removeChild(link);
      // 无法确切知道是否下载成功，按触发成功处理
      resolve(true);
    });
  }
}

export async function copyImageUrl(url: string): Promise<boolean> {
  try {
    await navigator.clipboard.writeText(url);
    return true;
  } catch {
    return false;
  }
}

export async function shareImage(url: string, title?: string): Promise<boolean> {
  try {
    if (navigator.share) {
      await navigator.share({
        title: title ?? '分享图片',
        url,
      });
      return true;
    }
  } catch {
    // 用户取消分享不算失败
    return false;
  }
  return false;
}
