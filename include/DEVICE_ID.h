enum class deviceId : uint8_t {
    nodev = 0,      // Không có thiết bị
    LoRaMesh = 1,   // Thiết bị LoRa Mesh
    Bluetooth = 2,  // Thiết bị Bluetooth
    WiFi = 3,       // Thiết bị WiFi
    Mqtt = 4,       // MQTT
    InternalPort = 5, // Cổng nội bộ
    Rs485 = 6,      // Giao tiếp RS485
    Display = 7     // Thiết bị hiển thị
};
