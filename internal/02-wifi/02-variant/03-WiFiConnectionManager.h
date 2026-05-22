#ifdef ESP_PLATFORM
#ifndef WIFICONNECTIONMANAGER_H
#define WIFICONNECTIONMANAGER_H

#include "../01-interface/03-IWiFiConnectionManager.h"
#include "../01-interface/01-IWiFiConnectionStatusStore.h"
#include <StandardDefines.h>
#include "util/StringUtils.h"
#include "logger/ILogger.h"
#include "../../../endpoint/service/IWiFiService.h"
#include "../../../endpoint/entity/WiFiCredentials.h"

#include "wifi/IWiFiManager.h"
#include "wifi/IHotspotManager.h"
#include "util/GenericUtil.h"
#include "Thread.h"

/* @Component */
class WiFiConnectionManager : public IWiFiConnectionManager {
    /* @Autowired */
    Private IWiFiServicePtr wiFiService;
    /* @Autowired */
    Private IWiFiConnectionStatusStorePtr statusStore;
    /* @Autowired */
    Private ILoggerPtr logger;
    /* @Autowired */
    Private IWiFiManagerPtr wifiManager;
    /* @Autowired */
    Private IHotspotManagerPtr hotspotManager;

    Private ULong wiFiConnectionId_ = 0;
    Private ULong hotspotConnectionId_ = 0;

    Private Void UpdateStore(Bool wiFiConnected, Bool hotspotConnected, ULong wiFiConnectionId, ULong hotspotConnectionId) {
        statusStore->SetState(wiFiConnected, hotspotConnected, wiFiConnectionId, hotspotConnectionId);
    }

    Private Bool TryConnectToWiFi(const StdString& ssid, const StdString& password) {
        logger->Info(Tag::Untagged, StdString("[WiFiConnection] Attempting to connect to WiFi - SSID: " + ssid));
        logger->Info(Tag::Untagged, StdString(password.empty() ? "[WiFiConnection] No password (open network)" : "[WiFiConnection] Using password"));
        wifiManager->Disconnect();
        wifiManager->Connect(ssid, Optional<StdString>(password));
        wifiManager->WaitForConnection(10000);

        if (wifiManager->IsConnected()) {
            wiFiConnectionId_ = GenericUtil::GenerateConnectionId();
            hotspotConnectionId_ = 0;
            UpdateStore(true, false, wiFiConnectionId_, 0);
            if(wifiManager->GetIPAddress().has_value()) {
                logger->Info(Tag::Untagged, StdString("[WiFiConnection] WiFi connected successfully! IP Address: " + wifiManager->GetIPAddress().value()));
            } else {
                logger->Info(Tag::Untagged, StdString("[WiFiConnection] WiFi connected successfully! No IP Address available"));
            }
            return true;
        }
        logger->Error(Tag::Untagged, StdString("[WiFiConnection] WiFi connection failed or timeout for SSID: " + ssid));
        return false;
    }

    /** Step 1: Try last connected WiFi. Returns true if connected. */
    Private Bool TryLastConnectedWiFi() {
        logger->Info(Tag::Untagged, StdString("[WiFiConnection] Step 1: Checking for last connected WiFi..."));
        optional<WiFiCredentials> lastWiFi = wiFiService->GetLastConnectedWiFi();
        if (!lastWiFi.has_value() || !lastWiFi.value().ssid.has_value() || lastWiFi.value().ssid.value().empty()) {
            logger->Info(Tag::Untagged, StdString("[WiFiConnection] No last connected WiFi found"));
            return false;
        }
        StdString ssid = lastWiFi.value().ssid.value();
        StdString password = lastWiFi.value().password.has_value() ? lastWiFi.value().password.value() : "";
        logger->Info(Tag::Untagged, StdString("[WiFiConnection] Last connected WiFi found - SSID: " + ssid));
        if (!TryConnectToWiFi(ssid, password)) {
            logger->Warning(Tag::Untagged, StdString("[WiFiConnection] Failed to connect to last connected WiFi, trying other credentials..."));
            return false;
        }
        wiFiService->UpdateLastConnectedSsid(ssid);
        UpdateStore(true, false, wiFiConnectionId_, 0);
        logger->Info(Tag::Untagged, StdString("[WiFiConnection] Successfully connected to last connected WiFi"));
        return true;
    }

