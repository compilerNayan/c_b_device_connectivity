#ifndef REQUESTMANAGERTHREAD_H
#define REQUESTMANAGERTHREAD_H

#include <StandardDefines.h>
#include "threading/IRunnable.h"
#include "Thread.h"

#include "IHttpRequestManager.h"

/** Interval between checks (ms). Run() is called every loop; we only run the check when this much time has passed. */
static constexpr ULong kRequestManagerCheckIntervalMs = 50;

class RequestManagerThread : public IRunnable {
    /* @Autowired */
    Private IHttpRequestManagerPtr httpRequestManager;

    Public Void Run() override {
        while (true) {
            httpRequestManager->HandleRequest();
            Thread::Sleep(kRequestManagerCheckIntervalMs);
        }
    }
};

#endif // REQUESTMANAGERTHREAD_H