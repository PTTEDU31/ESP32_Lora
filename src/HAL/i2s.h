#pragma once

#include "stdint.h"

#define DMA_BUFFER_LEN 1024

void i2s_gpio();
void i2s_init(void);
void i2s_write(uint8_t pin, uint8_t val);
void load_buf();

void send_595();
