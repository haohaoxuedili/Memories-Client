import React, { useEffect, useRef, useState } from 'react';
import { ChevronLeft, ChevronRight, Info, Play, X } from 'lucide-react';
import { toast } from 'sonner';
import { motion, AnimatePresence } from 'motion/react';
import { Dialog, DialogContent } from '@/components/ui/dialog';
import { Button } from '@/components/ui/button';
import ImageToolbar from './ImageToolbar';
import MetadataPanel from './MetadataPanel';
import { cacheImage, copyImageUrl, downloadImage, shareImage } from '@/lib/imageCache';
import { fetchImageMetadata } from '@/lib/api';
import type { ImageItem, ImageMetadata, ImageTransform } from '@/types';

interface ImageViewerProps {
  images: ImageItem[];
  currentIndex: number;
  open: boolean;
  onClose: () => void;
  onIndexChange: (index: number) => void;
  onStartSlideshow?: () => void;
}

const initialTransform: ImageTransform = {
  rotate: 0,
  scale: 1,
  flipX: false,
  flipY: false,
  translateX: 0,
  translateY: 0,
};

const ImageViewer: React.FC<ImageViewerProps> = ({
  images,
  currentIndex,
  open,
  onClose,
  onIndexChange,
  onStartSlideshow,
}) => {
  const image = images[currentIndex] ?? null;
  const [transform, setTransform] = useState<ImageTransform>(initialTransform);
  const [isDragging, setIsDragging] = useState(false);
  const [metadataOpen, setMetadataOpen] = useState(false);
  const [metadata, setMetadata] = useState<ImageMetadata | null>(null);
  const [metadataLoading, setMetadataLoading] = useState(false);
  const [swipeDirection, setSwipeDirection] = useState<'left' | 'right' | null>(null);
  const dragStart = useRef({ x: 0, y: 0 });
  const pointersRef = useRef<Map<number, { x: number; y: number }>>(new Map());
  const pinchStartDistRef = useRef(0);
  const pinchStartScaleRef = useRef(1);
  const isPinching = useRef(false);
  const touchStartX = useRef(0);
  const touchStartY = useRef(0);

  useEffect(() => {
    if (image) {
      setTransform(initialTransform);
      setMetadata(null);
      setMetadataOpen(false);
      cacheImage(image.url).catch(() => {});
    }
  }, [image?.id]);

  // 键盘事件：左右切换，ESC 关闭
  useEffect(() => {
    if (!open) return;

    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.key === 'ArrowLeft') {
        goPrev();
      } else if (e.key === 'ArrowRight') {
        goNext();
      } else if (e.key === 'Escape') {
        onClose();
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [open, currentIndex, images.length]);

  const updateTransform = (patch: Partial<ImageTransform>) => {
    setTransform((prev) => ({ ...prev, ...patch }));
  };

  const goPrev = () => {
    if (currentIndex > 0) {
      setSwipeDirection('right');
      onIndexChange(currentIndex - 1);
    }
  };

  const goNext = () => {
    if (currentIndex < images.length - 1) {
      setSwipeDirection('left');
      onIndexChange(currentIndex + 1);
    }
  };

  const handleRotate = () => updateTransform({ rotate: transform.rotate + 90 });
  const handleFlipVertical = () => updateTransform({ flipY: !transform.flipY });
  const handleFlipHorizontal = () => updateTransform({ flipX: !transform.flipX });
  const handleZoomIn = () => updateTransform({ scale: Math.min(transform.scale + 0.25, 4) });
  const handleZoomOut = () => updateTransform({ scale: Math.max(transform.scale - 0.25, 0.5) });
  const handleReset = () => updateTransform(initialTransform);

  const handleMouseDown = (e: React.MouseEvent) => {
    if (transform.scale <= 1) return;
    setIsDragging(true);
    dragStart.current = { x: e.clientX - transform.translateX, y: e.clientY - transform.translateY };
  };

  const handleMouseMove = (e: React.MouseEvent) => {
    if (!isDragging) return;
    updateTransform({
      translateX: e.clientX - dragStart.current.x,
      translateY: e.clientY - dragStart.current.y,
    });
  };

  const handleMouseUp = () => setIsDragging(false);

  const handleTouchStart = (e: React.TouchEvent) => {
    if (e.touches.length === 2) {
      isPinching.current = true;
      pointersRef.current.clear();
      for (let i = 0; i < e.touches.length; i++) {
        const t = e.touches[i];
        pointersRef.current.set(t.identifier, { x: t.clientX, y: t.clientY });
      }
      const dist = getPointersDistance(pointersRef.current);
      pinchStartDistRef.current = dist;
      pinchStartScaleRef.current = transform.scale;
      return;
    }

    if (transform.scale > 1) {
      const touch = e.touches[0];
      setIsDragging(true);
      dragStart.current = {
        x: touch.clientX - transform.translateX,
        y: touch.clientY - transform.translateY,
      };
    } else {
      const touch = e.touches[0];
      touchStartX.current = touch.clientX;
      touchStartY.current = touch.clientY;
    }
  };

  const handleTouchMove = (e: React.TouchEvent) => {
    if (e.touches.length === 2 && isPinching.current) {
      e.preventDefault();
      pointersRef.current.clear();
      for (let i = 0; i < e.touches.length; i++) {
        const t = e.touches[i];
        pointersRef.current.set(t.identifier, { x: t.clientX, y: t.clientY });
      }
      const dist = getPointersDistance(pointersRef.current);
      if (pinchStartDistRef.current > 0 && dist > 0) {
        const ratio = dist / pinchStartDistRef.current;
        updateTransform({ scale: Math.min(Math.max(pinchStartScaleRef.current * ratio, 0.5), 4) });
      }
      return;
    }

    if (isDragging && transform.scale > 1) {
      const touch = e.touches[0];
      updateTransform({
        translateX: touch.clientX - dragStart.current.x,
        translateY: touch.clientY - dragStart.current.y,
      });
    }
  };

  const handleTouchEnd = (e: React.TouchEvent) => {
    if (isPinching.current) {
      if (e.touches.length < 2) {
        isPinching.current = false;
        pointersRef.current.clear();
      }
      return;
    }

    setIsDragging(false);

    if (transform.scale <= 1 && e.changedTouches.length > 0) {
      const touch = e.changedTouches[0];
      const dx = touch.clientX - touchStartX.current;
      const dy = touch.clientY - touchStartY.current;
      // 水平位移明显大于垂直位移，且超过阈值，才切换图片
      if (Math.abs(dx) > Math.abs(dy) && Math.abs(dx) > 60) {
        if (dx > 0) {
          goPrev();
        } else {
          goNext();
        }
      }
    }
  };

  const getFilenameFromUrl = (url: string): string => {
    try {
      const pathname = new URL(url).pathname;
      const basename = pathname.split('/').pop();
      return basename && basename.includes('.') ? basename : pathname;
    } catch {
      return url;
    }
  };

  const handleOpenMetadata = async () => {
    if (!image) return;
    setMetadataOpen(true);
    if (metadata?.id === image.id) return;

    const query = getFilenameFromUrl(image.url);
    setMetadataLoading(true);
    try {
      const result = await fetchImageMetadata(query);
      if (result.data) {
        setMetadata(result.data);
      }
    } catch (err) {
      toast.error(err instanceof Error ? err.message : '查询图片信息失败');
    } finally {
      setMetadataLoading(false);
    }
  };

  if (!image) return null;

  const handleShare = async () => {
    const ok = await shareImage(image.url, `图片 #${image.id}`);
    if (ok) {
      toast.success('已唤起系统分享');
    }
  };

  const handleDownload = async () => {
    await downloadImage(image.url, `image-${image.id}`);
    toast.success('已开始下载');
  };

  const handleCopyUrl = async () => {
    const ok = await copyImageUrl(image.url);
    if (ok) {
      toast.success('图片链接已复制');
    } else {
      toast.error('复制失败');
    }
  };

  const imageStyle: React.CSSProperties = {
    transform: `
      translate(${transform.translateX}px, ${transform.translateY}px)
      scale(${transform.scale})
      rotate(${transform.rotate}deg)
      scaleX(${transform.flipX ? -1 : 1})
      scaleY(${transform.flipY ? -1 : 1})
    `,
    transition: isDragging || isPinching.current ? 'none' : 'transform 300ms cubic-bezier(0.25, 0.46, 0.45, 0.94)',
    cursor: transform.scale > 1 ? (isDragging ? 'grabbing' : 'grab') : 'default',
    maxHeight: '100%',
    maxWidth: '100%',
    objectFit: 'contain',
  };

  const canGoPrev = currentIndex > 0;
  const canGoNext = currentIndex < images.length - 1;

  return (
    <Dialog open={open} onOpenChange={(open) => !open && onClose()}>
      <DialogContent className="max-w-[calc(100%-2rem)] md:max-w-6xl h-[92dvh] md:h-[90dvh] p-0 overflow-hidden border-none bg-black/92 backdrop-blur-3xl flex flex-col shadow-2xl [&>button]:hidden">
        {/* 顶部工具栏 */}
        <div className="flex items-center justify-between px-3 md:px-4 py-3 border-b border-white/10 shrink-0">
          <div className="flex items-center gap-2 text-white/90">
            <span className="text-sm font-medium">{currentIndex + 1}</span>
            <span className="text-white/40">/</span>
            <span className="text-sm text-white/60">{images.length}</span>
          </div>
          <div className="flex items-center gap-1">
            {onStartSlideshow && (
              <Button
                variant="ghost"
                size="icon"
                onClick={onStartSlideshow}
                className="rounded-full h-11 w-11 text-white/80 hover:bg-white/10 hover:text-white"
                aria-label="幻灯片播放"
              >
                <Play className="h-5 w-5" />
              </Button>
            )}
            <Button
              variant="ghost"
              size="icon"
              onClick={handleOpenMetadata}
              className="rounded-full h-11 w-11 text-white/80 hover:bg-white/10 hover:text-white"
              aria-label="查看图片信息"
            >
              <Info className="h-5 w-5" />
            </Button>
            <Button
              variant="ghost"
              size="icon"
              onClick={onClose}
              className="rounded-full h-11 w-11 text-white/80 hover:bg-white/10 hover:text-white"
              aria-label="关闭"
            >
              <X className="h-5 w-5" />
            </Button>
          </div>
        </div>

        {/* 图片区域 */}
        <div className="relative flex-1 overflow-hidden">
          <AnimatePresence initial={false} mode="wait">
            <motion.div
              key={image.id}
              initial={{
                opacity: 0,
                x: swipeDirection === 'left' ? 80 : swipeDirection === 'right' ? -80 : 0,
              }}
              animate={{ opacity: 1, x: 0 }}
              exit={{
                opacity: 0,
                x: swipeDirection === 'left' ? -80 : swipeDirection === 'right' ? 80 : 0,
              }}
              transition={{ duration: 0.25, ease: 'easeOut' }}
              className="absolute inset-0 flex items-center justify-center p-3 md:p-4"
              onMouseMove={handleMouseMove}
              onMouseUp={handleMouseUp}
              onMouseLeave={handleMouseUp}
            >
              <img
                src={image.url}
                alt={`图片 ${image.id}`}
                draggable={false}
                style={imageStyle}
                onMouseDown={handleMouseDown}
                onTouchStart={handleTouchStart}
                onTouchMove={handleTouchMove}
                onTouchEnd={handleTouchEnd}
                className="select-none rounded-lg shadow-2xl max-h-full max-w-full"
              />
            </motion.div>
          </AnimatePresence>

          {/* 左右切换按钮 */}
          {canGoPrev && (
            <button
              type="button"
              onClick={goPrev}
              className="absolute left-1 md:left-4 top-1/2 -translate-y-1/2 h-12 w-12 md:h-14 md:w-14 rounded-full bg-white/10 backdrop-blur-md text-white flex items-center justify-center hover:bg-white/20 active:scale-95 transition-all"
              aria-label="上一张"
            >
              <ChevronLeft className="h-7 w-7" />
            </button>
          )}
          {canGoNext && (
            <button
              type="button"
              onClick={goNext}
              className="absolute right-1 md:right-4 top-1/2 -translate-y-1/2 h-12 w-12 md:h-14 md:w-14 rounded-full bg-white/10 backdrop-blur-md text-white flex items-center justify-center hover:bg-white/20 active:scale-95 transition-all"
              aria-label="下一张"
            >
              <ChevronRight className="h-7 w-7" />
            </button>
          )}
        </div>

        {/* 底部工具栏 */}
        <div className="shrink-0 p-3 md:p-4 border-t border-white/10 bg-black/40 backdrop-blur-md">
          <ImageToolbar
            onRotate={handleRotate}
            onFlipVertical={handleFlipVertical}
            onFlipHorizontal={handleFlipHorizontal}
            onZoomIn={handleZoomIn}
            onZoomOut={handleZoomOut}
            onReset={handleReset}
            onShare={handleShare}
            onDownload={handleDownload}
            onCopyUrl={handleCopyUrl}
            scale={transform.scale}
            light={false}
          />
        </div>

        <MetadataPanel
          metadata={metadata}
          loading={metadataLoading}
          open={metadataOpen}
          onOpenChange={setMetadataOpen}
        />
      </DialogContent>
    </Dialog>
  );
};

function getPointersDistance(pointers: Map<number, { x: number; y: number }>): number {
  const points = Array.from(pointers.values());
  if (points.length < 2) return 0;
  const dx = points[0].x - points[1].x;
  const dy = points[0].y - points[1].y;
  return Math.sqrt(dx * dx + dy * dy);
}

export default ImageViewer;
