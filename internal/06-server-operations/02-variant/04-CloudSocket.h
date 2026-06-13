#ifdef ESP_PLATFORM
#ifndef CLOUDSOCKET_INTERNAL_H
#define CLOUDSOCKET_INTERNAL_H

#include <cerrno>
#include <cstring>
#include <deque>
#include <mutex>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <unistd.h>

#include <StandardDefines.h>
#include "logger/ILogger.h"
#include "../01-interface/03-ICloudSocket.h"

/* @Component */
class CloudSocket final : public ICloudSocket {
    Private Static constexpr const char* kStreamHost =
            "water-meter-data-injection-env.eba-udmynr49.ap-south-1.elasticbeanstalk.com";
    Private Static constexpr Int kStreamPort = 9100;
    Private Static constexpr Int kConnectTimeoutMs = 10000;

    /* @Autowired */
    Private ILoggerPtr logger = Implementation<ILogger>::type::GetInstance();

    Private Int clientSock_;
    Private CStdString host_;
    Private Int port_;
    Private StdDeque<CStdString> sendBuffer_;
    Private mutable std::mutex sendMutex_;

    Public CloudSocket() : clientSock_(-1), host_(kStreamHost), port_(kStreamPort) {}

    Public Virtual ~CloudSocket() override {
        CloseSocket();
    }

    Public Virtual Bool OpenSocket(CStdString host, Int port) override {
        (void)host;
        (void)port;
        CloseSocket();

        host_ = kStreamHost;
        port_ = kStreamPort;

        struct addrinfo hints = {};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        CStdString portText = std::to_string(port_);
        struct addrinfo* result = nullptr;
        Int resolveStatus =
                getaddrinfo(host_.c_str(), portText.c_str(), &hints, &result);
        if (resolveStatus != 0 || result == nullptr) {
            LogError("DNS resolve failed for " + host_);
            return false;
        }

        Int sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (sock < 0) {
            freeaddrinfo(result);
            LogError("Socket create failed");
            return false;
        }

        struct timeval timeout = {};
        timeout.tv_sec = kConnectTimeoutMs / 1000;
        timeout.tv_usec = (kConnectTimeoutMs % 1000) * 1000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

        if (connect(sock, result->ai_addr, result->ai_addrlen) != 0) {
            freeaddrinfo(result);
            close(sock);
            LogError("Socket connect failed for " + host_ + ":" + portText);
            return false;
        }

        freeaddrinfo(result);
        clientSock_ = sock;
        if (logger != nullptr) {
            logger->Info(
                    Tag::Untagged,
                    "[CloudSocket] Connected to " + host_ + ":" + portText);
        }
        return true;
    }

    Public Virtual Bool CloseSocket() override {
        if (clientSock_ < 0) {
            return true;
        }
        close(clientSock_);
        clientSock_ = -1;
        return true;
    }

    Public Virtual Bool IsSocketOpen() const override {
        return clientSock_ >= 0;
    }

    Public Virtual Bool QueueDataToSend(CStdString data) override {
        if (data.empty()) {
            return false;
        }
        std::lock_guard<std::mutex> lock(sendMutex_);
        sendBuffer_.push_back(data);
        return true;
    }

    Public Virtual Void SendData() override {
        if (clientSock_ < 0) {
            return;
        }

        CStdString payload;
        {
            std::lock_guard<std::mutex> lock(sendMutex_);
            if (sendBuffer_.empty()) {
                return;
            }
            payload = sendBuffer_.front();
            sendBuffer_.pop_front();
        }

        if (!TransmitPayload(payload)) {
            std::lock_guard<std::mutex> lock(sendMutex_);
            sendBuffer_.push_front(payload);
        }
    }

    Public Virtual Size GetPendingSendCount() const override {
        std::lock_guard<std::mutex> lock(sendMutex_);
        return sendBuffer_.size();
    }

    Public Virtual Bool Reconnect() override {
        CloseSocket();
        return OpenSocket(host_, port_);
    }

    Private Bool TransmitPayload(CStdString data) {
        if (clientSock_ < 0 || data.empty()) {
            return false;
        }

        const char* buffer = data.c_str();
        size_t total = data.size();
        size_t sentTotal = 0;

        while (sentTotal < total) {
            ssize_t sent =
                    send(clientSock_, buffer + sentTotal, total - sentTotal, 0);
            if (sent > 0) {
                sentTotal += static_cast<size_t>(sent);
                continue;
            }
            if (sent < 0 && errno == EINTR) {
                continue;
            }
            LogError("Socket send failed");
            CloseSocket();
            return false;
        }
        return true;
    }

    Private Void LogError(CStdString message) {
        if (logger != nullptr) {
            logger->Error(Tag::Untagged, "[CloudSocket] " + message);
        }
    }

    public:
    static ICloudSocketPtr GetInstance() {
        static ICloudSocketPtr instance(new CloudSocket());
        return instance;
    }
};

template <>
struct Implementation<ICloudSocket> {
    using type = CloudSocket;
};

template <>
struct Implementation<ICloudSocket*> {
    using type = CloudSocket*;
};

#endif // CLOUDSOCKET_INTERNAL_H
#endif // ESP_PLATFORM
