#ifndef IWIFICONNECTIONSTATUSPROVIDER_H
#define IWIFICONNECTIONSTATUSPROVIDER_H

#include <StandardDefines.h>

/**
 * Provides read-only network and WiFi connection status (e.g. from a store).
 */
DefineStandardPointers(IWiFiConnectionStatusProvider)
class IWiFiConnectionStatusProvider {
    Public Virtual ~IWiFiConnectionStatusProvider() = default;

    Public Virtual Bool IsNetworkConnected() const = 0;
    Public Virtual Bool IsWiFiConnected() const = 0;
    Public Virtual Bool IsHotspotConnected() const = 0;
    Public Virtual ULong GetWiFiConnectionId() const = 0;
    Public Virtual ULong GetHotspotConnectionId() const = 0;
    Public Virtual ULong GetNetworkConnectionId() const = 0;
};

#endif // IWIFICONNECTIONSTATUSPROVIDER_H
