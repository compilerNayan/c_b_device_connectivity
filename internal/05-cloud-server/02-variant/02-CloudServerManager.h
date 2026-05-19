#ifndef CLOUD_SERVER_MANAGER_INTERNAL_H
#define CLOUD_SERVER_MANAGER_INTERNAL_H

#include "../01-interface/02-ICloudServerManager.h"
#include "../01-interface/01-ICloudServer.h"

/* @Component */
class CloudServerManager final : public ICloudServerManager {
    Public CloudServerManager() = default;
    Public Virtual ~CloudServerManager() override = default;

    /* @Autowired */
    Private ICloudServerPtr cloudServer;

    /* @Autowired */
    Private IInternetConnectionStatusProviderPtr internetConnectionStatusProvider;

    Public Virtual Void EnsureCloudServerConnectivity() override {
        if((PreCheck())) {
            cloudServer->ReceiveMessage();
            cloudServer->SendMessage();
        }
    }

    Private Bool PreCheck() {
        // 1. Null checks
        if (!cloudServer || !internetConnectionStatusProvider) {
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
            if (!cloudServer->Restart()) {
                return false;
            }
        }
        // 5. If last ID != 0 and current ID != last ID → internet changed → restart server
        else if (lastInternetConnectionId != 0 && currentId != lastInternetConnectionId) {
            if (!cloudServer->Restart()) {
                return false;
            }
        }

        // 6. Update last known ID
        lastInternetConnectionId = currentId;

        // 7. Ensure server is running
        if (!cloudServer->IsRunning()) {
            if (!cloudServer->Restart()) {
                return false;
            }
        }

        // 8. Final return
        return cloudServer->IsRunning();
    }
};

#endif // CLOUD_SERVER_MANAGER_INTERNAL_H