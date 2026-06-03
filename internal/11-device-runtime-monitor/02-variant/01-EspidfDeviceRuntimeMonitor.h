#ifdef ESP_PLATFORM
#ifndef ESPIDF_DEVICE_RUNTIME_MONITOR_H
#define ESPIDF_DEVICE_RUNTIME_MONITOR_H

#include "../01-interface/01-IDeviceRuntimeMonitor.h"

#include <StandardDefines.h>
#include <cstdlib>
#include "logger/ILogger.h"

#include <esp_system.h>
#include <esp_heap_caps.h>
#include <esp_timer.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* @Component */
class EspidfDeviceRuntimeMonitor final : public IDeviceRuntimeMonitor {
    /* @Autowired */
    Private ILoggerPtr logger;

    Private Void LogLine(CLogLevel level, CStdString& line) {
        if (!logger) {
            return;
        }
        logger->Log(level, Tag::Untagged, line);
    }

    Private Static CStdString TaskStateToString(eTaskState state) {
        switch (state) {
            case eRunning: return "Running";
            case eReady: return "Ready";
            case eBlocked: return "Blocked";
            case eSuspended: return "Suspended";
            case eDeleted: return "Deleted";
            default: return "Unknown";
        }
    }

    Private Void LogHeapRegion(const char* label, UInt32 caps) {
        multi_heap_info_t info{};
        heap_caps_get_info(&info, caps);

        const ULong freeBytes = static_cast<ULong>(info.total_free_bytes);
        const ULong allocatedBytes = static_cast<ULong>(info.total_allocated_bytes);
        const ULong largestBlock = static_cast<ULong>(info.largest_free_block);
        const ULong minEverFree = static_cast<ULong>(info.minimum_free_bytes);
        const ULong totalBytes = freeBytes + allocatedBytes;

        StdString line = "[DeviceRuntime] Heap " + StdString(label)
            + ": total=" + std::to_string(totalBytes)
            + " used=" + std::to_string(allocatedBytes)
            + " free=" + std::to_string(freeBytes)
            + " min_ever_free=" + std::to_string(minEverFree)
            + " largest_block=" + std::to_string(largestBlock)
            + " alloc_blocks=" + std::to_string(info.allocated_blocks)
            + " free_blocks=" + std::to_string(info.free_blocks);
        LogLine(LogLevel::Info, line);
    }

    Private Void LogMemory() {
        StdString header = "[DeviceRuntime] --- Memory ---";
        LogLine(LogLevel::Info, header);

        LogHeapRegion("internal", MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

#if CONFIG_SPIRAM
        LogHeapRegion("spiram", MALLOC_CAP_SPIRAM);
#endif

        StdString legacy = "[DeviceRuntime] Heap legacy: free="
            + std::to_string(static_cast<ULong>(esp_get_free_heap_size()))
            + " min_free=" + std::to_string(static_cast<ULong>(esp_get_minimum_free_heap_size()));
        LogLine(LogLevel::Info, legacy);
    }

    Private Void LogTasks() {
        StdString header = "[DeviceRuntime] --- Tasks ---";
        LogLine(LogLevel::Info, header);

#if CONFIG_FREERTOS_USE_TRACE_FACILITY
        const UBaseType_t taskCount = uxTaskGetNumberOfTasks();
        if (taskCount == 0) {
            StdString line = "[DeviceRuntime] No tasks reported";
            LogLine(LogLevel::Warning, line);
            return;
        }

        TaskStatus_t* taskStatus = static_cast<TaskStatus_t*>(
            std::malloc(taskCount * sizeof(TaskStatus_t)));
        if (!taskStatus) {
            StdString line = "[DeviceRuntime] Task snapshot alloc failed";
            LogLine(LogLevel::Error, line);
            return;
        }

        uint32_t totalRunTime = 0;
        const UBaseType_t captured = uxTaskGetSystemState(
            taskStatus, taskCount, &totalRunTime);

        StdString summary = "[DeviceRuntime] Task count=" + std::to_string(captured);
#if CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
        summary += " total_runtime_ticks=" + std::to_string(totalRunTime);
#endif
        LogLine(LogLevel::Info, summary);

        for (UBaseType_t i = 0; i < captured; ++i) {
            const TaskStatus_t& t = taskStatus[i];
            const ULong stackFreeWords = static_cast<ULong>(t.usStackHighWaterMark);
            const ULong stackFreeBytes = stackFreeWords * sizeof(StackType_t);

            StdString line = "[DeviceRuntime] Task name=" + StdString(t.pcTaskName)
                + " state=" + TaskStateToString(t.eCurrentState)
                + " prio=" + std::to_string(static_cast<ULong>(t.uxCurrentPriority))
                + " stack_min_free_bytes=" + std::to_string(stackFreeBytes);

#if CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
            if (totalRunTime > 0) {
                const ULong cpuPct = (static_cast<ULong>(t.ulRunTimeCounter) * 100UL) / totalRunTime;
                line += " cpu_pct=" + std::to_string(cpuPct);
            }
#endif
            LogLine(LogLevel::Info, line);
        }

        std::free(taskStatus);
#else
        StdString line = "[DeviceRuntime] Per-task stats disabled; enable "
            "CONFIG_FREERTOS_USE_TRACE_FACILITY in sdkconfig";
        LogLine(LogLevel::Warning, line);
#endif
    }

    Private Void LogSystem() {
        const ULong uptimeSec = static_cast<ULong>(esp_timer_get_time() / 1000000ULL);

        StdString line = "[DeviceRuntime] --- System --- uptime_sec="
            + std::to_string(uptimeSec)
            + " idf=" + StdString(esp_get_idf_version());
        LogLine(LogLevel::Info, line);
    }

    Public Virtual Void LogDeviceState() override {
        StdString begin = "[DeviceRuntime] ===== Device state snapshot =====";
        LogLine(LogLevel::Info, begin);
        LogSystem();
        LogMemory();
        LogTasks();
        StdString end = "[DeviceRuntime] ===== End snapshot =====";
        LogLine(LogLevel::Info, end);
    }
};

#endif // ESPIDF_DEVICE_RUNTIME_MONITOR_H
#endif // ESP_PLATFORM
