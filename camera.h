#pragma once
#include "esp_err.h"
#include "driver/ledc.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CAMERA_PF_RGB565 = 0,       //!< RGB, 2 bytes per pixel (not implemented)
    CAMERA_PF_YUV422 = 1,       //!< YUYV, 2 bytes per pixel (not implemented)
    CAMERA_PF_GRAYSCALE = 2,    //!< 1 byte per pixel
    CAMERA_PF_JPEG = 3,         //!< JPEG compressed
} camera_pixelformat_t;

typedef enum {
    CAMERA_FS_QQVGA = 4,     //!< 160x120
    CAMERA_FS_QVGA = 8,      //!< 320x240
    CAMERA_FS_VGA = 10,      //!< 640x480
    CAMERA_FS_SVGA = 11,     //!< 800x600
} camera_framesize_t;

typedef enum {
    CAMERA_NONE = 0,
    CAMERA_UNKNOWN = 1,
    CAMERA_OV7725 = 7725,
    CAMERA_OV2640 = 2640,
} camera_model_t;

typedef struct {
    int pin_reset;          /*!< GPIO pin for camera reset line */
    int pin_xclk;           /*!< GPIO pin for camera XCLK line */
    int pin_sscb_sda;       /*!< GPIO pin for camera SDA line */
    int pin_sscb_scl;       /*!< GPIO pin for camera SCL line */
    int pin_d7;             /*!< GPIO pin for camera D7 line */
    int pin_d6;             /*!< GPIO pin for camera D6 line */
    int pin_d5;             /*!< GPIO pin for camera D5 line */
    int pin_d4;             /*!< GPIO pin for camera D4 line */
    int pin_d3;             /*!< GPIO pin for camera D3 line */
    int pin_d2;             /*!< GPIO pin for camera D2 line */
    int pin_d1;             /*!< GPIO pin for camera D1 line */
    int pin_d0;             /*!< GPIO pin for camera D0 line */
    int pin_vsync;          /*!< GPIO pin for camera VSYNC line */
    int pin_href;           /*!< GPIO pin for camera HREF line */
    int pin_pclk;           /*!< GPIO pin for camera PCLK line */

    int xclk_freq_hz;       /*!< Frequency of XCLK signal, in Hz */

    ledc_timer_t ledc_timer;        /*!< LEDC timer to be used for generating XCLK  */
    ledc_channel_t ledc_channel;    /*!< LEDC channel to be used for generating XCLK  */

    camera_pixelformat_t pixel_format;
    camera_framesize_t frame_size;

    int jpeg_quality;
} camera_config_t;

#define ESP_ERR_CAMERA_BASE 0x20000
#define ESP_ERR_CAMERA_NOT_DETECTED             (ESP_ERR_CAMERA_BASE + 1)
#define ESP_ERR_CAMERA_FAILED_TO_SET_FRAME_SIZE (ESP_ERR_CAMERA_BASE + 2)
#define ESP_ERR_CAMERA_NOT_SUPPORTED            (ESP_ERR_CAMERA_BASE + 3)

void      cameramode();
int       camera_get_fb_width();
int       camera_get_fb_height();
esp_err_t camera_probe(const camera_config_t* config, camera_model_t* out_camera_model);
esp_err_t camera_init(const camera_config_t* config);
esp_err_t camera_run(const char *pictureFilename);
uint8_t*  camera_get_fb();
size_t    camera_get_data_size();

#ifdef __cplusplus
}
#endif
