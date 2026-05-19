#ifndef ILOCALSERVER_EXPORT_H
#define ILOCALSERVER_EXPORT_H

#include <StandardDefines.h>

#include "communication/IServer.h"

DefineStandardPointers(ILocalServer)
class ILocalServer : public IServer {
    Public Virtual ~ILocalServer() = default;
};

#endif // ILOCALSERVER_EXPORT_H