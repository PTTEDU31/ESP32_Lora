#include "Arduino.h"
#include <ESP_I2S.h>

const int buff_size = 128;
int available_bytes, read_bytes;
uint8_t buffer[buff_size];
I2SClass I2S;

void setup() {
  I2S.setPins(5, 25, 26, 35, 0); //SCK, WS, SDOUT, SDIN, MCLK
  I2S.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
  I2S.read();
  available_bytes = I2S.available();
  I2S.write(buffer, read_bytes);
  I2S.end();
}

void loop() {}