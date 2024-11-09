#pragma once

#include "targets.h"
// #include "elrs_eeprom.h"
#include "options.h"
#include "common.h"

#if defined(PLATFORM_ESP32)
#include <nvs_flash.h>
#include <nvs.h>
#endif