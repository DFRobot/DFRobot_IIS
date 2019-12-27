#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "time.h"
#include "sys/time.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/Event_groups.h"
#include "rom/lldesc.h"
#include "soc/soc.h"
#include "soc/gpio_sig_map.h"
#include "soc/i2s_reg.h"
extern "C" {
#include "soc/i2s_struct.h"
#include "wiring.h"
}
#include "soc/io_mux_reg.h"
#include "driver/gpio.h"
#include "driver/periph_ctrl.h"
#include "esp_intr_alloc.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"
#include "sensor.h"
#include "sccb.h"
#include "camera.h"
#include "camera_common.h"
#include "xclk.h"
#include "bmp.h"
#include "SDcard.h"
#include "sys/socket.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#define CONFIG_OV7725_SUPPORT 1
#if CONFIG_OV7725_SUPPORT
#include "ov7725.h"
#endif

#define ENABLE_TEST_PATTERN CONFIG_ENABLE_TEST_PATTERN
#define REG_PID        0x0A
#define REG_VER        0x0B
#define REG_MIDH       0x1C
#define REG_MIDL       0x1D

static camera_pixelformat_t s_pixel_format;
static void i2s_init();
static void i2s_run();
static void IRAM_ATTR gpio_isr(void* arg);
static void IRAM_ATTR i2s_isr(void* arg);
static void dma_desc_deinit();
static void dma_filter_task(void *pvParameters);
static void dma_filter_grayscale(const dma_elem_t* src, lldesc_t* dma_desc, uint8_t* dst);
static void dma_filter_grayscale_highspeed(const dma_elem_t* src, lldesc_t* dma_desc, uint8_t* dst);
static void dma_filter_jpeg(const dma_elem_t* src, lldesc_t* dma_desc, uint8_t* dst);
static void dma_filter_rgb565(const dma_elem_t* src, lldesc_t* dma_desc, uint8_t* dst);
static void i2s_stop();
static void get_bmp(const char*pictureFilename);
static esp_err_t dma_desc_init();
static const char* TAG = "camera";

camera_state_t* s_state = NULL;
HANDLE_BMP bmp= (HANDLE_BMP)calloc(1,sizeof(struct BMP));

int state = 0;
const static char http_hdr[] = "HTTP/1.1 200 OK\r\n";
const static char http_stream_hdr[] = 
        "Content-type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n\r\n";
const static char http_jpg_hdr[] =
        "Content-type: image/jpg\r\n\r\n";
const static char http_pgm_hdr[] =
        "Content-type: image/x-portable-graymap\r\n\r\n";
const static char http_stream_boundary[] = "--123456789000000000000987654321\r\n";

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
static ip4_addr_t s_ip_addr;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            s_ip_addr = event->event_info.got_ip.ip_info.ip;
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            /* This is a workaround as ESP32 WiFi libs don't currently
             auto-reassociate. */
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void initialise_wifi(const char* ssid,const char* password)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t  wifi_config={ 0 };
    strncpy((char*)wifi_config.sta.ssid,ssid,sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password,password,sizeof(wifi_config.sta.password));
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_set_ps(WIFI_PS_NONE) );
    ESP_LOGE(TAG, "Connecting to \"%s\"", wifi_config.sta.ssid);
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
    ESP_LOGE(TAG, "Connected");
}

