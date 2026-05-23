#ifndef WIFIHEALTHCHECKERTHREAD_H
#define WIFIHEALTHCHECKERTHREAD_H

#include <StandardDefines.h>
#include "threading/IRunnable.h"    
#include "Thread.h"

#include "../../02-wifi/00-public/02-IWiFiConnectionManager.h"
#include "../../02-wifi/00-public/03-WiFiCommandProcessor.h"

/** Interval between checks (ms). Run() is called every loop; we only run the check when this much time has passed. */
static constexpr ULong kWiFiHealthCheckIntervalMs = 2000;

/**
 * Ensures network connectivity periodically. Call Run() every loop from main thread; throttled so the check runs at most every kWiFiHealthCheckIntervalMs.
 */
class WiFiHealthCheckerThread : public IRunnable {
    /* @Autowired */
    Private IWiFiConnectionManagerPtr wifiConnectionManager;
    Private ULong lastRunMs_{0};
    Private WiFiCommandProcessor wifiCommandProcessor;

    Public Void Run() override {
        Thread::Sleep(3000);
        while (true) {
            wifiCommandProcessor.ProcessCommands();
            wifiConnectionManager->EnsureNetworkConnectivity();
            Thread::Sleep(kWiFiHealthCheckIntervalMs);
        }
    }
};

#endif // WIFIHEALTHCHECKERTHREAD_H
