#ifndef IWIFICONNECTIONSTATUSSTORE_H
#define IWIFICONNECTIONSTATUSSTORE_H

#include <StandardDefines.h>

/**
 * Thread-safe store for WiFi/hotspot connection status (no network-level getters).
 */
DefineStandardPointers(IWiFiConnectionStatusStore)
class IWiFiConnectionStatusStore {
    Public Virtual ~IWiFiConnectionStatusStore() = default;

    Public Virtual Bool IsWiFiConnected() const = 0;
    Public Virtual Bool IsHotspotConnected() const = 0;
    Public Virtual ULong GetWiFiConnectionId() const = 0;
    Public Virtual ULong GetHotspotConnectionId() const = 0;

    /** Set all connection state in one transaction (thread-safe atomic update). */
    Public Virtual Void SetState(Bool wiFiConnected, Bool hotspotConnected,
                                 ULong wiFiConnectionId, ULong hotspotConnectionId) = 0;
};

#endif // IWIFICONNECTIONSTATUSSTORE_H
