/*!
 * @file DFRobot_IIS.h
 * @brief Basic struct of DFRobot_IIS class
 * @details This library provides the codes for driving FireBeetle camera and the camera, microphones, etc. of audio expansion board.
 * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT license (MIT)
 * @author [fengli](li.feng@dfrobot.com)
 * @version  V1.0.1
 * @date  2022-03-18
 * @https://github.com/DFRobot/DFRobot_IIS
*/

#ifndef _DFROBOT_IIS_H_
#define _DFROBOT_IIS_H_

#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string.h>
#include "esp32/rom/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "driver/ledc.h"
#include "driver/sdmmc_types.h"
#include "sdmmc_cmd.h"
#include "vfs_api.h"
#include "FS.h"
#include "src/camera.h"

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

#define I2S_MCLK                    22 ///< Clock pin for I2S device
#define WRITE_BIT                   I2C_MASTER_WRITE ///< I2C master write enable bit
#define READ_BIT                    I2C_MASTER_READ ///< I2C master read enable bit
#define ESP_SLAVE_ADDR              (0X34) ///<ESP slave address 
#define ACK_CHECK_EN                0x1 ///<Enable ACK
#define ACK_CHECK_DIS               0x0 ///<Disable ACK
#define I2C_MASTER_NUM              0  ///<The serial number of the I2C device to be used
#define I2C_MASTER_SCL_IO           GPIO_NUM_27 ///<The I2C SCL pin used
#define I2C_MASTER_SDA_IO           GPIO_NUM_26 ///<The I2C SDA pin used
#define I2C_MASTER_TX_BUF_DISABLE   0         ///<I2C master use transmitting data buffer
#define I2C_MASTER_RX_BUF_DISABLE   0         ///<I2C master use receiving data buffer
#define I2C_MASTER_FREQ_HZ          100000    ///<I2C master use the frequency of 100k

#define AUDIO      0
#define CAMERA     1

#define PLAY       1 ///<Start play
#define BEGIN      1 ///<Start play
#define PAUSE      2 ///<Pause play
#define STOP       3 ///<Stop play
#define SET        4 
///<Pixel
#define QQVGA     4  ///<160x120
#define QQVGA2    5  ///<128x160
#define QICF      6  
#define HQVGA     7  ///<240x160
#define QVGA      8  ///<320x240
#define RGB555    0  ///<RGB565 true color
#define GRAYSCALE 2  ///Grayscale image

/**
 * @struct sWavHeader_t
 * @brief Definition of byte segments of WAV file header
 */
typedef struct 
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
}sWavHeader_t;

/**
 * @struct sWav_t
 * @brief Save wav file
 */
typedef struct 
{
    sWavHeader_t header;
    FILE *fp;
}sWav_t;



class DFRobot_IIS
{
public:


  /**
   * @fn init
   * @brief Init function
   * @details Initialize I2S mode, I2S can be used as audio and camera controller in esp32
   * @param mode I2S mode, AUDIO or CAMERA
   * @return Data of bool type
   * @retval true Init succeeded
   * @retval false Init failed
   */
  bool init(uint8_t mode);

  /**
   * @fn SDCardInit
   * @brief Mount SD card
   * @return Data of bool type
   * @retval true Init succeeded
   * @retval false Init failed
   */
  bool SDCardInit(void);

  /**
   * @fn sendPhoto
   * @brief Send the photo obtained from the camera on the network
   */
  void sendPhoto(void);

  /**
   * @fn setSpeakersVolume
   * @brief Set speakers volume
   * @param volume Volume, range 0-99
   */
  void setSpeakersVolume(uint8_t volume);
  
  /**
   * @fn muteSpeakers
   * @brief Mute speakers
   */
  void muteSpeakers(void);

  /**
   * @fn setHeadphonesVolume
   * @brief Set headphones volume
   * @param volume Volume, range 0-99
   */
  void setHeadphonesVolume(uint8_t volume);
  
  /**
   * @fn muteHeadphones
   * @brief Mute speakers
   */
  void muteHeadphones(void);
  
  /**
   * @fn setFreamsize
   * @brief Set camera pixel
   * @param photoSize  optional(QQVGA,QQVGA2,QICF,HQVGA,QVGA,RGB555,GRAYSCALE)
   * @return Return the set pixel
   */
  uint8_t setFreamsize(uint8_t photoSize);

  /**
   * @fn setPixformat
   * @brief Set color depth
   * @param pixelFormat  RGB555(RGB565 true color), GRAYSCALE(grayscale image)
   * @return Return the set color depth
   */
  uint8_t setPixformat(uint8_t pixelFormat);

  /**
   * @fn snapshot
   * @brief Take photo
   * @param pictureFilename The name of the file to save picture
   */ 
  void snapshot(const char *pictureFilename);
  
  /**
   * @fn connectNet
   * @brief Connect to WIFI
   * @param ssid     WIFI name
   * @param password WIFI password
   */ 
  void connectNet(const char* ssid,const char* password);

  /**
   * @fn initPlayer
   * @brief init music player
   */
  void initPlayer();
  
  /**
   * @fn initRecorder
   * @brief Init recorder
   */
  void initRecorder();

  /**
   * @fn playMusic
   * @brief Play music
   * @param filename The name of the file used to play
   */
  void playMusic(const char *Filename);

  /**
   * @fn record
   * @brief record sound
   * @param filename he name of the file to save record
   */
  void record(const char *Filename);

  /**
   * @fn playerControl
   * @brief Control the music player
   * @param cmd Command to control music player
   * @n    PLAY   Begin or continue play music
   * @n    PAUSE  Pause play music
   * @n    STOP   Stop play music
   */
  void playerControl(uint8_t cmd);

  /**
   * @fn recorderControl
   * @brief Control the recorder
   * @param cmd Command to control recorder
   * @n   BEGIN  Begin recording
   * @n   STOP   Stop recording
   */
  void recorderControl(uint8_t cmd);
  
  /**
   * @fn SDcard_Init
   * @brief Initialize SD card
   * @param mountpoint SD card name
   */
  bool SDcard_Init(const char* mountpoint="/sdcard");
};

typedef sWav_t *  handleWav;
void i2sMcklkInit(unsigned int sampleRate);
void i2sMasterInit(uint32_t sampleRate,i2s_bits_per_sample_t bitsPerSample);
void i2sSlaveInit(uint32_t sampleRate,i2s_bits_per_sample_t bitsPerSample);
void i2cMasterInit();
void i2cWriteNAU8822(int8_t addr,int16_t data);
void i2cSetupNAU8822Play();
void i2cSetupNAU8822Record();
unsigned int littleEndian32(unsigned int v);
unsigned short littleEndian16(short v); 

#endif
