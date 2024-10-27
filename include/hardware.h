#include <stdint.h>

typedef enum {
    // Serial
    HARDWARE_serial_rx,
    HARDWARE_serial_tx,
    HARDWARE_serial1_rx,
    HARDWARE_serial1_tx,

    // Radio
    HARDWARE_radio_busy,
    HARDWARE_radio_busy_2,
    HARDWARE_radio_dio0,
    HARDWARE_radio_dio0_2,
    HARDWARE_radio_dio1,
    HARDWARE_radio_dio1_2,
    HARDWARE_radio_dio2,
    HARDWARE_radio_miso,
    HARDWARE_radio_mosi,
    HARDWARE_radio_nss,
    HARDWARE_radio_nss_2,
    HARDWARE_radio_rst,
    HARDWARE_radio_rst_2,
    HARDWARE_radio_sck,
    HARDWARE_radio_dcdc,
    HARDWARE_radio_rfo_hf,
    HARDWARE_radio_rfsw_ctrl,
    HARDWARE_radio_rfsw_ctrl_count,

    // Radio Antenna
    HARDWARE_ant_ctrl,
    HARDWARE_ant_ctrl_compl,

    // Radio power
    HARDWARE_power_enable,
    HARDWARE_power_apc1,
    HARDWARE_power_apc2,
    HARDWARE_power_rxen,
    HARDWARE_power_txen,
    HARDWARE_power_rxen_2,
    HARDWARE_power_txen_2,
    HARDWARE_power_lna_gain,
    HARDWARE_power_min,
    HARDWARE_power_high,
    HARDWARE_power_max,
    HARDWARE_power_default,

    HARDWARE_power_pdet,
    HARDWARE_power_pdet_intercept,
    HARDWARE_power_pdet_slope,

    HARDWARE_power_control,
    HARDWARE_power_values,
    HARDWARE_power_values2,
    HARDWARE_power_values_dual,

    // Input
    HARDWARE_joystick,
    HARDWARE_joystick_values,

    HARDWARE_five_way1,
    HARDWARE_five_way2,
    HARDWARE_five_way3,

    HARDWARE_button,
    HARDWARE_button_led_index,
    HARDWARE_button2,
    HARDWARE_button2_led_index,

    // Lighting
    HARDWARE_led,
    HARDWARE_led_blue,
    HARDWARE_led_blue_invert,
    HARDWARE_led_green,
    HARDWARE_led_green_invert,
    HARDWARE_led_green_red,
    HARDWARE_led_red,
    HARDWARE_led_red_invert,
    HARDWARE_led_red_green,
    HARDWARE_led_rgb,
    // GPIO_PIN_LED_WS2812_FAST // stm32
    HARDWARE_led_rgb_isgrb,
    HARDWARE_ledidx_rgb_status,
    HARDWARE_ledidx_rgb_status_count,
    HARDWARE_ledidx_rgb_vtx,
    HARDWARE_ledidx_rgb_vtx_count,
    HARDWARE_ledidx_rgb_boot,
    HARDWARE_ledidx_rgb_boot_count,

    // OLED/TFT
    HARDWARE_screen_cs,       // SPI
    HARDWARE_screen_dc,       // SPI
    HARDWARE_screen_mosi,     // SPI
    HARDWARE_screen_rst,      // SPI & I2c (optional)
    HARDWARE_screen_sck,      // clock for SPI & I2C
    HARDWARE_screen_sda,      // I2C data

    // screen_type == 0 is no screen
    HARDWARE_screen_type,
    HARDWARE_screen_reversed,
    HARDWARE_screen_bl,

    //W5500
    HARDWARE_use_W5500,
    HARDWARE_rs_W5500,
    HARDWARE_intr_W5500,
    HARDWARE_mac_W5500,
    HARDWARE_ip_W5500,
    HARDWARE_gateway_W5500,
    HARDWARE_subnet_W5500,


    // Backpack
    HARDWARE_use_backpack,
    HARDWARE_debug_backpack_baud,
    HARDWARE_debug_backpack_rx,
    HARDWARE_debug_backpack_tx,
    HARDWARE_backpack_boot,
    HARDWARE_backpack_en,
    HARDWARE_passthrough_baud,

    // I2C & Misc devices
    HARDWARE_i2c_scl,
    HARDWARE_i2c_sda,
    HARDWARE_misc_gsensor_int,
    HARDWARE_misc_buzzer,  // stm32 only
    HARDWARE_misc_fan_en,
    HARDWARE_misc_fan_pwm,
    HARDWARE_misc_fan_tacho,
    HARDWARE_misc_fan_speeds,
    HARDWARE_misc_fan_speeds_count,
    HARDWARE_gsensor_stk8xxx,
    HARDWARE_thermal_lm75a,
    HARDWARE_misc_bmi280,
    HARDWARE_misc_ina226,
    

    // PWM
    HARDWARE_pwm_outputs,
    HARDWARE_pwm_outputs_count,
    // PWM Outputs
    HARDWARE_pwm_out1,           // PWM đầu ra 1
    HARDWARE_pwm_out2,           // PWM đầu ra 2
    HARDWARE_pwm_out3,           // PWM đầu ra 3
    HARDWARE_pwm_out4,           // PWM đầu ra 4
    HARDWARE_pwm_out5,           // PWM đầu ra 5
    HARDWARE_pwm_out6,           // PWM đầu ra 6
    HARDWARE_pwm_out7,           // PWM đầu ra 7
    HARDWARE_pwm_out8,           // PWM đầu ra 8


    // VBAT
    HARDWARE_vbat,
    HARDWARE_vbat_offset,
    HARDWARE_vbat_scale,
    HARDWARE_vbat_atten,

    // Relay Control
    HARDWARE_relay1,             // Relay điều khiển 1
    HARDWARE_relay2,             // Relay điều khiển 2

    // High Current Outputs
    HARDWARE_high_current_out1,  // Đầu ra dòng cao 1
    HARDWARE_high_current_out2,  // Đầu ra dòng cao 2

    // Medium Current Outputs
    HARDWARE_med_current_out1,   // Đầu ra dòng trung bình 1
    HARDWARE_med_current_out2,   // Đầu ra dòng trung bình 2
    HARDWARE_med_current_out3,   // Đầu ra dòng trung bình 3
    HARDWARE_med_current_out4,   // Đầu ra dòng trung bình 4

    // Analog Inputs
    HARDWARE_analog_in1,         // Đầu vào analog 1
    HARDWARE_analog_in2,         // Đầu vào analog 2

    // Digital Inputs
    HARDWARE_digital_in1,        // Đầu vào digital 1
    HARDWARE_digital_in2,        // Đầu vào digital 2

        // I2S Pins
    HARDWARE_i2s_clk,        // Chân xung clock của I2S
    HARDWARE_i2s_data,       // Chân dữ liệu của I2S
    HARDWARE_i2s_latch,      // Chân latch của I2S
    
    HARDWARE_LAST
} nameType;
 
bool hardware_init();
const int hardware_pin(nameType name);
const bool hardware_flag(nameType name);
const int hardware_int(nameType name);
const float hardware_float(nameType name);
const int16_t* hardware_i16_array(nameType name);
const uint16_t* hardware_u16_array(nameType name);
