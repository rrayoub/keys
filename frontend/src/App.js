import React from 'react';
import { Routes, Route, Navigate } from 'react-router-dom';
import { ThemeProvider, createTheme } from '@mui/material/styles';
import CssBaseline from '@mui/material/CssBaseline';
import { useAuth } from './context/AuthContext';

// Components
import LoginPage from './pages/LoginPage';
import RegisterPage from './pages/RegisterPage';
import Dashboard from './pages/Dashboard';
import DevicesPage from './pages/DevicesPage';
import LogsPage from './pages/LogsPage';
import LogDetailsPage from './pages/LogDetailsPage';
import Layout from './components/Layout';

const darkTheme = createTheme({
  palette: {
    mode: 'dark',
    primary: {
      main: '#90caf9',
    },
    secondary: {
      main: '#f48fb1',
    },
  },
});

const App = () => {
  const { isAuthenticated } = useAuth();

  return (
    <ThemeProvider theme={darkTheme}>
      <CssBaseline />
      <Routes>
        <Route path="/" element={<LoginPage />} />
        <Route path="/register" element={<RegisterPage />} />
        
        {/* Protected Routes */}
        <Route 
          path="/dashboard" 
          element={isAuthenticated ? <Layout><Dashboard /></Layout> : <Navigate to="/" />} 
        />
        <Route 
          path="/devices" 
          element={isAuthenticated ? <Layout><DevicesPage /></Layout> : <Navigate to="/" />} 
        />
        <Route 
          path="/logs" 
          element={isAuthenticated ? <Layout><LogsPage /></Layout> : <Navigate to="/" />} 
        />
        <Route 
          path="/logs/:id" 
          element={isAuthenticated ? <Layout><LogDetailsPage /></Layout> : <Navigate to="/" />} 
        />
      </Routes>
    </ThemeProvider>
  );
};

export default App; 