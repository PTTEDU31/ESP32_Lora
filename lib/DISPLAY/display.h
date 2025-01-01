#pragma once

#include "targets.h"

class Display
{
public:
    virtual void init() = 0;
    virtual void display_print_addr() = 0 ;
    virtual void display_wifi_status() = 0;
    
protected:
    std::vector<String> displayTextVector;
};
