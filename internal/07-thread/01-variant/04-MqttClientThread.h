#ifndef MQTT_CLIENT_THREAD_H
#define MQTT_CLIENT_THREAD_H

#include <StandardDefines.h>
#include "threading/IRunnable.h"
#include "Thread.h"

#include "../../05-mqtt-client/00-public/01-IMqttClientManager.h"
#include "../../05-mqtt-client/00-public/03-ICloudSocketManager.h"
#include "../../05-mqtt-client/02-variant/02-MqttCommandProcessor.h"

/** Interval between checks (ms). Run() is called every loop; we only run the check when this much time has passed. */
static constexpr ULong kMqttClientCheckIntervalMs = 100;

class MqttClientThread : public IRunnable {
    /* @Autowired */
    Private IMqttClientManagerPtr mqttClientManager;

    /* @Autowired */
    Private ICloudSocketManagerPtr cloudSocketManager;

    Private MqttCommandProcessor mqttCommandProcessor;

    Public Void Run() override {
        while (true) {
            mqttCommandProcessor.ProcessCommands();
            mqttClientManager->EnsureMqttClientConnectivity();
            cloudSocketManager->EnsureCloudSocketConnectivity();
            Thread::Sleep(kMqttClientCheckIntervalMs);
        }
    }
};

#endif // MQTT_CLIENT_THREAD_H