#ifndef INETWORKSTATUSPROVIDER_H
#define INETWORKSTATUSPROVIDER_H

#include <StandardDefines.h>

/**
 * Provides read-only network connection status (e.g. from a store).
 */
DefineStandardPointers(INetworkStatusProvider)
class INetworkStatusProvider {
    Public Virtual ~INetworkStatusProvider() = default;

    Public Virtual Bool IsNetworkConnected() const = 0;
    Public Virtual ULong GetNetworkConnectionId() const = 0;
};

#endif // INETWORKSTATUSPROVIDER_H
