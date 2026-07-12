import React, { Suspense } from 'react';
import { BrowserRouter as Router, Routes, Route, Navigate } from 'react-router-dom';
import IntersectObserver from '@/components/common/IntersectObserver';
import { Toaster } from '@/components/ui/sonner';
import { SettingsProvider } from '@/contexts/SettingsContext';

import { routes } from './routes';

const routerBasename = import.meta.env.BASE_URL.replace(/\/$/, '');

const App: React.FC = () => {
  return (
    <Router basename={routerBasename}>
      <SettingsProvider>
        <IntersectObserver />
        <Suspense fallback={null}>
          <Routes>
            {routes.map((route, index) => (
              <Route
                key={index}
                path={route.path}
                element={route.element}
              />
            ))}
            <Route path="*" element={<Navigate to="/" replace />} />
          </Routes>
        </Suspense>
        <Toaster position="top-center" />
      </SettingsProvider>
    </Router>
  );
};

export default App;
