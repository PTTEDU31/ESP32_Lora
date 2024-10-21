#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "i2s.h"
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_log.h"  // Sử dụng esp_log.h thay vì esp_check.h để sử dụng hàm log
#include "sdkconfig.h"

static i2s_chan_handle_t tx_chan; // I2S tx channel handler
static const char* TAG = "I2S";   // Thẻ (tag) cho log

// Constructor config struct
typedef struct config {
  gpio_num_t pin_clk = GPIO_NUM_NC;  // Chân cho CLK (BCK)
  gpio_num_t pin_ws = GPIO_NUM_NC;   // Chân cho WS (Word Select, thường là LRCLK)
  gpio_num_t pin_data = GPIO_NUM_NC; // Chân cho DOUT (Data Out)
} config_t;

config_t _cfg; // Lưu trữ cấu hình

static IRAM_ATTR bool i2s_tx_queue_overflow_callback(i2s_chan_handle_t handle, i2s_event_data_t *event, void *user_ctx)
{
     ESP_LOGW(TAG, "OVERFLOW Detected");
    return false; // Không yêu cầu xử lý thêm sau khi callback được gọi
}
static IRAM_ATTR bool i2s_tx_sent_callback(i2s_chan_handle_t handle, i2s_event_data_t *event, void *user_ctx)
{
     ESP_LOGW(TAG, "sent done Detected");
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
    .dma_frame_num = 254, \
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

	send_595();
}

static uint8_t *w_buf = (uint8_t *)calloc(1, 1029);
static size_t w_bytes = 1029;
void send_595(){
    assert(w_buf); // Check if w_buf allocation success

    for (int i = 0; i < 1029; i += 8) {
        w_buf[i]     = 0x12;
        w_buf[i + 1] = 0x34;
        w_buf[i + 2] = 0x56;
        w_buf[i + 3] = 0x78;
        w_buf[i + 4] = 0x9A;
        w_buf[i + 5] = 0xBC;
        w_buf[i + 6] = 0xDE;
        w_buf[i + 7] = 0xF0;
    }
	
    
    ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));
	
	ESP_LOGE(TAG, "Writing data...");


	// free(w_buf); // Giải phóng bộ nhớ sau khi sử dụng
}

void load_buf(){
	ESP_ERROR_CHECK(i2s_channel_write(tx_chan, w_buf, 1029, &w_bytes, 100));
	vTaskDelay(pdMS_TO_TICKS(5000));
}