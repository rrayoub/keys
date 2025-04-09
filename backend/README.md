# Keylogger Backend Server

## Description
This is the backend server for the educational keylogger project. It provides APIs for authentication, device management, and keylog data storage.

## IMPORTANT DISCLAIMER
**This software is provided for EDUCATIONAL PURPOSES ONLY.**

The use of keyloggers and monitoring software without explicit consent from the users being monitored is:
- Illegal in most jurisdictions
- Unethical
- A violation of privacy

## Features
- User authentication with JWT
- Device management with API keys
- Keylog data storage (file and direct data)
- RESTful API

## Installation

### Prerequisites
- Node.js (v14 or higher)
- MongoDB
- npm or yarn

### Setup
1. Clone the repository
2. Install dependencies
   ```
   cd backend
   npm install
   ```
3. Create a `.env` file with the following variables:
   ```
   PORT=5000
   MONGODB_URI=mongodb://localhost:27017/keylogger
   JWT_SECRET=your_jwt_secret_key_change_this
   SMTP_HOST=smtp.gmail.com
   SMTP_PORT=587
   SMTP_USER=your_email@gmail.com
   SMTP_PASS=your_app_password
   ```
4. Start the server
   ```
   npm start
   ```

## API Documentation

### Authentication
- `POST /api/users/register` - Register a new user
- `POST /api/users/login` - Login user

### Devices
- `POST /api/devices/register` - Register a new device
- `GET /api/devices` - Get all devices for a user

### Keylogs
- `POST /api/keylogs/upload` - Upload keylog file
- `POST /api/keylogs/data` - Submit keylog data directly
- `GET /api/keylogs` - Get keylogs for a user
- `GET /api/keylogs/:id` - Get keylog details 