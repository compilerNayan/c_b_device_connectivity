#ifndef ITCPSERVERMANAGER_INTERNAL_H
#define ITCPSERVERMANAGER_INTERNAL_H

#include <StandardDefines.h>

DefineStandardPointers(ITcpServerManager)
class ITcpServerManager {
    Public Virtual ~ITcpServerManager() = default;

    Public Virtual Void EnsureTcpServerConnectivity() = 0;
};

#endif // ITCPSERVERMANAGER_INTERNAL_H