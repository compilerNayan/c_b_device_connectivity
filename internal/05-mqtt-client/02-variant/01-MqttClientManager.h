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
    static constexpr Int kPostEnrollmentSettleMs = 3000;

    Private Bool IsInternetAvailable() const {
        return internetConnectionStatusProvider->IsInternetConnected()
            && internetConnectionStatusProvider->GetInternetConnectionId() != 0;
    }

    Public Virtual Void EnsureMqttClientConnectivity() override {
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
        const Bool enrolledNow = fleetProvisioningService->IsEnrolled();
        const Bool justEnrolled = enrolledNow && !wasEnrolledOnLastCheck;
        wasEnrolledOnLastCheck = enrolledNow;

        if (!enrolledNow) {
            return false;
        }

        if (justEnrolled) {
            lastInternetConnectionId = 0;
        }

        // 1. Null checks
        if (!internetConnectionStatusProvider) {
            return false;
        }

        // 2–3. No internet → tear down mqtt (stops ESP-IDF auto-reconnect / BEFORE_CONNECT loop)
        if (!IsInternetAvailable()) {
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


        return true;
    }
};
#endif // CLOUD_SERVER_MANAGER_INTERNAL_H