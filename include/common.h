#pragma once

#ifndef UNIT_TEST
#include "targets.h"

// #if defined(RADIO_SX127X)
// #include "SX127xDriver.h"
// #elif defined(RADIO_LR1121)
// #include "LR1121Driver.h"
// #elif defined(RADIO_SX128X)
// #include "SX1280Driver.h"
// #else
// #error "Radio configuration is not valid!"
#endif

#define OTA_VERSION_ID      4
#define UID_LEN             6

typedef enum
{
    connected,
    // tentative,        // RX only
    // awaitingModelId,  // TX only
    connected_STA,
    disconnected,
    MODE_STATES,
    // States below here are special mode states
    // noCrossfire,
    // bleJoystick,
    NO_CONFIG_SAVE_STATES,
    wifiUpdate,
    serialUpdate,
    // Failure states go below here to display immediately
    FAILURE_STATES,
    radioFailed,
    hardwareUndefined
} connectionState_e;

enum {
    RADIO_TYPE_SX127x_LORA,
    RADIO_TYPE_LR1121_LORA_900,
    RADIO_TYPE_LR1121_LORA_2G4,
    RADIO_TYPE_LR1121_GFSK_900,
    RADIO_TYPE_LR1121_GFSK_2G4,
    RADIO_TYPE_LR1121_LORA_DUAL,
    RADIO_TYPE_SX128x_LORA,
    RADIO_TYPE_SX128x_FLRC,
};
typedef enum {
    NODE,
    GATEWAY,
} _isGateway;

extern uint8_t UID[UID_LEN];
extern connectionState_e connectionState;
extern bool teamraceHasModelMatch;
extern _isGateway isGateway;
# define MAX_CONNECTION_TRY 10