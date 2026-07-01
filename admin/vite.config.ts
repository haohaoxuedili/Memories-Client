import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

export default defineConfig({
  base: './',
  plugins: [react()],
  server: {
    port: 5173,
    strictPort: false,
    proxy: {
      '/admin/token': {
        target: 'https://memories-api.mrcwoods.com',
        changeOrigin: true,
      },
      '/memories-api': {
        target: 'https://memories-api.mrcwoods.com',
        changeOrigin: true,
        rewrite: (path) => path.replace(/^\/memories-api/, ''),
      },
      '/public-image-api': {
        target: 'https://img.scdn.io',
        changeOrigin: true,
        rewrite: (path) => path.replace(/^\/public-image-api/, '/api/v1.php'),
      },
    },
  },
});