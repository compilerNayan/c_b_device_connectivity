#ifndef ICLUDSERVER_EXPORT_H
#define ICLUDSERVER_EXPORT_H

#include <StandardDefines.h>

#include "communication/IServer.h"

DefineStandardPointers(ICloudServer)
class ICloudServer : public IServer {
    Public Virtual ~ICloudServer() = default;
    Public Virtual Bool PublishLogs(CStdString logs) = 0;

    Public Virtual Bool PublishSecondPulse(CStdString payload) = 0;

    Public Virtual Bool PublishThirtyMinuteBucket(CStdString payload) = 0;

    Public Virtual Bool PublishEnrollmentComplete(CStdString payload) = 0;

    /**
     * Notify the cloud that this device is ready to be enrolled.
     * Payload is JSON in data (e.g. serialNumber, deviceType, firmwareVersion).
     * Empty payload is allowed; the server implementation may fill defaults.
     */
    Public Virtual Bool BeginEnrollment(CStdString payload) = 0;

    /**
     * Notify the cloud that the device application has started (MCU boot).
     * Payload is JSON in data; use "{}" when no extra fields are needed.
     */
    Public Virtual Bool PublishDeviceBoot(CStdString payload) = 0;
};

#endif // ICLUDSERVER_EXPORT_H