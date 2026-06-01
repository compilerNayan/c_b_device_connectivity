#ifndef MQTTCLOUD_SERVER_MANAGER_INTERNAL_H
#define MQTTCLOUD_SERVER_MANAGER_INTERNAL_H

#include "../01-interface/01-IMqttClientManager.h"
#include "server/IMqttClient.h"
#include "IInternetConnectionStatusProvider.h"
#include "IFleetProvisioningService.h"
#include "server/IDeviceService.h"
#include "pubsub/ICommandBus.h"

/* @Component */
class MqttClientManager final : public IMqttClientManager {
    Public MqttClientManager() = default;
    Public Virtual ~MqttClientManager() override = default;

    /* @Autowired */
    Private IMqttClientPtr mqttClient;

    /* @Autowired */
    Private IInternetConnectionStatusProviderPtr internetConnectionStatusProvider;

    /* @Autowired */
    Private IFleetProvisioningServicePtr fleetProvisioningService;

    /* @Autowired */
    Private IDeviceServicePtr deviceService;

    /* @Autowired */
    Private ICommandBusPtr commandBus;

    // Track last known internet connection ID
    Private ULong lastInternetConnectionId = 0;

    Public Virtual Void EnsureMqttClientConnectivity() override {
        if((PreCheck())) {
            mqttClient->SendMessage();
        }
    }

    Public Virtual Void EnrollDevice() override {
        if (!EnrollmentPreCheck()) {
            return;
        }
        fleetProvisioningService->EnrollDevice();
    }

    Private Bool EnrollmentPreCheck() {
        // 0. Check if device is already enrolled
        if (fleetProvisioningService->IsEnrolled()) {
            return false;
        }

        // 1. Null checks
        if (!fleetProvisioningService || !internetConnectionStatusProvider) {
            return false;
        }

        // 2. Get current internet connection ID
        ULong internetConnectionId = internetConnectionStatusProvider->GetInternetConnectionId();

        // 3. If internet connection ID == 0 → network down
        if (internetConnectionId == 0) {
            return false;
        }


        return true;
    }

    Private Bool PreCheck() {
        // 0. Check if device is enrolled
        if (!fleetProvisioningService->IsEnrolled()) {
            return false;
        }

        // 1. Null checks
        if (!mqttClient || !internetConnectionStatusProvider) {
            return false;
        }

        // 2. Get current internet connection ID
        ULong currentId = internetConnectionStatusProvider->GetInternetConnectionId();

        // 3. If current ID == 0 → network down
        if (currentId == 0) {
            // Reset last ID to 0 and return false
            lastInternetConnectionId = 0;
            return false;
        }

        // 4. Get device identity profile
        Val deviceIdentityProfileOpt = deviceService->GetDeviceIdentityProfile();
        if (!deviceIdentityProfileOpt.has_value()) {
            return false;
        }

        Val deviceIdentityProfile = deviceIdentityProfileOpt.value();

        // 4. If last ID == 0 and now connected → restart server
        if (lastInternetConnectionId == 0 && currentId != 0) {
            if (!mqttClient->RefreshConnection(deviceIdentityProfile)) {
                return false;
            } else {
                mqttClient->WaitForConnection(10000);
                if(mqttClient->IsConnected()) {
                    mqttClient->Subscribe(deviceIdentityProfile.subscribeTopics.commandTopic);
                    mqttClient->Subscribe(deviceIdentityProfile.subscribeTopics.otaUpdateTopic);
                    mqttClient->Subscribe(deviceIdentityProfile.subscribeTopics.featureFlagTopic);
                } else {
                    return false;
                }
            }
        }
        // 5. If last ID != 0 and current ID != last ID → internet changed → restart server
        else if (lastInternetConnectionId != 0 && currentId != lastInternetConnectionId) {
            if (!mqttClient->RefreshConnection(deviceIdentityProfile)) {
                return false;
            } else {
                mqttClient->WaitForConnection(10000);
                if(mqttClient->IsConnected()) {
                    mqttClient->Subscribe(deviceIdentityProfile.subscribeTopics.commandTopic);
                    mqttClient->Subscribe(deviceIdentityProfile.subscribeTopics.otaUpdateTopic);
                    mqttClient->Subscribe(deviceIdentityProfile.subscribeTopics.featureFlagTopic);
                } else {
                    return false;
                }
            }
        }

        // 6. Update last known ID
        lastInternetConnectionId = currentId;

        // 7. Ensure server is running
        if (!mqttClient->IsConnected()) {
            if (!mqttClient->RefreshConnection(deviceIdentityProfile)) {
                return false;
            }
            else {
                mqttClient->WaitForConnection(10000);
                if(mqttClient->IsConnected()) {
                    mqttClient->Subscribe(deviceIdentityProfile.subscribeTopics.commandTopic);
                    mqttClient->Subscribe(deviceIdentityProfile.subscribeTopics.otaUpdateTopic);
                    mqttClient->Subscribe(deviceIdentityProfile.subscribeTopics.featureFlagTopic);
                } else {
                    return false;
                }
            }
        }

        // 8. Final return
        return mqttClient->IsConnected();
    }
};

#endif // CLOUD_SERVER_MANAGER_INTERNAL_H