static esp_err_t camerarun()
{
    if (s_state == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    struct timeval tv_start;
    gettimeofday(&tv_start, NULL);
#ifndef _NDEBUG
    memset(s_state->fb, 0, s_state->fb_size);
#endif // _NDEBUG
    i2s_run();
    ESP_LOGD(TAG, "Waiting for frame");
    xSemaphoreTake(s_state->frame_ready, portMAX_DELAY);
    struct timeval tv_end;
    gettimeofday(&tv_end, NULL);
    int time_ms = (tv_end.tv_sec - tv_start.tv_sec) * 1000 + (tv_end.tv_usec - tv_start.tv_usec) / 1000;
    ESP_LOGI(TAG, "Frame %d done in %d ms", s_state->frame_count, time_ms);
    s_state->frame_count++;
    return ESP_OK;
}

static void http_server_netconn_serve(struct netconn *conn)
{
    struct netbuf *inbuf;
    char *buf;
    u16_t buflen;
    err_t err;
    /* Read the data from the port, blocking if nothing yet there.
     We assume the request (the part we care about) is in one netbuf */
    err = netconn_recv(conn, &inbuf);
    if (err == ERR_OK) {
        netbuf_data(inbuf, (void**) &buf, &buflen);
        /* Is this an HTTP GET command? (only check the first 5 chars, since
         there are other formats for GET, and we're keeping it very simple )*/
        if (buflen >= 5 && buf[0] == 'G' && buf[1] == 'E' && buf[2] == 'T'
                && buf[3] == ' ' && buf[4] == '/') {
          /* Send the HTTP header
             * subtract 1 from the size, since we dont send the \0 in the string
             * NETCONN_NOCOPY: our data is const static, so no need to copy it
             */
            netconn_write(conn, http_hdr, sizeof(http_hdr) - 1,
                    NETCONN_NOCOPY);
            //check if a stream is requested.
            if (buf[5] == 's') {
                //Send mjpeg stream header
                err = netconn_write(conn, http_stream_hdr, sizeof(http_stream_hdr) - 1,
                    NETCONN_NOCOPY);
                ESP_LOGD(TAG, "Stream started.");
                //Run while everyhting is ok and connection open.
                while(err == ERR_OK) {
                    ESP_LOGD(TAG, "Capture frame");
                    err = camerarun();
                    if (err != ESP_OK) {
                        ESP_LOGD(TAG, "Camera capture failed with error = %d", err);
                    } else {
                        ESP_LOGD(TAG, "Done");
                        //Send jpeg header
                        err = netconn_write(conn, http_jpg_hdr, sizeof(http_jpg_hdr) - 1,
                            NETCONN_NOCOPY);
                        //Send frame
                        err = netconn_write(conn, camera_get_fb(), camera_get_data_size(),
                            NETCONN_NOCOPY);
                        if(err == ERR_OK)
                        {
                            //Send boundary to next jpeg
                            err = netconn_write(conn, http_stream_boundary,
                                    sizeof(http_stream_boundary) -1, NETCONN_NOCOPY);
                        }
                    }
                }
                ESP_LOGD(TAG, "Stream ended.");
            } else {
                if (s_pixel_format == CAMERA_PF_JPEG) {
                    netconn_write(conn, http_jpg_hdr, sizeof(http_jpg_hdr) - 1, NETCONN_NOCOPY);
                } else if (s_pixel_format == CAMERA_PF_GRAYSCALE) {
                    netconn_write(conn, http_pgm_hdr, sizeof(http_pgm_hdr) - 1, NETCONN_NOCOPY);
                    if (memcmp(&buf[5], "pgm", 3) == 0) {
                        char pgm_header[32];
                        snprintf(pgm_header, sizeof(pgm_header), "P5 %d %d %d\n", camera_get_fb_width(), camera_get_fb_height(), 255);
                        netconn_write(conn, pgm_header, strlen(pgm_header), NETCONN_COPY);
                    }
                }
                ESP_LOGD(TAG, "Image requested.");
                err = camerarun();
                if (err != ESP_OK) {
                    ESP_LOGD(TAG, "Camera capture failed with error = %d", err);
                } else {
                    ESP_LOGD(TAG, "Done");
                    //Send jpeg
                    err = netconn_write(conn, camera_get_fb(), camera_get_data_size(),
                        NETCONN_NOCOPY);
                }
            }
        }
    }
    /* Close the connection (server closes in HTTP) */
    netconn_close(conn);
    /* Delete the buffer (netconn_recv gives us ownership,
     so we have to make sure to deallocate the buffer) */
    netbuf_delete(inbuf);
}

static void http_server(void *pvParameters)
{
    struct netconn *conn, *newconn;
    err_t err;
    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, NULL, 80);
    netconn_listen(conn);
    do {
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK) {
            http_server_netconn_serve(newconn);
            netconn_delete(newconn);
        }
    } while (err == ERR_OK);
    netconn_close(conn);
    netconn_delete(conn);
}

const int resolution[][2] = {
        { 40, 30 }, /* 40x30 */
        { 64, 32 }, /* 64x32 */
        { 64, 64 }, /* 64x64 */
        { 88, 72 }, /* QQCIF */
        { 160, 120 }, /* QQVGA */
        { 128, 160 }, /* QQVGA2*/
        { 176, 144 }, /* QCIF  */
        { 240, 160 }, /* HQVGA */
        { 320, 240 }, /* QVGA  */
        { 352, 288 }, /* CIF   */
        { 640, 480 }, /* VGA   */
        { 800, 600 }, /* SVGA  */
        { 1280, 1024 }, /* SXGA  */
        { 1600, 1200 }, /* UXGA  */
};

static bool is_hs_mode()
{
    return s_state->config.xclk_freq_hz > 10000000;
}

static size_t i2s_bytes_per_sample(i2s_sampling_mode_t mode)
{
    switch(mode){
        case SM_0A00_0B00:
            return 4;
        case SM_0A0B_0B0C:
            return 4;
        case SM_0A0B_0C0D:
            return 2;
        default:
            assert(0 && "invalid sampling mode");
            return 0;
    }
}

esp_err_t camera_probe(const camera_config_t* config, camera_model_t* out_camera_model)
{
    if(s_state != NULL){
        ESP_LOGE(TAG, "s_state != NULL");
        return ESP_ERR_INVALID_STATE;
    }

    s_state = (camera_state_t*) calloc(sizeof(*s_state), 1);
    if(!s_state){
        return ESP_ERR_NO_MEM;
    }
    ESP_LOGE(TAG, "Enabling XCLK output");
    camera_enable_out_clock(config);
    ESP_LOGE(TAG, "Initializing SSCB");
    SCCB_Init(config->pin_sscb_sda, config->pin_sscb_scl);

    ESP_LOGE(TAG, "Resetting camera");
    gpio_config_t conf = { 0 };
    conf.pin_bit_mask = 1LL << config->pin_reset;
    conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&conf);

    gpio_set_level((gpio_num_t)config->pin_reset, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    gpio_set_level((gpio_num_t)config->pin_reset, 1);
    vTaskDelay(10 / portTICK_PERIOD_MS);

    ESP_LOGE(TAG, "Searching for camera address");
    /* Probe the sensor */
    vTaskDelay(10 / portTICK_PERIOD_MS);
    uint8_t slv_addr = SCCB_Probe();
    if(slv_addr == 0){
        *out_camera_model = CAMERA_NONE;
        return ESP_ERR_CAMERA_NOT_DETECTED;
    }
    s_state->sensor.slv_addr = slv_addr;
    ESP_LOGE(TAG, "Detected camera at address=0x%02x", slv_addr);
    sensor_id_t* id = &s_state->sensor.id;
    id->PID = SCCB_Read(slv_addr, REG_PID);
    id->VER = SCCB_Read(slv_addr, REG_VER);
    id->MIDL = SCCB_Read(slv_addr, REG_MIDL);
    id->MIDH = SCCB_Read(slv_addr, REG_MIDH);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    ESP_LOGE(TAG, "Camera PID=0x%02x VER=0x%02x MIDL=0x%02x MIDH=0x%02x",
            id->PID, id->VER, id->MIDH, id->MIDL);

    switch (id->PID){
#if CONFIG_OV7725_SUPPORT
        case OV7725_PID:
            *out_camera_model = CAMERA_OV7725;
            ESP_LOGE(TAG, "init 0v7725");
            ov7725_init(&s_state->sensor);
            break;
#endif
        default:
            id->PID = 0;
            *out_camera_model = CAMERA_UNKNOWN;
            ESP_LOGE(TAG, "Detected camera not supported.");
            return ESP_ERR_CAMERA_NOT_SUPPORTED;
    }
    ESP_LOGE(TAG, "Doing SW reset of sensor");
    s_state->sensor.reset(&s_state->sensor);
    return ESP_OK;
}

