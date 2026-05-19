#ifndef NETWORKSTATUSPROVIDER_H
#define NETWORKSTATUSPROVIDER_H

#include "../01-interface/02-INetworkStatusProvider.h"
#include "../01-interface/01-INetworkStatusStore.h"
#include <StandardDefines.h>

/**
 * Implementation that reads network status from INetworkStatusStore.
 */
/* @Component */
class NetworkStatusProvider : public INetworkStatusProvider {
    /* @Autowired */
    Private INetworkStatusStorePtr store;

    Public Bool IsNetworkConnected() const override {
        return store->IsNetworkConnected();
    }

    Public ULong GetNetworkConnectionId() const override {
        return store->GetNetworkConnectionId();
    }
};

#endif // NETWORKSTATUSPROVIDER_H
