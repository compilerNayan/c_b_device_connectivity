#ifndef NETWORKCOMMANDPROCESSOR_INTERNAL_H
#define NETWORKCOMMANDPROCESSOR_INTERNAL_H

#include <CommandRegistry.h>
#include <pubsub/CommandProcessor.h>

class NetworkCommandProcessor final : public CommandProcessor {
    Public NetworkCommandProcessor() : CommandProcessor(TOPIC_NETWORK) {}

    Public Virtual ~NetworkCommandProcessor() override = default;

    Public Virtual Void OnCommandReceived(const Command& cmd) override {
        switch (cmd.command) {
            case COMMAND_RESTART_NETWORK:
                RestartNetwork();
                break;
            default:
                break;
        }
    }
};

#endif // NETWORKCOMMANDPROCESSOR_INTERNAL_H