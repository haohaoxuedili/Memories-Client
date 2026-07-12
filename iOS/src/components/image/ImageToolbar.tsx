import React from 'react';
import {
  Copy,
  Download,
  FlipHorizontal,
  FlipVertical,
  Maximize2,
  Minimize2,
  RefreshCw,
  RotateCw,
  Share2,
} from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Tooltip, TooltipContent, TooltipProvider, TooltipTrigger } from '@/components/ui/tooltip';

interface ImageToolbarProps {
  onRotate: () => void;
  onFlipVertical: () => void;
  onFlipHorizontal: () => void;
  onZoomIn: () => void;
  onZoomOut: () => void;
  onReset: () => void;
  onShare: () => void;
  onDownload: () => void;
  onCopyUrl: () => void;
  scale: number;
  light?: boolean;
}

const ImageToolbar: React.FC<ImageToolbarProps> = ({
  onRotate,
  onFlipVertical,
  onFlipHorizontal,
  onZoomIn,
  onZoomOut,
  onReset,
  onShare,
  onDownload,
  onCopyUrl,
  scale,
  light = true,
}) => {
  const iconColor = light ? 'text-foreground' : 'text-white';
  const hoverBg = light ? 'hover:bg-muted' : 'hover:bg-white/10';

  const tools = [
    { icon: RotateCw, label: '旋转', onClick: onRotate },
    { icon: FlipVertical, label: '上下翻转', onClick: onFlipVertical },
    { icon: FlipHorizontal, label: '右翻转', onClick: onFlipHorizontal },
    { icon: Maximize2, label: '放大', onClick: onZoomIn },
    { icon: Minimize2, label: '缩小', onClick: onZoomOut },
    { icon: RefreshCw, label: '还原', onClick: onReset },
    { icon: Share2, label: '分享', onClick: onShare },
    { icon: Download, label: '下载', onClick: onDownload },
    { icon: Copy, label: '复制链接', onClick: onCopyUrl },
  ];

  return (
    <TooltipProvider>
      <div className={`flex flex-wrap items-center justify-center gap-1.5 rounded-2xl ${light ? 'bg-muted/60' : 'bg-white/10'} backdrop-blur-md px-2.5 py-2 border ${light ? 'border-border/30' : 'border-white/10'}`}>
        {tools.slice(0, 6).map((tool) => (
          <Tooltip key={tool.label}>
            <TooltipTrigger asChild>
              <Button
                variant="ghost"
                size="icon"
                onClick={tool.onClick}
                className={`rounded-full h-11 w-11 ${iconColor} ${hoverBg}`}
              >
                <tool.icon className="h-5 w-5" />
              </Button>
            </TooltipTrigger>
            <TooltipContent side="top">
              <span>{tool.label}</span>
            </TooltipContent>
          </Tooltip>
        ))}
        <div className={`mx-1 h-6 w-px ${light ? 'bg-border/60' : 'bg-white/20'}`} />
        <div className={`rounded-full px-3 py-1.5 text-xs font-semibold ${iconColor}`}>
          {Math.round(scale * 100)}%
        </div>
        {tools.slice(6).map((tool) => (
          <Tooltip key={tool.label}>
            <TooltipTrigger asChild>
              <Button
                variant="ghost"
                size="icon"
                onClick={tool.onClick}
                className={`rounded-full h-11 w-11 ${iconColor} ${hoverBg}`}
              >
                <tool.icon className="h-5 w-5" />
              </Button>
            </TooltipTrigger>
            <TooltipContent side="top">
              <span>{tool.label}</span>
            </TooltipContent>
          </Tooltip>
        ))}
      </div>
    </TooltipProvider>
  );
};

export default ImageToolbar;
