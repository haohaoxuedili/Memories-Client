import React, { useState } from 'react';
import { Check, ImageOff } from 'lucide-react';
import { motion } from 'motion/react';
import { useLongPress } from '@/hooks/useLongPress';
import type { ImageItem } from '@/types';

interface ImageCardProps {
  image: ImageItem;
  selected: boolean;
  selectionMode: boolean;
  onSelect: (selected: boolean) => void;
  onClick: () => void;
  onLongPress: () => void;
  index: number;
}

const ImageCard: React.FC<ImageCardProps> = ({
  image,
  selected,
  selectionMode,
  onSelect,
  onClick,
  onLongPress,
  index,
}) => {
  const [loaded, setLoaded] = useState(false);
  const [error, setError] = useState(false);

  const longPressProps = useLongPress({
    onLongPress,
    onClick: selectionMode ? () => onSelect(!selected) : onClick,
    delay: 500,
  });

  return (
    <motion.div
      initial={{ opacity: 0, y: 24 }}
      animate={{ opacity: 1, y: 0 }}
      transition={{ duration: 0.4, delay: (index % 10) * 0.05, ease: [0.25, 0.46, 0.45, 0.94] }}
      className={`group relative overflow-hidden rounded-2xl bg-card border transition-all duration-300 tap-highlight-transparent ${
        selected
          ? 'ring-2 ring-primary shadow-lg scale-[1.02]'
          : 'border-border/40 shadow-sm hover:shadow-lg hover:scale-[1.01]'
      }`}
    >
      <button
        type="button"
        {...longPressProps}
        className="block w-full h-full focus:outline-none focus-visible:ring-2 focus-visible:ring-primary rounded-2xl select-none"
        aria-label={selectionMode ? (selected ? '取消选择' : '选择图片') : `查看图片 ${image.id}`}
        aria-pressed={selectionMode ? selected : undefined}
      >
        <div className="aspect-square w-full overflow-hidden bg-muted relative">
          {!error ? (
            <img
              src={image.url}
              alt={`图片 ${image.id}`}
              loading="lazy"
              draggable={false}
              onLoad={() => setLoaded(true)}
              onError={() => setError(true)}
              className={`h-full w-full object-cover transition-transform duration-500 group-hover:scale-105 ${
                loaded ? 'opacity-100' : 'opacity-0'
              }`}
            />
          ) : (
            <div className="flex h-full w-full flex-col items-center justify-center text-muted-foreground px-4 text-center">
              <ImageOff className="h-8 w-8 mb-2" />
              <span className="text-xs">加载失败</span>
              <span className="text-[10px] mt-1 opacity-70">轻触重试</span>
            </div>
          )}

          {/* 骨架屏占位 */}
          {!loaded && !error && (
            <div className="absolute inset-0 overflow-hidden bg-muted/80">
              <div className="absolute inset-0 -translate-x-full animate-[shimmer_1.5s_infinite] bg-gradient-to-r from-transparent via-white/20 to-transparent" />
            </div>
          )}
        </div>
      </button>

      {/* 选择指示器 */}
      <div
        className={`absolute top-3 right-3 h-6 w-6 rounded-full border-2 flex items-center justify-center transition-all ${
          selected
            ? 'bg-primary border-primary text-primary-foreground scale-100'
            : selectionMode
              ? 'bg-white/80 border-white/80 scale-100'
              : 'scale-0 opacity-0'
        }`}
        aria-hidden="true"
      >
        <Check className="h-3.5 w-3.5" />
      </div>
    </motion.div>
  );
};

export default ImageCard;
