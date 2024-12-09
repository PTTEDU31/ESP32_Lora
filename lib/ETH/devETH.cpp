
#include "devETH.h"
#include "common.h"
#include "logging.h"
#include "loraMeshService.h"
static int print_Routing () {
    // DBGLN("\n Routing table: %s",LoRaMeshService::getInstance().getRoutingTable());
    return 100;
}
device_t eth_device = {
    .initialize = nullptr,
    .start = nullptr,
    .event = nullptr,
    .timeout = print_Routing,
    .id        = deviceId::nodev,
};