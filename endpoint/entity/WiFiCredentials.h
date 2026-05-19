#ifndef WIFICREDENTIALS_H
#define WIFICREDENTIALS_H

#include <StandardDefines.h>

/* @Entity */
class WiFiCredentials {

    /* @Id */
    /* @NotNull */
    Public optional<StdString> ssid;

    Public optional<StdString> password;
};

#endif
