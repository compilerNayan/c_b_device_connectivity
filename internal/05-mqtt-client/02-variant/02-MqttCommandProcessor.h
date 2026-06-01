#ifndef MQTTCOMMANDPROCESSOR_INTERNAL_H
#define MQTTCOMMANDPROCESSOR_INTERNAL_H

#include <StandardDefines.h>
#include <pubsub/CommandProcessor.h>
#include <CommandRegistry.h>
#include <logger/ILogger.h>
#include "../01-interface/01-IMqttClientManager.h"

class MqttCommandProcessor final : public CommandProcessor {

    /* @Autowired */
    Private IMqttClientManagerPtr mqttClientManager;

    /* @Autowired */
    Private ILoggerPtr logger;

    Public MqttCommandProcessor() : CommandProcessor(TOPIC_MQTT_CLIENT) {}

    Public Virtual ~MqttCommandProcessor() override = default;

    Public Virtual Void OnCommandReceived(const Command& cmd) override {
        switch (cmd.id) {
            case COMMAND_ENROLL_DEVICE:
                logger->Info(Tag::Untagged, "Received command to enroll device, sender: " + to_string(cmd.senderId) + " with payload: " + cmd.payload);
                mqttClientManager->EnrollDevice();
                break;
            default:
                //logger->Error(Tag::Untagged, "Unknown command received: " + std::to_string(cmd.id));
                break;
        }
    }
}