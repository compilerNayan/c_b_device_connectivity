#ifndef WIFICONNECTIONSTATUSSTORE_H
#define WIFICONNECTIONSTATUSSTORE_H

#include "../01-interface/01-IWiFiConnectionStatusStore.h"
#include <StandardDefines.h>
#include <mutex>

/**
 * Thread-safe implementation: all state is updated under a mutex so reads/writes are consistent.
 */
/* @Component */
class WiFiConnectionStatusStore : public IWiFiConnectionStatusStore {
    Private mutable std::mutex mutex_;
    Private Bool wiFiConnected_{false};
    Private Bool hotspotConnected_{false};
    Private ULong wiFiConnectionId_{0};
    Private ULong hotspotConnectionId_{0};

    Public Bool IsWiFiConnected() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return wiFiConnected_;
    }

    Public Bool IsHotspotConnected() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return hotspotConnected_;
    }

    Public ULong GetWiFiConnectionId() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return wiFiConnectionId_;
    }

    Public ULong GetHotspotConnectionId() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return hotspotConnectionId_;
    }

    Public Void SetState(Bool wiFiConnected, Bool hotspotConnected,
                        ULong wiFiConnectionId, ULong hotspotConnectionId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        wiFiConnected_ = static_cast<bool>(wiFiConnected);
        hotspotConnected_ = static_cast<bool>(hotspotConnected);
        wiFiConnectionId_ = wiFiConnectionId;
        hotspotConnectionId_ = hotspotConnectionId;
    }
};

#endif // WIFICONNECTIONSTATUSSTORE_H
