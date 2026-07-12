export interface ImageItem {
  id: number;
  url: string;
  uploaded_at: number;
}

export interface ImageListResponse {
  data: ImageItem[];
  next_after_id: number | null;
  limit: number;
}

export interface ImageMetadata {
  id: number;
  filename: string;
  original_filename: string;
  original_size_bytes: number;
  compressed_size_bytes: number;
  size_display: string;
  current_hash: string;
  upload_date: string;
  last_accessed: string | null;
  uploader_masked: string;
  location: string;
  tags: string;
  tags_array: string[];
  tag_updated_at: string | null;
  content_description: string;
  content_desc_updated_at: string | null;
  storage_backend: string;
  storage_location: string;
  image_url: string;
  cdn_domain: string;
  password_protected: boolean;
}

export interface MetadataResponse {
  success: boolean;
  data?: ImageMetadata;
  error?: string;
  message?: string;
}

export type AppTheme = 'minimal' | 'moss' | 'sunset' | 'cyan' | 'neon';
export type AppMode = 'light' | 'dark';
export type FontFamily =
  | 'system'
  | 'noto-sans'
  | 'noto-serif'
  | 'lxgw'
  | 'alibabapuhuiti'
  | 'zpix'
  | 'opposans'
  | 'ma-shan-zheng'
  | 'zcool-xiaowei'
  | 'jetbrains-mono';

export interface AppSettings {
  theme: AppTheme;
  mode: AppMode;
  font: FontFamily;
  fontScale: number;
}

export const DEFAULT_SETTINGS: AppSettings = {
  theme: 'cyan',
  mode: 'light',
  font: 'system',
  fontScale: 1,
};

export interface NetworkInfo {
  latency: number | null;
  speed: number | null;
  ip: string | null;
}

export interface ImageTransform {
  rotate: number;
  scale: number;
  flipX: boolean;
  flipY: boolean;
  translateX: number;
  translateY: number;
}
