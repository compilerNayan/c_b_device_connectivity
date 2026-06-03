#ifndef IDEVICERUNTIMEMONITOR_H
#define IDEVICERUNTIMEMONITOR_H

#include <StandardDefines.h>

/**
 * Logs current device runtime state: heap, tasks, uptime (live monitoring).
 * Separate from IDeviceDiagnostics (previous-boot crash / reset reason).
 */
DefineStandardPointers(IDeviceRuntimeMonitor)
class IDeviceRuntimeMonitor {
    Public Virtual ~IDeviceRuntimeMonitor() = default;

    /** Logs memory, task list, and other runtime metrics via ILogger. */
    Public Virtual Void LogDeviceState() = 0;
};

#endif // IDEVICERUNTIMEMONITOR_H
