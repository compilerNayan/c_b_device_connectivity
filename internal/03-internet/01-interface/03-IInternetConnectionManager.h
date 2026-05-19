#ifndef IINTERNETCONNECTIONMANAGER_H
#define IINTERNETCONNECTIONMANAGER_H

#include <StandardDefines.h>

/**
 * Internet/network connection: same as IWiFiConnectionManager plus internet reachability.
 */
DefineStandardPointers(IInternetConnectionManager)
class IInternetConnectionManager {
    Public Virtual ~IInternetConnectionManager() = default;

    Public Virtual Void ConnectNetwork() = 0;
    Public Virtual Void DisconnectNetwork() = 0;
    Public Virtual Bool IsNetworkConnected() = 0;
    Public Virtual Bool IsWiFiConnected() = 0;
    Public Virtual Bool IsHotspotConnected() = 0;
    Public Virtual Bool IsInternetConnected() = 0;
    Public Virtual ULong GetWiFiConnectionId() = 0;
    Public Virtual ULong GetNetworkConnectionId() = 0;
    Public Virtual Bool EnsureNetworkConnectivity() = 0;
    Public Virtual Bool VerifyInternetConnectivity() = 0;
    Public Virtual Void RestartNetwork() = 0;
};

#endif // IINTERNETCONNECTIONMANAGER_H
