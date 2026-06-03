#ifndef MQTTCLOUD_SERVER_MANAGER_INTERNAL_H
#define MQTTCLOUD_SERVER_MANAGER_INTERNAL_H

#include <cstdio>

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

    /** After network restore TLS can exceed 10s; also avoids hammering the broker. */
    static constexpr Int kMqttReconnectWaitMs = 30000;

    Private Bool TryReconnectAndSubscribe(
            const DeviceIdentityProfileData& deviceIdentityProfile,
            const char* reason) {
        printf("[Nayan] MqttClientManager TryReconnect reason=%s lastId=%lu currentId=%lu connected=%d\n",
               reason,
               (unsigned long)lastInternetConnectionId,
               (unsigned long)internetConnectionStatusProvider->GetInternetConnectionId(),
               mqttClient->IsConnected() ? 1 : 0);

        if (!mqttClient->RefreshConnection(deviceIdentityProfile)) {
            printf("[Nayan] MqttClientManager RefreshConnection returned false\n");
            return false;
        }

        if (!mqttClient->WaitForConnection(kMqttReconnectWaitMs)) {
            printf("[Nayan] MqttClientManager WaitForConnection failed after %dms (client may still be connecting)\n",
                   kMqttReconnectWaitMs);
            return false;
        }

        mqttClient->Subscribe(deviceIdentityProfile.subscribeTopics.commandTopic);
        mqttClient->Subscribe(deviceIdentityProfile.subscribeTopics.otaUpdateTopic);
        mqttClient->Subscribe(deviceIdentityProfile.subscribeTopics.featureFlagTopic);
        printf("[Nayan] MqttClientManager reconnect OK, subscribed\n");
        return true;
    }

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
            if (lastInternetConnectionId != 0) {
                printf("[Nayan] MqttClientManager PreCheck: internet lost (lastId=%lu -> 0)\n",
                       (unsigned long)lastInternetConnectionId);
            }
            lastInternetConnectionId = 0;
            return false;
        }

        // 4. Get device identity profile
        Val deviceIdentityProfileOpt = deviceService->GetDeviceIdentityProfile();
        if (!deviceIdentityProfileOpt.has_value()) {
            printf("[Nayan] MqttClientManager PreCheck: no device identity profile\n");
            return false;
        }

        Val deviceIdentityProfile = deviceIdentityProfileOpt.value();

        Bool internetJustRestored = (lastInternetConnectionId == 0 && currentId != 0);
        Bool internetChanged = (lastInternetConnectionId != 0 && currentId != lastInternetConnectionId);

        if (internetJustRestored) {
            printf("[Nayan] MqttClientManager PreCheck: internet restored currentId=%lu\n",
                   (unsigned long)currentId);
            // Mark internet seen so we do not re-enter "restored" and call full Refresh every loop.
            lastInternetConnectionId = currentId;
            if (!TryReconnectAndSubscribe(deviceIdentityProfile, "internet_restored")) {
                return false;
            }
        } else if (internetChanged) {
            printf("[Nayan] MqttClientManager PreCheck: internet changed %lu -> %lu\n",
                   (unsigned long)lastInternetConnectionId, (unsigned long)currentId);
            lastInternetConnectionId = currentId;
            if (!TryReconnectAndSubscribe(deviceIdentityProfile, "internet_changed")) {
                return false;
            }
        } else {
            lastInternetConnectionId = currentId;
        }

        // 7. Ensure server is running (wait on in-flight connect; full refresh only when no client)
        if (!mqttClient->IsConnected()) {
            printf("[Nayan] MqttClientManager PreCheck: not connected, ensure running\n");
            if (!TryReconnectAndSubscribe(deviceIdentityProfile, "ensure_connected")) {
                return false;
            }
        }

        return mqttClient->IsConnected();
    }
};

#endif // CLOUD_SERVER_MANAGER_INTERNAL_H