esp_err_t camera_init(const camera_config_t* config)
{
    if(!s_state){
        return ESP_ERR_INVALID_STATE;
    }
    if(s_state->sensor.id.PID == 0){
        return ESP_ERR_CAMERA_NOT_SUPPORTED;
    }
    memcpy(&s_state->config, config, sizeof(*config));
    esp_err_t err = ESP_OK;
    framesize_t frame_size = (framesize_t) config->frame_size;
    pixformat_t pix_format = (pixformat_t) config->pixel_format;
    s_state->width = resolution[frame_size][0];
    s_state->height = resolution[frame_size][1];
    s_state->sensor.set_pixformat(&s_state->sensor, pix_format);

    ESP_LOGD(TAG, "Setting frame size to %dx%d", s_state->width, s_state->height);
    if(s_state->sensor.set_framesize(&s_state->sensor, frame_size) != 0){
        ESP_LOGE(TAG, "Failed to set frame size");
        err = ESP_ERR_CAMERA_FAILED_TO_SET_FRAME_SIZE;
        goto fail;
    }
    s_state->sensor.set_pixformat(&s_state->sensor, pix_format);

#if ENABLE_TEST_PATTERN
    /* Test pattern may get handy
     if you are unable to get the live image right.
     Once test pattern is enable, sensor will output
     vertical shaded bars instead of live image.
     */
    s_state->sensor.set_colorbar(&s_state->sensor, 1);
    ESP_LOGD(TAG, "Test pattern enabled");
#endif

    if(pix_format == PIXFORMAT_GRAYSCALE){
        if(s_state->sensor.id.PID != OV7725_PID){
            ESP_LOGE(TAG, "Grayscale format is only supported for ov7225");
            err = ESP_ERR_NOT_SUPPORTED;
            goto fail;
        }
        s_state->fb_size = s_state->width * s_state->height;
        if(is_hs_mode()){
            s_state->sampling_mode = SM_0A0B_0B0C;
            s_state->dma_filter = &dma_filter_grayscale_highspeed;
        } else {
            s_state->sampling_mode = SM_0A0B_0C0D;
            s_state->dma_filter = &dma_filter_grayscale;
        }
        s_state->in_bytes_per_pixel = 2;       // camera sends YUYV
        s_state->fb_bytes_per_pixel = 1;       // frame buffer stores Y8
    } else if(pix_format == PIXFORMAT_RGB565){
        if(s_state->sensor.id.PID != OV7725_PID){
            ESP_LOGE(TAG, "RGB565 format is only supported for ov7225");
            err = ESP_ERR_NOT_SUPPORTED;
            goto fail;
        }
        s_state->fb_size = s_state->width * s_state->height * 2;
        if(is_hs_mode()){
            ESP_LOGI(TAG, "s_state->sampling_mode = SM_0A0B_0B0C");
            s_state->sampling_mode = SM_0A0B_0B0C;
        } else {
            ESP_LOGI(TAG, "s_state->sampling_mode = SM_0A00_0B00");
            s_state->sampling_mode = SM_0A00_0B00;
        }
        s_state->in_bytes_per_pixel = 2;       // camera sends RGB565 (2 bytes)
        s_state->fb_bytes_per_pixel = 2;       // frame buffer stores RGB555
        s_state->dma_filter = &dma_filter_rgb565;
        ESP_LOGI(TAG, "get rgb buf");
    } else if(pix_format == PIXFORMAT_JPEG){
        if(s_state->sensor.id.PID != OV2640_PID){
            ESP_LOGE(TAG, "JPEG format is only supported for ov2640");
            err = ESP_ERR_NOT_SUPPORTED;
            goto fail;
        }
        int qp = config->jpeg_quality;
        int compression_ratio_bound;
        if(qp >= 30){
            compression_ratio_bound = 5;
        } else if(qp >= 10){
            compression_ratio_bound = 10;
        } else {
            compression_ratio_bound = 20;
        }
        (*s_state->sensor.set_quality)(&s_state->sensor, qp);
        size_t equiv_line_count = s_state->height / compression_ratio_bound;
        s_state->fb_size = s_state->width * equiv_line_count * 2 /* bpp */;
        s_state->dma_filter = &dma_filter_jpeg;
        if(is_hs_mode()){
            s_state->sampling_mode = SM_0A0B_0B0C;
        } else {
            s_state->sampling_mode = SM_0A00_0B00;
        }
        s_state->in_bytes_per_pixel = 2;
        s_state->fb_bytes_per_pixel = 2;
    } else {
        ESP_LOGE(TAG, "Requested format is not supported");
        err = ESP_ERR_NOT_SUPPORTED;
        goto fail;
    }
    ESP_LOGE(TAG, "in_bpp: %d, fb_bpp: %d, fb_size: %d, mode: %d, width: %d height: %d",
            s_state->in_bytes_per_pixel, s_state->fb_bytes_per_pixel,
            s_state->fb_size, s_state->sampling_mode,
            s_state->width, s_state->height);
    ESP_LOGE(TAG, "Allocating frame buffer (%d bytes)", s_state->fb_size);
    if(pix_format == CAMERA_PF_RGB565){
        s_state->fb = (uint8_t*) calloc(112000, 1);
        if(s_state->fb == NULL){
            ESP_LOGE(TAG, "Failed to allocate frame buffer");
            err = ESP_ERR_NO_MEM;
            goto fail;
        }
        s_state->buffer = (uint8_t*) calloc(50000,1);
        if(s_state->buffer == NULL){
            ESP_LOGE(TAG, "Failed to allocate frame buffer");
            err = ESP_ERR_NO_MEM;
            goto fail;
        }
    }else{
        s_state->fb = (uint8_t*) calloc(76800, 1);
        if(s_state->fb == NULL){
            ESP_LOGE(TAG, "Failed to allocate frame buffer");
            err = ESP_ERR_NO_MEM;
            goto fail;
        }
    }
    ESP_LOGD(TAG, "Initializing I2S and DMA");
    i2s_init();
    err = dma_desc_init();
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Failed to initialize I2S and DMA");
        goto fail;
    }

    s_state->data_ready = xQueueCreate(16, sizeof(size_t));
    s_state->frame_ready = xSemaphoreCreateBinary();
    if(s_state->data_ready == NULL || s_state->frame_ready == NULL){
        ESP_LOGE(TAG, "Failed to create semaphores");
        err = ESP_ERR_NO_MEM;
        goto fail;
    }
    if(!xTaskCreatePinnedToCore(&dma_filter_task, "dma_filter", 4096, NULL, 10, &s_state->dma_filter_task, 1)){
        ESP_LOGE(TAG, "Failed to create DMA filter task");
        err = ESP_ERR_NO_MEM;
        goto fail;
    }

    ESP_LOGD(TAG, "Initializing GPIO interrupts");
    gpio_set_intr_type((gpio_num_t)s_state->config.pin_vsync, GPIO_INTR_NEGEDGE);
    gpio_intr_enable((gpio_num_t)s_state->config.pin_vsync);
    err = gpio_isr_register(&gpio_isr, (void*) TAG,
            ESP_INTR_FLAG_INTRDISABLED | ESP_INTR_FLAG_IRAM,
            &s_state->vsync_intr_handle);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "gpio_isr_register failed (%x)", err);
        goto fail;
    }

    // skip at least one frame after changing camera settings
    while (gpio_get_level((gpio_num_t)s_state->config.pin_vsync) == 0){
        ;
    }
    while (gpio_get_level((gpio_num_t)s_state->config.pin_vsync) != 0){
        ;
    }
    while (gpio_get_level((gpio_num_t)s_state->config.pin_vsync) == 0){
        ;
    }
    s_state->frame_count = 0;
    ESP_LOGD(TAG, "Init done");
    return ESP_OK;

