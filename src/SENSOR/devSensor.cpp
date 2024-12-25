#include "devSensor.h"
#include "targets.h"
#include "CurrentSensor/INA219.h"
#include "logging.h"
#include "Wire.h"
#include "messageManager.h"
#include "SensorMessage.h"

static INA219 INA(0x40);
size_t sensorMessageId = 0;
#include "PHSensor/PHSensor.h"
PHSensor *phSensor = new PHSensor();

bool SensorService::initialize()
{
#ifdef OPT_HAS_CURRENT
#ifdef OPT_HAS_CURRENT_INA219
    // devCurrent = new devINA219();
    if (!INA.begin())
    {
        DBGLN("INA219 init false");
        return 0;
    }
    return 1;

#endif

#endif
}

String SensorService::getJSON(DataMessage *message)
{
    MeasurementMessage *sensorMessage = (MeasurementMessage *)message;

    StaticJsonDocument<2000> doc;

    JsonObject root = doc.to<JsonObject>();

    sensorMessage->serialize(root);

    String json;
    serializeJson(doc, json);

    return json;
}

static int stat()
{
    return 60000;
}
DataMessage *SensorService::getDataMessage(JsonObject data)
{
    MeasurementMessage *measurement = new MeasurementMessage();
    measurement->deserialize(data);
    measurement->messageSize = sizeof(MeasurementMessage) - sizeof(DataMessageGeneric);
    return ((DataMessage *)measurement);
}

static int timeout()
{
    DBGLN("Current INA219 %f", INA.getBusVoltage_mV());
    MeasurementMessage *message = new MeasurementMessage();

    message->phSensorMessage = phSensor->read();
    message->appPortDst = appPort::MQTTApp;
    message->appPortSrc = appPort::SensorApp;
    message->addrSrc = LoraMesher::getInstance().getLocalAddress();
    message->addrDst = 0;
    message->messageId = sensorMessageId;

    sensorMessageId++;
    message->messageSize = sizeof(MeasurementMessage) - sizeof(DataMessageGeneric);
    MessageManager::getInstance().sendMessage(messagePort::MqttPort, (DataMessage *)message);
    // Delete the message
    MessageManager::getInstance().printDataMessageHeader("SendLora", (DataMessage *)message);
    delete message;
    return 60000;
}

device_t Sensor_dev = {
    .initialize = SensorService::getInstance().initialize,
    .start = stat,
    .event = nullptr,
    .timeout = timeout};