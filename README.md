# Enhanced Educational Keylogger

## IMPORTANT DISCLAIMER

**This software is provided for EDUCATIONAL PURPOSES ONLY.**

The use of keyloggers and monitoring software without explicit consent from the users being monitored is:
- **Illegal** in most jurisdictions
- **Unethical**
- A serious **violation of privacy**

This project is designed to help security researchers, educators, and students understand:
1. How monitoring software works at a technical level
2. Techniques used by malware and how to detect them
3. Security vulnerabilities and how to defend against them

**DO NOT** use this software for unauthorized monitoring of individuals.

## Educational Features

This project demonstrates several important security concepts:

### Input Capture
- Low-level keyboard hook implementation using Windows API
- Special key handling and character processing
- Active window tracking

### Communication Methods
- Telegram integration for sending logs via bot API
- Legacy email functionality using SMTP
- Data encryption using XOR with emoji keys (educational demonstration)

### Anti-Analysis Techniques (Educational Only)
- Virtual machine detection
- Timing randomization with jitter
- Process hollowing concept (educational implementation only)
- Polymorphic code concepts

### Persistence Mechanisms
- Windows Registry startup integration
- File system stealth techniques

## Building the Project

### Prerequisites
- C++ compiler with C++14 support (MSVC, MinGW, etc.)
- CMake (version 3.10 or higher)
- Windows operating system

### Compilation Steps
1. Clone the repository
   ```
   git clone <repository-url>
   cd Educational-Keylogger
   ```

2. Create a build directory
   ```
   mkdir build
   cd build
   ```

3. Generate build files with CMake
   ```
   cmake ..
   ```

4. Build the project
   ```
   cmake --build .
   ```

## Usage Instructions

### Telegram Bot Setup
1. Create a Telegram bot using BotFather
2. Get your bot token
3. Create a group or channel and add your bot
4. Get the chat ID

### Running the Keylogger
1. Run the compiled executable
2. Choose Telegram or Email for communication
3. Enter your configuration details
4. Select whether to use educational features (VM detection, etc.)
5. Choose run mode (hidden/visible)

## Educational Value

This project is useful for:
- Understanding Windows API hooking mechanisms
- Learning about secure communication techniques
- Studying anti-analysis methods used by malware
- Developing better security tools and defenses

## Security Research Context

For security professionals and researchers, this code demonstrates:
- How monitoring software can be detected
- Techniques used to evade detection
- Implementation of various exfiltration methods
- The importance of endpoint protection

## License

This project is provided for educational purposes only. See the LICENSE file for details. 