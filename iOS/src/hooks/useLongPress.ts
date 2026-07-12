import { useCallback, useRef } from 'react';

interface UseLongPressOptions {
  onLongPress: () => void;
  onClick?: () => void;
  delay?: number;
  moveThreshold?: number;
}

export function useLongPress({ onLongPress, onClick, delay = 500, moveThreshold = 8 }: UseLongPressOptions) {
  const timerRef = useRef<ReturnType<typeof setTimeout> | null>(null);
  const isLongPressRef = useRef(false);
  const hasMovedRef = useRef(false);
  const touchHandledRef = useRef(false);
  const clearTouchHandledTimerRef = useRef<ReturnType<typeof setTimeout> | null>(null);
  const touchStartRef = useRef<{ x: number; y: number } | null>(null);

  const start = useCallback(
    (x: number, y: number) => {
      isLongPressRef.current = false;
      hasMovedRef.current = false;
      touchStartRef.current = { x, y };
      timerRef.current = setTimeout(() => {
        isLongPressRef.current = true;
        hasMovedRef.current = false;
        // 提供触觉反馈，增强长按感知
        if (typeof navigator !== 'undefined' && navigator.vibrate) {
          try {
            navigator.vibrate(40);
          } catch {
            // 忽略不支持或权限导致的错误
          }
        }
        onLongPress();
      }, delay);
    },
    [onLongPress, delay]
  );

  const cancel = useCallback(() => {
    if (timerRef.current) {
      clearTimeout(timerRef.current);
      timerRef.current = null;
    }
  }, []);

  const markTouchHandled = useCallback(() => {
    touchHandledRef.current = true;
    if (clearTouchHandledTimerRef.current) {
      clearTimeout(clearTouchHandledTimerRef.current);
    }
    clearTouchHandledTimerRef.current = setTimeout(() => {
      touchHandledRef.current = false;
    }, 100);
  }, []);

  const handleTouchStart = useCallback(
    (e: React.TouchEvent) => {
      const touch = e.touches[0];
      start(touch.clientX, touch.clientY);
    },
    [start]
  );

  const handleTouchMove = useCallback(
    (e: React.TouchEvent) => {
      if (!touchStartRef.current) return;
      const touch = e.touches[0];
      const dx = touch.clientX - touchStartRef.current.x;
      const dy = touch.clientY - touchStartRef.current.y;
      // 移动超过阈值时取消长按并标记为滑动，避免 touchend 误触发 click
      if (Math.sqrt(dx * dx + dy * dy) > moveThreshold) {
        hasMovedRef.current = true;
        cancel();
      }
    },
    [cancel, moveThreshold]
  );

  const handleTouchEnd = useCallback(() => {
    cancel();
    if (!isLongPressRef.current && !hasMovedRef.current) {
      onClick?.();
    }
    markTouchHandled();
    isLongPressRef.current = false;
    hasMovedRef.current = false;
    touchStartRef.current = null;
  }, [cancel, onClick, markTouchHandled]);

  const handleMouseDown = useCallback(
    (e: React.MouseEvent) => {
      // 忽略由触摸事件合成的鼠标事件
      if (touchHandledRef.current) return;
      start(e.clientX, e.clientY);
    },
    [start]
  );

  const handleMouseUp = useCallback(() => {
    // 忽略由触摸事件合成的鼠标事件
    if (touchHandledRef.current) return;
    cancel();
    if (!isLongPressRef.current && !hasMovedRef.current) {
      onClick?.();
    }
    isLongPressRef.current = false;
    hasMovedRef.current = false;
    touchStartRef.current = null;
  }, [cancel, onClick]);

  const handleContextMenu = useCallback((e: React.MouseEvent | React.TouchEvent) => {
    e.preventDefault();
  }, []);

  return {
    onTouchStart: handleTouchStart,
    onTouchMove: handleTouchMove,
    onTouchEnd: handleTouchEnd,
    onMouseDown: handleMouseDown,
    onMouseUp: handleMouseUp,
    onContextMenu: handleContextMenu,
  };
}
