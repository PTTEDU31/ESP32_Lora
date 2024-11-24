#pragma once

#include "Arduino.h"

#include "commandService.h"

#include "loraMeshMessage.h"

class LoRaMeshCommandService: public CommandService {
public:
    LoRaMeshCommandService();
};