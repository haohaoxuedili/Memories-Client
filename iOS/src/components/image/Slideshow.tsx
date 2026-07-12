import React, { useCallback, useEffect, useRef, useState } from 'react';
import { ChevronLeft, ChevronRight, Maximize2, Pause, Play, X } from 'lucide-react';
import { motion, AnimatePresence } from 'motion/react';
import type { ImageItem } from '@/types';

interface SlideshowProps {
  images: ImageItem[];
  startIndex: number;
  open: boolean;
  onClose: () => void;
}

const INTERVAL = 4000;
const MAX_DOTS = 9; // 超出时用 ... 省略

const Slideshow: React.FC<SlideshowProps> = ({ images, startIndex, open, onClose }) => {
  const [currentIndex, setCurrentIndex] = useState(startIndex);
  const [playing, setPlaying] = useState(true);
  const [direction, setDirection] = useState<'left' | 'right'>('left');
  const [progress, setProgress] = useState(0);
  const progressRef = useRef<ReturnType<typeof setInterval> | null>(null);
  const touchStartX = useRef<number | null>(null);
  const touchStartY = useRef<number | null>(null);

  useEffect(() => {
    if (open) {
      setCurrentIndex(startIndex);
      setPlaying(true);
      setProgress(0);
    }
  }, [open, startIndex]);

  const goNext = useCallback(() => {
    setDirection('left');
    setCurrentIndex((prev) => (prev + 1) % images.length);
    setProgress(0);
  }, [images.length]);

  const goPrev = useCallback(() => {
    setDirection('right');
    setCurrentIndex((prev) => (prev - 1 + images.length) % images.length);
    setProgress(0);
  }, [images.length]);

  const goTo = useCallback((index: number) => {
    setDirection(index > currentIndex ? 'left' : 'right');
    setCurrentIndex(index);
    setProgress(0);
    setPlaying(true);
  }, [currentIndex]);

  // 自动播放计时器
  useEffect(() => {
    if (!playing || !open) return;
    const timer = setInterval(goNext, INTERVAL);
    return () => clearInterval(timer);
  }, [playing, open, goNext]);

  // 进度条动画
  useEffect(() => {
    if (progressRef.current) clearInterval(progressRef.current);
    setProgress(0);
    if (!playing || !open) return;
    const step = 100 / (INTERVAL / 50);
    progressRef.current = setInterval(() => {
      setProgress((p) => {
        if (p >= 100) {
          clearInterval(progressRef.current!);
          return 100;
        }
        return p + step;
      });
    }, 50);
    return () => {
      if (progressRef.current) clearInterval(progressRef.current);
    };
  }, [playing, open, currentIndex]);

  // 键盘
  useEffect(() => {
    if (!open) return;
    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.key === 'ArrowLeft') { setPlaying(false); goPrev(); }
      else if (e.key === 'ArrowRight') { setPlaying(false); goNext(); }
      else if (e.key === ' ' || e.key === 'k') { e.preventDefault(); setPlaying((p) => !p); }
      else if (e.key === 'Escape') onClose();
    };
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [open, goNext, goPrev, onClose]);

  if (!open || images.length === 0) return null;

  const image = images[currentIndex];

  // 构建底部指示点列表
  const renderDots = () => {
    const total = images.length;
    if (total <= MAX_DOTS) {
      return images.map((_, i) => (
        <button
          key={i}
          type="button"
          onClick={() => goTo(i)}
          aria-label={`跳转到第 ${i + 1} 张`}
          className={`rounded-full transition-all duration-300 ${
            i === currentIndex
              ? 'w-5 h-2 bg-white'
              : 'w-2 h-2 bg-white/40 hover:bg-white/70'
          }`}
        />
      ));
    }
    // 超出 MAX_DOTS 时只显示当前附近的点
    const half = Math.floor(MAX_DOTS / 2);
    let start = Math.max(0, currentIndex - half);
    const end = Math.min(total - 1, start + MAX_DOTS - 1);
    if (end - start < MAX_DOTS - 1) start = Math.max(0, end - MAX_DOTS + 1);
    return Array.from({ length: end - start + 1 }, (_, idx) => {
      const i = start + idx;
      return (
        <button
          key={i}
          type="button"
          onClick={() => goTo(i)}
          aria-label={`跳转到第 ${i + 1} 张`}
          className={`rounded-full transition-all duration-300 ${
            i === currentIndex
              ? 'w-5 h-2 bg-white'
              : 'w-2 h-2 bg-white/40 hover:bg-white/70'
          }`}
        />
      );
    });
  };

  return (
    <motion.div
      className="fixed inset-0 z-[100] flex flex-col bg-black"
      initial={{ opacity: 0 }}
      animate={{ opacity: 1 }}
      exit={{ opacity: 0 }}
      transition={{ duration: 0.25 }}
      onTouchStart={(e) => {
        touchStartX.current = e.touches[0].clientX;
        touchStartY.current = e.touches[0].clientY;
      }}
      onTouchEnd={(e) => {
        if (touchStartX.current === null || touchStartY.current === null) return;
        const dx = e.changedTouches[0].clientX - touchStartX.current;
        const dy = e.changedTouches[0].clientY - touchStartY.current;
        if (Math.abs(dx) > Math.abs(dy) && Math.abs(dx) > 40) {
          setPlaying(false);
          if (dx < 0) goNext(); else goPrev();
        }
        touchStartX.current = null;
        touchStartY.current = null;
      }}
    >
      {/* 背景：图片模糊虚化 */}
      <div
        className="absolute inset-0 scale-110 bg-cover bg-center opacity-20 blur-2xl pointer-events-none"
        style={{ backgroundImage: `url(${image.url})` }}
      />
      <div className="absolute inset-0 bg-black/50 pointer-events-none" />

      {/* 顶部进度条 */}
      <div className="absolute top-0 left-0 right-0 z-20 h-0.5 bg-white/15">
        <motion.div
          className="h-full bg-white/70"
          style={{ width: `${progress}%` }}
          transition={{ duration: 0 }}
        />
      </div>

      {/* 顶部控制栏 */}
      <div className="absolute top-0 left-0 right-0 z-10 flex items-center justify-between px-4 pt-4 pb-8 bg-gradient-to-b from-black/70 to-transparent">
        <div className="flex items-center gap-3 text-white/90">
          <span className="text-sm font-semibold tabular-nums">{currentIndex + 1}</span>
          <span className="text-white/40 text-xs">/</span>
          <span className="text-sm text-white/50 tabular-nums">{images.length}</span>
        </div>
        <div className="flex items-center gap-2">
          <button
            type="button"
            onClick={() => setPlaying((p) => !p)}
            className="h-10 w-10 rounded-full bg-white/10 backdrop-blur-md text-white flex items-center justify-center hover:bg-white/20 active:scale-95 transition-all"
            aria-label={playing ? '暂停' : '播放'}
          >
            {playing
              ? <Pause className="h-4 w-4" />
              : <Play className="h-4 w-4 ml-0.5" />
            }
          </button>
          <button
            type="button"
            onClick={onClose}
            className="h-10 w-10 rounded-full bg-white/10 backdrop-blur-md text-white flex items-center justify-center hover:bg-white/20 active:scale-95 transition-all"
            aria-label="退出幻灯片"
          >
            <X className="h-5 w-5" />
          </button>
        </div>
      </div>

      {/* 图片主区域 */}
      <div className="relative flex-1 overflow-hidden">
        <AnimatePresence initial={false} mode="wait">
          <motion.div
            key={image.id}
            initial={{
              opacity: 0,
              scale: 1.04,
              x: direction === 'left' ? 40 : -40,
            }}
            animate={{ opacity: 1, scale: 1, x: 0 }}
            exit={{
              opacity: 0,
              scale: 0.97,
              x: direction === 'left' ? -40 : 40,
            }}
            transition={{ duration: 0.4, ease: [0.25, 0.46, 0.45, 0.94] }}
            className="absolute inset-0 flex items-center justify-center p-6 md:p-12"
          >
            <img
              src={image.url}
              alt={`幻灯片 ${image.id}`}
              draggable={false}
              className="max-h-full max-w-full object-contain rounded-xl shadow-[0_24px_64px_-16px_rgba(0,0,0,0.8)] select-none"
            />
          </motion.div>
        </AnimatePresence>

        {/* 侧边切换按钮 */}
        <button
          type="button"
          onClick={() => { setPlaying(false); goPrev(); }}
          className="absolute left-2 md:left-5 top-1/2 -translate-y-1/2 h-12 w-12 md:h-14 md:w-14 rounded-full bg-white/10 backdrop-blur-md text-white flex items-center justify-center hover:bg-white/20 active:scale-95 transition-all"
          aria-label="上一张"
        >
          <ChevronLeft className="h-7 w-7" />
        </button>
        <button
          type="button"
          onClick={() => { setPlaying(false); goNext(); }}
          className="absolute right-2 md:right-5 top-1/2 -translate-y-1/2 h-12 w-12 md:h-14 md:w-14 rounded-full bg-white/10 backdrop-blur-md text-white flex items-center justify-center hover:bg-white/20 active:scale-95 transition-all"
          aria-label="下一张"
        >
          <ChevronRight className="h-7 w-7" />
        </button>
      </div>

      {/* 底部控制栏 */}
      <div className="absolute bottom-0 left-0 right-0 z-10 flex flex-col items-center gap-4 px-4 pt-10 pb-safe-or-6 pb-6 bg-gradient-to-t from-black/80 to-transparent">
        {/* 指示点 */}
        <div className="flex items-center gap-1.5">
          {renderDots()}
        </div>

        {/* 大播放按钮 */}
        <button
          type="button"
          onClick={() => setPlaying((p) => !p)}
          className="h-14 w-14 rounded-full flex items-center justify-center active:scale-95 transition-all shadow-lg"
          style={{ background: 'rgba(255,255,255,0.92)' }}
          aria-label={playing ? '暂停' : '播放'}
        >
          {playing
            ? <Pause className="h-6 w-6 text-black" />
            : <Play className="h-6 w-6 ml-0.5 text-black" />
          }
        </button>

        {/* 键盘提示 */}
        <p className="hidden md:block text-xs text-white/30 pb-1">
          ← → 切换 &nbsp;|&nbsp; 空格 暂停/播放 &nbsp;|&nbsp; Esc 退出
        </p>
      </div>
    </motion.div>
  );
};

export default Slideshow;
