#ifndef CLOUDSERVER_INTERNAL_H
#define CLOUDSERVER_INTERNAL_H

#include <StandardDefines.h>
#include "communication/ICloudServer.h"
#include "server/IMqttClient.h"
#include "server/IDeviceService.h"

/* @Component */
class CloudServer final : public ICloudServer {
    Public CloudServer() = default;
    Public Virtual ~CloudServer() override = default;

    /* @Autowired */
    Private IMqttClientPtr mqttClient;
    
    /* @Autowired */
    Private IDeviceServicePtr deviceService;

    Private Optional<DeviceIdentityProfileData> deviceIdentityProfile;

    Public Bool Start() override {
        deviceIdentityProfile = deviceService->GetDeviceIdentityProfile();
        if(!deviceIdentityProfile.has_value()) {
            return false;
        }
        mqttClient->Connect(deviceIdentityProfile.value().mqttEndpoint, deviceIdentityProfile.value().thingName, deviceIdentityProfile.value().caCertificatePem, deviceIdentityProfile.value().clientCertificatePem, deviceIdentityProfile.value().clientPrivateKeyPem);
        if(mqttClient->WaitForConnection(10000)) {
            if(mqttClient->IsConnected()) { 
                mqttClient->Subscribe(deviceIdentityProfile.value().subscribeTopics.commandTopic);
                mqttClient->Subscribe(deviceIdentityProfile.value().subscribeTopics.otaUpdateTopic);
                mqttClient->Subscribe(deviceIdentityProfile.value().subscribeTopics.featureFlagTopic);
            }
            return true;
        }
        return false;
    }

    Public Void Stop() override {
        mqttClient->Disconnect();
    }

    Public Bool Restart() override {
        deviceIdentityProfile = deviceService->GetDeviceIdentityProfile();
        if(!deviceIdentityProfile.has_value()) {
            return false;
        }
        mqttClient->Disconnect();
        Thread::Sleep(5000);
        mqttClient->Connect(deviceIdentityProfile.value().mqttEndpoint, deviceIdentityProfile.value().thingName, deviceIdentityProfile.value().caCertificatePem, deviceIdentityProfile.value().clientCertificatePem, deviceIdentityProfile.value().clientPrivateKeyPem);
        if(mqttClient->WaitForConnection(10000)) {
            if(mqttClient->IsConnected()) { 
                mqttClient->Subscribe(deviceIdentityProfile.value().subscribeTopics.commandTopic);
                mqttClient->Subscribe(deviceIdentityProfile.value().subscribeTopics.otaUpdateTopic);
                mqttClient->Subscribe(deviceIdentityProfile.value().subscribeTopics.featureFlagTopic);
                return true;
            }
        }
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
};
#endif // CLOUDSERVER_INTERNAL_H