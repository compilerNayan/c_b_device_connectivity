#ifndef IMQTTCLIENTMANAGER_INTERNAL_H
#define IMQTTCLIENTMANAGER_INTERNAL_H

#include <StandardDefines.h>

DefineStandardPointers(IMqttClientManager)
class IMqttClientManager {
    Public Virtual ~IMqttClientManager() = default;

    Public Virtual Void EnsureMqttClientConnectivity() = 0;

    Public Virtual Void EnrollDevice() = 0;
};

#endif // IMQTTCLIENTMANAGER_INTERNAL_H