#ifndef _WIFI_CREDENTIALS_REPOSITORY_H_
#define _WIFI_CREDENTIALS_REPOSITORY_H_

#include <StandardDefines.h>
#include "CpaRepository.h"
#include "../entity/WiFiCredentials.h"

/* @Repository */
DefineStandardPointers(WiFiCredentialsRepository)
class WiFiCredentialsRepository : public CpaRepository<WiFiCredentials, StdString> {
    Public Virtual ~WiFiCredentialsRepository() = default;
    
    // Additional custom repository methods can be added here
    // The base CRUD methods are inherited from CpaRepository
    // Note: SSID is the ID field, so FindById(ssid) can be used to find by SSID
};

#endif // _WIFI_CREDENTIALS_REPOSITORY_H_

