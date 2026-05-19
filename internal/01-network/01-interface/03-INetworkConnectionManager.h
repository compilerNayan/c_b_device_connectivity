#ifndef INETWORKCONNECTIONMANAGER_H
#define INETWORKCONNECTIONMANAGER_H

#include <StandardDefines.h>

/**
 * Network connection: connect/disconnect, query status (minimal network-level API).
 */
DefineStandardPointers(INetworkConnectionManager)
class INetworkConnectionManager {
    Public Virtual ~INetworkConnectionManager() = default;

    Public Virtual Void ConnectNetwork() = 0;
    Public Virtual Void DisconnectNetwork() = 0;
    Public Virtual Bool IsNetworkConnected() = 0;
    Public Virtual ULong GetNetworkConnectionId() = 0;
};

#endif // INETWORKCONNECTIONMANAGER_H
