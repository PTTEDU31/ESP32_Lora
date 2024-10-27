#pragma once

#include "stdint.h"

#define DMA_BUFFER_LEN 1024

extern uint32_t i2s_port_data;

void i2s_gpio();
void i2s_init(void);
void i2s_write(uint8_t pin, uint8_t val);
uint8_t i2s_state(uint8_t pin);
