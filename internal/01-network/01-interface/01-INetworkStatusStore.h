#ifndef INETWORKSTATUSSTORE_H
#define INETWORKSTATUSSTORE_H

#include <StandardDefines.h>

/**
 * Minimal store for network connection status: connected flag and connection id.
 */
DefineStandardPointers(INetworkStatusStore)
class INetworkStatusStore {
    Public Virtual ~INetworkStatusStore() = default;

    Public Virtual Bool IsNetworkConnected() const = 0;
    Public Virtual ULong GetNetworkConnectionId() const = 0;

    /** Set connection state (thread-safe atomic update). */
    Public Virtual Void SetState(Bool networkConnected, ULong networkConnectionId) = 0;
};

#endif // INETWORKSTATUSSTORE_H
