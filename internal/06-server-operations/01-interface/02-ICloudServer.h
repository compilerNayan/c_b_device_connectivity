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
};

#endif // ICLUDSERVER_EXPORT_H