fail:
    free(s_state->fb);
    if(s_state->data_ready){
        vQueueDelete(s_state->data_ready);
    }
    if(s_state->frame_ready){
        vSemaphoreDelete(s_state->frame_ready);
    }
    if(s_state->dma_filter_task){
        vTaskDelete(s_state->dma_filter_task);
    }
    dma_desc_deinit();
    return err;
}

size_t camera_get_data_size()
{
    if(s_state == NULL){
        return 0;
    }
    //ESP_LOGE(TAG,"size = %d",s_state->data_size)
    return s_state->data_size;
}

uint8_t* camera_get_fb()
{
    if (s_state == NULL) {
        return NULL;
    }
    return s_state->fb;
}

int camera_get_fb_width()
{
    if (s_state == NULL) {
        return 0;
    }
    return s_state->width;
}

int camera_get_fb_height()
{
    if (s_state == NULL) {
        return 0;
    }
    return s_state->height;
}

esp_err_t camera_run(const char *pictureFilename)
{
    if(s_state == NULL){
        return ESP_ERR_INVALID_STATE;
    }
    struct timeval tv_start;
    gettimeofday(&tv_start, NULL);
#ifndef _NDEBUG
    memset(s_state->fb, 0, s_state->fb_size);
#endif // _NDEBUG
    i2s_run();
    ESP_LOGI(TAG, "Waiting for frame");
    xSemaphoreTake(s_state->frame_ready,portMAX_DELAY);
    get_bmp(pictureFilename);
    struct timeval tv_end;
    gettimeofday(&tv_end, NULL);
    int time_ms = (tv_end.tv_sec - tv_start.tv_sec) * 1000 + (tv_end.tv_usec - tv_start.tv_usec) / 1000;
    ESP_LOGI(TAG, "Frame %d done in %d ms", s_state->frame_count, time_ms);
    s_state->frame_count++;
    return ESP_OK;
}

