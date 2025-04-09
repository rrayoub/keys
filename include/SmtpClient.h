/**
 * @file SmtpClient.h
 * @brief Simple SMTP client for email sending
 * 
 * This is for EDUCATIONAL PURPOSES ONLY.
 */

#pragma once

#include <string>
#include <vector>

// Forward declarations to avoid including winsock2.h in header
struct sockaddr_in;
typedef unsigned int SOCKET;

/**
 * @class SmtpClient
 * @brief A simple SMTP client for sending emails with attachments
 */
class SmtpClient {
private:
    std::string smtpServer;
    int smtpPort;
    std::string username;
    std::string password;
    bool isInitialized;
    
    // Helper methods
    bool initializeWinsock();
    std::string base64Encode(const std::string& data);
    
public:
    /**
     * @brief Constructor for SmtpClient
     * @param server SMTP server address
     * @param port SMTP server port (default: 587)
     * @param user Username for authentication
     * @param pass Password for authentication
     */
    SmtpClient(
        const std::string& server,
        int port,
        const std::string& user,
        const std::string& pass
    );
    
    /**
     * @brief Destructor for SmtpClient
     */
    ~SmtpClient();
    
    /**
     * @brief Structure for email attachments
     */
    struct Attachment {
        std::string filename;
        std::string content;
        std::string contentType;
    };
    
    /**
     * @brief Sends an email
     * @param from Sender email address
     * @param to Recipient email address
     * @param subject Email subject
     * @param body Email body text
     * @param attachments Vector of attachments (optional)
     * @return true if email was sent successfully, false otherwise
     */
    bool sendEmail(
        const std::string& from,
        const std::string& to,
        const std::string& subject,
        const std::string& body,
        const std::vector<Attachment>& attachments = std::vector<Attachment>()
    );
}; 