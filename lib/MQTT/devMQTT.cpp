#include "devMQTT.h"
#include "logging.h"
#include "WiFi.h"
#include "ArduinoJson.h"
#include "options.h"
#include <AsyncMqttClient.h>
#include "loraMeshService.h"
#include <string.h>
AsyncMqttClient mqttClient;
bool DEV_MQTT::initialized = false;
static bool en_connected = 0;
void DEV_MQTT::connectToMqtt()
{
  DBGLN("Connecting to MQTT...");
  mqttClient.connect();
}
bool DEV_MQTT::writeToMqtt(String message)
{
  if (!mqttClient.connected())
  {
    DBGLN("MQTT Client is not connected.");
    return false;
  }

  uint16_t packetId = mqttClient.publish("test/lol", 1, true, message.c_str());
  DBGLN("Publishing message: %s, Packet ID: %d", message.c_str(), packetId);
  return true;
}
bool DEV_MQTT::writeToMqtt(DataMessage *message)
{
  if (!mqttClient.connected())
  {
    DBGLN("MQTT Client is not connected.");
    return false;
  }

  String json = MessageManager::getInstance().getJSON(message);

  // Gửi chuỗi JSON qua MQTT

  String topic = String(firmwareOptions.mqtt_topic_out) + String(message->addrSrc);
  uint16_t packetId = mqttClient.publish(topic.c_str(), 1, true, json.c_str());
  DBGLN("Publishing DataMessage: %s, Packet ID: %d", json.c_str(), packetId);
  return true;
}
void DEV_MQTT::processReceivedMessage(messagePort port, DataMessage *message)
{
  // TODO: Add some checks?
  writeToMqtt(message);
}
void onMqttConnect(bool sessionPresent)
{
  DBGLN("Connected to MQTT.");
  NodeBackpack->print("Session present: ");
  DBGLN("%d", sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe(firmwareOptions.mqtt_topic_sub, 2);
  DBGLN("Subscribing to topic %s at QoS 2, packetId: %d", firmwareOptions.mqtt_topic_sub, packetIdSub);
}
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  DBGLN("Message arrived on topic: %s", topic);
  DataMessage *message = MessageManager::getInstance().getDataMessage(payload);
  if (message == NULL)
  {
    DBGLN("Error parsing message");
    return;
  }
  if (message->addrDst == 0)
  {
    const char *lastSlash = strrchr(topic, '/');

    if (lastSlash != nullptr)
    {
      // Lấy phần chuỗi sau dấu '/'
      const char *addrPart = lastSlash + 1;

      // Chuyển đổi phần chuỗi này thành số nguyên
      message->addrDst = atoi(addrPart);
    }
    else
    {
      DBGLN("Error parsing destination address");
      delete message;
      return;
    }
  }

  MessageManager::getInstance().processReceivedMessage(messagePort::MqttPort, message);

  DBGLN("Message sent to services");

  delete message;
}
void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
  NodeBackpack->println("Subscribe acknowledged.");
  NodeBackpack->print("  packetId: ");
  NodeBackpack->println(packetId);
  NodeBackpack->print("  qos: ");
  NodeBackpack->println(qos);
}

void DEV_MQTT::initialize()
{
  initialized = true;
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onSubscribe(onMqttSubscribe);
  char buffer[5];
  sprintf(buffer, "%04X", LoRaMeshService::getInstance().getLocalAddress());
  mqttClient.setClientId(buffer);
  mqttClient.setServer(firmwareOptions.mqtt_server, firmwareOptions.mqtt_port);
}
static int start()
{
  return DURATION_NEVER;
}
static int event()
{
  if (connectionState == connected_STA)
  {

    DBGLN("Connecting to MQTT...");
    en_connected = 1;
    return DURATION_IMMEDIATELY;
  }
  else if (connectionState == disconnected_STA)
  {
    mqttClient.clearQueue();
    return DURATION_NEVER;
  }
  if (en_connected == 1)
    return DURATION_IMMEDIATELY;
  return DURATION_NEVER;
}

static int timeout()
{
  // Kiểm tra nếu MQTT client chưa kết nối
  if (!mqttClient.connected())
  {
    DBGLN("MQTT client is not connected, skipping publish.");
    mqttClient.connect();
  }
  return 60000;
}

device_t MQTT_device = {
    .initialize = DEV_MQTT::getInstance().initialize,
    .start = start,
    .event = event,
    .timeout = timeout,
};