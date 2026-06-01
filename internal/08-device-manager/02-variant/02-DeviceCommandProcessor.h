#ifndef DEVICECOMMANDPROCESSOR_INTERNAL_H
#define DEVICECOMMANDPROCESSOR_INTERNAL_H

#include <StandardDefines.h>
#include <pubsub/CommandProcessor.h>
#include <CommandRegistry.h>
#include <logger/ILogger.h>

class DeviceCommandProcessor final : public CommandProcessor {
    Public DeviceCommandProcessor() : CommandProcessor(TOPIC_DEVICE_MANAGER) {}

    Public Virtual ~DeviceCommandProcessor() override = default;

    /* @Autowired */
    Private ILoggerPtr logger;

    Public Virtual Void OnCommandReceived(const Command& cmd) override {
        switch (cmd.id) {
            default:
                //logger->Error(Tag::Untagged, "Unknown command received: " + std::to_string(cmd.id));
                break;
        }
    }
}