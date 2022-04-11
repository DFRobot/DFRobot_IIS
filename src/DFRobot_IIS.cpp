/*!
 * @file DFRobot_IIS.cpp
 * @brief Basic struct of DFRobot_IIS class
 * @details This library provides the codes for driving FireBeetle camera and the camera, microphones, etc. of audio expansion board.
 * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT license (MIT)
 * @author [fengli](li.feng@dfrobot.com)
 * @version  V1.0.1
 * @date  2022-03-21
 * @https://github.com/DFRobot/DFRobot_IIS
*/
 
#include "DFRobot_IIS.h"
#include "vfs_api.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "string.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_err.h"
#include "esp_log.h"

#include "SD_MMC.h"

char     filename[100];
char     pictureFilename[30];
char     outputFilename[30];
uint8_t  mark=STOP;
uint8_t  rmark=STOP;
uint8_t  Volume1=0;
uint8_t  Volume2=0;
uint8_t  Size=8;
uint8_t  Pixel=0;
uint8_t  set=0;
static xTaskHandle  xPlayWAV     = NULL;
static xTaskHandle  xRecordSound = NULL;

bool DFRobot_IIS::SDCardInit(void)
{
    int ret=SDcard_Init();
    if(ret==0){
        return false;
    }
    return true;
}

bool DFRobot_IIS::init(uint8_t mode)
{
    if(mode==AUDIO){
        mark=STOP;
        rmark=STOP;
        return true;
    }else if(mode==CAMERA){
        i2cMasterInit();
        i2cWriteNAU8822(0,  0x000);
        i2cWriteNAU8822(52, 0x040);
        i2cWriteNAU8822(53, 0x040);
        i2cWriteNAU8822(54, 0x040);
        i2cWriteNAU8822(55, 0x040);
        i2c_driver_delete(I2C_MASTER_NUM);
        return true;
    }else{
        printf("No such mode ");
        return false;
    }
}

void DFRobot_IIS::setSpeakersVolume(uint8_t volume)
{
    if(mark == PLAY){
        mark=SET;
    }
    if(volume>99){
        Volume2=99;
    }
    if(volume<1){
        Volume2=0;
    }
    Volume1=(volume*64/100);
    if(mark != PAUSE){
        i2cMasterInit();
        i2cWriteNAU8822(54, Volume1);
        i2cWriteNAU8822(55, Volume1+256);
        i2c_driver_delete(I2C_MASTER_NUM);
    }
    if(mark == SET){
        mark=PLAY;
    }
}

void DFRobot_IIS::muteSpeakers(void)
{
    mark=SET;
    i2cMasterInit();
    i2cWriteNAU8822(54, 0x140);
    i2cWriteNAU8822(55, 0x140);
    i2c_driver_delete(I2C_MASTER_NUM);
    mark=PLAY;
}

void DFRobot_IIS::setHeadphonesVolume(uint8_t volume)
{
    if(mark == PLAY){
        mark=SET;
    }
    if(volume>99){
        Volume2=99;
    }
    if(volume<1){
        Volume2=0;
    }
    Volume2=(volume*64/100);
    if(mark != PAUSE){
        i2cMasterInit();
        i2cWriteNAU8822(52, Volume2);
        i2cWriteNAU8822(53, Volume2+256);
        i2c_driver_delete(I2C_MASTER_NUM);
    }
    if(mark == SET){
        mark=PLAY;
    }
}

void DFRobot_IIS::muteHeadphones(void)
{
    i2cMasterInit();
    i2cWriteNAU8822(52, 0x140);
    i2cWriteNAU8822(53, 0x140);
    i2c_driver_delete(I2C_MASTER_NUM);
}

