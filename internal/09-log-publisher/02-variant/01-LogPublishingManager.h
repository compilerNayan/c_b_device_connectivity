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
            StdString jsonLogs = GetLogsJson(logs);
            cloudServer->PublishLogs(jsonLogs);
        }
        return true;
    }

    Private StdString GetLogsJson(const StdMap<ULongLong, StdString>& logs) {
        StdString jsonLogs = "[";

        int logCount = logs.size();
        int index = 0;

        
        for (const auto& log : logs) {

            time_t nowSec = (time_t)log.first;
            char timeBuf[24];
            if (nowSec != (time_t)-1 && nowSec > 0) {
                struct tm* t = localtime(&nowSec);
                if (t && strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", t) > 0) { /* ok */ }
                else snprintf(timeBuf, sizeof(timeBuf), "(time?)");
            } else {
                snprintf(timeBuf, sizeof(timeBuf), "(no time)");
            }
            StdString timeStr = timeBuf;
    


            jsonLogs += "{\"" + timeStr + " \": \"" + log.second + "\"}";
            index++;
            if(index < logCount) {
                jsonLogs += ",";
            }
        }

        jsonLogs += "]";
        return jsonLogs;
    }
};




#endif /* LOGPUBLISHINGMANAGER_INTERNAL_H */