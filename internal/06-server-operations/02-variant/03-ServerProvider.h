#ifndef SERVERPROVIDER_H
#define SERVERPROVIDER_H

#include <StandardDefines.h>
#include "communication/IServerProvider.h"

#include "../01-interface/01-ILocalServer.h"
#include "../01-interface/02-ICloudServer.h"

/**
 * Provider class for managing server instances
 * Manages server lifecycle and provides singleton access to the default server
 */
/* @Component */
class ServerProvider final : public IServerProvider {
    Public ServerProvider() = default;
    Public Virtual ~ServerProvider() override = default;

    /* @Autowired */
    Private ILocalServerPtr localServer;

    /* @Autowired */
    Private ICloudServerPtr cloudServer;

    Public IServerPtr GetLocalServer() const override{
        return localServer;
    }
    Public ILocalServerPtr GetLocalServerPtr() const override {
        return localServer;
    }
    Public IServerPtr GetCloudServer() const override {
        return cloudServer;
    }
    Public ICloudServerPtr GetCloudServerPtr() const override {
        return cloudServer;
    }


};

#endif // SERVERPROVIDER_H