void playWAV(void *arg)
{
    while(1){
        while(mark==STOP){
            vTaskDelay(100);
        }
        i2cSetupNAU8822Play();
        handleWav wav = (handleWav)calloc(1, sizeof(sWav_t));
        if(wav == NULL){
            printf("playWAV(): Unable to allocate WAV struct.\n");
            break;
        }
        vTaskDelay(100);
        wav->fp = fopen(filename, "rb");
        if(wav->fp == NULL){
            printf("playWAV(): Unable to open wav file. %s\n", filename);
            break;
        }
        if(fread(&(wav->header.riffType), 1, 4, wav->fp) != 4){
            printf("playWAV(): couldn't read RIFF_ID\n");
            break;  /* bad error "couldn't read RIFF_ID" */
        }
        if(strncmp("RIFF", wav->header.riffType, 4)){
            printf("playWAV(): RIFF descriptor not found.\n") ;
            break;
        }
        fread(&(wav->header.riffSize), 4, 1, wav->fp);
        if(fread(&wav->header.waveType, 1, 4, wav->fp) !=4){
            printf("playWAV(): couldn't read format\n");
            break;  /* bad error "couldn't read format" */
        }
        if(strncmp("WAVE", wav->header.waveType, 4)){
            printf("playWAV(): WAVE chunk ID not found.\n") ;
            break;
        }
        if(fread(&(wav->header.formatType), 1, 4, wav->fp) != 4){
            printf("playWAV(): couldn't read format_ID\n");
            break;  /* bad error "couldn't read format_ID" */
        }
        if(strncmp("fmt", wav->header.formatType, 3)){
            printf("playWAV(): fmt chunk format not found.\n") ;
            break;
        }
        fread(&(wav->header.formatSize), 4, 1, wav->fp);
        fread(&(wav->header.compressionCode), 2, 1, wav->fp);
        fread(&(wav->header.numChannels), 2, 1, wav->fp);
        fread(&(wav->header.sampleRate), 4, 1, wav->fp);
        fread(&(wav->header.bytesPerSecond), 4, 1, wav->fp);
        fread(&(wav->header.blockAlign), 2, 1, wav->fp);
        fread(&(wav->header.bitsPerSample), 2, 1, wav->fp);
        while(1){
            if(fread(&wav->header.dataType1, 1, 1, wav->fp) != 1){
                printf("playWAV(): Unable to read data chunk ID.\n");
                free(wav);
                break;
            }
            if(strncmp("d", wav->header.dataType1, 1) == 0){
                fread(&wav->header.dataType2, 3, 1, wav->fp);
                if(strncmp("ata", wav->header.dataType2, 3) == 0){
                    fread(&(wav->header.dataSize),4,1,wav->fp);
                    break;
                }
            }
        }
        i2sMcklkInit(wav->header.sampleRate);
        i2sMasterInit(wav->header.sampleRate ,wav->header.bitsPerSample);
        i2s_set_sample_rates(I2S_NUM_0, wav->header.sampleRate);
        while(fread(&wav->header.test, 1 , 800 , wav->fp)){
            char *buf=(char *)&wav->header.test;
            int bytes_left=800,bytes_written = 0;
            while(bytes_left > 0){
              //  bytes_written = i2s_write(I2S_NUM_0 , buf ,((bits+8)/16)*SAMPLE_PER_CYCLE*4, bytes_left , 0);
				//i2s_write(I2S_NUM, samples_data, ((bits+8)/16)*SAMPLE_PER_CYCLE*4, &i2s_bytes_write, 100);
                bytes_left -= bytes_written;
                buf += bytes_written;
                if(mark==PAUSE){
                    i2cMasterInit();
                    i2cWriteNAU8822(52, 0x140);
                    i2cWriteNAU8822(53, 0x140);
                    i2cWriteNAU8822(54, 0x140);
                    i2cWriteNAU8822(55, 0x140);
                    i2c_driver_delete(I2C_MASTER_NUM);
                    vTaskDelay(500);
                    while(mark==PAUSE){
                        vTaskDelay(100);
                    }
                    vTaskDelay(100);
                    i2cMasterInit();
                    i2cWriteNAU8822(52, Volume2);
                    i2cWriteNAU8822(53, Volume2+256);
                    i2cWriteNAU8822(54, Volume1);
                    i2cWriteNAU8822(55, Volume1+256);
                    i2c_driver_delete(I2C_MASTER_NUM);
                }
                while(mark==SET){
                    vTaskDelay(10);
                }
            }
            if(mark==STOP){
                i2cMasterInit();
                i2cWriteNAU8822(52, 0x140);
                i2cWriteNAU8822(53, 0x140);
                i2cWriteNAU8822(54, 0x140);
                i2cWriteNAU8822(55, 0x140);
                i2c_driver_delete(I2C_MASTER_NUM);
                break;
            }
        }
        i2s_stop(I2S_NUM_0);
        i2s_driver_uninstall(I2S_NUM_0);
        fclose(wav->fp);
        free(wav);
        mark=STOP;
    }
    vTaskDelete(xPlayWAV);
}

