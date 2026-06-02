#ifndef CLOUDSERVER_INTERNAL_H
#define CLOUDSERVER_INTERNAL_H

#include <StandardDefines.h>
#include "communication/ICloudServer.h"
#include "server/IMqttClient.h"
#include "server/IDeviceService.h"

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

    Private Optional<DeviceIdentityProfileData> deviceIdentityProfile;

    Public Bool Start() override {
        // Do nothing as MqttClient will be managed by MqttClientManager
        deviceIdentityProfile = deviceService->GetDeviceIdentityProfile();
        return mqttClient->IsConnected();        
    }

    Public Void Stop() override {
        // Do nothing as MqttClient will be managed by MqttClientManager
    }

    Public Bool Restart() override {
        // Do nothing as MqttClient will be managed by MqttClientManager
        deviceIdentityProfile = deviceService->GetDeviceIdentityProfile();
        return false;
    }
    
    Public Bool IsRunning() const override {
        return mqttClient->IsConnected();
    }
    
    Public IHttpRequestPtr ReceiveMessage() override {
        if(!deviceIdentityProfile.has_value()) {
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
        if(!deviceIdentityProfile.has_value()) {
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
        CStdString path = deviceIdentityProfile.value().publishTopics.logsTopic;
        MqttMessage mqttMessage = {
            .guid = "logs",
            .payload = logs,
        };
        return mqttClient->QueueMessageToSend(path, mqttMessage);
    }
};
#endif // CLOUDSERVER_INTERNAL_H