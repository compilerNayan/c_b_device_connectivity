#ifndef DEVICEMANAGER_INTERNAL_H
#define DEVICEMANAGER_INTERNAL_H

#include <StandardDefines.h>
#include "../01-interface/01-IDeviceManager.h"

/* @Component */
class DeviceManager : public IDeviceManager {
    Public Virtual ~DeviceManager() = default;

    Public Virtual Void Setup() override {
    }

    Public Virtual Void Loop() override {

    }
};

#endif // DEVICEMANAGER_INTERNAL_H