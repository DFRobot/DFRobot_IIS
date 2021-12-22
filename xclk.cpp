#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "xclk.h"
#include "Arduino.h"
static const char* TAG = "camera_xclk";

esp_err_t camera_enable_out_clock(const camera_config_t* config)
{
    periph_module_enable(PERIPH_LEDC_MODULE);
    ledc_timer_config_t timer_conf;
    timer_conf.duty_resolution = LEDC_TIMER_1_BIT;
    timer_conf.freq_hz = config->xclk_freq_hz;
    timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    timer_conf.timer_num = LEDC_TIMER_0;
	timer_conf.clk_cfg = LEDC_AUTO_CLK;
    esp_err_t err = ledc_timer_config(&timer_conf);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "ledc_timer_config failed, rc=%x", err);
        return err;
    }
    ledc_channel_config_t ch_conf;
    ch_conf.channel = LEDC_CHANNEL_0;
    ch_conf.timer_sel = LEDC_TIMER_0;
    ch_conf.intr_type = LEDC_INTR_DISABLE;
    ch_conf.duty = 1;
    ch_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    ch_conf.gpio_num = 21;
    ch_conf.hpoint         = 0;
    err = ledc_channel_config(&ch_conf);
    if(err != ESP_OK){
         ESP_LOGE(TAG, "ledc_channel_config failed, rc=%x", err);
        return err;
    }
    return ESP_OK;
}

