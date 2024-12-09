#include "devMQTT.h"
#include"common.h"
#include "logging.h"
 #include "WiFi.h"
#include <AsyncMqttClient.h>

AsyncMqttClient mqttClient;

void connectToMqtt() {
    DBGLN("Connecting to MQTT...");
    mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  DBGLN("Connected to MQTT.");
  Serial.print("Session present: ");
  DBGLN(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe("test/lol", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  DBGLN(packetIdSub);
  mqttClient.publish("test/lol", 0, true, "test 1");
  DBGLN("Publishing at QoS 0");
  uint16_t packetIdPub1 = mqttClient.publish("test/lol", 1, true, "test 2");
  Serial.print("Publishing at QoS 1, packetId: ");
  DBGLN(packetIdPub1);
  uint16_t packetIdPub2 = mqttClient.publish("test/lol", 2, true, "test 3");
  Serial.print("Publishing at QoS 2, packetId: ");
  DBGLN(packetIdPub2);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}
void initialize(){
     

}

device_t MQTT_device = {
    .initialize = initialize,
    .start      = nullptr,
    .event      = nullptr,
    .timeout    = nullptr,
}