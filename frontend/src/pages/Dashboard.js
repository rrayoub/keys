import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import axios from 'axios';
import { useAuth } from '../context/AuthContext';

// MUI Components
import { 
  Grid, 
  Paper, 
  Typography, 
  Box, 
  Card, 
  CardContent, 
  Divider,
  List,
  ListItem,
  ListItemText,
  ListItemIcon,
  Chip
} from '@mui/material';
import { 
  DesktopWindows as DesktopIcon,
  Storage as StorageIcon,
  Schedule as ScheduleIcon,
  Keyboard as KeyboardIcon
} from '@mui/icons-material';

// Chart Components
import { Chart as ChartJS, ArcElement, Tooltip, Legend, CategoryScale, LinearScale, BarElement, Title } from 'chart.js';
import { Pie, Bar } from 'react-chartjs-2';

ChartJS.register(ArcElement, Tooltip, Legend, CategoryScale, LinearScale, BarElement, Title);

const Dashboard = () => {
  const [stats, setStats] = useState({
    totalLogs: 0,
    totalDevices: 0,
    recentLogs: [],
    windowStats: {},
    keyCount: 0,
  });
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const { user } = useAuth();
  const navigate = useNavigate();

  useEffect(() => {
    const fetchDashboardData = async () => {
      try {
        // Fetch devices
        const devicesRes = await axios.get('/api/devices');
        
        // Fetch recent logs
        const logsRes = await axios.get('/api/keylogs?limit=5');
        
        // Process data for charts
        const windowStats = {};
        let keyCount = 0;
        
        logsRes.data.keylogs.forEach(log => {
          // Count windows
          if (log.windowTitle) {
            windowStats[log.windowTitle] = (windowStats[log.windowTitle] || 0) + 1;
          }
          
          // Estimate key count from logData length
          if (log.logData) {
            keyCount += log.logData.length;
          }
        });
        
        setStats({
          totalLogs: logsRes.data.total,
          totalDevices: devicesRes.data.length,
          recentLogs: logsRes.data.keylogs,
          windowStats,
          keyCount
        });
        
        setLoading(false);
      } catch (err) {
        console.error('Dashboard data fetch error:', err);
        setError('Failed to load dashboard data');
        setLoading(false);
      }
    };
    
    fetchDashboardData();
  }, []);
  
  // Prepare data for pie chart
  const windowData = {
    labels: Object.keys(stats.windowStats).slice(0, 5),
    datasets: [
      {
        data: Object.values(stats.windowStats).slice(0, 5),
        backgroundColor: [
          'rgba(255, 99, 132, 0.6)',
          'rgba(54, 162, 235, 0.6)',
          'rgba(255, 206, 86, 0.6)',
          'rgba(75, 192, 192, 0.6)',
          'rgba(153, 102, 255, 0.6)',
        ],
        borderColor: [
          'rgba(255, 99, 132, 1)',
          'rgba(54, 162, 235, 1)',
          'rgba(255, 206, 86, 1)',
          'rgba(75, 192, 192, 1)',
          'rgba(153, 102, 255, 1)',
        ],
        borderWidth: 1,
      },
    ],
  };
  
  // Format timestamp
  const formatDate = (dateString) => {
    const date = new Date(dateString);
    return date.toLocaleString();
  };

  return (
    <Box sx={{ flexGrow: 1 }}>
      <Typography variant="h4" gutterBottom component="div">
        Dashboard
      </Typography>
      
      {error && (
        <Paper sx={{ p: 2, mb: 2, backgroundColor: 'error.dark' }}>
          <Typography color="white">{error}</Typography>
        </Paper>
      )}
      
      {/* Stats Cards */}
      <Grid container spacing={3} sx={{ mb: 3 }}>
        <Grid item xs={12} sm={6} md={3}>
          <Card sx={{ height: '100%' }}>
            <CardContent>
              <Typography variant="h6" component="div" gutterBottom>
                Total Logs
              </Typography>
              <Typography variant="h3" component="div" color="primary">
                {stats.totalLogs}
              </Typography>
              <Box sx={{ display: 'flex', alignItems: 'center', mt: 1 }}>
                <StorageIcon color="primary" fontSize="small" />
                <Typography variant="body2" component="div" sx={{ ml: 1 }}>
                  Logs collected
                </Typography>
              </Box>
            </CardContent>
          </Card>
        </Grid>
        
        <Grid item xs={12} sm={6} md={3}>
          <Card sx={{ height: '100%' }}>
            <CardContent>
              <Typography variant="h6" component="div" gutterBottom>
                Active Devices
              </Typography>
              <Typography variant="h3" component="div" color="secondary">
                {stats.totalDevices}
              </Typography>
              <Box sx={{ display: 'flex', alignItems: 'center', mt: 1 }}>
                <DesktopIcon color="secondary" fontSize="small" />
                <Typography variant="body2" component="div" sx={{ ml: 1 }}>
                  Registered devices
                </Typography>
              </Box>
            </CardContent>
          </Card>
        </Grid>
        
        <Grid item xs={12} sm={6} md={3}>
          <Card sx={{ height: '100%' }}>
            <CardContent>
              <Typography variant="h6" component="div" gutterBottom>
                Keystrokes
              </Typography>
              <Typography variant="h3" component="div" color="success.main">
                {stats.keyCount.toLocaleString()}
              </Typography>
              <Box sx={{ display: 'flex', alignItems: 'center', mt: 1 }}>
                <KeyboardIcon color="success" fontSize="small" />
                <Typography variant="body2" component="div" sx={{ ml: 1 }}>
                  Keys logged
                </Typography>
              </Box>
            </CardContent>
          </Card>
        </Grid>
        
        <Grid item xs={12} sm={6} md={3}>
          <Card sx={{ height: '100%' }}>
            <CardContent>
              <Typography variant="h6" component="div" gutterBottom>
                Last Updated
              </Typography>
              <Typography variant="h6" component="div" color="info.main">
                {stats.recentLogs.length > 0 
                  ? formatDate(stats.recentLogs[0].timestamp) 
                  : 'No logs yet'}
              </Typography>
              <Box sx={{ display: 'flex', alignItems: 'center', mt: 1 }}>
                <ScheduleIcon color="info" fontSize="small" />
                <Typography variant="body2" component="div" sx={{ ml: 1 }}>
                  Most recent log
                </Typography>
              </Box>
            </CardContent>
          </Card>
        </Grid>
      </Grid>
      
      {/* Charts and Lists */}
      <Grid container spacing={3}>
        <Grid item xs={12} md={6}>
          <Paper sx={{ p: 2, height: '100%' }}>
            <Typography variant="h6" gutterBottom>
              Top Active Windows
            </Typography>
            <Box sx={{ height: 300, display: 'flex', justifyContent: 'center', alignItems: 'center' }}>
              {Object.keys(stats.windowStats).length > 0 ? (
                <Pie data={windowData} options={{ maintainAspectRatio: false }} />
              ) : (
                <Typography color="text.secondary">No window data available</Typography>
              )}
            </Box>
          </Paper>
        </Grid>
        
        <Grid item xs={12} md={6}>
          <Paper sx={{ p: 2, height: '100%' }}>
            <Typography variant="h6" gutterBottom>
              Recent Logs
            </Typography>
            {stats.recentLogs.length > 0 ? (
              <List>
                {stats.recentLogs.map((log) => (
                  <React.Fragment key={log._id}>
                    <ListItem 
                      button
                      onClick={() => navigate(`/logs/${log._id}`)}
                    >
                      <ListItemIcon>
                        <StorageIcon />
                      </ListItemIcon>
                      <ListItemText 
                        primary={log.windowTitle || 'Unknown Window'} 
                        secondary={formatDate(log.timestamp)}
                      />
                      <Chip 
                        label={log.device ? log.device.name : 'Unknown Device'} 
                        size="small" 
                        color="primary" 
                        variant="outlined"
                      />
                    </ListItem>
                    <Divider />
                  </React.Fragment>
                ))}
              </List>
            ) : (
              <Box sx={{ p: 3, textAlign: 'center' }}>
                <Typography color="text.secondary">No logs available</Typography>
              </Box>
            )}
          </Paper>
        </Grid>
      </Grid>
      
      {/* Educational Notice */}
      <Box mt={4} mb={2}>
        <Paper sx={{ p: 2, bgcolor: 'background.paper' }}>
          <Typography variant="body2" align="center" color="text.secondary">
            <strong>EDUCATIONAL PURPOSE ONLY:</strong> This dashboard is for educational purposes to demonstrate how monitoring software works.
            Usage of keyloggers without explicit consent is illegal and unethical.
          </Typography>
        </Paper>
      </Box>
    </Box>
  );
};

export default Dashboard; 