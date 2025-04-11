/**
 * @file SmtpClient.cpp
 * @brief Implementation of the SmtpClient class
 */

// Include winsock2.h before other Windows headers
#include <winsock2.h>
#include <ws2tcpip.h>
#include "../include/SmtpClient.h"

#pragma comment(lib, "ws2_32.lib")
#include <sstream>

SmtpClient::SmtpClient(
    const std::string& server,
    int port,
    const std::string& user,
    const std::string& pass
) : smtpServer(server),
    smtpPort(port),
    username(user),
    password(pass),
    isInitialized(false) {
    
    isInitialized = initializeWinsock();
}

SmtpClient::~SmtpClient() {
    WSACleanup();
}

bool SmtpClient::initializeWinsock() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

std::string SmtpClient::base64Encode(const std::string& data) {
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

bool SmtpClient::sendEmail(
    const std::string& from,
    const std::string& to,
    const std::string& subject,
    const std::string& body,
    const std::vector<Attachment>& attachments
) {
    if (!isInitialized) {
        return false;
    }
    
    // Create a socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        return false;
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
        return false;
    }
    
    server.sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
    freeaddrinfo(res);
    
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) != 0) {
        closesocket(sock);
        return false;
    }
    
    // Buffer for receiving server responses
    char buffer[1024];
    recv(sock, buffer, sizeof(buffer), 0);
    
    // Get hostname for EHLO command
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    
    // EHLO command
    std::string ehlo = "EHLO " + std::string(hostname) + "\r\n";
    send(sock, ehlo.c_str(), ehlo.length(), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    
    // Start TLS if using port 587
    if (smtpPort == 587) {
        std::string starttls = "STARTTLS\r\n";
        send(sock, starttls.c_str(), starttls.length(), 0);
        recv(sock, buffer, sizeof(buffer), 0);
        
        // Note: Real implementation should upgrade to SSL/TLS here
        // For educational purposes, this is simplified
    }
    
    // Authentication
    std::string authLogin = "AUTH LOGIN\r\n";
    send(sock, authLogin.c_str(), authLogin.length(), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    
    // Send username (Base64 encoded)
    std::string encodedUser = base64Encode(username) + "\r\n";
    send(sock, encodedUser.c_str(), encodedUser.length(), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    
    // Send password (Base64 encoded)
    std::string encodedPass = base64Encode(password) + "\r\n";
    send(sock, encodedPass.c_str(), encodedPass.length(), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    
    // MAIL FROM command
    std::string mailFrom = "MAIL FROM:<" + from + ">\r\n";
    send(sock, mailFrom.c_str(), mailFrom.length(), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    
    // RCPT TO command
    std::string rcptTo = "RCPT TO:<" + to + ">\r\n";
    send(sock, rcptTo.c_str(), rcptTo.length(), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    
    // DATA command
    std::string dataCmd = "DATA\r\n";
    send(sock, dataCmd.c_str(), dataCmd.length(), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    
    // Email content with attachments
    std::string boundary = "==Multipart_Boundary_" + std::to_string(time(NULL));
    
    std::stringstream emailContent;
    emailContent << "From: " << from << "\r\n";
    emailContent << "To: " << to << "\r\n";
    emailContent << "Subject: " << subject << "\r\n";
    emailContent << "MIME-Version: 1.0\r\n";
    
    // If we have attachments, use multipart
    if (!attachments.empty()) {
        emailContent << "Content-Type: multipart/mixed; boundary=\"" << boundary << "\"\r\n\r\n";
        
        // Main body
        emailContent << "--" << boundary << "\r\n";
        emailContent << "Content-Type: text/plain; charset=UTF-8\r\n";
        emailContent << "Content-Transfer-Encoding: 7bit\r\n\r\n";
        emailContent << body << "\r\n\r\n";
        
        // Attachments
        for (const auto& attachment : attachments) {
            emailContent << "--" << boundary << "\r\n";
            emailContent << "Content-Type: " << attachment.contentType << "; name=\"" << attachment.filename << "\"\r\n";
            emailContent << "Content-Transfer-Encoding: base64\r\n";
            emailContent << "Content-Disposition: attachment; filename=\"" << attachment.filename << "\"\r\n\r\n";
            emailContent << base64Encode(attachment.content) << "\r\n\r\n";
        }
        
        emailContent << "--" << boundary << "--\r\n";
    } else {
        // No attachments, just plain text
        emailContent << "Content-Type: text/plain; charset=UTF-8\r\n\r\n";
        emailContent << body << "\r\n";
    }
    
    // End of email marker
    emailContent << ".\r\n";
    
    // Send email content
    std::string emailStr = emailContent.str();
    send(sock, emailStr.c_str(), emailStr.length(), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    
    // QUIT command
    std::string quit = "QUIT\r\n";
    send(sock, quit.c_str(), quit.length(), 0);
    
    // Close socket
    closesocket(sock);
    
    return true;
} 