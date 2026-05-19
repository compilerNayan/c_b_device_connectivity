#ifndef ILOCALSERVERMANAGER_INTERNAL_H
#define ILOCALSERVERMANAGER_INTERNAL_H

#include <StandardDefines.h>

DefineStandardPointers(ILocalServerManager)
class ILocalServerManager {
    Public Virtual ~ILocalServerManager() = default;

    Public Virtual Void EnsureLocalServerConnectivity() = 0;
};

#endif // ILOCALSERVERMANAGER_INTERNAL_H