import React from 'react';
import { CheckSquare, Copy, Download, Share2, Square } from 'lucide-react';
import { Button } from '@/components/ui/button';
import type { ImageItem } from '@/types';

interface BatchToolbarProps {
  images: ImageItem[];
  selectedIds: Set<number>;
  onSelectAll: () => void;
  onClear: () => void;
  onBatchShare: () => void;
  onBatchDownload: () => void;
  onBatchCopy: () => void;
}

const BatchToolbar: React.FC<BatchToolbarProps> = ({
  images,
  selectedIds,
  onSelectAll,
  onClear,
  onBatchShare,
  onBatchDownload,
  onBatchCopy,
}) => {
  const allSelected = images.length > 0 && selectedIds.size === images.length;

  return (
    <div className="fixed bottom-6 left-1/2 z-30 w-[calc(100%-2rem)] max-w-2xl -translate-x-1/2">
      <div className="ios-card flex items-center justify-between gap-3 px-3 py-2.5 md:px-4">
        <div className="flex items-center gap-2 min-w-0">
          <Button
            variant="ghost"
            size="sm"
            onClick={allSelected ? onClear : onSelectAll}
            className="rounded-full text-foreground hover:bg-muted shrink-0 h-10 px-3"
          >
            {allSelected ? <Square className="h-5 w-5 md:mr-1.5" /> : <CheckSquare className="h-5 w-5 md:mr-1.5" />}
            <span className="hidden md:inline">{allSelected ? '取消全选' : '全选'}</span>
          </Button>
          <span className="text-xs md:text-sm text-muted-foreground px-1 truncate">
            已选 {selectedIds.size} 张
          </span>
        </div>

        <div className="flex items-center gap-2 shrink-0">
          <BatchButton icon={Share2} label="分享" onClick={onBatchShare} disabled={selectedIds.size === 0} />
          <BatchButton icon={Download} label="下载" onClick={onBatchDownload} disabled={selectedIds.size === 0} />
          <BatchButton icon={Copy} label="复制" onClick={onBatchCopy} disabled={selectedIds.size === 0} />
        </div>
      </div>
    </div>
  );
};

const BatchButton: React.FC<{
  icon: React.ElementType;
  label: string;
  onClick: () => void;
  disabled?: boolean;
}> = ({ icon: Icon, label, onClick, disabled }) => (
  <Button
    variant="ghost"
    size="sm"
    onClick={onClick}
    disabled={disabled}
    className="rounded-full text-foreground hover:bg-muted disabled:opacity-40 h-10 px-3 min-w-10"
  >
    <Icon className="h-5 w-5 sm:mr-1.5" />
    <span className="hidden sm:inline">{label}</span>
  </Button>
);

export default BatchToolbar;
