#ifndef TCPSERVERTHREAD_H
#define TCPSERVERTHREAD_H

#include <StandardDefines.h>
#include "threading/IRunnable.h"
#include "Thread.h"

#include "../../04-tcp-server/00-public/01-ITcpServerManager.h"

/** Interval between checks (ms). Run() is called every loop; we only run the check when this much time has passed. */
static constexpr ULong kTcpServerCheckIntervalMs = 100;

class TcpServerThread : public IRunnable {
    /* @Autowired */
    Private ITcpServerManagerPtr tcpServerManager;

    Public Void Run() override {
        while (true) {
            tcpServerManager->EnsureTcpServerConnectivity();
            Thread::Sleep(kTcpServerCheckIntervalMs);
        }
    }
};

#endif // TCPSERVERTHREAD_H