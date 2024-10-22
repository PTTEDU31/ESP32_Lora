#include "i2s.h"
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_log.h"  // Sử dụng esp_log.h thay vì esp_check.h để sử dụng hàm log
#include "sdkconfig.h"

#include"include.h"
static i2s_chan_handle_t tx_chan; // I2S tx channel handler
static const char* TAG = "I2S";   // Thẻ (tag) cho log
uint32_t i2s_port_data = 0;
// Constructor config struct
typedef struct config {
  gpio_num_t pin_clk = GPIO_NUM_NC;  // Chân cho CLK (BCK)
  gpio_num_t pin_ws = GPIO_NUM_NC;   // Chân cho WS (Word Select, thường là LRCLK)
  gpio_num_t pin_data = GPIO_NUM_NC; // Chân cho DOUT (Data Out)
} config_t;

config_t _cfg; // Lưu trữ cấu hình

static IRAM_ATTR bool i2s_tx_queue_overflow_callback(i2s_chan_handle_t handle, i2s_event_data_t *event, void *user_ctx)
{
    //  ESP_LOGW(TAG, "OVERFLOW Detected");
    return false; // Không yêu cầu xử lý thêm sau khi callback được gọi
}
static IRAM_ATTR bool i2s_tx_sent_callback(i2s_chan_handle_t handle, i2s_event_data_t *event, void *user_ctx)
{
    //  ESP_LOGW(TAG, "sent done Detected");
    return false; // Không yêu cầu xử lý thêm sau khi callback được gọi
}
void i2s_gpio() 
{
	if (_cfg.pin_clk == GPIO_NUM_NC)
	{
		_cfg.pin_clk = (gpio_num_t) I2S_BCK; // Chân mặc định cho BCK
	}
	if (_cfg.pin_ws == GPIO_NUM_NC)
	{
		_cfg.pin_ws = (gpio_num_t) I2S_WS; // Chân mặc định cho WS
	}
	if (_cfg.pin_data == GPIO_NUM_NC)
	{
		_cfg.pin_data = (gpio_num_t) I2S_DATA; // Chân mặc định cho DOUT
	}
}

#define I2S_CHANNEL_CONFIG(i2s_num, i2s_role) { \
    .id = i2s_num, \
    .role = i2s_role, \
    .dma_desc_num = 8, \
    .dma_frame_num = DMA_BUFFER_LEN, \
    .auto_clear = true, \
}

void i2s_init(void)
{
	
	i2s_gpio();
	i2s_chan_config_t tx_chan_cfg = I2S_CHANNEL_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
	ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_cfg, &tx_chan, NULL));

	i2s_std_config_t tx_std_cfg = {
		.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(250000),
		.slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
		.gpio_cfg = {
			.mclk = I2S_GPIO_UNUSED, 
			.bclk = _cfg.pin_clk,
			.ws = _cfg.pin_ws,
			.dout = _cfg.pin_data,
			.din = I2S_GPIO_UNUSED,
			.invert_flags = {
				.mclk_inv = false,
				.bclk_inv = false,
				.ws_inv = false,
			},
		},
	};
	ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &tx_std_cfg));

	i2s_event_callbacks_t cbs = {
        .on_recv = NULL,
        .on_recv_q_ovf = NULL,
        .on_sent = i2s_tx_sent_callback,
        .on_send_q_ovf = i2s_tx_queue_overflow_callback,
	};
	ESP_ERROR_CHECK(i2s_channel_register_event_callback(tx_chan, &cbs, NULL));

	ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));
	
	ESP_LOGE(TAG, "Writing data...");
}

static uint8_t *w_buf = (uint8_t *)calloc(1, DMA_BUFFER_LEN);
static size_t w_bytes = DMA_BUFFER_LEN;
void i2s_push_sample(){
	for(uint8_t p = 0; p < MAX_EX_PIN;p++){
		if(hal.pwm_pin_data[p].pwm_duty_ticks > 0){
		if (TEST32(i2s_port_data, p)) {  // hi->lo
          CBI32(i2s_port_data, p);
          hal.pwm_pin_data[p].pwm_tick_count = hal.pwm_pin_data[p].pwm_cycle_ticks - hal.pwm_pin_data[p].pwm_duty_ticks;
        }
		        else { // lo->hi
          SBI32(i2s_port_data, p);
          hal.pwm_pin_data[p].pwm_tick_count = hal.pwm_pin_data[p].pwm_duty_ticks;
        }
		}
		else
        hal.pwm_pin_data[p].pwm_tick_count--;
    }
	w_buf = i2s_port_data;
}
void load_buf(){
	i2s_channel_write(tx_chan, w_buf, DMA_BUFFER_LEN, &w_bytes, 1000);
}