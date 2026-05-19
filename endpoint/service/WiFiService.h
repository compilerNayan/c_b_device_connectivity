#ifndef WIFISERVICE_H
#define WIFISERVICE_H

#include <StandardDefines.h>
#include "IWiFiService.h"
#include "../repository/WiFiCredentialsRepository.h"
#include "../repository/WiFiConnectionRepository.h"
#include "../entity/WiFiCredentials.h"
#include "../entity/WiFiConnection.h"
//#include "../IArduinoSpringBootApp.h"

/* @Service */
class WiFiService : public IWiFiService {
    Public Virtual ~WiFiService() = default;

    /* @Autowired */
    WiFiCredentialsRepositoryPtr wiFiCredentialsRepository;

    /* @Autowired */
    WiFiConnectionRepositoryPtr wiFiConnectionRepository;

    /* -- @Autowired */
    //IArduinoSpringBootAppPtr arduinoSpringBootApp;

    // Helper method to check last connected WiFi and restart app if invalid
    Private Void CheckAndRestartIfNeeded() {
        optional<WiFiCredentials> lastWiFi = GetLastConnectedWiFi();
        
        // Check if last connected WiFi is not present or SSID is null/empty
        if (!lastWiFi.has_value() || 
            !lastWiFi.value().ssid.has_value() || 
            lastWiFi.value().ssid.value().empty()) {
            // Last connected WiFi is invalid, restart the app
            /*if (arduinoSpringBootApp != nullptr) {
                arduinoSpringBootApp->RestartApp();
            } */
        }
    }

    // Add WiFi credentials
    Public Virtual WiFiCredentials AddWiFiCredentials(const WiFiCredentials& credentials) override {
        // Save credentials
        WiFiCredentials creds = credentials;
        WiFiCredentials saved = wiFiCredentialsRepository->Save(creds);
        
        // Check if last connected WiFi is still valid and restart if needed
        CheckAndRestartIfNeeded();
        
        return saved;
    }

    // Update WiFi credentials
    Public Virtual WiFiCredentials UpdateWiFiCredentials(const WiFiCredentials& credentials) override {
        // Update credentials
        WiFiCredentials creds = credentials;
        WiFiCredentials updated = wiFiCredentialsRepository->Update(creds);
        
        // Check if last connected WiFi is still valid and restart if needed
        CheckAndRestartIfNeeded();
        
        return updated;
    }

    // Delete WiFi credentials by SSID
    Public Virtual Void DeleteWiFiCredentials(const StdString& ssid) override {
        // Check if this SSID is the last connected one
        StdVector<WiFiConnection> connections = wiFiConnectionRepository->FindAll();
        if (!connections.empty()) {
            WiFiConnection connection = connections[0];
            if (connection.lastConnectedSsid.has_value() && 
                connection.lastConnectedSsid.value() == ssid) {
                // Clear the last connected SSID since we're deleting it
                connection.lastConnectedSsid = std::nullopt;
                wiFiConnectionRepository->Update(connection);
            }
        }
        
        // Delete the credentials
        wiFiCredentialsRepository->DeleteById(ssid);
        
        // Check if last connected WiFi is still valid and restart if needed
        CheckAndRestartIfNeeded();
    }

    // Read WiFi credentials by SSID
    Public Virtual WiFiCredentials GetWiFiCredentials(const StdString& ssid) override {
        optional<WiFiCredentials> result = wiFiCredentialsRepository->FindById(ssid);
        if (result.has_value()) {
            return result.value();
        }
        // Return empty WiFiCredentials if not found
        return WiFiCredentials();
    }

    // Get all WiFi credentials
    Public Virtual StdVector<WiFiCredentials> GetAllWiFiCredentials() override {
        return wiFiCredentialsRepository->FindAll();
    }

    // Get last connected WiFi details
    Public Virtual optional<WiFiCredentials> GetLastConnectedWiFi() override {
        // Get all connection records (should only be one, but we'll check all)
        StdVector<WiFiConnection> connections = wiFiConnectionRepository->FindAll();
        
        if (connections.empty()) {
            return std::nullopt;
        }
        
        // Get the first connection record (assuming single record for last connected)
        // If multiple exist, we'll use the first one
        WiFiConnection connection = connections[0];
        
        if (!connection.lastConnectedSsid.has_value() || connection.lastConnectedSsid.value().empty()) {
            return std::nullopt;
        }
        
        // Fetch credentials using the last connected SSID
        optional<WiFiCredentials> credentials = wiFiCredentialsRepository->FindById(connection.lastConnectedSsid.value());
        
        // Return nullopt if credentials don't exist
        if (!credentials.has_value()) {
            return std::nullopt;
        }
        
        return credentials;
    }

    // Update last connected SSID
    Public Virtual Void UpdateLastConnectedSsid(const StdString& ssid) override {
        // Get all connection records
        StdVector<WiFiConnection> connections = wiFiConnectionRepository->FindAll();
        
        WiFiConnection connection;
        
        if (connections.empty()) {
            // Create new connection record
            connection.id = 1;
            connection.lastConnectedSsid = ssid;
            wiFiConnectionRepository->Save(connection);
        } else {
            // Update existing connection record (use first one)
            connection = connections[0];
            connection.lastConnectedSsid = ssid;
            wiFiConnectionRepository->Update(connection);
        }
    }
};

#endif // WIFISERVICE_H

