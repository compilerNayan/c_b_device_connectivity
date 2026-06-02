#ifndef LOGPUBLISHINGMANAGER_INTERNAL_H
#define LOGPUBLISHINGMANAGER_INTERNAL_H

#include <StandardDefines.h>
#include "../01-interface/01-ILogPublishingManager.h"
#include "communication/IServerProvider.h"
#include "IFleetProvisioningService.h"

/* @Component */
class LogPublishingManager : public ILogPublishingManager {
    Public LogPublishingManager() {
        cloudServer = serverProvider->GetCloudServerPtr();
    }
    Public Virtual ~LogPublishingManager() override = default;

    /** Approx max payload bytes per cloud publish. */
    // Smaller batch lowers transient JSON/TLS pressure on ESP32 heap.
    Static const Size kApproxMaxBytesPerPublish = 1800;

    /* @Autowired */
    Private IServerProviderPtr serverProvider;

    Private ICloudServerPtr cloudServer;

    /* @Autowired */
    Private ILogBufferPtr logBuffer;

    /* @Autowired */
    Private ILoggerPtr logger;

    /* @Autowired */
    Private IFleetProvisioningServicePtr fleetProvisioningService;

    Public Bool PublishLogs() override {
        if (!fleetProvisioningService->IsEnrolled()) return true;
        for (;;) {
            StdMap<ULongLong, StdString> logs = logBuffer->TakeLogsByApproxBytes(kApproxMaxBytesPerPublish);
            if (logs.empty()) return true;
            String jsonLogs = GetLogsJson(logs);
            cloudServer->PublishLogs(jsonLogs);
        }
        return true;
    }

    Private String GetLogsJson(const StdMap<ULongLong, StdString>& logs) {
        String jsonLogs = "[";
        
        for (const auto& log : logs) {
            jsonLogs += "{\"" + log.first + " \": \"" + log.second + "\"},";
        }

        jsonLogs += "]";
        return json.toString();
    }
};




#endif /* LOGPUBLISHINGMANAGER_INTERNAL_H */