#ifndef LOGPUBLISHINGMANAGER_INTERNAL_H
#define LOGPUBLISHINGMANAGER_INTERNAL_H

#include <ArduinoJson.h>
#include <ctime>

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
        DynamicJsonDocument doc(4096);
        JsonArray arr = doc.to<JsonArray>();
    
        for (const auto& log : logs) {
            // Convert back to seconds
            time_t nowSec = static_cast<time_t>(log.first / 1000ULL);
    
            char timeBuf[24];
            if (nowSec > 0) {
                struct tm* t = localtime(&nowSec);
                if (t && strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", t) > 0) {
                    // formatted successfully
                } else {
                    snprintf(timeBuf, sizeof(timeBuf), "(time?)");
                }
            } else {
                snprintf(timeBuf, sizeof(timeBuf), "(no time)");
            }
    
            JsonObject obj = arr.createNestedObject();
            obj[timeBuf] = log.second;
        }
    
        StdString output;
        serializeJson(arr, output);
        return output;
    }    
};




#endif /* LOGPUBLISHINGMANAGER_INTERNAL_H */