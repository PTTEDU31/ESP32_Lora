#include "devSensor.h"
#include "targets.h"
#include "CurrentSensor/INA219.h"
#include "logging.h"
#include "Wire.h"
#include "messageManager.h"
#include "SensorMessage.h"
#include "./Baro/baro_bmp280.h"
#include "./Baro/baro_spl06.h"

#define BARO_STARTUP_INTERVAL 100

static BaroBase *baro;
static eBaroReadState BaroReadState;
static INA219 INA(0x40);
size_t sensorMessageId = 0;
#include "PHSensor/PHSensor.h"
PHSensor *phSensor = new PHSensor();

extern bool i2c_enabled;

static bool Baro_Detect()
{
    // I2C Baros
#if defined(USE_I2C)
    if (i2c_enabled)
    {
        if (SPL06::detect())
        {
            DBGLN("Detected baro: SPL06");
            baro = new SPL06();
            return true;
        }
        if (BMP280::detect())
        {
            DBGLN("Detected baro: BMP280");
            baro = new BMP280();
            return true;
        }
        DBGLN("Detected baro: NONENONE");
    } // I2C
    baro = nullptr;
#endif
    return false;
}

static int Baro_Init()
{
    baro->initialize();
    if (baro->isInitialized())
    {
        // Slow down Vbat updates to save bandwidth
        Vbat_enableSlowUpdate(true);
        BaroReadState = brsReadTemp;
        return DURATION_IMMEDIATELY;
    }

    // Did not init, try again later
    return BARO_STARTUP_INTERVAL;
}

bool SensorService::initialize()
{
#ifdef OPT_HAS_CURRENT
#ifdef OPT_HAS_CURRENT_INA219
    // devCurrent = new devINA219();
    if (!INA.begin())
    {
        DBGLN("INA219 init false");
    }
    DBGLN("INA219 init true");
    return 1;

#endif

#endif
}

String SensorService::getJSON(DataMessage *message)
{
    MeasurementMessage *sensorMessage = (MeasurementMessage *)message;

    DynamicJsonDocument doc(2000); // Mới

    JsonObject root = doc.to<JsonObject>();

    sensorMessage->serialize(root);
    String json;
    serializeJson(doc, json);

    return json;
}

static int stat()
{
    if (Baro_Detect())
    {
        BaroReadState = brsUninitialized;
        return BARO_STARTUP_INTERVAL;
    }

    BaroReadState = brsNoBaro;
    return DURATION_NEVER;
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
    // DBGLN("Current INA219 %f", INA.getBusVoltage_mV());
    switch (BaroReadState)
    {
    default: // fallthrough
    case brsNoBaro:
        return DURATION_NEVER;

    case brsUninitialized:
        return Baro_Init();

    case brsWaitingTemp:
    {
        int32_t temp = baro->getTemperature();
        if (temp == BaroBase::TEMPERATURE_INVALID)
            return 3000;
    }
        // fallthrough

    case brsReadPres:
    {
        uint8_t pressDuration = baro->getPressureDuration();
        BaroReadState = brsWaitingPress;
        if (pressDuration != 0)
        {
            baro->startPressure();
            return pressDuration;
        }
    }
        // fallthrough

    case brsWaitingPress:
    {
        uint32_t press = baro->getPressure();
        if (press == BaroBase::PRESSURE_INVALID)
            return 3000;
        // Baro_PublishPressure(press);
    }
        // fallthrough

    case brsReadTemp:
    {
        uint8_t tempDuration = baro->getTemperatureDuration();
        if (tempDuration == 0)
        {
            BaroReadState = brsReadPres;
            return 3000;
        }
        BaroReadState = brsWaitingTemp;
        baro->startTemperature();
        return tempDuration;
    }
    }
}

static int senmessagetomqtt()
{
    MeasurementMessage *message = new MeasurementMessage();
    message->phSensorMessage = phSensor->read();
    message->baro = baro->read();
    message->ina = INA.read();


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
    return 22000;
}
static int sensorsenevent()
{
    if (connectionState == connected_STA || connectionState == connected)
    {
        return 2000;
    }
    return DURATION_NEVER;
}

device_t Send_message = {
    .initialize = []
    { return true; },
    .start = []
    { return DURATION_NEVER; },
    .event = sensorsenevent,
    .timeout = senmessagetomqtt,
    .subscribe = EVENT_CONNECTION_CHANGED};

device_t Sensor_dev = {
    .initialize = SensorService::getInstance().initialize,
    .start = stat,
    .event = nullptr,
    .timeout = timeout,
    .subscribe = EVENT_NONE};