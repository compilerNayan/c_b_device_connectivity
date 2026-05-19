#ifndef _WIFI_CONNECTION_REPOSITORY_H_
#define _WIFI_CONNECTION_REPOSITORY_H_

#include <StandardDefines.h>
#include "CpaRepository.h"
#include "../entity/WiFiConnection.h"

/* @Repository */
DefineStandardPointers(WiFiConnectionRepository)
class WiFiConnectionRepository : public CpaRepository<WiFiConnection, int> {
    Public Virtual ~WiFiConnectionRepository() = default;
    
};

#endif // _WIFI_CONNECTION_REPOSITORY_H_

