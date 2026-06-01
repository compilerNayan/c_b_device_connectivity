#ifndef DEVICEMANAGERTHREAD_H
#define DEVICEMANAGERTHREAD_H

#include <StandardDefines.h>
#include "threading/IRunnable.h"
#include "Thread.h"
#include "IHttpRequestProcessor.h"

#include "logger/ILogger.h"
#include "../../08-device-manager/01-interface/01-IDeviceManager.h"
#include "../../08-device-manager/02-variant/02-DeviceCommandProcessor.h"

class DeviceManagerThread : public IRunnable {
    /* @Autowired */
    Private IDeviceManagerPtr deviceManager;

    /* @Autowired */
    Private ILoggerPtr logger;

    /* @Autowired */
    Private IHttpRequestProcessorPtr httpRequestProcessor;

    Private DeviceCommandProcessor deviceCommandProcessor;

    Public Void Run() override {
        while (true) {
            deviceCommandProcessor.ProcessCommands();
            ProcessRequests();
            deviceManager->Loop();
            Thread::Sleep(100);
        }
    }

    Private Bool ProcessRequests() {
        if (httpRequestProcessor == nullptr) {
            return false;
        }
        
        Bool processedAny = false;
        while (httpRequestProcessor->HasRequests()) {
            if (httpRequestProcessor->ProcessRequest()) {
                processedAny = true;
            } else {
                break;
            }
        }
        
        return processedAny;
    }

};

#endif // DEVICEMANAGERTHREAD_H