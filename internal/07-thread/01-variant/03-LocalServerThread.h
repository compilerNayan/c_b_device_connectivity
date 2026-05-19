#ifndef LOCALSERVERTHREAD_H
#define LOCALSERVERTHREAD_H

#include <StandardDefines.h>
#include "threading/IRunnable.h"
#include "Thread.h"

#include "../../04-local-server/00-public/02-ILocalServerManager.h"

/** Interval between checks (ms). Run() is called every loop; we only run the check when this much time has passed. */
static constexpr ULong kLocalServerCheckIntervalMs = 100;

class LocalServerThread : public IRunnable {
    /* @Autowired */
    Private ILocalServerManagerPtr localServerManager;

    Public Void Run() override {
        while (true) {
            localServerManager->EnsureLocalServerConnectivity();
            Thread::Sleep(kLocalServerCheckIntervalMs);
        }
    }
};

#endif // LOCALSERVERTHREAD_H