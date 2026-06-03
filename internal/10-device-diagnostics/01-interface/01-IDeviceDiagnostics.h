#ifndef IDEVICEDIAGNOSTICS_H
#define IDEVICEDIAGNOSTICS_H

#include <StandardDefines.h>

/**
 * Reports whether the previous boot ended abnormally and logs reset/crash diagnostics.
 */
DefineStandardPointers(IDeviceDiagnostics)
class IDeviceDiagnostics {
    Public Virtual ~IDeviceDiagnostics() = default;

    /** True when the last reset was panic, watchdog, brownout, or similar. */
    Public Virtual Bool HadPreviousCrash() const = 0;

    /** Logs reset/crash details via ILogger. No-op when HadPreviousCrash() is false. */
    Public Virtual Void LogPreviousCrashDetails() = 0;

    /** Checks previous boot; logs details when a crash-related reset is detected. */
    Public Virtual Void CheckAndLogPreviousCrash() = 0;
};

#endif // IDEVICEDIAGNOSTICS_H
