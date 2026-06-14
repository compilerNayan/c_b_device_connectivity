#ifdef ESP_PLATFORM
#ifndef SOCKETCLOUDSERVER_INTERNAL_H
#define SOCKETCLOUDSERVER_INTERNAL_H

#include <StandardDefines.h>
#include "communication/ICloudServer.h"
#include "../01-interface/03-ICloudSocket.h"
#include "logger/ILogger.h"
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
 *   enrollment_request  - device asks cloud to enroll it (JSON object in data)
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

    /* @Autowired */
    Private ILoggerPtr logger;

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
        if (!cloudSocket) {
            return nullptr;
        }

        auto message = cloudSocket->ReceiveData();
        if (!message.has_value()) {
            return nullptr;
        }

        return IHttpRequest::GetRequest(
                message.value().guid,
                RequestSource::CloudServer,
                message.value().payload);
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
        if (deviceService != nullptr) {
            deviceService->Refresh();
        }
        deviceIdentityProfile = deviceService->GetDeviceIdentityProfile();
        if (!QueueEnvelope("lifecycle_enrolled", payload)) {
            return false;
        }
        if (cloudSocket != nullptr) {
            cloudSocket->SendData();
        }
        return true;
    }

    Public Bool BeginEnrollment(CStdString payload) override {
        if (!cloudSocket) {
            return false;
        }

        StdString dataJson = payload.empty()
                                     ? BuildDefaultEnrollmentRequestPayload()
                                     : StdString(payload);
        if (dataJson.empty()) {
            return false;
        }

        if (!cloudSocket->IsSocketOpen()) {
            if (!cloudSocket->OpenSocket("", 0)) {
                return false;
            }
        }

        return QueuePreEnrollmentEnvelope("enrollment_request", dataJson);
    }

    Private Bool QueuePreEnrollmentEnvelope(CStdString category, CStdString dataJson) {
        if (!cloudSocket || dataJson.empty()) {
            return false;
        }

        StdString serialNumber = connectionDetailsProvider->GetSerialNumber();
        if (serialNumber.empty()) {
            return false;
        }

        StdString line =
                "{\"v\":" + std::to_string(kProtocolVersion)
                + ",\"category\":\"" + StdString(category) + "\""
                + ",\"tenantId\":\"\""
                + ",\"serialNumber\":\"" + serialNumber + "\""
                + ",\"data\":" + StdString(dataJson) + "}\n";
        if (!cloudSocket->QueueDataToSend(line)) {
            return false;
        }
        cloudSocket->SendData();
        return true;
    }

    Private StdString BuildDefaultEnrollmentRequestPayload() const {
        StdString serialNumber = connectionDetailsProvider->GetSerialNumber();
        StdString deviceType = connectionDetailsProvider->GetDeviceType();
        StdString firmwareVersion = connectionDetailsProvider->GetFirmwareVersion();
        if (serialNumber.empty()) {
            return "";
        }

        return "{\"serialNumber\":\"" + EscapeJsonString(serialNumber) + "\""
               + ",\"deviceType\":\"" + EscapeJsonString(deviceType) + "\""
               + ",\"firmwareVersion\":\"" + EscapeJsonString(firmwareVersion) + "\"}";
    }

    Private Bool QueueEnvelope(CStdString category, CStdString dataJson) {
        if (!cloudSocket) {
            LogQueueEnvelopeFailure(category, "cloudSocket is null");
            return false;
        }
        if (dataJson.empty()) {
            LogQueueEnvelopeFailure(category, "payload is empty");
            return false;
        }
        if (!deviceIdentityProfile.has_value()) {
            //LogQueueEnvelopeFailure(category, "deviceIdentityProfile missing");
            return false;
        }

        StdString tenantId = connectionDetailsProvider->GetTenantId();
        StdString serialNumber = connectionDetailsProvider->GetThingName();
        if (serialNumber.empty()) {
            serialNumber = deviceIdentityProfile.value().thingName;
        }
        if (serialNumber.empty()) {
            serialNumber = connectionDetailsProvider->GetSerialNumber();
        }
        if (tenantId.empty() || serialNumber.empty()) {
            LogQueueEnvelopeFailure(
                    category,
                    "tenantId or serialNumber missing tenantId=" + tenantId +
                            " thingName=" + connectionDetailsProvider->GetThingName() +
                            " serial=" + connectionDetailsProvider->GetSerialNumber());
            return false;
        }

        StdString line =
                "{\"v\":" + std::to_string(kProtocolVersion)
                + ",\"category\":\"" + StdString(category) + "\""
                + ",\"tenantId\":\"" + tenantId + "\""
                + ",\"serialNumber\":\"" + serialNumber + "\""
                + ",\"data\":" + StdString(dataJson) + "}\n";
        if (!cloudSocket->QueueDataToSend(line)) {
            LogQueueEnvelopeFailure(category, "QueueDataToSend failed");
            return false;
        }
        return true;
    }

    Private Void LogQueueEnvelopeFailure(CStdString category, CStdString reason) const {
        if (logger == nullptr) {
            return;
        }
        logger->Warning(
                Tag::Untagged,
                "[SocketCloudServer] QueueEnvelope failed category=" + StdString(category) +
                        " reason=" + StdString(reason));
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
