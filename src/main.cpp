#include "common.h"

#include "devLED.h"
void device1_initialize() { /* Cấu hình IO cho thiết bị 1 */ }
int device1_start() { return 1000; }  // Gọi `timeout` sau 1000 ms
int device1_event() { return DURATION_IGNORE; }
int device1_timeout() { /* Thực hiện hành động khi timeout */ return 2000; }

void device2_initialize() { /* Cấu hình IO cho thiết bị 2 */ }
int device2_start() { return 500; }
int device2_event() { return 1000; }  // Thời gian mới cho `timeout`
int device2_timeout() { /* Thực hiện hành động cho thiết bị 2 */ return 500; }

// Khởi tạo các thiết bị
device_t device1 = {device1_initialize, device1_start, device1_event, device1_timeout};
device_t device2 = {device2_initialize, device2_start, device2_event, device2_timeout};

device_affinity_t devices[] = {
    {&device1, 1},  // Chạy trên core loop chính
    {&device2, 0},   // Chạy trên core phụ
    {&RGB_device, 0},
};

void setup() {
    devicesRegister(devices, ARRAY_SIZE(devices));  // Đăng ký 2 thiết bị
    devicesInit();
    devicesStart();
}

void loop() {
    unsigned long now = millis();
    devicesUpdate(now);
    // Thực hiện các công việc khác
}
