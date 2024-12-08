
#include "devETH.h"
#include "common.h"
#include "logging.h"
#include "loraMeshService.h"
LoRaMeshService &loraMeshService = LoRaMeshService::getInstance();
static int print_Routing {
    DBGLN("\ni2s_init thành công");
    return 1000;
}
device_t eth_device = {
    .initialize = nullptr,
    .start = nullptr,
    .event = nullptr,
    .timeout = print_Routing,
    .id        = deviceId::nodev,
};