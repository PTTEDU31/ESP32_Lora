#pragma once

#include "stdint.h"
#include "driver/gpio.h" // Thêm thư viện này để định nghĩa gpio_num_t
#include "../pins/pin_esp32.h"




void i2s_gpio();
void i2s_init(void);
void i2s_write(uint8_t pin, uint8_t val);
void load_buf();

void send_595();
