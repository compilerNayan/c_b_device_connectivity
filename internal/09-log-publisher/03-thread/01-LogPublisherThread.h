#ifndef LOGPUBLISHERTHREAD_INTERNAL_H
#define LOGPUBLISHERTHREAD_INTERNAL_H

#include <StandardDefines.h>
#include "threading/IRunnable.h"
#include "Thread.h"
#include "../01-interface/01-ILogPublishingManager.h"

class LogPublisherThread : public IRunnable {
    /* @Autowired */
    Private ILogPublishingManagerPtr logPublishingManager;

    Public LogPublisherThread() = default;
    
    Public Virtual ~LogPublisherThread() override = default;

    Public Void Run() override {
        Thread::Sleep(60000);
        while (true) {
            logPublishingManager->PublishLogs();
            Thread::Sleep(60000);
        }
    }
};

#endif /* LOGPUBLISHERTHREAD_INTERNAL_H */