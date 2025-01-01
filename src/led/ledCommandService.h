#pragma once

#include "Arduino.h"

#include "commandService.h"

#include "ledMessage.h"

class LedCommandService: public CommandService {
public:
    LedCommandService();
};