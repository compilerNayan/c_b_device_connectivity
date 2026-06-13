#ifdef ESP_PLATFORM
#ifndef SOCKETCLOUDSERVER_INTERNAL_H
#define SOCKETCLOUDSERVER_INTERNAL_H

#include <StandardDefines.h>
#include "communication/ICloudServer.h"
#include "../01-interface/03-ICloudSocket.h"
#include "server/IDeviceService.h"
#include "service/IConnectionDetailsProvider.h"

/**
 * Cloud server that routes all uplink traffic through ICloudSocket using a
 * single NDJSON envelope per line:
 *
 * {"v":1,"category":"<name>","tenantId":"...","serialNumber":"...","data":<json>}\n
 *
 * Categories:
 *   log                 - device log batch (JSON array in data)
 *   water_pulse         - 1s flow sample (JSON object in data)
 *   water_30m           - 30-minute bucket (JSON object in data)
 *   lifecycle_enrolled  - enrollment complete (JSON object in data)
 *   device_message      - device response (requestId + payload in data)
 */
/* @Component */
class SocketCloudServer final : public ICloudServer {
    Private Static constexpr Int kProtocolVersion = 1;

    Public SocketCloudServer() {
        deviceIdentityProfile = deviceService->GetDeviceIdentityProfile();
    }
    Public Virtual ~SocketCloudServer() override = default;

    /* @Autowired */
    Private ICloudSocketPtr cloudSocket;

    /* @Autowired */
    Private IDeviceServicePtr deviceService;

    /* @Autowired */
    Private IConnectionDetailsProviderPtr connectionDetailsProvider;

    Private Optional<DeviceIdentityProfileData> deviceIdentityProfile;

    Public Bool Start() override {
        deviceIdentityProfile = deviceService->GetDeviceIdentityProfile();
        if (!cloudSocket) {
            return false;
        }
        return cloudSocket->OpenSocket("", 0);
    }

    Public Void Stop() override {
        if (cloudSocket) {
            cloudSocket->CloseSocket();
        }
    }

    Public Bool Restart() override {
        deviceIdentityProfile = deviceService->GetDeviceIdentityProfile();
        if (!cloudSocket) {
            return false;
        }
        return cloudSocket->Reconnect();
    }

    Public Bool IsRunning() const override {
        return cloudSocket && cloudSocket->IsSocketOpen();
    }

    Public IHttpRequestPtr ReceiveMessage() override {
        return nullptr;
    }

    Public Bool SendMessage(CStdString& requestId, CStdString& message) override {
        if (!cloudSocket || requestId.empty() || message.empty()) {
            return false;
        }

        StdString dataJson =
                "{\"requestId\":\"" + EscapeJsonString(requestId) + "\""
                + ",\"payload\":" + StdString(message) + "}";
        return QueueEnvelope("device_message", dataJson);
    }

    Public Bool PublishLogs(CStdString logs) override {
        return QueueEnvelope("log", logs);
    }

    Public Bool PublishSecondPulse(CStdString payload) override {
        return QueueEnvelope("water_pulse", payload);
    }

    Public Bool PublishThirtyMinuteBucket(CStdString payload) override {
        return QueueEnvelope("water_30m", payload);
    }

    Public Bool PublishEnrollmentComplete(CStdString payload) override {
        deviceIdentityProfile = deviceService->GetDeviceIdentityProfile();
        return QueueEnvelope("lifecycle_enrolled", payload);
    }

    Private Bool QueueEnvelope(CStdString category, CStdString dataJson) {
        if (!cloudSocket || dataJson.empty()) {
            return false;
        }
        if (!deviceIdentityProfile.has_value()) {
            return false;
        }

        StdString tenantId = connectionDetailsProvider->GetTenantId();
        StdString serialNumber = connectionDetailsProvider->GetThingName();
        if (serialNumber.empty()) {
            serialNumber = deviceIdentityProfile.value().thingName;
        }
        if (tenantId.empty() || serialNumber.empty()) {
            return false;
        }

        StdString line =
                "{\"v\":" + std::to_string(kProtocolVersion)
                + ",\"category\":\"" + StdString(category) + "\""
                + ",\"tenantId\":\"" + tenantId + "\""
                + ",\"serialNumber\":\"" + serialNumber + "\""
                + ",\"data\":" + StdString(dataJson) + "}\n";
        return cloudSocket->QueueDataToSend(line);
    }

    Private static StdString EscapeJsonString(CStdString value) {
        StdString escaped;
        escaped.reserve(value.size());
        for (char ch : value) {
            switch (ch) {
                case '\\':
                    escaped += "\\\\";
                    break;
                case '"':
                    escaped += "\\\"";
                    break;
                case '\n':
                    escaped += "\\n";
                    break;
                case '\r':
                    escaped += "\\r";
                    break;
                case '\t':
                    escaped += "\\t";
                    break;
                default:
                    escaped += ch;
                    break;
            }
        }
        return escaped;
    }
};

#endif // SOCKETCLOUDSERVER_INTERNAL_H
#endif // ESP_PLATFORM
