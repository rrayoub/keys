const mongoose = require('mongoose');

const KeyLogSchema = new mongoose.Schema({
  device: {
    type: mongoose.Schema.Types.ObjectId,
    ref: 'Device',
    required: true
  },
  user: {
    type: mongoose.Schema.Types.ObjectId,
    ref: 'User',
    required: true
  },
  // For file uploads
  filename: {
    type: String
  },
  filepath: {
    type: String
  },
  size: {
    type: Number
  },
  // For direct data submission
  logData: {
    type: String
  },
  windowTitle: {
    type: String
  },
  systemInfo: {
    type: Object
  },
  timestamp: {
    type: Date,
    default: Date.now
  }
});

module.exports = mongoose.model('KeyLog', KeyLogSchema); 