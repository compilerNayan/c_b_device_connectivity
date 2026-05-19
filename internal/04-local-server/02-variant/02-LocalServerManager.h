#ifndef LOCAL_SERVER_MANAGER_INTERNAL_H
#define LOCAL_SERVER_MANAGER_INTERNAL_H

#include "../01-interface/02-ILocalServerManager.h"
#include "../01-interface/01-ILocalServer.h"
#include "IWiFiConnectionStatusProvider.h"

/* @Component */
class LocalServerManager final : public ILocalServerManager {
    Public LocalServerManager() = default;
    Public Virtual ~LocalServerManager() override = default;

    /* @Autowired */
    Private IWiFiConnectionStatusProviderPtr wifiConnectionStatusProvider;

    /* @Autowired */
    Private ILocalServerPtr localServer;

    // Track last known network connection ID
    Private ULong lastNetworkConnectionId = 0;
    
    Public Virtual Void EnsureLocalServerConnectivity() override {
        if((PreCheck())) {
            localServer->ReceiveMessage();
            localServer->SendMessage();
        }
    }

    Private Bool PreCheck() {
        if (!localServer || !wifiConnectionStatusProvider) {
            return false;
        }

        ULong currentId = wifiConnectionStatusProvider->GetNetworkConnectionId();

        // Case 1: network disconnected (ID = 0)
        if (currentId == 0) {
            // If previously connected, now disconnected → do nothing, return false
            lastNetworkConnectionId = 0;
            return false;
        }

        // Case 2: network reconnected (ID != 0)
        if (lastNetworkConnectionId == 0) {
            // Previously disconnected, now connected → restart server
            if (!localServer->Restart()) {
                return false;
            }
        } else if (currentId != lastNetworkConnectionId) {
            // Network changed → restart server
            if (!localServer->Restart()) {
                return false;
            }
        }

        // Update last known ID
        lastNetworkConnectionId = currentId;

        // Case 3: ensure server is running
        if (!localServer->IsRunning()) {
            if (!localServer->Restart()) {
                return false;
            }
        }

        return localServer->IsRunning();
    }

};

#endif // LOCAL_SERVER_MANAGER_INTERNAL_H