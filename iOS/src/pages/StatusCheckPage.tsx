import React, { useEffect, useRef, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { motion, AnimatePresence } from 'motion/react';
import { Activity, Globe, RefreshCw, Wifi, Zap } from 'lucide-react';
import { checkHealth, fetchNetworkInfo } from '@/lib/api';
import { Button } from '@/components/ui/button';
import type { NetworkInfo } from '@/types';

const StatusCheckPage: React.FC = () => {
  const navigate = useNavigate();
  const [status, setStatus] = useState<'checking' | 'success' | 'error'>('checking');
  const [info, setInfo] = useState<NetworkInfo>({ latency: null, speed: null, ip: null });
  const [message, setMessage] = useState('正在检查服务状态…');
  const abortRef = useRef(false);

  const runCheck = async () => {
    if (abortRef.current) return;
    setStatus('checking');
    setMessage('正在测量网络延迟…');

    const [isHealthy, networkInfo] = await Promise.all([
      checkHealth(),
      fetchNetworkInfo(),
    ]);

    if (abortRef.current) return;
    setInfo(networkInfo);

    if (isHealthy) {
      setStatus('success');
      setMessage('服务连接成功');
      setTimeout(() => {
        if (!abortRef.current) {
          navigate('/gallery', { replace: true });
        }
      }, 1400);
    } else {
      setStatus('error');
      setMessage('无法连接到服务，请检查网络后重试');
    }
  };

  useEffect(() => {
    runCheck();
    return () => {
      abortRef.current = true;
    };
  }, []);

  return (
    <div className="min-h-screen w-full flex flex-col items-center justify-center px-6 py-12 bg-background relative overflow-hidden">
      {/* 背景装饰 */}
      <div className="absolute inset-0 pointer-events-none overflow-hidden">
        <div
          className="absolute -top-1/4 -right-1/4 h-[70vh] w-[70vh] rounded-full blur-3xl opacity-40"
          style={{ background: 'var(--gradient-accent)' }}
        />
        <div
          className="absolute -bottom-1/4 -left-1/4 h-[70vh] w-[70vh] rounded-full blur-3xl opacity-30"
          style={{ background: 'var(--gradient-primary)' }}
        />
      </div>

      <motion.div
        initial={{ opacity: 0, y: 24, scale: 0.96 }}
        animate={{ opacity: 1, y: 0, scale: 1 }}
        transition={{ duration: 0.6, ease: [0.25, 0.46, 0.45, 0.94] }}
        className="w-full max-w-md ios-card p-8 md:p-10 text-center relative z-10"
      >
        <div className="relative mx-auto mb-8 h-24 w-24">
          <div className="absolute inset-0 rounded-full bg-gradient-to-tr from-primary/30 to-accent/30 animate-ping" />
          <div className="absolute inset-2 rounded-full bg-primary/10 animate-pulse" />
          <div
            className={`relative flex h-full w-full items-center justify-center rounded-full shadow-xl transition-colors duration-500 ${
              status === 'error'
                ? 'bg-destructive text-destructive-foreground'
                : 'text-primary-foreground'
            }`}
            style={status === 'error' ? undefined : { background: 'var(--gradient-primary)' }}
          >
            <AnimatePresence mode="wait">
              {status === 'error' ? (
                <motion.div key="error" initial={{ scale: 0.6 }} animate={{ scale: 1 }} exit={{ scale: 0.6 }}>
                  <Activity className="h-10 w-10" />
                </motion.div>
              ) : status === 'success' ? (
                <motion.div key="success" initial={{ scale: 0.6 }} animate={{ scale: 1 }} exit={{ scale: 0.6 }}>
                  <Zap className="h-10 w-10" />
                </motion.div>
              ) : (
                <motion.div key="checking" initial={{ rotate: -90 }} animate={{ rotate: 0 }}>
                  <RefreshCw className="h-10 w-10 animate-spin" />
                </motion.div>
              )}
            </AnimatePresence>
          </div>
        </div>

        <h1 className="text-2xl md:text-3xl font-bold tracking-tight text-foreground mb-2">
          {status === 'success' ? '连接成功' : status === 'error' ? '连接失败' : '检查服务状态'}
        </h1>
        <p className="text-muted-foreground mb-8">{message}</p>

        <div className="space-y-3 mb-8">
          <InfoRow
            icon={<Zap className="h-5 w-5" />}
            label="网络延迟"
            value={info.latency !== null ? `${info.latency} ms` : '检测中'}
            highlight={info.latency !== null && info.latency < 200}
          />
          <InfoRow
            icon={<Wifi className="h-5 w-5" />}
            label="连接网速"
            value={info.speed !== null ? `${info.speed} kbps` : '检测中'}
          />
          <InfoRow
            icon={<Globe className="h-5 w-5" />}
            label="当前 IP"
            value={info.ip ?? '检测中'}
            monospace
          />
        </div>

        {status === 'error' && (
          <Button onClick={runCheck} className="ios-btn w-full bg-primary text-primary-foreground">
            <RefreshCw className="h-4 w-4 mr-2" />
            重新检查
          </Button>
        )}

        {status === 'checking' && (
          <div className="mt-4 h-1.5 w-full rounded-full bg-muted overflow-hidden">
            <div
              className="h-full w-1/2 rounded-full animate-[shimmer_1.5s_infinite]"
              style={{ background: 'var(--gradient-primary)' }}
            />
          </div>
        )}
      </motion.div>

      <footer className="mt-12 text-sm text-muted-foreground relative z-10">
        <span className="font-medium">memories</span>
        <span className="mx-2">·</span>
        <a href="mailto:mail@mrcwoods.com" className="hover:text-primary transition-colors">
          mail@mrcwoods.com
        </a>
      </footer>
    </div>
  );
};

const InfoRow: React.FC<{
  icon: React.ReactNode;
  label: string;
  value: string;
  highlight?: boolean;
  monospace?: boolean;
}> = ({ icon, label, value, highlight, monospace }) => (
  <div className="flex items-center justify-between rounded-2xl bg-muted/50 px-4 py-3.5 border border-border/30">
    <div className="flex items-center gap-3 text-muted-foreground">
      <span className="text-primary">{icon}</span>
      <span className="text-sm">{label}</span>
    </div>
    <span
      className={`font-semibold text-sm ${monospace ? 'font-mono' : ''} ${
        highlight ? 'text-accent' : 'text-foreground'
      }`}
    >
      {value}
    </span>
  </div>
);

export default StatusCheckPage;
