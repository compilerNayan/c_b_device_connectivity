#ifdef ESP_PLATFORM
#ifndef CLOUDSOCKETMANAGER_INTERNAL_H
#define CLOUDSOCKETMANAGER_INTERNAL_H

#include "../01-interface/03-ICloudSocketManager.h"
#include "../../06-server-operations/01-interface/03-ICloudSocket.h"
#include "Thread.h"
#include <cstring>
#include "IInternetConnectionStatusProvider.h"
#include "IFleetProvisioningService.h"
#include "logger/ILogger.h"

/* @Component */
class CloudSocketManager final : public ICloudSocketManager {
    Public CloudSocketManager() = default;
    Public Virtual ~CloudSocketManager() override = default;

    /* @Autowired */
    Private ICloudSocketPtr cloudSocket;

    /* @Autowired */
    Private IInternetConnectionStatusProviderPtr internetConnectionStatusProvider;

    /* @Autowired */
    Private IFleetProvisioningServicePtr fleetProvisioningService;

    /* @Autowired */
    Private ILoggerPtr logger;

    Private ULong lastInternetConnectionId = 0;
    Private Bool wasEnrolledOnLastCheck = false;

    static constexpr Int kPostEnrollmentSettleMs = 3000;
    static constexpr Int kInternetRestoreSettleMs = 3000;

    Public Virtual Void EnsureCloudSocketConnectivity() override {
        if (PreCheck()) {
            cloudSocket->SendData();
        }
    }

    Private Void StopSocketWhenNoInternet() {
        if (cloudSocket && cloudSocket->IsSocketOpen()) {
            cloudSocket->CloseSocket();
        }
        lastInternetConnectionId = 0;
    }

    Private Bool IsInternetAvailable() const {
        return internetConnectionStatusProvider->IsInternetConnected()
            && internetConnectionStatusProvider->GetInternetConnectionId() != 0;
    }

    Private Bool TryOpenSocket(const char* reason) {
        if (strcmp(reason, "post_enrollment") == 0) {
            Thread::Sleep(kPostEnrollmentSettleMs);
        } else if (strcmp(reason, "internet_restored") == 0) {
            Thread::Sleep(kInternetRestoreSettleMs);
        }

        if (!cloudSocket->OpenSocket("", 0)) {
            LogInfo(StdString("[CloudSocketManager] Open failed reason=") + reason);
            return false;
        }

        LogInfo(StdString("[CloudSocketManager] Socket open reason=") + reason);
        return true;
    }

    Private Bool PreCheck() {
        const Bool enrolledNow = fleetProvisioningService->IsEnrolled();
        const Bool justEnrolled = enrolledNow && !wasEnrolledOnLastCheck;
        wasEnrolledOnLastCheck = enrolledNow;

        if (!enrolledNow) {
            if (cloudSocket && cloudSocket->IsSocketOpen()) {
                cloudSocket->CloseSocket();
            }
            return false;
        }

        if (justEnrolled) {
            lastInternetConnectionId = 0;
            if (cloudSocket && cloudSocket->IsSocketOpen()) {
                cloudSocket->CloseSocket();
            }
            Thread::Sleep(2500);
        }

        if (!cloudSocket || !internetConnectionStatusProvider) {
            return false;
        }

        if (!IsInternetAvailable()) {
            StopSocketWhenNoInternet();
            return false;
        }

        ULong currentId = internetConnectionStatusProvider->GetInternetConnectionId();
        Bool internetJustRestored = (lastInternetConnectionId == 0 && currentId != 0);
        Bool internetChanged =
                (lastInternetConnectionId != 0 && currentId != lastInternetConnectionId);

        if (justEnrolled) {
            lastInternetConnectionId = currentId;
            if (!TryOpenSocket("post_enrollment")) {
                return false;
            }
        } else if (internetJustRestored) {
            lastInternetConnectionId = currentId;
            if (cloudSocket->IsSocketOpen()) {
                cloudSocket->CloseSocket();
            }
            if (!TryOpenSocket("internet_restored")) {
                return false;
            }
        } else if (internetChanged) {
            lastInternetConnectionId = currentId;
            if (cloudSocket->IsSocketOpen()) {
                cloudSocket->CloseSocket();
            }
            if (!TryOpenSocket("internet_changed")) {
                return false;
            }
        } else {
            lastInternetConnectionId = currentId;
        }

        if (!cloudSocket->IsSocketOpen()) {
            if (!TryOpenSocket("ensure_connected")) {
                return false;
            }
        }

        return cloudSocket->IsSocketOpen();
    }

    Private Void LogInfo(CStdString message) {
        if (logger != nullptr) {
            logger->Info(Tag::Untagged, message);
        }
    }
};

#endif // CLOUDSOCKETMANAGER_INTERNAL_H
#endif // ESP_PLATFORM
