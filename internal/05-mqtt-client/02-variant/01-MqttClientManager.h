#ifndef MQTTCLOUD_SERVER_MANAGER_INTERNAL_H
#define MQTTCLOUD_SERVER_MANAGER_INTERNAL_H

#include "../01-interface/01-IMqttClientManager.h"
#include "Thread.h"
#include "esp_heap_caps.h"
#include <cstdio>
#include <cstring>
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
    Private Bool wasEnrolledOnLastCheck = false;

    /** After network restore TLS can exceed 10s; also avoids hammering the broker. */
    static constexpr Int kMqttReconnectWaitMs = 30000;
    static constexpr Int kPostEnrollmentSettleMs = 3000;

    /** Stop ESP-IDF mqtt client so it does not auto-retry while internet is down. */
    Private Void StopMqttWhenNoInternet() {
        if (mqttClient && mqttClient->IsClientStarted()) {
            mqttClient->Disconnect();
        }
        lastInternetConnectionId = 0;
    }

    Private Bool IsInternetAvailable() const {
        return internetConnectionStatusProvider->IsInternetConnected()
            && internetConnectionStatusProvider->GetInternetConnectionId() != 0;
    }

    Private Bool TryReconnectAndSubscribe(
            const DeviceIdentityProfileData& deviceIdentityProfile,
            const char* reason) {

        if (strcmp(reason, "post_enrollment") == 0) {
            Thread::Sleep(kPostEnrollmentSettleMs);
        }

        if (!mqttClient->RefreshConnection(deviceIdentityProfile)) {
            fflush(stdout);
            return false;
        }

        if (!mqttClient->WaitForConnection(kMqttReconnectWaitMs)) {
            if (mqttClient->IsClientStarted()) {
                mqttClient->Disconnect();
            }
            return false;
        }

        const Bool subCmd = mqttClient->Subscribe(deviceIdentityProfile.subscribeTopics.commandTopic);
        const Bool subOta = mqttClient->Subscribe(deviceIdentityProfile.subscribeTopics.otaUpdateTopic);
        const Bool subFf = mqttClient->Subscribe(deviceIdentityProfile.subscribeTopics.featureFlagTopic);
        return subCmd && subOta && subFf;
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

        // 4. Release production MQTT/TLS heap before enrollment handshake (separate client).
        if (mqttClient && mqttClient->IsClientStarted()) {
            mqttClient->Disconnect();
            Thread::Sleep(2500);
        }

        return true;
    }

    Private Bool PreCheck() {
        const Bool enrolledNow = fleetProvisioningService->IsEnrolled();
        const Bool justEnrolled = enrolledNow && !wasEnrolledOnLastCheck;
        wasEnrolledOnLastCheck = enrolledNow;

        if (!enrolledNow) {
            return false;
        }

        if (justEnrolled) {
            lastInternetConnectionId = 0;
            if (mqttClient && mqttClient->IsClientStarted()) {
                mqttClient->Disconnect();
                Thread::Sleep(2500);
            }
        }

        // 1. Null checks
        if (!mqttClient || !internetConnectionStatusProvider) {
            return false;
        }

        // 2–3. No internet → tear down mqtt (stops ESP-IDF auto-reconnect / BEFORE_CONNECT loop)
        if (!IsInternetAvailable()) {
            StopMqttWhenNoInternet();
            return false;
        }

        ULong currentId = internetConnectionStatusProvider->GetInternetConnectionId();

        // 4. Get device identity profile
        Val deviceIdentityProfileOpt = deviceService->GetDeviceIdentityProfile();
        if (!deviceIdentityProfileOpt.has_value()) {
            return false;
        }

        Val deviceIdentityProfile = deviceIdentityProfileOpt.value();

        Bool internetJustRestored = (lastInternetConnectionId == 0 && currentId != 0);
        Bool internetChanged = (lastInternetConnectionId != 0 && currentId != lastInternetConnectionId);

        if (justEnrolled) {
            lastInternetConnectionId = currentId;
            if (!TryReconnectAndSubscribe(deviceIdentityProfile, "post_enrollment")) {
                return false;
            }
        } else if (internetJustRestored) {
            lastInternetConnectionId = currentId;
            Thread::Sleep(3000);
            if (!TryReconnectAndSubscribe(deviceIdentityProfile, "internet_restored")) {
                return false;
            }
        } else if (internetChanged) {
            lastInternetConnectionId = currentId;
            if (!TryReconnectAndSubscribe(deviceIdentityProfile, "internet_changed")) {
                return false;
            }
        } else {
            lastInternetConnectionId = currentId;
        }

        if (!mqttClient->IsConnected()) {
            if (!TryReconnectAndSubscribe(deviceIdentityProfile, "ensure_connected")) {
                return false;
            }
        }

        return mqttClient->IsConnected();
    }
};
#endif // CLOUD_SERVER_MANAGER_INTERNAL_H