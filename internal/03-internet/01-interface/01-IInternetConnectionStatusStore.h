#ifndef IINTERNETCONNECTIONSTATUSSTORE_H
#define IINTERNETCONNECTIONSTATUSSTORE_H

#include <StandardDefines.h>

/**
 * Thread-safe store for internet connection status.
 */
DefineStandardPointers(IInternetConnectionStatusStore)
class IInternetConnectionStatusStore {
    Public Virtual ~IInternetConnectionStatusStore() = default;

    Public Virtual Bool IsInternetConnected() const = 0;
    Public Virtual ULong GetInternetConnectionId() const = 0;

    /** Set connection state in one transaction (thread-safe atomic update). */
    Public Virtual Void SetState(Bool internetConnected, ULong internetConnectionId) = 0;
};

#endif // IINTERNETCONNECTIONSTATUSSTORE_H
