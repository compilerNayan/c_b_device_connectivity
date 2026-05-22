#ifndef WIFICOMMANDPROCESSOR_INTERNAL_H
#define WIFICOMMANDPROCESSOR_INTERNAL_H

#include <StandardDefines.h>
#include <pubsub/CommandProcessor.h>
#include <CommandRegistry.h>
#include <logger/ILogger.h>

class WiFiCommandProcessor final : public CommandProcessor {

    /* @Autowired */
    Private IWiFiConnectionManagerPtr connectionManager;

    /* @Autowired */
    Private ILoggerPtr logger;

    Public WiFiCommandProcessor() : CommandProcessor(TOPIC_WIFI) {}

    Public Virtual ~WiFiCommandProcessor() override = default;

    Public Virtual Void OnCommandReceived(const Command& cmd) override {
        switch (cmd.command) {
            case COMMAND_RESTART_WIFI:
                logger->Info(Tag::Untagged, "Received command to restart WiFi connection, sender: " + to_string(cmd.senderId) + " with payload: " + cmd.payload);
                connectionManager->RestartNetwork();
                break;
            default:
                logger->Warning(Tag::Untagged, "Received unknown command: " + to_string(cmd.command) + " from sender: " + to_string(cmd.senderId) + " with payload: " + cmd.payload);
                break;
        }
    }
}
#endif // WIFICOMMANDPROCESSOR_INTERNAL_H