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
#define MAX_BUTON_ACTION           3
typedef enum
{
    connected,
    // tentative,        // RX only
    // awaitingModelId,  // TX only
    connected_STA,
    disconnected_STA,
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
typedef enum : uint8_t {
    ACTION_NONE,
    ACTION_INCREASE_POWER,
    ACTION_START_WIFI,
    ACTION_BIND,
    ACTION_BLE,
    ACTION_RESET_REBOOT,

    ACTION_LAST
} action_e;
typedef struct {
    uint8_t     pressType:1,    // 0 short, 1 long
                count:3,        // 1-8 click count for short, .5sec hold count for long
                action:4;       // action to execute
} button_action_t;

typedef union {
    struct {
        uint8_t color;                  // RRRGGGBB
        button_action_t actions[MAX_BUTON_ACTION];
        uint8_t unused;
    } val;
    // uint32_t raw;
} button_color_t;

extern uint8_t UID[UID_LEN];
extern connectionState_e connectionState;
extern bool teamraceHasModelMatch;
# define MAX_CONNECTION_TRY 10