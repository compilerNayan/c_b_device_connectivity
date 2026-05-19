#ifndef IINTERNETCONNECTIONSTATUSPROVIDER_H
#define IINTERNETCONNECTIONSTATUSPROVIDER_H

#include <StandardDefines.h>

/**
 * Provides read-only internet connection status (e.g. from a store).
 */
DefineStandardPointers(IInternetConnectionStatusProvider)
class IInternetConnectionStatusProvider {
    Public Virtual ~IInternetConnectionStatusProvider() = default;

    Public Virtual Bool IsNetworkConnected() const = 0;
    Public Virtual Bool IsWiFiConnected() const = 0;
    Public Virtual Bool IsHotspotConnected() const = 0;
    Public Virtual Bool IsInternetConnected() const = 0;
    Public Virtual ULong GetWiFiConnectionId() const = 0;
    Public Virtual ULong GetHotspotConnectionId() const = 0;
    Public Virtual ULong GetNetworkConnectionId() const = 0;
    Public Virtual ULong GetInternetConnectionId() const = 0;
};

#endif // IINTERNETCONNECTIONSTATUSPROVIDER_H
