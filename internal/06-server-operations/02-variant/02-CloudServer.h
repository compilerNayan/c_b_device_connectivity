#ifndef CLOUDSERVER_INTERNAL_H
#define CLOUDSERVER_INTERNAL_H

#include <StandardDefines.h>
#include <ctime>
#include "communication/ICloudServer.h"
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
        return PublishWaterTelemetry(deviceIdentityProfile.value().publishTopics.water1sBucketTopic, payload);
    }

    Public Bool PublishThirtyMinuteBucket(CStdString payload) override {
        if (!deviceIdentityProfile.has_value()) {
            return false;
        }
        return PublishWaterTelemetry(deviceIdentityProfile.value().publishTopics.water30mBucketTopic, payload);
    }

    Public Bool PublishEnrollmentComplete(CStdString payload) override {
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
};
#endif // CLOUDSERVER_INTERNAL_H