static void get_bmp(const char *pictureFilename)
{
    bmp->fp = fopen(pictureFilename, "wb");
    ESP_LOGD(TAG, "calloc bmp");
    if(bmp == NULL){
        printf("BMP_OutputOpen(): Unable to allocate BMP struct.\n");
    }
    ESP_LOGD(TAG, "open file");
    bmp->header.BfType=19778;
    ESP_LOGD(TAG, "set BM");
    bmp->header.BfSize=54;
    bmp->header.BfReserved1=0;
    bmp->header.BfReserved2=0;
    bmp->header.BfOffBits=54;
    bmp->header.BitsSize=40;
    bmp->header.BiWidth=s_state->width;
    bmp->header.BiHighth=s_state->height;
    bmp->header.BiPlanes=1;
    bmp->header.BitCount=8*(s_state->fb_bytes_per_pixel);
    printf(TAG, "set data");
    bmp->header.BiCompression=0;
    bmp->header.BiSizeTmage=0;
    bmp->header.Bixpels=2400;
    bmp->header.Biypels=2400;
    bmp->header.BiClrUsed=0;
    bmp->header.BiClrImportant=0;
    ESP_LOGD(TAG, "set head");
    fwrite(&(bmp->header.BfType) , 1, 54 , bmp->fp);
    ESP_LOGD(TAG, "write file");
    int g,b,j=0;
    if(s_state->fb_bytes_per_pixel == 1){
        for(int i=0;i<256;i++){
            fwrite(&i , 1, 1, bmp->fp);
            fwrite(&i , 1, 1, bmp->fp);
            fwrite(&i , 1, 1, bmp->fp);
            fwrite(&j , 1, 1, bmp->fp);
        }
        bmp->header.BfSize+=1024;
        bmp->header.BiSizeTmage+=1024;
    }
    for(j=0;j<s_state->fb_size;j+=2)
    {
        if(j<112000){
            b=s_state->fb[j];
            g=s_state->fb[j+1];
        }else{
            b=s_state->buffer[j-112000];
            g=s_state->buffer[j-112000+1];
        }
        fwrite(&b , 1, 1, bmp->fp);
        fwrite(&g , 1, 1, bmp->fp);
        bmp->header.BfSize+=2;
        bmp->header.BiSizeTmage+=2;
    }
    fseek(bmp->fp,2,0);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    fwrite(&(bmp->header.BfSize) , 1, 52, bmp->fp);
    fclose(bmp->fp);
}

static esp_err_t dma_desc_init()
{
    assert(s_state->width % 4 == 0);
    size_t line_size = s_state->width * s_state->in_bytes_per_pixel *
            i2s_bytes_per_sample(s_state->sampling_mode);
    ESP_LOGD(TAG, "Line width (for DMA): %d bytes", line_size);
    size_t dma_per_line = 1;
    size_t buf_size = line_size;
    while (buf_size >= 4096){
        buf_size /= 2;
        dma_per_line *= 2;
    }
    size_t dma_desc_count = dma_per_line * 4;
    s_state->dma_buf_width = line_size;
    s_state->dma_per_line = dma_per_line;
    s_state->dma_desc_count = dma_desc_count;
    ESP_LOGD(TAG, "DMA buffer size: %d, DMA buffers per line: %d", buf_size, dma_per_line);
    ESP_LOGD(TAG, "DMA buffer count: %d", dma_desc_count);
    s_state->dma_buf = (dma_elem_t**) malloc(sizeof(dma_elem_t*) * dma_desc_count);
    if(s_state->dma_buf == NULL){
        return ESP_ERR_NO_MEM;
    }
    s_state->dma_desc = (lldesc_t*) malloc(sizeof(lldesc_t) * dma_desc_count);
    if(s_state->dma_desc == NULL){
        return ESP_ERR_NO_MEM;
    }
    size_t dma_sample_count = 0;
    for (int i = 0; i < dma_desc_count; ++i){
        ESP_LOGD(TAG, "Allocating DMA buffer #%d, size=%d", i, buf_size);
        dma_elem_t* buf = (dma_elem_t*) malloc(buf_size);
        if(buf == NULL){
            return ESP_ERR_NO_MEM;
        }
        s_state->dma_buf[i] = buf;
        ESP_LOGV(TAG, "dma_buf[%d]=%p", i, buf);
        lldesc_t* pd = &s_state->dma_desc[i];
        pd->length = buf_size;
        if(s_state->sampling_mode == SM_0A0B_0B0C &&
            (i + 1) % dma_per_line == 0){
            pd->length -= 4;
        }
        dma_sample_count += pd->length / 4;
        pd->size = pd->length;
        pd->owner = 1;
        pd->sosf = 1;
        pd->buf = (uint8_t*) buf;
        pd->offset = 0;
        pd->empty = 0;
        pd->eof = 1;
        pd->qe.stqe_next = &s_state->dma_desc[(i + 1) % dma_desc_count];
    }
    s_state->dma_done = false;
    s_state->dma_sample_count = dma_sample_count;
    return ESP_OK;
}

static void dma_desc_deinit()
{
    if(s_state->dma_buf){
        for (int i = 0; i < s_state->dma_desc_count; ++i){
            free(s_state->dma_buf[i]);
        }
    }
    free(s_state->dma_buf);
    free(s_state->dma_desc);
}

