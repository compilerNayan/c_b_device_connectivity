#ifndef CLOUD_SERVER_THREAD_H
#define CLOUD_SERVER_THREAD_H

#include <StandardDefines.h>
#include "threading/IRunnable.h"
#include "Thread.h"

#include "../../05-cloud-server/00-public/02-ICloudServerManager.h"

/** Interval between checks (ms). Run() is called every loop; we only run the check when this much time has passed. */
static constexpr ULong kCloudServerCheckIntervalMs = 100;

class CloudServerThread : public IRunnable {
    /* @Autowired */
    Private ICloudServerManagerPtr cloudServerManager;

    Public Void Run() override {
        while (true) {
            cloudServerManager->EnsureCloudServerConnectivity();
            Thread::Sleep(kCloudServerCheckIntervalMs);
        }
    }
};

#endif // CLOUD_SERVER_THREAD_H