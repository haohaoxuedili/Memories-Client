import type { ImageListResponse, MetadataResponse, NetworkInfo } from '@/types';

const BASE_URL = 'https://memories-api.mrcwoods.com';
const IMAGE_METADATA_BASE_URL = 'https://img.scdn.io';

export async function checkHealth(): Promise<boolean> {
  try {
    const response = await fetch(`${BASE_URL}/health`, {
      method: 'GET',
      cache: 'no-store',
    });
    if (!response.ok) return false;
    const data = (await response.json()) as { ok?: boolean };
    return data.ok === true;
  } catch {
    return false;
  }
}

export async function fetchNetworkInfo(): Promise<NetworkInfo> {
  const info: NetworkInfo = {
    latency: null,
    speed: null,
    ip: null,
  };

  try {
    const start = performance.now();
    const response = await fetch(`${BASE_URL}/health`, { cache: 'no-store' });
    const end = performance.now();
    if (response.ok) {
      info.latency = Math.round(end - start);
    }
  } catch {
    info.latency = null;
  }

  // 通过下载一张已知大小图片估算网速（使用 1x1 透明像素图占位）
  try {
    const speedStart = performance.now();
    const speedResponse = await fetch(`${BASE_URL}/health`, { cache: 'no-store' });
    const blob = await speedResponse.blob();
    const speedEnd = performance.now();
    const duration = (speedEnd - speedStart) / 1000;
    if (duration > 0 && blob.size > 0) {
      const bits = blob.size * 8;
      info.speed = Math.round(bits / duration / 1024);
    }
  } catch {
    info.speed = null;
  }

  // 获取公网 IP
  try {
    const ipResponse = await fetch('https://api.ipify.org?format=json', { cache: 'no-store' });
    if (ipResponse.ok) {
      const ipData = (await ipResponse.json()) as { ip?: string };
      info.ip = ipData.ip ?? null;
    }
  } catch {
    info.ip = null;
  }

  return info;
}

export async function fetchImages(afterId = 0): Promise<ImageListResponse> {
  const response = await fetch(`${BASE_URL}/images?after_id=${afterId}`, {
    cache: 'no-store',
  });
  if (!response.ok) {
    throw new Error(`加载图片列表失败：${response.status}`);
  }
  return (await response.json()) as ImageListResponse;
}

export async function fetchImageMetadata(query: string | number): Promise<MetadataResponse> {
  const response = await fetch(`${IMAGE_METADATA_BASE_URL}/api/v1.php?q=${encodeURIComponent(query)}`, {
    cache: 'no-store',
  });

  // 某些图片可能返回 404 或空响应体，先检查状态与内容长度，避免直接调用 .json() 报错
  const contentLength = response.headers.get('content-length');
  const hasBody = contentLength ? Number(contentLength) > 0 : true;
  if (!response.ok || !hasBody) {
    throw new Error(`查询失败：${response.status}`);
  }

  let data: MetadataResponse;
  try {
    data = (await response.json()) as MetadataResponse;
  } catch {
    throw new Error('元数据格式异常，请稍后重试');
  }

  if (!data.success) {
    throw new Error(data.error ?? data.message ?? '查询失败');
  }
  return data;
}
