#ifdef JDSLKJDLKS
#ifndef CLOUDSERVER_INTERNAL_H
#define CLOUDSERVER_INTERNAL_H

#include <StandardDefines.h>
#include <ctime>
#include <cstdlib>
#include "communication/ICloudServer.h"
#include "../01-interface/03-ICloudSocket.h"
#include "server/IMqttClient.h"
#include "server/IDeviceService.h"
#include "service/IConnectionDetailsProvider.h"

/* @Component */
class CloudServer final : public ICloudServer {
    Public CloudServer() {
        deviceIdentityProfile = deviceService->GetDeviceIdentityProfile();
    };
    Public Virtual ~CloudServer() override = default;

    /* @Autowired */
    Private IMqttClientPtr mqttClient;

    /* @Autowired */
    Private IDeviceServicePtr deviceService;

    /* @Autowired */
    Private IConnectionDetailsProviderPtr connectionDetailsProvider;

    /* @Autowired */
    Private ICloudSocketPtr cloudSocket;

    Private Optional<DeviceIdentityProfileData> deviceIdentityProfile;

    Public Bool Start() override {
        deviceIdentityProfile = deviceService->GetDeviceIdentityProfile();
        return mqttClient->IsConnected();
    }

    Public Void Stop() override {
    }

    Public Bool Restart() override {
        deviceIdentityProfile = deviceService->GetDeviceIdentityProfile();
        return false;
    }

    Public Bool IsRunning() const override {
        return mqttClient->IsConnected();
    }

    Public IHttpRequestPtr ReceiveMessage() override {
        if (!deviceIdentityProfile.has_value()) {
            return nullptr;
        }

        CStdString path = deviceIdentityProfile.value().subscribeTopics.commandTopic;
        auto message = mqttClient->GetNextReceivedMessage(path);
        if (!message.has_value()) {
            return nullptr;
        }
        auto request = IHttpRequest::GetRequest(message.value().guid, RequestSource::CloudServer, message.value().payload);
        return request;
    }

    Public Bool SendMessage(CStdString& requestId, CStdString& message) override {
        if (!deviceIdentityProfile.has_value()) {
            return false;
        }

        CStdString path = deviceIdentityProfile.value().publishTopics.statusTopic;
        MqttMessage mqttMessage = {
            .guid = requestId,
            .payload = message,
        };
        return mqttClient->QueueMessageToSend(path, mqttMessage);
    }

    Public Bool PublishLogs(CStdString logs) override {
        if (!deviceIdentityProfile.has_value()) {
            return false;
        }
        CStdString path = deviceIdentityProfile.value().publishTopics.logsTopic;
        MqttMessage mqttMessage = {
            .guid = "logs",
            .payload = logs,
        };
        return mqttClient->QueueMessageToSend(path, mqttMessage);
    }

    Public Bool PublishSecondPulse(CStdString payload) override {
        if (!deviceIdentityProfile.has_value()) {
            return false;
        }

        StdString tenantId = connectionDetailsProvider->GetTenantId();
        StdString serialNumber = connectionDetailsProvider->GetThingName();
        if (serialNumber.empty()) {
            serialNumber = deviceIdentityProfile.value().thingName;
        }

        double ml = ExtractJsonNumber(payload, "ml");
        double cumulativeLiters = ExtractJsonNumber(payload, "cumulativeLiters");
        StdString ts = ExtractJsonString(payload, "ts");
        if (ts.empty()) {
            ts = FormatUtcSecondTimestamp();
        }

        StdString streamPayload =
                "{\"tenantId\":\"" + tenantId + "\",\"serialNumber\":\"" + serialNumber
                + "\",\"ml\":" + std::to_string(ml) + ",\"cumulativeLiters\":"
                + std::to_string(cumulativeLiters) + ",\"ts\":\"" + ts + "\"}\n";
        return cloudSocket->QueueDataToSend(streamPayload);
    }

    Public Bool PublishThirtyMinuteBucket(CStdString payload) override {
        if (!deviceIdentityProfile.has_value()) {
            return false;
        }
        return PublishWaterTelemetry(deviceIdentityProfile.value().publishTopics.water30mBucketTopic, payload);
    }

    Public Bool PublishEnrollmentComplete(CStdString payload) override {
        deviceIdentityProfile = deviceService->GetDeviceIdentityProfile();
        if (!deviceIdentityProfile.has_value()) {
            return false;
        }
        CStdString topic = deviceIdentityProfile.value().publishTopics.lifecycleEnrolledTopic;
        MqttMessage mqttMessage = {
            .guid = "lifecycle-enrolled",
            .payload = payload,
        };
        return mqttClient->QueueMessageToSend(topic, mqttMessage);
    }

    Private Bool PublishWaterTelemetry(CStdString topic, CStdString payload) {
        if (!mqttClient->IsConnected()) {
            return false;
        }
        CStdString guid = topic + "-" + std::to_string(static_cast<unsigned long long>(time(nullptr)));
        MqttMessage mqttMessage = {
            .guid = guid,
            .payload = payload,
        };
        return mqttClient->QueueMessageToSend(topic, mqttMessage);
    }

    Private static StdString ExtractJsonString(CStdString json, CStdString key) {
        StdString pattern = "\"" + StdString(key) + "\":\"";
        size_t pos = json.find(pattern);
        if (pos == StdString::npos) {
            return "";
        }
        pos += pattern.size();
        size_t end = json.find("\"", pos);
        if (end == StdString::npos) {
            return "";
        }
        return json.substr(pos, end - pos);
    }

    Private static double ExtractJsonNumber(CStdString json, CStdString key) {
        CStdString pattern = "\"" + key + "\":";
        size_t pos = json.find(pattern);
        if (pos == CStdString::npos) {
            return 0.0;
        }
        pos += pattern.size();
        return strtod(json.c_str() + pos, nullptr);
    }

    Private static StdString FormatUtcSecondTimestamp() {
        time_t now = time(nullptr);
        struct tm utc;
        gmtime_r(&now, &utc);
        char buffer[32];
        strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utc);
        return StdString(buffer);
    }
};
#endif // CLOUDSERVER_INTERNAL_H
#endif // JDSLKJDLKS