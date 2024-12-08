#include "common.h"
uint8_t UID[UID_LEN] = {0};  // "bind phrase" ID
bool connectionHasModelMatch = false;
bool teamraceHasModelMatch = true; // true if isTx or teamrace disabled or (enabled and channel in correct postion)
connectionState_e connectionState = disconnected;
_isGateway isGateway = NODE;
// #if defined(RADIO_SX128X)

// #include "SX1280Driver.h"
// SX1280Driver DMA_ATTR Radio;

// #endif