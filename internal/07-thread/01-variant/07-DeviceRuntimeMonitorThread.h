#ifndef DEVICE_RUNTIME_MONITOR_THREAD_H
#define DEVICE_RUNTIME_MONITOR_THREAD_H

#include <StandardDefines.h>
#include "threading/IRunnable.h"
#include "Thread.h"

#include "../../11-device-runtime-monitor/00-public/01-IDeviceRuntimeMonitor.h"

/** How often to log full device state while running (ms). */
static constexpr ULong kDeviceRuntimeMonitorIntervalMs = 60000;

/**
 * Periodically logs heap and per-task runtime stats for field monitoring.
 */
class DeviceRuntimeMonitorThread : public IRunnable {
    /* @Autowired */
    Private IDeviceRuntimeMonitorPtr deviceRuntimeMonitor;

    Public Void Run() override {
        Thread::Sleep(15000);
        while (true) {
            if (deviceRuntimeMonitor) {
                deviceRuntimeMonitor->LogDeviceState();
            }
            Thread::Sleep(kDeviceRuntimeMonitorIntervalMs);
        }
    }
};

#endif // DEVICE_RUNTIME_MONITOR_THREAD_H
