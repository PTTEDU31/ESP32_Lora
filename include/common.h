#pragma once

#include "targets.h"
#include "common.h"
#include "config.h"
#include "helpers.h"
#include "logging.h"
// #include "device.h"

#define UID_LEN             6


typedef enum
{
    connected,
    tentative,        // RX only
    awaitingModelId,  // TX only
    disconnected,
    MODE_STATES,
    // States below here are special mode states
    noCrossfire,
    bleJoystick,
    NO_CONFIG_SAVE_STATES,
    wifiUpdate,
    serialUpdate,
    // Failure states go below here to display immediately
    FAILURE_STATES,
    radioFailed,
    hardwareUndefined
} connectionState_e;

extern bool teamraceHasModelMatch;
extern connectionState_e connectionState;
void setupTargetCommon();
