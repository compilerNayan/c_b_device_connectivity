#ifndef NETWORKCONNECTIONMANAGER_H
#define NETWORKCONNECTIONMANAGER_H

#include "../01-interface/03-INetworkConnectionManager.h"
#include "../01-interface/01-INetworkStatusStore.h"
#include <StandardDefines.h>
#include <ILogger.h>

/**
 * Dummy implementation: ConnectNetwork only logs and sets store to a dummy connected state;
 * DisconnectNetwork logs and clears store. No real hardware/OSAL calls.
 */
/* @Component */
class NetworkConnectionManager : public INetworkConnectionManager {
    /* @Autowired */
    Private INetworkStatusStorePtr statusStore;
    /* @Autowired */
    Private ILoggerPtr logger;

    Public Virtual ~NetworkConnectionManager() = default;

    Public Virtual Void ConnectNetwork() override {
        logger->Info(Tag::Untagged, StdString("[NetworkConnection] ConnectNetwork() called (dummy)"));
        statusStore->SetState(true, 1);
        logger->Info(Tag::Untagged, StdString("[NetworkConnection] Dummy connected, connection id: 1"));
    }

    Public Virtual Void DisconnectNetwork() override {
        logger->Info(Tag::Untagged, StdString("[NetworkConnection] DisconnectNetwork() called"));
        statusStore->SetState(false, 0);
        logger->Info(Tag::Untagged, StdString("[NetworkConnection] Disconnected"));
    }

    Public Virtual Bool IsNetworkConnected() override {
        return statusStore->IsNetworkConnected();
    }

    Public Virtual ULong GetNetworkConnectionId() override {
        return statusStore->GetNetworkConnectionId();
    }
};

#endif // NETWORKCONNECTIONMANAGER_H
