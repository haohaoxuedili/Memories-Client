import React, { useCallback, useEffect, useMemo, useState } from 'react';
import { useSearchParams } from 'react-router-dom';
import { Loader2, Play, RefreshCw } from 'lucide-react';
import { Button } from '@/components/ui/button';
import { toast } from 'sonner';
import Header from '@/components/layout/Header';
import Footer from '@/components/layout/Footer';
import ImageCard from '@/components/image/ImageCard';
import ImageViewer from '@/components/image/ImageViewer';
import BatchToolbar from '@/components/image/BatchToolbar';
import Slideshow from '@/components/image/Slideshow';
import { fetchImages } from '@/lib/api';
import { cacheImage, copyImageUrl, downloadImage, shareImage } from '@/lib/imageCache';
import type { ImageItem } from '@/types';

const GalleryPage: React.FC = () => {
  const [searchParams, setSearchParams] = useSearchParams();
  const urlImageId = searchParams.get('id');

  const [images, setImages] = useState<ImageItem[]>([]);
  const [afterId, setAfterId] = useState<number | null>(0);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [selectedIds, setSelectedIds] = useState<Set<number>>(new Set());
  const [selectionMode, setSelectionMode] = useState(false);
  const [viewerIndex, setViewerIndex] = useState(0);
  const [viewerOpen, setViewerOpen] = useState(false);
  const [slideshowOpen, setSlideshowOpen] = useState(false);
  const [slideshowIndex, setSlideshowIndex] = useState(0);

  const loadMore = useCallback(async () => {
    if (loading || afterId === null) return;
    setLoading(true);
    setError(null);

    try {
      const response = await fetchImages(afterId);
      const newImages = response.data.filter(
        (item) => !images.some((existing) => existing.url === item.url)
      );

      setImages((prev) => {
        const updated = [...prev, ...newImages];
        return updated;
      });
      setAfterId(response.next_after_id);

      for (const item of newImages) {
        cacheImage(item.url).catch(() => {});
      }

      return newImages;
    } catch (err) {
      setError(err instanceof Error ? err.message : '加载失败');
      toast.error('图片列表加载失败，请重试');
      return [];
    } finally {
      setLoading(false);
    }
  }, [afterId, images, loading]);

  // 首次加载
  useEffect(() => {
    loadMore();
  }, []);

  // 处理 URL 参数 ?id=xxx
  useEffect(() => {
    if (!urlImageId || images.length === 0) return;

    const targetId = Number(urlImageId);
    if (Number.isNaN(targetId)) return;

    const index = images.findIndex((img) => img.id === targetId);
    if (index >= 0) {
      setViewerIndex(index);
      setViewerOpen(true);
    } else if (afterId !== null && !loading) {
      // 尚未加载到目标图片，继续翻页查找
      loadMore();
    }
  }, [urlImageId, images, afterId, loading]);

  const exitSelectionMode = () => {
    setSelectionMode(false);
    setSelectedIds(new Set());
  };

  const toggleSelect = (id: number) => {
    setSelectedIds((prev) => {
      const next = new Set(prev);
      if (next.has(id)) {
        next.delete(id);
      } else {
        next.add(id);
      }
      return next;
    });
  };

  const handleLongPress = (id: number) => {
    if (!selectionMode) {
      setSelectionMode(true);
    }
    toggleSelect(id);
  };

  const handleSelectAll = () => {
    if (selectedIds.size === images.length) {
      setSelectedIds(new Set());
    } else {
      setSelectedIds(new Set(images.map((img) => img.id)));
    }
  };

  const handleClear = () => setSelectedIds(new Set());

  const selectedImages = useMemo(
    () => images.filter((img) => selectedIds.has(img.id)),
    [images, selectedIds]
  );

  const urlsText = useMemo(
    () => selectedImages.map((img) => img.url).join('\n'),
    [selectedImages]
  );

  const handleBatchShare = async () => {
    if (selectedImages.length === 0) return;
    if (selectedImages.length === 1) {
      await shareImage(selectedImages[0].url, `图片 #${selectedImages[0].id}`);
    } else {
      await shareImage(urlsText, `分享 ${selectedImages.length} 张图片`);
    }
  };

  const handleBatchDownload = async () => {
    if (selectedImages.length === 0) return;
    for (const img of selectedImages) {
      await downloadImage(img.url, `image-${img.id}`);
    }
    toast.success(`已下载 ${selectedImages.length} 张图片`);
  };

  const handleBatchCopy = async () => {
    if (selectedImages.length === 0) return;
    const ok = await copyImageUrl(urlsText);
    if (ok) {
      toast.success(`已复制 ${selectedImages.length} 张图片链接`);
    } else {
      toast.error('复制失败');
    }
  };

  const openViewer = (image: ImageItem) => {
    const index = images.findIndex((img) => img.id === image.id);
    setViewerIndex(index >= 0 ? index : 0);
    setViewerOpen(true);
  };

  const handleViewerIndexChange = (index: number) => {
    setViewerIndex(index);
  };

  const startSlideshow = () => {
    setSlideshowIndex(viewerIndex);
    setSlideshowOpen(true);
    setViewerOpen(false);
  };

  const launchSlideshowFromGrid = () => {
    setSlideshowIndex(0);
    setSlideshowOpen(true);
  };

  return (
    <div className="flex min-h-screen w-full flex-col bg-background">
      <Header
        title={selectionMode ? `已选择 ${selectedIds.size} 张` : '图片浏览'}
        showBack={selectionMode}
        onBack={exitSelectionMode}
      />

      <main className="flex-1 w-full max-w-7xl mx-auto px-4 py-5 md:px-6 pb-28">
        {/* 工具条 */}
        <div className="flex items-center justify-between mb-5">
          <p className="text-sm text-muted-foreground hidden md:block">
            长按图片进入选择模式
          </p>
          <Button
            onClick={launchSlideshowFromGrid}
            variant="outline"
            size="sm"
            className="rounded-full ml-auto"
            disabled={images.length === 0}
          >
            <Play className="h-4 w-4 mr-1.5" />
            幻灯片播放
          </Button>
        </div>

        {images.length === 0 && !loading && error && (
          <div className="ios-card flex flex-col items-center justify-center py-14 px-6 text-center max-w-md mx-auto">
            <RefreshCw className="h-10 w-10 text-muted-foreground mb-4" />
            <p className="text-foreground font-medium mb-2">加载失败</p>
            <p className="text-muted-foreground text-sm mb-5">{error}</p>
            <Button onClick={loadMore} variant="outline" className="rounded-full px-6">
              重新加载
            </Button>
          </div>
        )}

        {images.length === 0 && loading && (
          <div className="ios-card flex flex-col items-center justify-center py-14 px-6 max-w-md mx-auto">
            <Loader2 className="h-10 w-10 animate-spin text-primary mb-4" />
            <p className="text-foreground font-medium mb-1">正在加载图片</p>
            <p className="text-muted-foreground text-sm">请稍候…</p>
          </div>
        )}

        {images.length > 0 && (
          <>
            <div className="grid grid-cols-2 gap-3 sm:grid-cols-3 md:grid-cols-4 lg:grid-cols-5">
              {images.map((image, index) => (
                <ImageCard
                  key={image.id}
                  image={image}
                  index={index}
                  selected={selectedIds.has(image.id)}
                  selectionMode={selectionMode}
                  onSelect={() => toggleSelect(image.id)}
                  onClick={() => openViewer(image)}
                  onLongPress={() => handleLongPress(image.id)}
                />
              ))}
            </div>

            <div className="mt-8 flex flex-col items-center justify-center gap-3">
              {afterId !== null ? (
                <Button
                  onClick={loadMore}
                  disabled={loading}
                  className="ios-btn bg-primary text-primary-foreground min-w-32"
                >
                  {loading ? (
                    <Loader2 className="h-4 w-4 animate-spin" />
                  ) : (
                    '加载更多'
                  )}
                </Button>
              ) : (
                <p className="text-sm text-muted-foreground">已加载全部图片</p>
              )}
              {error && <p className="text-sm text-destructive">{error}</p>}
            </div>
          </>
        )}
      </main>

      <Footer />

      {(selectionMode || selectedIds.size > 0) && (
        <BatchToolbar
          images={images}
          selectedIds={selectedIds}
          onSelectAll={handleSelectAll}
          onClear={handleClear}
          onBatchShare={handleBatchShare}
          onBatchDownload={handleBatchDownload}
          onBatchCopy={handleBatchCopy}
        />
      )}

      <ImageViewer
        images={images}
        currentIndex={viewerIndex}
        open={viewerOpen}
        onClose={() => {
          setViewerOpen(false);
          // 关闭预览后，移除 URL 参数
          if (urlImageId) {
            const next = new URLSearchParams(searchParams);
            next.delete('id');
            setSearchParams(next, { replace: true });
          }
        }}
        onIndexChange={handleViewerIndexChange}
        onStartSlideshow={startSlideshow}
      />

      <Slideshow
        images={images}
        startIndex={slideshowIndex}
        open={slideshowOpen}
        onClose={() => setSlideshowOpen(false)}
      />
    </div>
  );
};

export default GalleryPage;
