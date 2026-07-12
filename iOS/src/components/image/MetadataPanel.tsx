import React from 'react';
import { X } from 'lucide-react';
import { Sheet, SheetContent, SheetHeader, SheetTitle } from '@/components/ui/sheet';
import { Badge } from '@/components/ui/badge';
import type { ImageMetadata } from '@/types';

interface MetadataPanelProps {
  metadata: ImageMetadata | null;
  loading: boolean;
  open: boolean;
  onOpenChange: (open: boolean) => void;
}

const MetadataPanel: React.FC<MetadataPanelProps> = ({ metadata, loading, open, onOpenChange }) => {
  return (
    <Sheet open={open} onOpenChange={onOpenChange}>
      <SheetContent side="right" className="bg-sidebar border-border w-full max-w-md p-0">
        <SheetHeader className="text-left px-6 py-4 border-b border-border">
          <SheetTitle className="text-lg font-semibold">图片信息</SheetTitle>
        </SheetHeader>

        <div className="px-6 py-5 space-y-6 overflow-y-auto max-h-[calc(100dvh-5rem)]">
          {loading && (
            <div className="text-sm text-muted-foreground py-8 text-center">正在查询…</div>
          )}

          {!loading && !metadata && (
            <div className="text-sm text-muted-foreground py-8 text-center">暂无信息</div>
          )}

          {!loading && metadata && (
            <>
              <InfoSection title="文件信息">
                <InfoRow label="文件名" value={metadata.filename} />
                <InfoRow label="原始文件名" value={metadata.original_filename} />
                <InfoRow label="大小" value={metadata.size_display} />
                <InfoRow label="上传时间" value={metadata.upload_date} />
                <InfoRow label="存储位置" value={`${metadata.storage_backend} · ${metadata.storage_location}`} />
              </InfoSection>

              <InfoSection title="内容与标签">
                <InfoRow label="画面描述" value={metadata.content_description || '暂无描述'} />
                <div>
                  <span className="text-xs text-muted-foreground block mb-2">标签</span>
                  <div className="flex flex-wrap gap-2">
                    {metadata.tags_array?.length > 0 ? (
                      metadata.tags_array.map((tag) => (
                        <Badge key={tag} variant="secondary" className="rounded-full">
                          {tag}
                        </Badge>
                      ))
                    ) : (
                      <span className="text-sm text-muted-foreground">无标签</span>
                    )}
                  </div>
                </div>
              </InfoSection>

              <InfoSection title="上传者">
                <InfoRow label="IP（打码）" value={metadata.uploader_masked} />
                <InfoRow label="归属地" value={metadata.location} />
              </InfoSection>

              {metadata.image_url && (
                <InfoSection title="访问链接">
                  <a
                    href={metadata.image_url}
                    target="_blank"
                    rel="noreferrer noopener"
                    className="break-words text-sm text-primary hover:underline"
                  >
                    {metadata.image_url}
                  </a>
                </InfoSection>
              )}
            </>
          )}
        </div>
      </SheetContent>
    </Sheet>
  );
};

const InfoSection: React.FC<{ title: string; children: React.ReactNode }> = ({ title, children }) => (
  <section>
    <h3 className="text-sm font-semibold text-foreground mb-3">{title}</h3>
    <div className="space-y-3">{children}</div>
  </section>
);

const InfoRow: React.FC<{ label: string; value: string }> = ({ label, value }) => (
  <div>
    <span className="text-xs text-muted-foreground block mb-1">{label}</span>
    <span className="text-sm text-foreground break-words">{value}</span>
  </div>
);

export default MetadataPanel;
