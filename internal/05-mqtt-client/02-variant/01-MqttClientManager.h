#ifndef MQTTCLOUD_SERVER_MANAGER_INTERNAL_H
#define MQTTCLOUD_SERVER_MANAGER_INTERNAL_H

#include "../01-interface/01-IMqttClientManager.h"
#include "server/IMqttClient.h"

/* @Component */
class MqttClientManager final : public IMqttClientManager {
    Public MqttClientManager() = default;
    Public Virtual ~MqttClientManager() override = default;

    /* @Autowired */
    Private IMqttClientPtr mqttClient;

    /* @Autowired */
    Private IInternetConnectionStatusProviderPtr internetConnectionStatusProvider;

    // Track last known internet connection ID
    Private ULong lastInternetConnectionId = 0;

    Public Virtual Void EnsureMqttClientConnectivity() override {
        if((PreCheck())) {
            mqttClient->SendMessage();
        }
    }

    Private Bool PreCheck() {
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

        // 4. If last ID == 0 and now connected → restart server
        if (lastInternetConnectionId == 0 && currentId != 0) {
            if (!mqttClient->RefreshConnection()) {
                return false;
            }
        }
        // 5. If last ID != 0 and current ID != last ID → internet changed → restart server
        else if (lastInternetConnectionId != 0 && currentId != lastInternetConnectionId) {
            if (!mqttClient->RefreshConnection()) {
                return false;
            }
        }

        // 6. Update last known ID
        lastInternetConnectionId = currentId;

        // 7. Ensure server is running
        if (!mqttClient->IsConnected()) {
            if (!mqttClient->RefreshConnection()) {
                return false;
            }
        }

        // 8. Final return
        return mqttClient->IsConnected();
    }
};

#endif // CLOUD_SERVER_MANAGER_INTERNAL_H