    /** Step 2: Try all stored credentials. Returns true if connected. */
    Private Bool TryAllStoredCredentials() {
        logger->Info(Tag::Untagged, StdString("[WiFiConnection] Step 2: Getting all WiFi credentials from database..."));
        StdVector<WiFiCredentials> allCredentials = wiFiService->GetAllWiFiCredentials();
        if (allCredentials.empty()) {
            logger->Warning(Tag::Untagged, StdString("[WiFiConnection] No WiFi credentials found in database"));
            return false;
        }
        logger->Info(Tag::Untagged, cpp_utils::Format("[WiFiConnection] Found {} WiFi credential(s) in database", allCredentials.size()));
        for (size_t i = 0; i < allCredentials.size(); i++) {
            const WiFiCredentials& cred = allCredentials[i];
            StdString ssid = cred.ssid.value();
            StdString password = cred.password.has_value() ? cred.password.value() : "";
            logger->Info(Tag::Untagged, StdString("[WiFiConnection] Trying credential " + cpp_utils::ToString(i + 1) + " of " + cpp_utils::ToString(allCredentials.size()) + " - SSID: " + ssid));
            if (TryConnectToWiFi(ssid, password)) {
                wiFiService->UpdateLastConnectedSsid(ssid);
                UpdateStore(true, false, wiFiConnectionId_, 0);
                logger->Info(Tag::Untagged, StdString("[WiFiConnection] Successfully connected to WiFi: " + ssid));
                logger->Info(Tag::Untagged, StdString("[WiFiConnection] Updated last connected WiFi"));
                return true;
            }
        }
        logger->Warning(Tag::Untagged, StdString("[WiFiConnection] All WiFi credentials failed to connect"));
        return false;
    }

    /** Step 3: Start hotspot when no WiFi is available. */
    Private Void StartHotspotFallback() {
        logger->Info(Tag::Untagged, StdString("[WiFiConnection] Step 3: Starting hotspot (no WiFi connections available or all failed)"));
        logger->Info(Tag::Untagged, StdString("[WiFiConnection] Hotspot SSID: SmartBoard (open, no password)"));

        hotspotManager->Stop();
        hotspotManager->Start("SmartBoard", std::nullopt);
        hotspotManager->WaitForStart(10000);
        if (hotspotManager->IsActive()) {
            hotspotConnectionId_ = GenericUtil::GenerateConnectionId();
            wiFiConnectionId_ = 0;
            UpdateStore(false, true, 0, hotspotConnectionId_);
            if(hotspotManager->GetIPAddress().has_value()) {
                logger->Info(Tag::Untagged, StdString("[WiFiConnection] Hotspot started successfully! AP IP Address: " + hotspotManager->GetIPAddress().value()));
            } else {
                logger->Info(Tag::Untagged, StdString("[WiFiConnection] Hotspot started successfully! No IP Address available"));
            }
        } else {
            logger->Error(Tag::Untagged, StdString("[WiFiConnection] Failed to start hotspot"));
        }
    }

    Public Virtual ~WiFiConnectionManager() = default;

    Public Virtual Void ConnectNetwork() override {
        logger->Info(Tag::Untagged, StdString("[WiFiConnection] ConnectNetwork() called"));
        if (TryLastConnectedWiFi()) return;
        if (TryAllStoredCredentials()) return;
        StartHotspotFallback();
    }

    Public Virtual Void DisconnectNetwork() override {
        logger->Info(Tag::Untagged, StdString("[WiFiConnection] DisconnectNetwork() called"));
        hotspotManager->Stop();
        wifiManager->Disconnect();
        UpdateStore(false, false, 0, 0);
        logger->Info(Tag::Untagged, StdString("[WiFiConnection] Network disconnected"));
    }

    Public Virtual Bool IsNetworkConnected() override {
        return wifiManager->IsConnected() || hotspotManager->IsActive();
    }

    Public Virtual Bool IsWiFiConnected() override {
        return wifiManager->IsConnected();
    }

    Public Virtual Bool IsHotspotConnected() override {
        return hotspotManager->IsActive();
    }

    Public Virtual ULong GetWiFiConnectionId() override {
        return wiFiConnectionId_;
    }

    Public Virtual ULong GetNetworkConnectionId() override {
        return wiFiConnectionId_ || hotspotConnectionId_;
    }

    Public Virtual Bool EnsureNetworkConnectivity() override {
        if (IsHotspotConnected()) {
            return IsNetworkConnected();
        }
        if (IsWiFiConnected()) {
            return true;
        }
        logger->Info(Tag::Untagged, StdString("[WiFiConnection] EnsureNetworkConnectivity: WiFi not connected, restarting to reconnect"));
        RestartNetwork();
        return IsNetworkConnected();
    }

    Public Virtual Void RestartNetwork() override {
        logger->Info(Tag::Untagged, StdString("[WiFiConnection] RestartNetwork() called"));
        DisconnectNetwork();
        Thread::Sleep(1000);
        logger->Info(Tag::Untagged, StdString("[WiFiConnection] Reconnecting..."));
        ConnectNetwork();
    }
};

#endif // WIFICONNECTIONMANAGER_H
#endif // ARDUINO
