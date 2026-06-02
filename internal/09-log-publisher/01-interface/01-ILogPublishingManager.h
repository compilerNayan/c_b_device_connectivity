#ifndef ILOGPUBLISHINGMANAGER_INTERNAL_H
#define ILOGPUBLISHINGMANAGER_INTERNAL_H

#include <StandardDefines.h>

DefineStandardPointers(ILogPublishingManager)
class ILogPublishingManager {
    Public Virtual ~ILogPublishingManager() = default;

    Public Virtual Bool PublishLogs() = 0;
};

#endif /* ILOGPUBLISHINGMANAGER_INTERNAL_H */