void DFRobot_IIS::initPlayer(){
    mark=STOP;
    xTaskCreate(&playWAV, "playWAV",2048, NULL, 5, &xPlayWAV);
}

void DFRobot_IIS::playerControl(uint8_t cmd)
{
    if(cmd == 1){
        rmark=STOP;
        vTaskDelay(500);
    }
    mark=cmd;
}

void DFRobot_IIS::playMusic(const char *Filename)
{
    char SDfilename[30]="/sdcard";
    strcat(SDfilename,Filename);
    strcpy(filename,SDfilename);
}

void recordSound(void *arg)
{
    while(1){
        handleWav wav = (handleWav)calloc(1, sizeof( sWav_t));
        while(rmark==STOP){
            vTaskDelay(100);
        }
        i2cSetupNAU8822Record();
        unsigned int size = 0;
        if(wav == NULL){
            printf("recordSound(): Unable to allocate WAV struct.\n");
            break;
        }
        wav->fp = fopen(outputFilename, "wb");
        if(wav->fp == NULL){
            printf("recordSound(): unable to create file %s\n", outputFilename);
            break;
        }
        strcpy(wav->header.riffType, "RIFF");
        wav->header.riffSize = 0;  
        strcpy(wav->header.waveType, "WAVE");
        strcpy(wav->header.formatType, "fmt ");
        wav->header.formatSize      = 0x00000010;
        fwrite(&wav->header.riffType, 1, 20 , wav->fp);
        wav->header.compressionCode = 1;
        fwrite(&wav->header.compressionCode, 1 , 2 , wav->fp);
        wav->header.numChannels     = I2S_CHANNEL_STEREO;
        fwrite(&wav->header.numChannels, 1, 2, wav->fp);
        wav->header.sampleRate      = 32000;
        fwrite(&wav->header.sampleRate, 1, 4 , wav->fp);
        wav->header.blockAlign      = (short)(I2S_CHANNEL_STEREO *(I2S_BITS_PER_SAMPLE_16BIT >> 3));
        wav->header.bytesPerSecond  = (32000)*(wav->header.blockAlign);
        fwrite(&wav->header.bytesPerSecond, 1, 4 , wav->fp);
        fwrite(&wav->header.blockAlign, 1, 2 , wav->fp);
        wav->header.bitsPerSample   = I2S_BITS_PER_SAMPLE_16BIT;
        fwrite(&wav->header.bitsPerSample, 1, 2 , wav->fp);
        strcpy(wav->header.dataType1, "d");
        strcpy(wav->header.dataType2, "ata");
        wav->header.dataSize        = 0;
        fwrite(&wav->header.dataType1 ,1,8, wav->fp);
        int bytes_written = 0;
        char *buf=(char *)&wav->header.test;
        i2sMcklkInit(32000);
        i2sSlaveInit(32000,I2S_BITS_PER_SAMPLE_16BIT);
        vTaskDelay(1000);
        while(rmark!=STOP){
           // bytes_written = i2s_read_bytes(I2S_NUM_0 ,buf, 800 , 100);
            wav->header.dataSize+=fwrite(buf, 1, bytes_written , wav->fp);
        }
        wav->header.riffSize =wav->header.dataSize+44;
        fseek(wav->fp,4,0);
        fwrite(&wav->header.riffSize, 1,4, wav->fp);
        fseek(wav->fp,40,0);
        fwrite(&wav->header.dataSize, 1,4, wav->fp);
        fclose(wav->fp);
        i2s_stop(I2S_NUM_0);
        i2s_driver_uninstall(I2S_NUM_0);
        vTaskDelay(1000);
    }
    vTaskDelete(xRecordSound);
}

void DFRobot_IIS::initRecorder()
{
    rmark=STOP;
    xTaskCreate(&recordSound, "recordSound",2048, NULL, 5, &xRecordSound);
}

void DFRobot_IIS::recorderControl(uint8_t cmd)
{
    if(cmd == 1){
        mark=STOP;
        vTaskDelay(500);
    }
    rmark=cmd;
}

void DFRobot_IIS::record(const char *Filename)
{
    char SDfilename[30]="/sdcard";
    strcat(SDfilename,Filename);
    strcpy(outputFilename,SDfilename);
}

