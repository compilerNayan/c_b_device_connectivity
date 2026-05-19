#ifndef INTERNETCONNECTIONSTATUSPROVIDER_H
#define INTERNETCONNECTIONSTATUSPROVIDER_H

#include "../01-interface/02-IInternetConnectionStatusProvider.h"
#include "../01-interface/01-IInternetConnectionStatusStore.h"
#include "../../02-wifi/01-interface/02-IWiFiConnectionStatusProvider.h"
#include <StandardDefines.h>

/**
 * Implementation that reads network/WiFi status from IWiFiConnectionStatusProvider
 * and internet status from IInternetConnectionStatusStore.
 */
/* @Component */
class InternetConnectionStatusProvider : public IInternetConnectionStatusProvider {
    /* @Autowired */
    Private IWiFiConnectionStatusProviderPtr wiFiStatusProvider;
    /* @Autowired */
    Private IInternetConnectionStatusStorePtr store;

    Public Bool IsNetworkConnected() const override {
        return wiFiStatusProvider->IsNetworkConnected();
    }

    Public Bool IsWiFiConnected() const override {
        return wiFiStatusProvider->IsWiFiConnected();
    }

    Public Bool IsHotspotConnected() const override {
        return wiFiStatusProvider->IsHotspotConnected();
    }

    Public Bool IsInternetConnected() const override {
        return store->IsInternetConnected();
    }

    Public ULong GetWiFiConnectionId() const override {
        return wiFiStatusProvider->GetWiFiConnectionId();
    }

    Public ULong GetHotspotConnectionId() const override {
        return wiFiStatusProvider->GetHotspotConnectionId();
    }

    Public ULong GetNetworkConnectionId() const override {
        return wiFiStatusProvider->GetNetworkConnectionId();
    }

    Public ULong GetInternetConnectionId() const override {
        return store->GetInternetConnectionId();
    }
};

#endif // INTERNETCONNECTIONSTATUSPROVIDER_H
