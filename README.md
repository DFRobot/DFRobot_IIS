# FireBeetle Covers-Camera&Audio Media Board

ESP32 Audio&Video Board with IIS 

![SVG1](https://raw.githubusercontent.com/DFRobot/binaryfiles/master/DFR0498/DFR0498svg1.png)
---------------------------------------------------------

# DFRobot_IIS Library for Arduino
Provides an Arduino library for FireBeetle Covers-Camera&Audio Media Board 
## Table of Contents

* [Summary](#summary)
* [Methods](#methods)
* [Compatibility](#Compatibility)
* [History](#history)
* [Credits](#credits)
<snippet>
<content>

## Summary

The library is used to Play music file from SD card,Record sound and save in SD card,take photo and save in SD card

## Methods

```C++

/*
 * @brief Init SD card
 */
void SDCardInit(void);

/*
 * @brief Set operational mode 
 *
 * @param mode Work mode settings
 *     AUDIO:Audio mode 
 *     CAMERA:Camera mode
 */
void init(uint8_t mode);

/*
 * @brief Set speakers volume in when play music
 *
 * @param volume Set volume from 0 to 99
 */
void setSpeakersVolume(uint8_t volume);

/*
 * @brief Set headphones volume in when play music
 *
 * @param volume Set volume from 0 to 99
 */
void setHeadphonesVolume(uint8_t volume);

/*
 * @brief Set speakers in mute mode
 */
void muteSpeakers(void);

/*
 * @brief Set headphones in mute mode
 */
void muteHeadphones(void);

/*
 * @brief Init music player
 */
void initPlayer();

/*
 * @brief Play music
 *
 * @param *filename The name of the file used to play
 */
int playMusic(const char *filename);

/*
 * @brief Control the music player
 *
 * @param cmd Command to control music player
 *     PLAY   Begin or continue play music
 *     PAUSE  Pause play music
 *     STOP   Stop play music
 */
void playerControl(uint8_t cmd);

/*
 * @brief Init Recorder
 */
void initRecorder();

/*
 * @brief Record sound
 *
 * @param *outputFilename The name of the file to save record
 */ 
int recordSound(const char *outputFilename);

/*
 * @brief Control the recorder
 *
 * @param cmd Command to control recorder
 *     BEGIN  Begin recording
 *     STOP   Stop recording
 */
void recorderControl(uint8_t cmd);

/*
 * @brief Set photo siez
 *
 * @param photoSize Size of photo
 *     Possible values: QQVGA(160x120) QQVGA2(128x160) QICF(176x144) HQVGA(240x160) QVGA(320x240)
 *
 */
void setFramesize(uint8_t photoSize);

/*
 * @brief Take photo
 *
 * @param *pictureFilename The name of the file to save picture
 */ 
void snapshot(const char *pictureFilename);

```

## Compatibility

MCU                | Work Well | Work Wrong | Untested  | Remarks
------------------ | :----------: | :----------: | :---------: | -----
FireBeetle-ESP32  |      √       |             |            | 
FireBeetle-ESP8266  |             |      √       |            | 
FireBeetle-BLE4.1 |             |       √      |            | 

## History

- date 2017-9-29
- version V1.0

## Credits

- author [Zhangjiawei  <jiawei.zhang@dfrobot.com>]