uint8_t DFRobot_IIS::setFreamsize(uint8_t photoSize)
{
    Size = photoSize;
    if(set){
        cameramode(Size,Pixel);
    }else{
        set=1;
    }
    return Size;
}

uint8_t DFRobot_IIS::setPixformat(uint8_t pixelFormat)
{
    Pixel = pixelFormat;
    if(set){
        cameramode(Size,Pixel);
    }else{
        set=1;
    }
    return Pixel;
}

void DFRobot_IIS::connectNet(const char* ssid,const char* password)
{
    connectnet( ssid , password );
}

void DFRobot_IIS::sendPhoto(void)
{
    sendtonet();
}

void DFRobot_IIS::snapshot(const char *Filename)
{
    char SDfilename[30]="/sdcard";
    //Serial.println("1");
    strcat(SDfilename,Filename);
    //Serial.println("2");
    strcpy(pictureFilename,SDfilename);
    //Serial.println("3");
    camera_run(pictureFilename);
}

void i2cMasterInit()
{
    i2c_port_t i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode             = I2C_MODE_MASTER;
    conf.sda_io_num       = (gpio_num_t)26;
    conf.sda_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.scl_io_num       = (gpio_num_t)27;
    conf.scl_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ ;
    //Serial.println( conf.clk_flags);
    conf.clk_flags = 0;
    i2c_param_config(i2c_master_port, &conf);
    
    i2c_driver_install(i2c_master_port, conf.mode,I2C_MASTER_RX_BUF_DISABLE,I2C_MASTER_TX_BUF_DISABLE, 0);
    //Serial.println("down");
}

void i2cWriteNAU8822(int8_t addr, int16_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte( cmd ,(ESP_SLAVE_ADDR) | WRITE_BIT           ,ACK_CHECK_EN);
    i2c_master_write_byte( cmd ,((addr<<1) |(data>>8))| WRITE_BIT      ,ACK_CHECK_EN);
    i2c_master_write_byte( cmd ,((int8_t) (data & 0x00ff) )| WRITE_BIT ,ACK_CHECK_EN);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 100);
    i2c_cmd_link_delete(cmd);
}

void i2cSetupNAU8822Play()
{
    i2cMasterInit();
    i2cWriteNAU8822(0,  0x000);
    vTaskDelay(10);
    i2cWriteNAU8822(1,  0x1FF);
    i2cWriteNAU8822(2,  0x1BF);
    i2cWriteNAU8822(3,  0x1FF);
    i2cWriteNAU8822(4,  0x010);
    i2cWriteNAU8822(5,  0x000);
    i2cWriteNAU8822(6,  0x00C);
    i2cWriteNAU8822(7,  0x000);
    i2cWriteNAU8822(13, 0x09f);
    i2cWriteNAU8822(10, 0x008);
    i2cWriteNAU8822(14, 0x108);
    i2cWriteNAU8822(15, 0x0FF);
    i2cWriteNAU8822(16, 0x1FF);
    i2cWriteNAU8822(45, 0x0bf);
    i2cWriteNAU8822(46, 0x1bf);
    i2cWriteNAU8822(47, 0x175);
    i2cWriteNAU8822(49, 0x042);
    i2cWriteNAU8822(50, 0x1DD);
    i2cWriteNAU8822(51, 0x001);
    if(Volume2 != 0){
        i2cWriteNAU8822(52, Volume2);
        i2cWriteNAU8822(53, Volume2+256);
    }else{
        i2cWriteNAU8822(52, 0x040);
        i2cWriteNAU8822(53, 0x040);
    }
    if(Volume1 != 0){
        i2cWriteNAU8822(54, Volume1);
        i2cWriteNAU8822(55, Volume1+256);
    }else{
        i2cWriteNAU8822(54, 0x040);
        i2cWriteNAU8822(55, 0x040);
    }
    i2c_driver_delete(I2C_MASTER_NUM);
}

