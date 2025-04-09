# Keylogger Dashboard Frontend

## Description
The frontend dashboard for the educational keylogger project. Built with React and Material UI.

## IMPORTANT DISCLAIMER
**This software is provided for EDUCATIONAL PURPOSES ONLY.**

The use of keyloggers and monitoring software without explicit consent from the users being monitored is:
- Illegal in most jurisdictions
- Unethical
- A violation of privacy

## Features
- User authentication
- Dashboard with statistics
- Device management
- Log viewing and analysis
- Modern, responsive UI

## Installation

### Prerequisites
- Node.js (v14 or higher)
- npm or yarn

### Setup
1. Install dependencies:
   ```
   npm install
   ```
2. Start the development server:
   ```
   npm start
   ```
3. Access the dashboard at http://localhost:3000

## Project Structure
```
frontend/
├── public/              # Static files
├── src/
│   ├── components/      # Reusable UI components
│   ├── context/         # React context for app state
│   ├── pages/           # Main page components
│   ├── App.js           # Main application component
│   └── index.js         # Entry point
└── package.json         # Dependencies
```

## Usage
1. Register a new account or log in
2. Add devices to monitor
3. View logs from connected devices
4. Analyze keystroke patterns and activity

## Technologies Used
- React
- Material UI
- React Router
- Axios for API requests
- Chart.js for data visualization

## Security Implementation
This frontend implements:
- JWT authentication
- Secure HTTP-only cookies
- Protected routes

## License
This project is provided for educational purposes only. 