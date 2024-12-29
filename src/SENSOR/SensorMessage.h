#include "dataMessage.h"
#include "PHSensor/PHSensor.h"
#include "./Sensor_base.h"
class MeasurementMessage : public DataMessageGeneric
{
public:
    PHSensorMessage phSensorMessage;
    Sensor_base *baro;
    void serialize(JsonObject &doc)
    {
        // Create a data object
        JsonObject dataObj = doc["data"].to<JsonObject>();

        // Add the data to the data object
        serializeDataSerialize(dataObj);
    }
    void deserialize(JsonObject &doc)
    {
        // Call the base class serialize function
        ((DataMessageGeneric *)(this))->deserialize(doc);
    }
    void serializeDataSerialize(JsonObject &doc)
    {
        // Call the base class serialize function
        ((DataMessageGeneric *)(this))->serialize(doc);

        // Add the GPS data to the JSON object
        // gps.serialize(doc);

        // Add that is a measurement message
        doc["message_type"] = "measurement";

        // Set all the measurements in an array called "message"
        JsonArray measurements = doc["message"].to<JsonArray>(); // Thay createNestedArray bằng to<JsonArray>()

        // // Add the PH sensor data to the JSON object
        phSensorMessage.serialize(measurements);
        if (baro != nullptr)
        {
            baro->serialize(measurements);
        }else
        {
            /* code */
            serializenullbaro(measurements);
        }
        
    }

private:
    void serializenullbaro(JsonArray &doc)
    {
        // Thêm nhiệt độ vào JSON
        JsonObject tempObj = doc.add<JsonObject>();
        tempObj["measurement"] = static_cast<float>(-273)  ; // Nhiệt độ (°C)
        tempObj["type"] = "BMP280_Temperature";                             // Loại cảm biến

        // Thêm áp suất vào JSON
        JsonObject pressureObj = doc.add<JsonObject>();
        pressureObj["measurement"] = static_cast<float>(-1) ; // Áp suất (hPa)
        pressureObj["type"] = "BMP280_Pressure";                                // Loại cảm biến
    }
};
