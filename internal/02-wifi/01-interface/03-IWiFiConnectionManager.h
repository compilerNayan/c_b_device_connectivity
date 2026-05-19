#ifndef IWIFICONNECTIONMANAGER_H
#define IWIFICONNECTIONMANAGER_H

#include <StandardDefines.h>

/**
 * WiFi connection: connect/disconnect WiFi or hotspot, query status, ensure connectivity.
 * Mirrors INetworkManager for use in 03_device_connectivity.
 */
DefineStandardPointers(IWiFiConnectionManager)
class IWiFiConnectionManager {
    Public Virtual ~IWiFiConnectionManager() = default;

    Public Virtual Void ConnectNetwork() = 0;
    Public Virtual Void DisconnectNetwork() = 0;
    Public Virtual Bool IsNetworkConnected() = 0;
    Public Virtual Bool IsWiFiConnected() = 0;
    Public Virtual Bool IsHotspotConnected() = 0;
    Public Virtual ULong GetWiFiConnectionId() = 0;
    Public Virtual ULong GetNetworkConnectionId() = 0;
    Public Virtual Bool EnsureNetworkConnectivity() = 0;
    Public Virtual Void RestartNetwork() = 0;
};

#endif // IWIFICONNECTIONMANAGER_H
