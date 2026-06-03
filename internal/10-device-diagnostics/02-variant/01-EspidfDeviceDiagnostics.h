#ifdef ESP_PLATFORM
#ifndef ESPIDF_DEVICE_DIAGNOSTICS_H
#define ESPIDF_DEVICE_DIAGNOSTICS_H

#include "../01-interface/01-IDeviceDiagnostics.h"

#include <StandardDefines.h>
#include "logger/ILogger.h"

#include <esp_system.h>
#include <esp_chip_info.h>
#include <esp_app_desc.h>
#include <esp_heap_caps.h>

#if defined(__has_include)
#if __has_include("esp_private/system_internal.h")
#include "esp_private/system_internal.h"
#define DEVICE_DIAG_RESET_HINT_AVAILABLE 1
#endif
#endif

#if (defined(CONFIG_ESP_COREDUMP_ENABLE) && CONFIG_ESP_COREDUMP_ENABLE) \
    || (defined(CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) && CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH) \
    || (defined(CONFIG_ESP_COREDUMP_ENABLE_TO_UART) && CONFIG_ESP_COREDUMP_ENABLE_TO_UART)
#include <esp_core_dump.h>
#define DEVICE_DIAG_COREDUMP_AVAILABLE 1
#endif

/* @Component */
class EspidfDeviceDiagnostics final : public IDeviceDiagnostics {
    /* @Autowired */
    Private ILoggerPtr logger;

    Private esp_reset_reason_t lastResetReason_{ESP_RST_UNKNOWN};

    Private Static Bool IsCrashResetReason(esp_reset_reason_t reason) {
        switch (reason) {
            case ESP_RST_PANIC:
            case ESP_RST_INT_WDT:
            case ESP_RST_TASK_WDT:
            case ESP_RST_WDT:
            case ESP_RST_BROWNOUT:
#if defined(ESP_RST_CPU_LOCKUP)
            case ESP_RST_CPU_LOCKUP:
#endif
                return true;
            default:
                return false;
        }
    }

    Private Static CStdString ResetReasonToString(esp_reset_reason_t reason) {
        switch (reason) {
            case ESP_RST_UNKNOWN: return "UNKNOWN";
            case ESP_RST_POWERON: return "POWERON";
            case ESP_RST_EXT: return "EXT";
            case ESP_RST_SW: return "SW";
            case ESP_RST_PANIC: return "PANIC";
            case ESP_RST_INT_WDT: return "INT_WDT";
            case ESP_RST_TASK_WDT: return "TASK_WDT";
            case ESP_RST_WDT: return "WDT";
            case ESP_RST_DEEPSLEEP: return "DEEPSLEEP";
            case ESP_RST_BROWNOUT: return "BROWNOUT";
            case ESP_RST_SDIO: return "SDIO";
#if defined(ESP_RST_USB)
            case ESP_RST_USB: return "USB";
#endif
#if defined(ESP_RST_JTAG)
            case ESP_RST_JTAG: return "JTAG";
#endif
#if defined(ESP_RST_CPU_LOCKUP)
            case ESP_RST_CPU_LOCKUP: return "CPU_LOCKUP";
#endif
            default: return "OTHER(" + std::to_string(static_cast<Int>(reason)) + ")";
        }
    }

    Private Void LogLine(CLogLevel level, CStdString& line) {
        if (!logger) {
            return;
        }
        logger->Log(level, Tag::Untagged, line);
    }

    Private Void LogChipInfo() {
        esp_chip_info_t chipInfo{};
        esp_chip_info(&chipInfo);

        StdString features;
        if (chipInfo.features & CHIP_FEATURE_WIFI_BGN) {
            features += "WiFi ";
        }
        if (chipInfo.features & CHIP_FEATURE_BLE) {
            features += "BLE ";
        }
        if (chipInfo.features & CHIP_FEATURE_BT) {
            features += "BT ";
        }
        if (chipInfo.features & CHIP_FEATURE_EMB_FLASH) {
            features += "Embedded-Flash ";
        }
        if (chipInfo.features & CHIP_FEATURE_EMB_PSRAM) {
            features += "PSRAM ";
        }
        if (features.empty()) {
            features = "none";
        }

        StdString line = "[DeviceDiagnostics] Chip: model=" + std::to_string(chipInfo.model)
            + " cores=" + std::to_string(chipInfo.cores)
            + " revision=" + std::to_string(chipInfo.revision)
            + " features=" + features;
        LogLine(LogLevel::Info, line);
    }

    Private Void LogAppInfo() {
        const esp_app_desc_t* app = esp_app_get_description();
        if (!app) {
            StdString line = "[DeviceDiagnostics] Firmware: unavailable";
            LogLine(LogLevel::Warning, line);
            return;
        }

        StdString line = "[DeviceDiagnostics] Firmware: project=" + StdString(app->project_name)
            + " version=" + StdString(app->version)
            + " built=" + StdString(app->date) + " " + StdString(app->time)
            + " idf=" + StdString(app->idf_ver);
        LogLine(LogLevel::Info, line);
    }

