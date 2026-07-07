import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";
import svgr from "vite-plugin-svgr";
import { VitePWA } from "vite-plugin-pwa";
import path from "path";

const basePath = process.env.VITE_BASE_PATH || "/";

export default defineConfig({
  base: basePath,
  plugins: [
    react(),
    svgr({
      svgrOptions: {
        icon: true,
        exportType: "named",
        namedExport: "ReactComponent",
      },
    }),
    VitePWA({
      registerType: "autoUpdate",
      includeAssets: [
        "assets/app-icon.svg",
        "assets/pwa-64x64.png",
        "assets/pwa-192x192.png",
        "assets/pwa-512x512.png",
        "assets/maskable-icon-512x512.png",
        "assets/apple-touch-icon-180x180.png",
        "assets/favicon.ico",
      ],
      manifest: {
        name: "Memories",
        short_name: "Memories",
        description: "校园记忆图片客户端，支持离线打开与主屏安装。",
        theme_color: "#1D6E5A",
        background_color: "#F4FBF7",
        display: "standalone",
        orientation: "portrait",
        start_url: basePath,
        scope: basePath,
        lang: "zh-CN",
        icons: [
          {
            src: `${basePath}assets/pwa-192x192.png`,
            sizes: "192x192",
            type: "image/png",
          },
          {
            src: `${basePath}assets/pwa-512x512.png`,
            sizes: "512x512",
            type: "image/png",
          },
          {
            src: `${basePath}assets/maskable-icon-512x512.png`,
            sizes: "512x512",
            type: "image/png",
            purpose: "maskable",
          },
        ],
      },
      workbox: {
        globPatterns: ["**/*.{js,css,html,svg,png,woff2}"],
        navigateFallback: `${basePath}offline.html`,
        cleanupOutdatedCaches: true,
        runtimeCaching: [
          {
            urlPattern: /^https:\/\/memories-api\.mrcwoods\.com\/.*/i,
            handler: "NetworkFirst",
            options: {
              cacheName: "memories-api-runtime",
              networkTimeoutSeconds: 4,
              expiration: {
                maxEntries: 60,
                maxAgeSeconds: 60 * 30,
              },
              cacheableResponse: {
                statuses: [0, 200],
              },
            },
          },
          {
            urlPattern: /^https:\/\/img\.scdn\.io\/.*/i,
            handler: "StaleWhileRevalidate",
            options: {
              cacheName: "memories-image-runtime",
              expiration: {
                maxEntries: 120,
                maxAgeSeconds: 60 * 60 * 24 * 14,
              },
              cacheableResponse: {
                statuses: [0, 200],
              },
            },
          },
          {
            urlPattern: /^https:\/\/(fonts\.googleapis\.com|fonts\.gstatic\.com|cdn\.jsdelivr\.net|statics\.moonshot\.cn)\/.*/i,
            handler: "CacheFirst",
            options: {
              cacheName: "memories-font-runtime",
              expiration: {
                maxEntries: 80,
                maxAgeSeconds: 60 * 60 * 24 * 30,
              },
              cacheableResponse: {
                statuses: [0, 200],
              },
            },
          },
        ],
      },
      devOptions: {
        enabled: false,
      },
    }),
  ],
  resolve: {
    alias: {
      "@": path.resolve(__dirname, "./src"),
    },
  },
  build: {
    rollupOptions: {
      output: {
        manualChunks(id) {
          if (id.includes("node_modules")) {
            if (id.includes("react") || id.includes("scheduler") || id.includes("react-router")) {
              return "react";
            }

            if (id.includes("@sentry")) {
              return "sentry";
            }

            if (id.includes("motion") || id.includes("framer-motion")) {
              return "motion";
            }

            if (id.includes("recharts")) {
              return "charts";
            }

            if (id.includes("react-hook-form") || id.includes("@hookform/resolvers") || id.includes("zod")) {
              return "forms";
            }

            if (id.includes("video-react")) {
              return "video";
            }

            if (id.includes("qrcode")) {
              return "qrcode";
            }

            if (id.includes("next-themes")) {
              return "themes";
            }

            if (id.includes("sonner") || id.includes("lucide-react")) {
              return "ui";
            }
          }

          return undefined;
        },
      },
    },
  },
});
