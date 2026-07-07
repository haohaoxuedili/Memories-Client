import { lazy } from 'react';
import type { ReactNode } from 'react';

const StatusCheckPage = lazy(() => import('./pages/StatusCheckPage'));
const GalleryPage = lazy(() => import('./pages/GalleryPage'));

export interface RouteConfig {
  name: string;
  path: string;
  element: ReactNode;
  visible?: boolean;
  /** Accessible without login. Routes without this flag require authentication. Has no effect when RouteGuard is not in use. */
  public?: boolean;
}

export const routes: RouteConfig[] = [
  {
    name: '服务状态',
    path: '/',
    element: <StatusCheckPage />,
    public: true,
  },
  {
    name: '图片浏览',
    path: '/gallery',
    element: <GalleryPage />,
    public: true,
  },
];
