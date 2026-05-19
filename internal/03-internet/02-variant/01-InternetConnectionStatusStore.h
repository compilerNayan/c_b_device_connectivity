#ifndef INTERNETCONNECTIONSTATUSSTORE_H
#define INTERNETCONNECTIONSTATUSSTORE_H

#include "../01-interface/04-IInternetConnectionStatusStore.h"
#include <StandardDefines.h>
#include <mutex>

/**
 * Thread-safe implementation: state updated under a mutex so reads/writes are consistent.
 */
/* @Component */
class InternetConnectionStatusStore : public IInternetConnectionStatusStore {
    Private mutable std::mutex mutex_;
    Private Bool internetConnected_{false};
    Private ULong internetConnectionId_{0};

    Public Bool IsInternetConnected() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return internetConnected_;
    }

    Public ULong GetInternetConnectionId() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return internetConnectionId_;
    }

    Public Void SetState(Bool internetConnected, ULong internetConnectionId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        internetConnected_ = static_cast<bool>(internetConnected);
        internetConnectionId_ = internetConnectionId;
    }
};

#endif // INTERNETCONNECTIONSTATUSSTORE_H