static inline void i2s_conf_reset()
{
    const uint32_t lc_conf_reset_flags = I2S_IN_RST_S | I2S_AHBM_RST_S
            | I2S_AHBM_FIFO_RST_S;
    I2S0.lc_conf.val |= lc_conf_reset_flags;
    I2S0.lc_conf.val &= ~lc_conf_reset_flags;

    const uint32_t conf_reset_flags = I2S_RX_RESET_M | I2S_RX_FIFO_RESET_M
            | I2S_TX_RESET_M | I2S_TX_FIFO_RESET_M;
    I2S0.conf.val |= conf_reset_flags;
    I2S0.conf.val &= ~conf_reset_flags;
    while (I2S0.state.rx_fifo_reset_back){
        ;
    }
}

static void i2s_init()
{
    camera_config_t* config = &s_state->config;
    gpio_num_t pins[] = {
            (gpio_num_t)config->pin_d7,
            (gpio_num_t)config->pin_d6,
            (gpio_num_t)config->pin_d5,
            (gpio_num_t)config->pin_d4,
            (gpio_num_t)config->pin_d3,
            (gpio_num_t)config->pin_d2,
            (gpio_num_t)config->pin_d1,
            (gpio_num_t)config->pin_d0,
            (gpio_num_t)config->pin_vsync,
            (gpio_num_t)config->pin_href,
            (gpio_num_t)config->pin_pclk
    };
    gpio_config_t conf;
            conf.mode = GPIO_MODE_INPUT;
            conf.pull_up_en = GPIO_PULLUP_ENABLE;
            conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            conf .intr_type = GPIO_INTR_DISABLE;
    for (int i = 0; i < sizeof(pins) / sizeof(gpio_num_t); ++i){
        conf.pin_bit_mask = 1LL << pins[i];
        gpio_config(&conf);
    }
    // Route input GPIOs to I2S peripheral using GPIO matrix
    gpio_matrix_in(config->pin_d0, I2S0I_DATA_IN0_IDX, false);
    gpio_matrix_in(config->pin_d1, I2S0I_DATA_IN1_IDX, false);
    gpio_matrix_in(config->pin_d2, I2S0I_DATA_IN2_IDX, false);
    gpio_matrix_in(config->pin_d3, I2S0I_DATA_IN3_IDX, false);
    gpio_matrix_in(config->pin_d4, I2S0I_DATA_IN4_IDX, false);
    gpio_matrix_in(config->pin_d5, I2S0I_DATA_IN5_IDX, false);
    gpio_matrix_in(config->pin_d6, I2S0I_DATA_IN6_IDX, false);
    gpio_matrix_in(config->pin_d7, I2S0I_DATA_IN7_IDX, false);
    gpio_matrix_in(config->pin_vsync, I2S0I_V_SYNC_IDX, false);
    gpio_matrix_in(0x38, I2S0I_H_SYNC_IDX, false);
    gpio_matrix_in(config->pin_href, I2S0I_H_ENABLE_IDX, false);
    gpio_matrix_in(config->pin_pclk, I2S0I_WS_IN_IDX, false);

    // Enable and configure I2S peripheral
    periph_module_enable(PERIPH_I2S0_MODULE);
    // Toggle some reset bits in LC_CONF register
    // Toggle some reset bits in CONF register
    i2s_conf_reset();
    // Enable slave mode (sampling clock is external)
    I2S0.conf.rx_slave_mod = 1;
    // Enable parallel mode
    I2S0.conf2.lcd_en = 1;
    // Use HSYNC/VSYNC/HREF to control sampling
    I2S0.conf2.camera_en = 1;
    // Configure clock divider
    I2S0.clkm_conf.clkm_div_a = 1;
    I2S0.clkm_conf.clkm_div_b = 0;
    I2S0.clkm_conf.clkm_div_num = 2;
    // FIFO will sink data to DMA
    I2S0.fifo_conf.dscr_en = 1;
    // FIFO configuration
    I2S0.fifo_conf.rx_fifo_mod = s_state->sampling_mode;
    I2S0.fifo_conf.rx_fifo_mod_force_en = 1;
    I2S0.conf_chan.rx_chan_mod = 1;
    // Clear flags which are used in I2S serial mode
    I2S0.sample_rate_conf.rx_bits_mod = 0;
    I2S0.conf.rx_right_first = 0;
    I2S0.conf.rx_msb_right = 0;
    I2S0.conf.rx_msb_shift = 0;
    I2S0.conf.rx_mono = 0;
    I2S0.conf.rx_short_sync = 0;
    I2S0.timing.val = 0;
    // Allocate I2S interrupt, keep it disabled
    esp_intr_alloc(ETS_I2S0_INTR_SOURCE,
    ESP_INTR_FLAG_INTRDISABLED | ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_IRAM,
            &i2s_isr, NULL, &s_state->i2s_intr_handle);
}

static void i2s_stop()
{
    esp_intr_disable(s_state->i2s_intr_handle);
    esp_intr_disable(s_state->vsync_intr_handle);
    i2s_conf_reset();
    I2S0.conf.rx_start = 0;
    size_t val = SIZE_MAX;
    BaseType_t higher_priority_task_woken;
    xQueueSendFromISR(s_state->data_ready, &val, &higher_priority_task_woken);
}

