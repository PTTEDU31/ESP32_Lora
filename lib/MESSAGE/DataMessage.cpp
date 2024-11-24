#include "MESSAGE.h"
#include "ArduinoJson.h"

uint32_t  DataMessageGeneric::getDataMessageSize()
{
    return sizeof(DataMessageGeneric) + messageSize;
}

void DataMessageGeneric::serialize(JsonObject &doc)
{
    // doc["appPortDst"] = appPortDst;
    // doc["appPortSrc"] = appPortSrc;
    doc["messageId"] = messageId;
    doc["addrSrc"] = addrSrc;
    doc["addrDst"] = addrDst;
    doc["messageSize"] = messageSize;
}

void DataMessageGeneric::deserialize(JsonObject &doc)
{
    appPortDst = (appPort)doc["appPortDst"];
    appPortSrc = (appPort)doc["appPortSrc"];
    messageId = doc["messageId"];
    addrSrc = doc["addrSrc"];
    addrDst = doc["addrDst"];
    messageSize = doc["messageSize"];
}
