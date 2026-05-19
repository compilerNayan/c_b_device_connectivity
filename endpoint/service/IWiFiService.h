#ifndef IWIFISERVICE_H
#define IWIFISERVICE_H

#include <StandardDefines.h>
#include "../entity/WiFiCredentials.h"

DefineStandardPointers(IWiFiService)
class IWiFiService {
    Public Virtual ~IWiFiService() = default;

    // Add WiFi credentials
    Public Virtual WiFiCredentials AddWiFiCredentials(const WiFiCredentials& credentials) = 0;

    // Update WiFi credentials
    Public Virtual WiFiCredentials UpdateWiFiCredentials(const WiFiCredentials& credentials) = 0;

    // Delete WiFi credentials by SSID
    Public Virtual Void DeleteWiFiCredentials(const StdString& ssid) = 0;

    // Read WiFi credentials by SSID
    Public Virtual WiFiCredentials GetWiFiCredentials(const StdString& ssid) = 0;

    // Get all WiFi credentials
    Public Virtual StdVector<WiFiCredentials> GetAllWiFiCredentials() = 0;

    // Get last connected WiFi details
    Public Virtual optional<WiFiCredentials> GetLastConnectedWiFi() = 0;

    // Update last connected SSID
    Public Virtual Void UpdateLastConnectedSsid(const StdString& ssid) = 0;
};

#endif // IWIFISERVICE_H

