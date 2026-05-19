#ifndef WIFICONNECTIONSTATUSPROVIDER_H
#define WIFICONNECTIONSTATUSPROVIDER_H

#include "../01-interface/02-IWiFiConnectionStatusProvider.h"
#include "../01-interface/01-IWiFiConnectionStatusStore.h"
#include "../../01-network/00-public/01-INetworkStatusProvider.h"
#include <StandardDefines.h>

/**
 * Implementation that reads network/WiFi status from IWiFiConnectionStatusStore.
 */
/* @Component */
class WiFiConnectionStatusProvider : public IWiFiConnectionStatusProvider {
    /* @Autowired */
    Private IWiFiConnectionStatusStorePtr store;

    /* @Autowired */
    Private INetworkStatusProviderPtr networkStatusProvider;

    Public Bool IsNetworkConnected() const override {
        return networkStatusProvider->IsNetworkConnected();
    }

    Public Bool IsWiFiConnected() const override {
        return store->IsWiFiConnected();
    }

    Public Bool IsHotspotConnected() const override {
        return store->IsHotspotConnected();
    }

    Public ULong GetWiFiConnectionId() const override {
        return store->GetWiFiConnectionId();
    }

    Public ULong GetHotspotConnectionId() const override {
        return store->GetHotspotConnectionId();
    }

    Public ULong GetNetworkConnectionId() const override {
        ULong w = store->GetWiFiConnectionId();
        ULong h = store->GetHotspotConnectionId();
        return w ? w : h;
    }
};

#endif // WIFICONNECTIONSTATUSPROVIDER_H
