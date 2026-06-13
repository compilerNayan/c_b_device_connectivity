#ifndef ICLOUDSOCKETMANAGER_INTERNAL_H
#define ICLOUDSOCKETMANAGER_INTERNAL_H

#include <StandardDefines.h>

DefineStandardPointers(ICloudSocketManager)
class ICloudSocketManager {
    Public Virtual ~ICloudSocketManager() = default;

    Public Virtual Void EnsureCloudSocketConnectivity() = 0;
};

#endif // ICLOUDSOCKETMANAGER_INTERNAL_H