static void i2s_run()
{
#ifndef _NDEBUG
    for (int i = 0; i < s_state->dma_desc_count; ++i){
        lldesc_t* d = &s_state->dma_desc[i];
        ESP_LOGV(TAG, "DMA desc %2d: %u %u %u %u %u %u %p %p",
                i, d->length, d->size, d->offset, d->eof, d->sosf, d->owner, d->buf, d->qe.stqe_next);
        memset(s_state->dma_buf[i], 0, d->length);
    }
#endif

    // wait for vsync
    ESP_LOGD(TAG, "Waiting for positive edge on VSYNC");
    while (gpio_get_level((gpio_num_t)s_state->config.pin_vsync) == 0){
        ;
    }
    while (gpio_get_level((gpio_num_t)s_state->config.pin_vsync) != 0){
        ;
    }
    ESP_LOGD(TAG, "Got VSYNC");

    s_state->dma_done = false;
    s_state->dma_desc_cur = 0;
    s_state->dma_received_count = 0;
    s_state->dma_filtered_count = 0;
    esp_intr_disable(s_state->i2s_intr_handle);
    i2s_conf_reset();

    I2S0.rx_eof_num = s_state->dma_sample_count;
    I2S0.in_link.addr = (uint32_t) &s_state->dma_desc[0];
    I2S0.in_link.start = 1;
    I2S0.int_clr.val = I2S0.int_raw.val;
    I2S0.int_ena.val = 0;
    I2S0.int_ena.in_done = 1;
    esp_intr_enable(s_state->i2s_intr_handle);
    if(s_state->config.pixel_format == CAMERA_PF_JPEG){
        esp_intr_enable(s_state->vsync_intr_handle);
    }
    I2S0.conf.rx_start = 1;
}

static void IRAM_ATTR signal_dma_buf_received(bool* need_yield)
{
    size_t dma_desc_filled = s_state->dma_desc_cur;
    s_state->dma_desc_cur = (dma_desc_filled + 1) % s_state->dma_desc_count;
    s_state->dma_received_count++;
    BaseType_t higher_priority_task_woken;
    BaseType_t ret = xQueueSendFromISR(s_state->data_ready, &dma_desc_filled, &higher_priority_task_woken);
    if(ret != pdTRUE){
        ESP_LOGI(TAG, "queue send failed (%d), dma_received_count=%d", ret, s_state->dma_received_count);
    }
    *need_yield = (ret == pdTRUE && higher_priority_task_woken == pdTRUE);
}

static void IRAM_ATTR i2s_isr(void* arg)
{
    I2S0.int_clr.val = I2S0.int_raw.val;
    bool need_yield;
    signal_dma_buf_received(&need_yield);
    ESP_EARLY_LOGV(TAG, "isr, cnt=%d", s_state->dma_received_count);
    if(s_state->dma_received_count == s_state->height * s_state->dma_per_line){
        i2s_stop();
    }
    if(need_yield){
        portYIELD_FROM_ISR();
    }
}

static void IRAM_ATTR gpio_isr(void* arg)
{
    GPIO.status1_w1tc.val = GPIO.status1.val;
    GPIO.status_w1tc = GPIO.status;
    bool need_yield = false;
    ESP_EARLY_LOGV(TAG, "gpio isr, cnt=%d", s_state->dma_received_count);
    if(gpio_get_level((gpio_num_t)s_state->config.pin_vsync) == 0 &&
            s_state->dma_received_count > 0 &&
            !s_state->dma_done){
        signal_dma_buf_received(&need_yield);
        i2s_stop();
    }
    if(need_yield){
        portYIELD_FROM_ISR();
    }
}

static size_t get_fb_pos()
{
    return s_state->dma_filtered_count * s_state->width *
            s_state->fb_bytes_per_pixel / s_state->dma_per_line;
}

static void IRAM_ATTR dma_filter_task(void *pvParameters)
{
    while (true){
        size_t buf_idx;
        xQueueReceive(s_state->data_ready, &buf_idx, portMAX_DELAY);
        if(buf_idx == SIZE_MAX){
            s_state->data_size = get_fb_pos();
            xSemaphoreGive(s_state->frame_ready);
            continue;
        }
        uint8_t* pfb;
        
        if(get_fb_pos()<112000){
            pfb = s_state->fb + get_fb_pos();
        }else{
            pfb = s_state->buffer + get_fb_pos() - 112000;
        }
        const dma_elem_t* buf = s_state->dma_buf[buf_idx];
        lldesc_t* desc = &s_state->dma_desc[buf_idx];
        ESP_LOGV(TAG, "dma_flt: pos=%d ", get_fb_pos());
        (*s_state->dma_filter)(buf, desc, pfb);
        s_state->dma_filtered_count++;
        ESP_LOGV(TAG, "dma_flt: flt_count=%d ", s_state->dma_filtered_count);
    }
}

static void IRAM_ATTR dma_filter_grayscale(const dma_elem_t* src, lldesc_t* dma_desc, uint8_t* dst)
{
    assert(s_state->sampling_mode == SM_0A0B_0C0D);
    size_t end = dma_desc->length / sizeof(dma_elem_t) / 4;
    for (size_t i = 0; i < end; ++i){
        dst[0] = src[0].sample1;
        dst[1] = src[1].sample1;
        dst[2] = src[2].sample1;
        dst[3] = src[3].sample1;
        src += 4;
        dst += 4;
    }
}

