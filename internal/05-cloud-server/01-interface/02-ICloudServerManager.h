#ifndef ICLOUD_SERVER_MANAGER_INTERNAL_H
#define ICLOUD_SERVER_MANAGER_INTERNAL_H

#include <StandardDefines.h>

DefineStandardPointers(ICloudServerManager)
class ICloudServerManager {
    Public Virtual ~ICloudServerManager() = default;

    Public Virtual Void EnsureCloudServerConnectivity() = 0;
};

#endif // ICLOUD_SERVER_MANAGER_INTERNAL_H