#ifdef ARDUINO
#ifndef INTERNETCONNECTIONMANAGER_H
#define INTERNETCONNECTIONMANAGER_H

#include "../01-interface/03-IInternetConnectionManager.h"
#include "../01-interface/01-IInternetConnectionStatusStore.h"
#include "../../02-wifi/01-interface/03-IWiFiConnectionManager.h"
#include <StandardDefines.h>
#include <ILogger.h>
#include <osal/Core.h>
#include <osal/WiFiClient.h>

namespace {
struct InternetCheckPair {
    const char* ip1;
    const char* ip2;
};
const InternetCheckPair kInternetCheckPairs[] = {
    { "208.67.222.123", "1.0.0.1" },       // FamilyShield, Cloudflare
    { "8.8.4.4", "208.67.220.220" },       // Google, OpenDNS
    { "156.154.70.1", "64.6.64.6" },       // Neustar, Verisign
    { "195.46.39.39", "199.85.127.10" },   // SafeDNS, Norton
    { "76.76.2.0", "94.140.15.15" },       // Control D, AdGuard
    { "208.67.220.123", "156.154.71.1" },   // FamilyShield, Neustar
    { "1.1.1.1", "208.67.222.222" },       // Cloudflare, OpenDNS
    { "8.8.8.8", "64.6.65.6" },            // Google, Verisign
    { "195.46.39.40", "76.76.10.0" },      // SafeDNS, Control D
    { "199.85.126.10", "76.76.19.19" },    // Norton, Alternate
};
const size_t kNumInternetCheckPairs = sizeof(kInternetCheckPairs) / sizeof(kInternetCheckPairs[0]);
}

/* @Component */
class InternetConnectionManager : public IInternetConnectionManager {
    /* @Autowired */
    Private IWiFiConnectionManagerPtr wiFiConnectionManager;
    /* @Autowired */
    Private IInternetConnectionStatusStorePtr internetStatusStore;
    /* @Autowired */
    Private ILoggerPtr logger;

    Private size_t nextInternetCheckPairIndex_ = 0;

    /** Check internet reachability via DNS server TCP connect (port 53). Updates store with result. */
    Private Bool HasInternet() {
        const InternetCheckPair& pair = kInternetCheckPairs[nextInternetCheckPairIndex_];
        nextInternetCheckPairIndex_ = (nextInternetCheckPairIndex_ + 1) % kNumInternetCheckPairs;

        OSAL_WiFiClient client;
        if (client.Connect(pair.ip1, 53, 2000)) {
            client.Stop();
            ULong currentId = internetStatusStore->GetInternetConnectionId();
            internetStatusStore->SetState(true, currentId != 0 ? currentId : OSAL_GenerateConnectionId());
            if (currentId == 0)
                logger->Info(Tag::Untagged, StdString("[InternetConnection] Internet check succeeded via " + StdString(pair.ip1) + " (connection id set)"));
            return true;
        }
        if (client.Connect(pair.ip2, 53, 2000)) {
            client.Stop();
            ULong currentId = internetStatusStore->GetInternetConnectionId();
            internetStatusStore->SetState(true, currentId != 0 ? currentId : OSAL_GenerateConnectionId());
            if (currentId == 0)
                logger->Info(Tag::Untagged, StdString("[InternetConnection] Internet check succeeded via " + StdString(pair.ip2) + " (connection id set)"));
            return true;
        }
        logger->Warning(Tag::Untagged, StdString("[InternetConnection] Internet check failed for both " + StdString(pair.ip1) + " and " + StdString(pair.ip2)));
        internetStatusStore->SetState(false, 0);
        return false;
    }

    Public Virtual ~InternetConnectionManager() = default;

    Public Virtual Void ConnectNetwork() override {
        wiFiConnectionManager->ConnectNetwork();
    }

    Public Virtual Void DisconnectNetwork() override {
        wiFiConnectionManager->DisconnectNetwork();
        internetStatusStore->SetState(false, 0);
        if (logger) logger->Info(Tag::Untagged, StdString("[InternetConnection] DisconnectNetwork: connection id cleared"));
    }

    Public Virtual Bool IsNetworkConnected() override {
        return wiFiConnectionManager->IsNetworkConnected();
    }

    Public Virtual Bool IsWiFiConnected() override {
        return wiFiConnectionManager->IsWiFiConnected();
    }

    Public Virtual Bool IsHotspotConnected() override {
        return wiFiConnectionManager->IsHotspotConnected();
    }

    Public Virtual Bool IsInternetConnected() override {
        if (!wiFiConnectionManager->IsNetworkConnected()) return false;
        return HasInternet();
    }

    Public Virtual ULong GetWiFiConnectionId() override {
        return wiFiConnectionManager->GetWiFiConnectionId();
    }

    Public Virtual ULong GetNetworkConnectionId() override {
        return wiFiConnectionManager->GetNetworkConnectionId();
    }

    Public Virtual Bool EnsureNetworkConnectivity() override {
        return wiFiConnectionManager->EnsureNetworkConnectivity();
    }

    Public Virtual Bool VerifyInternetConnectivity() override {
        if(!IsNetworkConnected()) {
            return false;
        }
        if(IsHotspotConnected()) {
            return false;
        }
        if(IsWiFiConnected()) {
            return HasInternet();
        }
        return false;
    }

    Public Virtual Void RestartNetwork() override {
        wiFiConnectionManager->RestartNetwork();
        internetStatusStore->SetState(false, 0);
        if (logger) logger->Info(Tag::Untagged, StdString("[InternetConnection] RestartNetwork: connection id cleared"));
    }
};

#endif // INTERNETCONNECTIONMANAGER_H
#endif // ARDUINO