static void IRAM_ATTR dma_filter_grayscale_highspeed(const dma_elem_t* src, lldesc_t* dma_desc, uint8_t* dst)
{
     ESP_LOGI(TAG, "grayscale");
    assert(s_state->sampling_mode == SM_0A0B_0B0C);
    size_t end = dma_desc->length / sizeof(dma_elem_t) / 8;
    for (size_t i = 0; i < end; ++i){
        dst[0] = src[0].sample1;
        dst[1] = src[2].sample1;
        dst[2] = src[4].sample1;
        dst[3] = src[6].sample1;
        src += 8;
        dst += 4;
    }
    if((dma_desc->length & 0x7) != 0){
        dst[0] = src[0].sample1;
        dst[1] = src[2].sample1;
    }
}

static void IRAM_ATTR dma_filter_jpeg(const dma_elem_t* src, lldesc_t* dma_desc, uint8_t* dst)
{
    ESP_LOGI(TAG, "jpeg");
    assert(s_state->sampling_mode == SM_0A0B_0B0C ||
           s_state->sampling_mode == SM_0A00_0B00 );
    size_t end = dma_desc->length / sizeof(dma_elem_t) / 4;
    for (size_t i = 0; i < end; ++i){
        dst[0] = src[0].sample1;
        dst[1] = src[1].sample1;
        dst[2] = src[2].sample1;
        dst[3] = src[3].sample1;
        src += 4;
        dst += 4;
    }
    if((dma_desc->length & 0x7) != 0){
        dst[0] = src[0].sample1;
        dst[1] = src[1].sample1;
        dst[2] = src[2].sample1;
        dst[3] = src[2].sample2;
    }
}

static inline void rgb565_to_555(uint8_t in1, uint8_t in2, uint8_t* dst)
{
    dst[0]=((in2&0x1f)|(in1&0x01)<<7|(in2&0xC0)>>1);
    dst[1]=in1>>1;
}

static void IRAM_ATTR dma_filter_rgb565(const dma_elem_t* src, lldesc_t* dma_desc, uint8_t* dst)
{
    assert(s_state->sampling_mode == SM_0A0B_0B0C ||
           s_state->sampling_mode == SM_0A00_0B00);
    const int unroll = 2;         
    const int samples_per_pixel = 2;
    const int bytes_per_pixel = 2;
    size_t end = dma_desc->length / sizeof(dma_elem_t) / unroll / samples_per_pixel;
    for (size_t i = 0; i < end; ++i){
        rgb565_to_555(src[0].sample1, src[1].sample1, &dst[0]);
        rgb565_to_555(src[2].sample1, src[3].sample1, &dst[2]);
        dst += bytes_per_pixel * unroll;
        src += samples_per_pixel * unroll;
    }
    if((dma_desc->length & 0x7) != 0){
        rgb565_to_555(src[0].sample1, src[1].sample1, &dst[0]);
        rgb565_to_555(src[2].sample1, src[2].sample2, &dst[2]);
    }
}

void cameramode(uint8_t photoSize,uint8_t pixelFormat)
{
    camera_config_t camera_config;
        camera_config.pin_d0 = 17;
        camera_config.pin_d1 = 5;
        camera_config.pin_d2 = 18;
        camera_config.pin_d3 = 19;
        camera_config.pin_d4 = 36;
        camera_config.pin_d5 = 39;
        camera_config.pin_d6 = 34;
        camera_config.pin_d7 = 35;
        camera_config.pin_xclk = 21;
        camera_config.pin_pclk = 22;
        camera_config.pin_vsync = 25;
        camera_config.pin_href = 23;
        camera_config.pin_sscb_sda = 26;
        camera_config.pin_sscb_scl = 27;
        camera_config.pin_reset = 0;
        camera_config .xclk_freq_hz = 20000000;
        camera_config.ledc_timer = LEDC_TIMER_0;
        camera_config.ledc_channel = LEDC_CHANNEL_0;
        camera_model_t camera_model;
    esp_err_t err = camera_probe(&camera_config, &camera_model);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Camera probe failed with error 0x%x", err);
        return;
    }
    if(camera_model == CAMERA_OV7725){
            ESP_LOGE(TAG, "Detected OV7725 camera");
            s_pixel_format = (camera_pixelformat_t)pixelFormat;
            camera_config.frame_size = (camera_framesize_t)photoSize;
    }else{
        ESP_LOGE(TAG, "Camera not supported");
        return;
    }
    camera_config.pixel_format = s_pixel_format;
    err = camera_init(&camera_config);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return;
    }
    ESP_LOGE(TAG,"Camera Ready");
}

void connectnet(const char* ssid,const char* password)
{
    initialise_wifi( ssid , password );
    delay(2000);
    ESP_LOGE(TAG, "Free heap: %u", xPortGetFreeHeapSize());
    ESP_LOGE(TAG, "Camera demo ready");
    ESP_LOGE(TAG, "open http://" IPSTR "/get for single frame", IP2STR(&s_ip_addr));
    if(s_pixel_format == CAMERA_PF_GRAYSCALE){
        ESP_LOGE(TAG, "open http://" IPSTR "/pgm for a single image/x-portable-graymap image", IP2STR(&s_ip_addr));
    }
    if(s_pixel_format == CAMERA_PF_JPEG){
        ESP_LOGE(TAG, "open http://" IPSTR "/stream for multipart/x-mixed-replace stream (use with JPEGs)", IP2STR(&s_ip_addr));
    }
}

void sendtonet(void)
{
    xTaskCreate(&http_server, "http_server", 2048, NULL, 5, NULL);
}