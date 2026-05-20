#ifndef TCP_SERVER_MANAGER_INTERNAL_H
#define TCP_SERVER_MANAGER_INTERNAL_H

#include "server/ITcpServer.h"
#include "IWiFiConnectionStatusProvider.h"

#include "../01-interface/01-ITcpServerManager.h"

/* @Component */
class TcpServerManager final : public ITcpServerManager {
    Public TcpServerManager() = default;
    Public Virtual ~TcpServerManager() override = default;

    /* @Autowired */
    Private IWiFiConnectionStatusProviderPtr wifiConnectionStatusProvider;

    /* @Autowired */
    Private ITcpServerPtr tcpServer;

    // Track last known network connection ID
    Private ULong lastNetworkConnectionId = 0;
    
    Public Virtual Void EnsureTcpServerConnectivity() override {
        if((PreCheck())) {
            tcpServer->ReceiveMessage();
            tcpServer->SendMessage();
        }
    }

    Private Bool PreCheck() {
        if (!tcpServer || !wifiConnectionStatusProvider) {
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
            if (!tcpServer->Restart()) {
                return false;
            }
        } else if (currentId != lastNetworkConnectionId) {
            // Network changed → restart server
            if (!tcpServer->Restart()) {
                return false;
            }
        }

        // Update last known ID
        lastNetworkConnectionId = currentId;

        // Case 3: ensure server is running
        if (!tcpServer->IsRunning()) {
            if (!tcpServer->Restart()) {
                return false;
            }
        }

        return tcpServer->IsRunning();
    }

};

#endif // TCP_SERVER_MANAGER_INTERNAL_H