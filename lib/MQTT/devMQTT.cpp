#include "devMQTT.h"
#include "MQTT.h"
#include "logging.h"
#include "WiFi.h"
#include "ArduinoJson.h"
#include "options.h"
#include <AsyncMqttClient.h>
#include "loraMeshService.h"
AsyncMqttClient mqttClient;

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

  // Tạo một JsonDocument để serialize dữ liệu
  JsonDocument jsonDoc;
  JsonObject jsonObj = jsonDoc.to<JsonObject>();

  // Serialize DataMessage vào JsonObject
  message->serialize(jsonObj);

  // Chuyển đổi JsonObject thành chuỗi JSON
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  // Gửi chuỗi JSON qua MQTT
  uint16_t packetId = mqttClient.publish("test/lol", 1, true, jsonString.c_str());
  DBGLN("Publishing DataMessage: %s, Packet ID: %d", jsonString.c_str(), packetId);
  return true;
}

void onMqttConnect(bool sessionPresent)
{
  DBGLN("Connected to MQTT.");
  NodeBackpack->print("Session present: ");
  DBGLN("%d", sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe("form-server", 2);
  DBGLN("Subscribing to topic 'form-server' at QoS 2, packetId: %d", packetIdSub);
}
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  NodeBackpack->println("Publish received.");
  NodeBackpack->print("  topic: ");
  NodeBackpack->println(topic);
  NodeBackpack->print("  qos: ");
  NodeBackpack->println(properties.qos);
  NodeBackpack->print("  dup: ");
  NodeBackpack->println(properties.dup);
  NodeBackpack->print("  retain: ");
  NodeBackpack->println(properties.retain);
  NodeBackpack->print("  len: ");
  NodeBackpack->println(len);
  NodeBackpack->print("  index: ");
  NodeBackpack->println(index);
  NodeBackpack->print("  total: ");
  NodeBackpack->println(total);

  // Kiểm tra nếu payload không null và có độ dài hợp lệ
  if (payload != nullptr && len > 0)
  {
    NodeBackpack->print("  payload: ");
    // Tạo buffer để chứa nội dung payload
    char buffer[len + 1]; // Thêm 1 byte để null-terminate
    memcpy(buffer, payload, len);
    buffer[len] = '\0'; // Đảm bảo buffer là chuỗi kết thúc bằng null
    NodeBackpack->println(buffer);
  }
  else
  {
    NodeBackpack->println("  payload: (empty or null)");
  }
}
void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
  NodeBackpack->println("Subscribe acknowledged.");
  NodeBackpack->print("  packetId: ");
  NodeBackpack->println(packetId);
  NodeBackpack->print("  qos: ");
  NodeBackpack->println(qos);
}

static void initialize()
{
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.setServer(firmwareOptions.mqtt_server, firmwareOptions.mqtt_port);
}
static int timeout()
{
  // Kiểm tra nếu MQTT client chưa kết nối
  if (!mqttClient.connected())
  {
    DBGLN("MQTT client is not connected, skipping publish.");
    return 20000;
  }

  // Tạo dữ liệu JSON để gửi
  JsonDocument jsonDoc;
  jsonDoc["messageId"] = 1;
  jsonDoc["addrSrc"] = 123;
  jsonDoc["addrDst"] = 456;
  jsonDoc["messageSize"] = 128;

  // Chuyển dữ liệu JSON thành chuỗi
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  // Gửi dữ liệu lên MQTT
  const char *topic = "test/timeout";
  uint16_t packetId = mqttClient.publish(topic, 1, true, jsonString.c_str());

  // Debug thông tin
  DBGLN("Published to topic %s, Packet ID: %d", topic, packetId);
  DBGLN("Payload: %s", jsonString.c_str());
  uint16_t packetIdPub3 = mqttClient.publish("test/lol", 2, true, LoRaMeshService::getInstance().getRoutingTable().c_str());
  NodeBackpack->print("Publishing at QoS 2, packetId: ");
  DBGLN("%d", packetIdPub3);
  return 20000;
}

device_t MQTT_device = {
    .initialize = initialize,
    .start = []() -> int
    { return DURATION_IMMEDIATELY; }, // Lambda function
    .event = nullptr,
    .timeout = timeout,
};