#ifndef IDEVICEMANAGER_INTERNAL_H
#define IDEVICEMANAGER_INTERNAL_H

#include <StandardDefines.h>

DefineStandardPointers(IDeviceManager)
class IDeviceManager {
    Public Virtual ~IDeviceManager() = default;

    Public Virtual Void HandleDeviceCommand() = 0;

    Public Virtual Void Setup() = 0;

    Public Virtual Void Loop() = 0;
};

#endif // IDEVICEMANAGER_INTERNAL_H