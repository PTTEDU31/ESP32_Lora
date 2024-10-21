#include "Arduino.h"
#include "HAL/i2s.h"

void setup() {
    Serial.begin(115200);
    i2s_init();
    ESP_LOGD("ESP32", "This is a debug log");

    // In thông báo thông tin
    ESP_LOGI("ESP32", "This is an info log");
}

void loop() {
    // Không cần loop vì quá trình truyền được thực hiện liên tục
    load_buf();
    vTaskDelay(1000);
}
