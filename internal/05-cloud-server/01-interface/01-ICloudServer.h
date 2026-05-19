#ifndef ICLUDSERVER_EXPORT_H
#define ICLUDSERVER_EXPORT_H

#include <StandardDefines.h>

#include "communication/IServer.h"

DefineStandardPointers(ICloudServer)
class ICloudServer : public IServer {
    Public Virtual ~ICloudServer() = default;
};

#endif // ICLUDSERVER_EXPORT_H