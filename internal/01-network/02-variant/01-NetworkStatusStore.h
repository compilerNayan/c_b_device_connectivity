#ifndef NETWORKSTATUSSTORE_H
#define NETWORKSTATUSSTORE_H

#include "../01-interface/01-INetworkStatusStore.h"
#include <StandardDefines.h>
#include <mutex>

/**
 * Thread-safe implementation: state updated under a mutex so reads/writes are consistent.
 */
/* @Component */
class NetworkStatusStore : public INetworkStatusStore {
    Private mutable std::mutex mutex_;
    Private Bool networkConnected_{false};
    Private ULong networkConnectionId_{0};

    Public Bool IsNetworkConnected() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return networkConnected_;
    }

    Public ULong GetNetworkConnectionId() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return networkConnectionId_;
    }

    Public Void SetState(Bool networkConnected, ULong networkConnectionId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        networkConnected_ = static_cast<bool>(networkConnected);
        networkConnectionId_ = networkConnectionId;
    }
};

#endif // NETWORKSTATUSSTORE_H
