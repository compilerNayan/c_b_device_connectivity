#ifndef IWIFI_CREDENTIALS_CONTROLLER_H
#define IWIFI_CREDENTIALS_CONTROLLER_H

#include <StandardDefines.h>
#include "../entity/WiFiCredentials.h"

DefineStandardPointers(IWiFiCredentialsController)
class IWiFiCredentialsController {
    Public Virtual ~IWiFiCredentialsController() = default;

    // Create - Add new WiFi credentials
    Public Virtual WiFiCredentials CreateWiFiCredentials(WiFiCredentials credentials) = 0;

    // Read - Get WiFi credentials by SSID
    Public Virtual WiFiCredentials GetWiFiCredentials(StdString ssid) = 0;

    // Read All - Get all WiFi credentials
    Public Virtual StdVector<WiFiCredentials> GetAllWiFiCredentials() = 0;

    // Update - Update existing WiFi credentials
    Public Virtual WiFiCredentials UpdateWiFiCredentials(WiFiCredentials credentials) = 0;

    // Delete - Delete WiFi credentials by SSID
    Public Virtual Void DeleteWiFiCredentials(StdString ssid) = 0;

    // Get last connected WiFi
    Public Virtual optional<WiFiCredentials> GetLastConnectedWiFi() = 0;
};

#endif // IWIFI_CREDENTIALS_CONTROLLER_H

