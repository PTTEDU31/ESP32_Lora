#include "mqttCommandService.h"
#include "devMQTT.h"

MqttCommandService::MqttCommandService() {
    // Send command to bluetooth
    addCommand(Command("/sendB", "Send a message to the mqtt device", 0, 1,
        [this](String args) {
        return DEV_MQTT::getInstance().writeToMqtt(args) ? "Message sent" : "Device not connected";
    }));
}