    Private Void LogHeapInfo() {
        const ULong freeHeap = static_cast<ULong>(esp_get_free_heap_size());
        const ULong minFreeHeap = static_cast<ULong>(esp_get_minimum_free_heap_size());
        const ULong largestBlock = static_cast<ULong>(
            heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));

        StdString line = "[DeviceDiagnostics] Heap: free=" + std::to_string(freeHeap)
            + " min_free=" + std::to_string(minFreeHeap)
            + " largest_block=" + std::to_string(largestBlock);
        LogLine(LogLevel::Info, line);
    }

    Private Void LogResetHint() {
#if defined(DEVICE_DIAG_RESET_HINT_AVAILABLE)
        const esp_reset_reason_t hint = esp_reset_reason_get_hint();
        if (hint != ESP_RST_UNKNOWN && hint != lastResetReason_) {
            StdString line = "[DeviceDiagnostics] Reset hint: "
                + ResetReasonToString(hint)
                + " code=" + std::to_string(static_cast<Int>(hint));
            LogLine(LogLevel::Info, line);
        }
#endif
    }

    Private Void LogCoreDumpSummary() {
#if defined(DEVICE_DIAG_COREDUMP_AVAILABLE)
        esp_err_t check = esp_core_dump_image_check();
        if (check == ESP_ERR_NOT_FOUND) {
            StdString line = "[DeviceDiagnostics] Core dump: none stored";
            LogLine(LogLevel::Info, line);
            return;
        }
        if (check != ESP_OK) {
            StdString line = "[DeviceDiagnostics] Core dump: check failed err="
                + std::to_string(static_cast<Int>(check));
            LogLine(LogLevel::Warning, line);
            return;
        }

        esp_core_dump_summary_t summary{};
        esp_err_t summaryErr = esp_core_dump_get_summary(&summary);
        if (summaryErr != ESP_OK) {
            StdString line = "[DeviceDiagnostics] Core dump: summary unavailable err="
                + std::to_string(static_cast<Int>(summaryErr));
            LogLine(LogLevel::Warning, line);
            return;
        }

        StdString line = "[DeviceDiagnostics] Core dump: excCause="
            + std::to_string(summary.ex_info.exc_cause)
            + " excVaddr=0x" + std::to_string(summary.ex_info.exc_vaddr)
            + " excPc=0x" + std::to_string(summary.exc_pc)
            + " task=" + StdString(summary.exc_task);
        LogLine(LogLevel::Info, line);

        const UInt depth = summary.exc_bt_info.depth;
        StdString bt = "[DeviceDiagnostics] Core dump backtrace depth="
            + std::to_string(depth);
        LogLine(LogLevel::Info, bt);
        for (UInt i = 0; i < depth && i < 16; ++i) {
            StdString frame = "[DeviceDiagnostics]   #" + std::to_string(i)
                + " pc=0x" + std::to_string(summary.exc_bt_info.bt[i]);
            LogLine(LogLevel::Info, frame);
        }
#else
        StdString line = "[DeviceDiagnostics] Core dump: not enabled in sdkconfig";
        LogLine(LogLevel::Info, line);
#endif
    }

    Public Explicit EspidfDeviceDiagnostics() {
        lastResetReason_ = esp_reset_reason();
    }

    Public Virtual Bool HadPreviousCrash() const override {
        return IsCrashResetReason(lastResetReason_);
    }

    Public Virtual Void LogPreviousCrashDetails() override {
        if (!HadPreviousCrash()) {
            return;
        }

        StdString header = "[DeviceDiagnostics] Previous boot ended abnormally (crash/watchdog/brownout)";
        LogLine(LogLevel::Error, header);

        StdString reasonLine = "[DeviceDiagnostics] Reset reason: "
            + ResetReasonToString(lastResetReason_)
            + " code=" + std::to_string(static_cast<Int>(lastResetReason_));
        LogLine(LogLevel::Error, reasonLine);

        LogResetHint();
        LogChipInfo();
        LogAppInfo();
        LogHeapInfo();
        LogCoreDumpSummary();
    }

    Public Virtual Void CheckAndLogPreviousCrash() override {
        if (HadPreviousCrash()) {
            LogPreviousCrashDetails();
        } else {
            StdString line = "[DeviceDiagnostics] Previous boot: normal (reset="
                + ResetReasonToString(lastResetReason_) + ")";
            LogLine(LogLevel::Info, line);
        }
    }
};

#endif // ESPIDF_DEVICE_DIAGNOSTICS_H
#endif // ESP_PLATFORM
