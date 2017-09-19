/*!
 * @file DFRobot_IIS.h
 * @brief DFRobot's IIS Module
 * @n IIS Module for playMusic、recordSound and takephoto
 *
 * @copyright    [DFRobot](http://www.dfrobot.com), 2017
 * @copyright    GNU Lesser General Public License
 *
 * @author [Zhangjiawei<jiawei.zhang@dfrobot.com>]
 * @version  V1.0
 * @date  2017-8-1
 */

#ifndef _DFROBOT_IIS_H_
#define _DFROBOT_IIS_H_

#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string.h>
#include "rom/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "driver/ledc.h"
#include "driver/sdmmc_types.h"
#include "sdmmc_cmd.h"
#include "vfs_api.h"
#include "FS.h"

extern "C" {
#include "driver/i2c.h"
#include "driver/i2s.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
}

#define I2S_MCLK                    22
#define WRITE_BIT                   I2C_MASTER_WRITE
#define READ_BIT                    I2C_MASTER_READ
#define ESP_SLAVE_ADDR              (0X34) 
#define ACK_CHECK_EN                0x1
#define ACK_CHECK_DIS               0x0
#define I2C_MASTER_NUM              I2C_NUM_1
#define I2C_MASTER_SCL_IO           GPIO_NUM_27
#define I2C_MASTER_SDA_IO           GPIO_NUM_26
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
#define I2C_MASTER_FREQ_HZ          600

#define AUDIO      0
#define CAMERA     1

#define PLAY       1
#define BEGIN      1
#define PAUSE      2
#define STOP       3
#define SET        4

typedef struct WAV_HEADER
{
  char                  riffType[4];
  unsigned int          riffSize;
  char                  waveType[4];
  char                  formatType[4];
  unsigned int          formatSize;
  uint16_t              compressionCode;
  i2s_channel_t         numChannels;
  uint32_t              sampleRate;
  unsigned int          bytesPerSecond;
  unsigned short        blockAlign;
  i2s_bits_per_sample_t bitsPerSample;
  char                  dataType1[1];
  char                  dataType2[3];
  unsigned int          dataSize;
  char                  test[800];
}WAV_HEADER;

struct WAV
{
  WAV_HEADER header;
  FILE *fp;
};

typedef struct WAV *HANDLE_WAV;

    void I2S_MCLK_Init(unsigned int SAMPLE_RATE);
    void I2S_Master_Init(uint32_t SAMPLE_RATE,i2s_bits_per_sample_t BITS_PER_SAMPLE);
    void I2S_Slave_Init(uint32_t SAMPLE_RATE,i2s_bits_per_sample_t BITS_PER_SAMPLE);
    void I2C_Master_Init();
    void I2C_WriteNAU8822(int8_t addr,int16_t data);
    void I2C_Setup_NAU8822_play();
    void I2C_Setup_NAU8822_record();
    unsigned int LittleEndian32(unsigned int v);
    unsigned short LittleEndian16(short v); 

class DFRobot_IIS
{
public:
    bool init(uint8_t mode);
    bool sdCardInit(void);
    void setSpeakersVolume(uint8_t volume);
    void muteSpeakers(void);
    void setHeadphonesVolume(uint8_t volume);
    void muteHeadphones(void);
    void takePhoto(const char *pictureFilename); 
    void initPlayer();
    void initRecorder();
    void playMusic(const char *Filename);
    void record(const char *Filename);
    void playerControl(uint8_t cmd);  
    void recorderControl(uint8_t cmd);
    bool SDcard_Init(const char* mountpoint="/sdcard");
};
#endif