void i2cSetupNAU8822Record()
{
    i2cMasterInit();
    i2cWriteNAU8822(0,  0x000);
    vTaskDelay(10);
    i2cWriteNAU8822(1,  0x1FF);
    i2cWriteNAU8822(2,  0x1BF);
    i2cWriteNAU8822(3,  0x1FF);
    i2cWriteNAU8822(4,  0x010);
    i2cWriteNAU8822(5,  0x000);
    i2cWriteNAU8822(6,  0x00D);
    i2cWriteNAU8822(7,  0x007);
    i2cWriteNAU8822(9 , 0x150);
    i2cWriteNAU8822(10, 0x008);
    i2cWriteNAU8822(14, 0x108);
    i2cWriteNAU8822(15, 0x0FF);
    i2cWriteNAU8822(16, 0x1FF);
    i2cWriteNAU8822(44, 0x033);
    i2cWriteNAU8822(45, 0x0bf);
    i2cWriteNAU8822(46, 0x1bf);
    i2cWriteNAU8822(47, 0x075);
    i2cWriteNAU8822(48, 0x075);
    i2cWriteNAU8822(50, 0x001);
    i2cWriteNAU8822(51, 0x001);
    i2cWriteNAU8822(52, 0x040);
    i2cWriteNAU8822(53, 0x040);
    i2cWriteNAU8822(54, 0x040);
    i2cWriteNAU8822(55, 0x040);
    i2cWriteNAU8822(74, 0x100);
    i2c_driver_delete(I2C_MASTER_NUM);
}

void i2sMcklkInit(unsigned int sampleRate)
{
    periph_module_enable(PERIPH_LEDC_MODULE);
    ledc_timer_bit_t bit_num = (ledc_timer_bit_t) 2;
    int duty                 = pow(2, (int) bit_num) / 2;
    ledc_timer_config_t timer_conf;
    timer_conf.bit_num       = bit_num;
    timer_conf.freq_hz       = sampleRate*256; 
    timer_conf.speed_mode    = LEDC_HIGH_SPEED_MODE;
    timer_conf.timer_num     = LEDC_TIMER_0;
    ledc_timer_config(&timer_conf);
    ledc_channel_config_t ch_conf;
    ch_conf.channel          = LEDC_CHANNEL_1;
    ch_conf.timer_sel        = LEDC_TIMER_0;
    ch_conf.intr_type        = LEDC_INTR_DISABLE; 
    ch_conf.duty             = duty;
    ch_conf.speed_mode       = LEDC_HIGH_SPEED_MODE;
    ch_conf.gpio_num         = I2S_MCLK ;  
    ledc_channel_config(&ch_conf);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1); 
}

void i2sMasterInit(uint32_t sampleRate,i2s_bits_per_sample_t bitsPerSample)
{
    i2s_config_t i2s_config  = {
        .mode                = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX) ,
        .sample_rate         = sampleRate,
        .bits_per_sample     = bitsPerSample,
        .channel_format      = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format= I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags    = ESP_INTR_FLAG_LEVEL1
    };
    i2s_config.dma_buf_count = 5;
    i2s_config.dma_buf_len   = 100;
    i2s_pin_config_t pin_config = {
        .bck_io_num   = 5,
        .ws_io_num    = 17,
        .data_out_num = 0,
        .data_in_num  = 39
    };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
}  

void i2sSlaveInit(uint32_t SAMPLE_RATE,i2s_bits_per_sample_t bitsPerSample)
{
    i2s_config_t i2s_config  = {
        .mode                = i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX),
        .sample_rate         = SAMPLE_RATE, 
        .bits_per_sample     = bitsPerSample,
        .channel_format      = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format= I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags    = ESP_INTR_FLAG_LEVEL1
    };
    i2s_config.dma_buf_count = 5;
    i2s_config.dma_buf_len   = 100;
    i2s_pin_config_t pin_config = {
        .bck_io_num   = 5,
        .ws_io_num    = 17,
        .data_out_num = 0,
        .data_in_num  = 39
    };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
}

bool DFRobot_IIS::SDcard_Init(const char* mountpoint)
{
    if(!SD_MMC.begin()){
        Serial.println("Card Mount Failed");
        return false;
    }
    return true;
}

unsigned int   littleEndian32(unsigned int v){
    return ((v & 0x000000FF) << 24 | (v & 0x0000FF00) <<  8 | (v & 0x00FF0000) >>  8 | (v & 0xFF000000) >> 24);
}

unsigned short littleEndian16(short v){
    return (short)(((v << 8) & 0xFF00) | ((v >> 8) & 0x00FF));
}
